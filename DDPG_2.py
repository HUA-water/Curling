"""
Note: This is a updated version from my previous code,
for the target network, I use moving average to soft replace target parameters instead using assign function.
By doing this, it has 20% speed up on my machine (CPU).

Deep Deterministic Policy Gradient (DDPG), Reinforcement Learning.
DDPG is Actor Critic based algorithm.
Pendulum example.

View more on my tutorial page: https://morvanzhou.github.io/tutorials/

Using:
tensorflow 1.0
gym 0.8.0
"""

import tensorflow.compat.v1 as tf
import numpy as np
import gym
import time
import env
import os
tf.disable_v2_behavior()


#####################  hyper parameters  ####################

MAX_EPISODES = 200
MAX_EP_STEPS = 3
VAR = [5, 1, 3]				 # control exploration
ACTION_RANDOM_RATE = 2
LR_A = 0.001	# learning rate for actor
LR_C = 0.002	# learning rate for critic
GAMMA = 0.9	 # reward discount
TAU = 0.005	  # soft replacement
MEMORY_CAPACITY = 10000
BATCH_SIZE = 16

RENDER = False
ENV_NAME = 'Pendulum-v0'
RANDOMSEED = int(time.time())			  # random seed


###############################  DDPG  ####################################


class DDPG(object):
	def __init__(self, a_dim, s_dim, a_bound, a_mean, name):
		self.memory = np.zeros((MEMORY_CAPACITY, s_dim * 2 + a_dim + 1), dtype=np.float32)
		self.pointer = 0
		self.sess = tf.Session()

		self.a_dim, self.s_dim, self.a_bound, self.a_mean = a_dim, s_dim, a_bound, a_mean,
		self.name = name
		self.S = tf.placeholder(tf.float32, [None, s_dim], 's')
		self.S_ = tf.placeholder(tf.float32, [None, s_dim], 's_')
		self.R = tf.placeholder(tf.float32, [None, 1], 'r')

		self.a = self._build_a(self.S,)
		q = self._build_c(self.S, self.a, )
		a_params = tf.get_collection(tf.GraphKeys.TRAINABLE_VARIABLES, scope='Actor' + self.name)
		c_params = tf.get_collection(tf.GraphKeys.TRAINABLE_VARIABLES, scope='Critic' + self.name)
		ema = tf.train.ExponentialMovingAverage(decay=1 - TAU)		  # soft replacement
		
		self.saver = tf.train.Saver()

		def ema_getter(getter, name, *args, **kwargs):
			return ema.average(getter(name, *args, **kwargs))

		target_update = [ema.apply(a_params), ema.apply(c_params)]	  # soft update operation
		a_ = self._build_a(self.S_, reuse=True, custom_getter=ema_getter)   # replaced target parameters
		q_ = self._build_c(self.S_, a_, reuse=True, custom_getter=ema_getter)

		a_loss = - tf.reduce_mean(q)  # maximize the q
		self.atrain = tf.train.AdamOptimizer(LR_A).minimize(a_loss, var_list=a_params)

		with tf.control_dependencies(target_update):	# soft replacement happened at here
			q_target = self.R + GAMMA * q_
			td_error = tf.losses.mean_squared_error(labels=q_target, predictions=q)
			self.ctrain = tf.train.AdamOptimizer(LR_C).minimize(td_error, var_list=c_params)

		self.sess.run(tf.global_variables_initializer())

	def choose_action(self, s):
		return self.sess.run(self.a, {self.S: s[np.newaxis, :]})[0] + self.a_mean

	def learn(self):
		indices = np.random.choice(np.min([self.pointer, MEMORY_CAPACITY]), size=BATCH_SIZE)
		bt = self.memory[indices, :]
		bs = bt[:, :self.s_dim]
		ba = bt[:, self.s_dim: self.s_dim + self.a_dim]
		br = bt[:, -self.s_dim - 1: -self.s_dim]
		bs_ = bt[:, -self.s_dim:]

		self.sess.run(self.atrain, {self.S: bs})
		self.sess.run(self.ctrain, {self.S: bs, self.a: ba, self.R: br, self.S_: bs_})

	def store_transition(self, s, a, r, s_):
		transition = np.hstack((s, a, [r], s_))
		index = self.pointer % MEMORY_CAPACITY  # replace the old memory with new memory
		self.memory[index, :] = transition
		self.pointer += 1

	def _build_a(self, s, reuse=None, custom_getter=None):
		trainable = True if reuse is None else False
		with tf.variable_scope('Actor' + self.name, reuse=reuse, custom_getter=custom_getter):
			net = tf.layers.dense(s, 80, activation=tf.nn.relu, name='l1' + self.name, trainable=trainable)
			hidden = tf.layers.dense(net, 80, activation=tf.nn.relu, name='hidden_a_1' + self.name, trainable=trainable)
			a = tf.layers.dense(hidden, self.a_dim, activation=tf.nn.tanh, name='a' + self.name, trainable=trainable)
			return tf.multiply(a, self.a_bound, name='scaled_a')

	def _build_c(self, s, a, reuse=None, custom_getter=None):
		trainable = True if reuse is None else False
		with tf.variable_scope('Critic' + self.name, reuse=reuse, custom_getter=custom_getter):
			n_l1 = 80
			w1_s = tf.get_variable('w1_s' + self.name, [self.s_dim, n_l1], trainable=trainable)
			w1_a = tf.get_variable('w1_a' + self.name, [self.a_dim, n_l1], trainable=trainable)
			b1 = tf.get_variable('b1' + self.name, [1, n_l1], trainable=trainable)
			net = tf.nn.relu(tf.matmul(s, w1_s) + tf.matmul(a, w1_a) + b1)
			hidden = tf.layers.dense(net, 80, activation=tf.nn.relu, name='hidden_c_1' + self.name, trainable=trainable)
			return tf.layers.dense(hidden, 1, trainable=trainable)  # Q(s,a)
	def save_ckpt(self, path):
		"""
		save trained weights
		:return: None
		"""
		if not os.path.exists(path):
			os.makedirs(path)
		"""
		tl.files.save_weights_to_hdf5(path + '/ddpg_actor.hdf5', self.actor)
		tl.files.save_weights_to_hdf5(path + '/ddpg_actor_target.hdf5', self.actor_target)
		tl.files.save_weights_to_hdf5(path + '/ddpg_critic.hdf5', self.critic)
		tl.files.save_weights_to_hdf5(path + '/ddpg_critic_target.hdf5', self.critic_target)
		"""
		
		save_path = self.saver.save(self.sess, path + "/save_net.ckpt")
		print("Save to path: ", save_path)

	def load_ckpt(self, path):
		"""
		load trained weights
		:return: None
		tl.files.load_hdf5_to_weights_in_order(path + '/ddpg_actor.hdf5', self.actor)
		tl.files.load_hdf5_to_weights_in_order(path + '/ddpg_actor_target.hdf5', self.actor_target)
		tl.files.load_hdf5_to_weights_in_order(path + '/ddpg_critic.hdf5', self.critic)
		tl.files.load_hdf5_to_weights_in_order(path + '/ddpg_critic_target.hdf5', self.critic_target)
		"""
		self.saver.restore(self.sess, path + "/save_net.ckpt")


