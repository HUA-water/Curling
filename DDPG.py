"""
Deep Deterministic Policy Gradient (DDPG)
-----------------------------------------
An algorithm concurrently learns a Q-function and a policy.
It uses off-policy data and the Bellman equation to learn the Q-function,
and uses the Q-function to learn the policy.
Reference
---------
Deterministic Policy Gradient Algorithms, Silver et al. 2014
Continuous Control With Deep Reinforcement Learning, Lillicrap et al. 2016
MorvanZhou's tutorial page: https://morvanzhou.github.io/tutorials/
Environment
-----------
Openai Gym Pendulum-v0, continual action space
Prerequisites
-------------
tensorflow >=2.0.0a0
tensorflow-probability 0.6.0
tensorlayer >=2.0.0
To run
------
python tutorial_DDPG.py --train/test
"""

import argparse
import os
import time

import env
import gym
import matplotlib.pyplot as plt
import numpy as np
import tensorflow as tf

import tensorlayer as tl

parser = argparse.ArgumentParser(description='Train or test neural net motor controller.')
parser.add_argument('--train', dest='train', action='store_true', default=True)
parser.add_argument('--test', dest='test', action='store_false')
args = parser.parse_args()

#####################  hyper parameters  ####################

ENV_NAME = 'Pendulum-v0'	# environment name
RANDOMSEED = int(time.time())			  # random seed

SHOW = True
LEARN_STEP = 1000
LR_A = 0.0001				# learning rate for actor
LR_C = 0.0003				# learning rate for critic
GAMMA = 0.6				 # reward discount
TAU = 0.3				  # soft replacement
MEMORY_CAPACITY = 10000	 # size of replay buffer
BATCH_SIZE = 32			 # update batchsize

MAX_EPISODES = 2000		  # total number of episodes for training
MAX_EP_STEPS = 5		  # total number of steps for each episode
TEST_PER_EPISODES = 10	  # test the model per episodes
VAR = [1, 0.5, 3]				 # control exploration
#VAR = 1
ACTION_RANDOM_RATE = 1

###############################  DDPG  ####################################