if __name__ == '__main__':
	
	#初始化环境
	env = env.Env()

	# reproducible，设置随机种子，为了能够重现
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

	for i in range(len(ddpg)):
		#ddpg[i].save_ckpt('DDPG2_' + str(i))
		pass
	for i in range(len(ddpg)):
		ddpg[i].load_ckpt('DDPG2_' + str(i))
		pass
		
	#训练部分：
	rewardList = []
	for i in range(MAX_EPISODES):
		for j in range(MAX_EP_STEPS):
			t0 = time.time()		#统计时间
			Test = j == 0
			s_last = env.reset()
			a = np.array(ddpg[0].choose_action(s_last))
			r_last = 0
			#print(a)
			if np.random.randint(ACTION_RANDOM_RATE) == 0 and Test == False:
				a = np.clip(np.random.normal(a, VAR), env.action_space.low, env.action_space.high) 
			s, r, _, _ = env.step(a)
			a_last = a
			for k in range(1,16):
				a = np.array(ddpg[k&1].choose_action(s))
				if k&1:
					a = [3, np.random.random()*4-2, 0]	
					pass
				if np.random.randint(ACTION_RANDOM_RATE) == 0 and Test == False:
					a = np.clip(np.random.normal(a, VAR), env.action_space.low, env.action_space.high) 				
				s_next, r_next, done, info = env.step(a)
				ddpg[(k&1)^1].store_transition(s_last, a_last, r_last-r_next, s_next)
				#print(a_last, r_last-r_next, s_last)
				s_last = s
				s = s_next
				r_last = r
				r = r_next
				a_last = a
			ddpg[1].store_transition(s_last, a_last, r + r_last, s_next)
			if Test:
				rewardList.append(env.GetReward(0))
				print("======\n", rewardList, "\n======")

			for id in [0,1]:
				if ddpg[id].pointer > BATCH_SIZE:
					for t in range(100):
						ddpg[id].learn()

			print('\n', i, j, ' Running time: ', time.time() - t0)
			for id in range(len(ddpg)):
				ddpg[id].save_ckpt('DDPG2_' + str(id))