class DDPG(object):
	"""
	DDPG class
	"""
	def __init__(self, a_dim, s_dim, a_bound, a_mean, name):
		# memory用于储存跑的数据的数组：
		# 保存个数MEMORY_CAPACITY，s_dim * 2 + a_dim + 1：分别是两个state，一个action，和一个reward
		self.memory = np.zeros((MEMORY_CAPACITY, s_dim * 2 + a_dim + 1), dtype=np.float32)
		self.pointer = 0
		self.a_dim, self.s_dim, self.a_bound, self.a_mean = a_dim, s_dim, a_bound, a_mean
		self.name = name
		self.totalLossList = [[], []]

		W_init = tf.random_normal_initializer(mean=0, stddev=0.3)
		b_init = tf.constant_initializer(0.1)

		# 建立actor网络，输入s，输出a
		def get_actor(input_state_shape, name=''):
			"""
			Build actor network
			:param input_state_shape: state
			:param name: name
			:return: act
			"""
			inputs = tl.layers.Input(input_state_shape, name='A_input' + name)
			x = tl.layers.Dense(n_units=400, act=tf.nn.relu, W_init=W_init, b_init=b_init, name='A_l1' + name)(inputs)
			x = tl.layers.Dense(n_units=100, act=tf.nn.relu, W_init=W_init, b_init=b_init, name='A_l2' + name)(x)
			x = tl.layers.Dense(n_units=a_dim, act=tf.nn.tanh, W_init=W_init, b_init=b_init, name='A_a' + name)(x)
			x = tl.layers.Lambda(lambda x: np.array(a_bound) * x + np.array(a_mean))(x)			#注意这里，先用tanh把范围限定在[-1,1]之间，再进行映射
			return tl.models.Model(inputs=inputs, outputs=x, name='Actor' + name)

		#建立Critic网络，输入s，a。输出Q值
		def get_critic(input_state_shape, input_action_shape, name=''):
			"""
			Build critic network
			:param input_state_shape: state
			:param input_action_shape: act
			:param name: name
			:return: Q value Q(s,a)
			"""
			s = tl.layers.Input(input_state_shape, name='C_s_input')
			a = tl.layers.Input(input_action_shape, name='C_a_input')
			x = tl.layers.Concat(1)([s, a])
			x = tl.layers.Dense(n_units=400, act=tf.nn.relu, W_init=W_init, b_init=b_init, name='C_l1')(x)
			x = tl.layers.Dense(n_units=100, act=tf.nn.relu, W_init=W_init, b_init=b_init, name='C_l2')(x)
			x = tl.layers.Dense(n_units=1, W_init=W_init, b_init=b_init, name='C_out')(x)
			return tl.models.Model(inputs=[s, a], outputs=x, name='Critic' + name)

		self.actor = get_actor([None, s_dim], name = '_' + self.name)
		self.critic = get_critic([None, s_dim], [None, a_dim], name = '_' + self.name)
		self.actor.train()
		self.critic.train()

		#更新参数，只用于首次赋值，之后就没用了
		def copy_para(from_model, to_model):
			"""
			Copy parameters for soft updating
			:param from_model: latest model
			:param to_model: target model
			:return: None
			"""
			for i, j in zip(from_model.trainable_weights, to_model.trainable_weights):
				j.assign(i)

		#建立actor_target网络，并和actor参数一致，不能训练
		self.actor_target = get_actor([None, s_dim], name = '_' + self.name + '_target')
		copy_para(self.actor, self.actor_target)
		self.actor_target.eval()

		#建立critic_target网络，并和actor参数一致，不能训练
		self.critic_target = get_critic([None, s_dim], [None, a_dim], name = '_' + self.name + '_target')
		copy_para(self.critic, self.critic_target)
		self.critic_target.eval()

		self.R = tl.layers.Input([None, 1], tf.float32, 'r')

		#建立ema，滑动平均值
		self.ema = tf.train.ExponentialMovingAverage(decay=1 - TAU)  # soft replacement



	def ema_update(self):
		"""
		滑动平均更新
		"""
		# 其实和之前的硬更新类似，不过在更新赋值之前，用一个ema.average。
		paras = self.actor.trainable_weights + self.critic.trainable_weights	#获取要更新的参数包括actor和critic的
		self.ema.apply(paras)												   #主要是建立影子参数
		for i, j in zip(self.actor_target.trainable_weights + self.critic_target.trainable_weights, paras):
			i.assign(self.ema.average(j))									   # 用滑动平均赋值

	# 选择动作，把s带进入，输出a
	def choose_action(self, s):
		"""
		Choose action
		:param s: state
		:return: act
		"""
		return self.actor(np.array([s], dtype=np.float32))[0]
	def get_value(self, s, a):
		"""
		Choose action
		:param s: state
		:return: act
		"""
		bs = np.array([s], dtype=np.float32)
		ba = np.array([a], dtype=np.float32)
		return self.critic([bs, ba])[0]

	def learn(self):
		"""
		Update parameters
		:return: None
		"""
		self.actor_opt = tf.optimizers.Adam(LR_A, decay = 1e-8)
		self.critic_opt = tf.optimizers.Adam(LR_C, decay = 1e-8)
		loss_c = []
		loss_a = []
		for t in range(LEARN_STEP):
			indices = np.random.choice(np.min([self.pointer, MEMORY_CAPACITY]), size=min([self.pointer, BATCH_SIZE]))	#随机BATCH_SIZE个随机数
			#print(indices)
			bt = self.memory[indices, :]					#根据indices，选取数据bt，相当于随机
			bs = bt[:, :self.s_dim]						 #从bt获得数据s
			ba = bt[:, self.s_dim:self.s_dim + self.a_dim]  #从bt获得数据a
			br = bt[:, -self.s_dim - 1:-self.s_dim]		 #从bt获得数据r
			bs_ = bt[:, -self.s_dim:]					   #从bt获得数据s'

			# Critic：
			# Critic更新和DQN很像，不过target不是argmax了，是用critic_target计算出来的。
			# br + GAMMA * q_
			with tf.GradientTape() as tape:
				a_ = self.actor_target(bs_)
				q_ = self.critic_target([bs_, a_])
				y = (1-GAMMA) * br + GAMMA * q_
				q = self.critic([bs, ba])
				td_error = tf.losses.mean_squared_error(y, q)
				#for i in range(len(indices)):
				#	print(i, np.array(ba[i]), np.array(y[i]), np.array(q[i]), np.array(td_error[i]))
				#print(np.mean(td_error))
				loss_c.append(np.mean(td_error))
				c_grads = tape.gradient(td_error, self.critic.trainable_weights)
				self.critic_opt.apply_gradients(zip(c_grads, self.critic.trainable_weights))

		for t in range(LEARN_STEP):
			indices = np.random.choice(np.min([self.pointer, MEMORY_CAPACITY]), size=min([self.pointer, BATCH_SIZE]))	#随机BATCH_SIZE个随机数
			#print(indices)
			bt = self.memory[indices, :]					#根据indices，选取数据bt，相当于随机
			bs = bt[:, :self.s_dim]						 #从bt获得数据s
			ba = bt[:, self.s_dim:self.s_dim + self.a_dim]  #从bt获得数据a
			br = bt[:, -self.s_dim - 1:-self.s_dim]		 #从bt获得数据r
			bs_ = bt[:, -self.s_dim:]	
			# Actor：
			# Actor的目标就是获取最多Q值的。
			with tf.GradientTape() as tape:
				a = self.actor(bs)
				q = self.critic([bs, a])
				a_loss = -tf.reduce_mean(q)  # 【敲黑板】：注意这里用负号，是梯度上升！也就是离目标会越来越远的，就是越来越大。
				#for i in range(len(indices)):
				#	print(i, np.array(ba[i]), np.array(a[i]), np.array(q[i]), np.array(a_loss))
				#print()
				loss_a.append(np.mean(q))
				a_grads = tape.gradient(a_loss, self.actor.trainable_weights)
				self.actor_opt.apply_gradients(zip(a_grads, self.actor.trainable_weights))
		self.ema_update()
		self.totalLossList[0].append(np.mean(loss_c))
		self.totalLossList[1].append(np.mean(loss_a))
		return loss_c, loss_a


	# 保存s，a，r，s_
	def store_transition(self, s, a, r, s_):
		"""
		Store data in data buffer
		:param s: state
		:param a: act
		:param r: reward
		:param s_: next state
		:return: None
		"""
		# 整理s，s_,方便直接输入网络计算
		#print(self.name, s, a, r, s_)
		s = s.astype(np.float32)
		s_ = s_.astype(np.float32)

		#把s, a, [r], s_横向堆叠
		transition = np.hstack((s, a, [r], s_))

		#pointer是记录了曾经有多少数据进来。
		#index是记录当前最新进来的数据位置。
		#所以是一个循环，当MEMORY_CAPACITY满了以后，index就重新在最底开始了
		index = self.pointer % MEMORY_CAPACITY  # replace the old memory with new memory
		#把transition，也就是s, a, [r], s_存进去。
		self.memory[index, :] = transition
		self.pointer += 1

	def save_ckpt(self, path):
		"""
		save trained weights
		:return: None
		"""
		if not os.path.exists(path):
			os.makedirs(path)

		tl.files.save_weights_to_hdf5(path + '/ddpg_actor.hdf5', self.actor)
		tl.files.save_weights_to_hdf5(path + '/ddpg_actor_target.hdf5', self.actor_target)
		tl.files.save_weights_to_hdf5(path + '/ddpg_critic.hdf5', self.critic)
		tl.files.save_weights_to_hdf5(path + '/ddpg_critic_target.hdf5', self.critic_target)

	def load_ckpt(self, path):
		"""
		load trained weights
		:return: None
		"""
		tl.files.load_hdf5_to_weights_in_order(path + '/ddpg_actor.hdf5', self.actor)
		tl.files.load_hdf5_to_weights_in_order(path + '/ddpg_actor_target.hdf5', self.actor_target)
		tl.files.load_hdf5_to_weights_in_order(path + '/ddpg_critic.hdf5', self.critic)
		tl.files.load_hdf5_to_weights_in_order(path + '/ddpg_critic_target.hdf5', self.critic_target)


if __name__ == '__main__':
	if SHOW:
		plt.ion()
		plt.subplot(4,1,1).cla()
		plt.pause(0.1)
	
	#初始化环境
	env = env.Env(True)

	# reproducible，设置随机种子
	#env.seed(RANDOMSEED)
	np.random.seed(RANDOMSEED)
	#tf.random.set_seed(RANDOMSEED)

	#定义状态空间，动作空间，动作幅度范围
	s_dim = env.observation_space.shape[0]
	a_dim = env.action_space.shape[0]
	a_bound = (env.action_space.high - env.action_space.low)/2
	a_mean = (env.action_space.high + env.action_space.low)/2

	#用DDPG算法，先后手两个智能体
	ddpg = [DDPG(a_dim, s_dim, a_bound, a_mean, '0'), DDPG(a_dim, s_dim, a_bound, a_mean, '1')]
	
	#保存和读取模型
	for i in range(len(ddpg)):
		#ddpg[i].save_ckpt('DDPG_' + str(i))
		pass
	for i in range(len(ddpg)):
		ddpg[i].load_ckpt('DDPG_' + str(i))
		pass
		
	#训练部分：
	rewardList = []
	for i in range(MAX_EPISODES):
		for j in range(MAX_EP_STEPS):
			t0 = time.time()		#统计时间
			Test = j == 0			#测试模式
			#初始状态及局面估价，对先手而言的局面估价与对后手而言的局面估价相反
			record = [[env.reset(), env.GetReward(1), (0,0,0)]]
			for k in range(0,16):
				#获取动作
				a = np.array(ddpg[k&1].choose_action(record[-1][0]))
				if Test and (k&1)==1:
					a = [2.95, np.random.random()*4-2, 0]
				#引入噪声
				if np.random.randint(ACTION_RANDOM_RATE) == 0 and Test == False:
					a = np.clip(np.random.normal(a, VAR), env.action_space.low, env.action_space.high) 
				#执行动作，获得接下来的状态和局面估价
				s, r, done, info = env.step(a)
				record.append([s, r, a])
			
			if Test:
				rewardList.append([env.GetReward(0), env.GetReward(1)])
				print("======\n", np.array(rewardList), "\n======")
				#rewardList.append(totalReward)
				#print(np.array(totalReward))
			
			if not Test:
				#存入记忆
				for k in range(1,16):
					ddpg[(k&1)^1].store_transition(record[k-1][0], record[k][2], -(record[k+1][1] - record[k-1][1]) + env.GetAdditionReward(record[k][2]), record[k+1][0])
				
				ddpg[1].store_transition(record[15][0], record[16][2], record[16][1] + record[15][1] + env.GetAdditionReward(record[16][2]), record[16][0])
				
				
				#学习
				if SHOW:
					plt.ion()
					plt.subplot(4,1,1).cla()
					plt.subplot(4,1,2).cla()
					plt.subplot(4,1,3).cla()
					plt.subplot(4,1,4).cla()
				for id in [0, 1]:
					loss_c, loss_a = ddpg[id].learn()
					if SHOW:
						plt.subplot(4,1,1).plot(loss_c, label = "loss_c_" + str(id))
						plt.subplot(4,1,2).plot(loss_a, label = "loss_a_" + str(id))
						plt.subplot(4,1,3).plot(ddpg[id].totalLossList[0], label = "loss_c_" + str(id))
						plt.subplot(4,1,4).plot(ddpg[id].totalLossList[1], label = "loss_a_" + str(id))
						plt.subplot(4,1,1).legend()
						plt.subplot(4,1,2).legend()
						plt.subplot(4,1,3).legend()
						plt.subplot(4,1,4).legend()
						plt.pause(0.1)
					

			print('\n', i, j, ' Running time: ', time.time() - t0)
			for id in range(len(ddpg)):
				ddpg[id].save_ckpt('DDPG_' + str(id))
				pass