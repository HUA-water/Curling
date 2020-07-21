import tensorflow as tf
from tensorflow.keras import backend as K
from tensorflow.keras.callbacks import ModelCheckpoint
from tensorflow.keras import layers
from tensorflow.keras import optimizers
from tensorflow.keras.models import load_model
import numpy as np
import env
import matplotlib.pyplot as plt



x = tf.keras.Input(35)

m = layers.Dense(500, activation = 'relu', kernel_initializer = 'random_uniform')(x)
m = layers.Dense(500, activation = 'relu', kernel_initializer = 'random_uniform')(m)

y = layers.Dense(34, kernel_initializer = 'random_uniform')(m)

physical_model = tf.keras.Model(inputs = x, outputs = y)
physical_model.compile(optimizer = optimizers.Adam(lr = 1e-4), loss="mse", metrics = ["mse"])

x = tf.keras.Input(35)

m = layers.Dense(500, activation = 'relu', kernel_initializer = 'random_uniform')(x)
m = layers.Dense(500, activation = 'relu', kernel_initializer = 'random_uniform')(m)

y = layers.Dense(1, kernel_initializer = 'random_uniform')(m)

value_model = tf.keras.Model(inputs = x, outputs = y)
value_model.compile(optimizer = optimizers.Adam(lr = 1e-2), loss="mse", metrics = ["mse"])


env = env.Env(False)

physical_model = load_model('physical.h5')
value_model = load_model('value.h5')
plt.ion()
loss = []
value_real = []
value_pred = []
value_loss = []
physical_memory = [[],[],[]]
physical_memory[0] = np.load("memory0.npy").tolist()
physical_memory[1] = np.load("memory1.npy").tolist()
physical_memory[2] = np.load("memory2.npy").tolist()

while False:
	value_model.fit(physical_memory[0], physical_memory[2], epochs = 50)
	value_model.save('value.h5')
	plt.cla()
	plt.plot(physical_memory[2][:20])
	plt.plot(value_model.predict(physical_memory[0][:20]))
	plt.pause(0.1)

for i in range(20000):
	env.reset()
	s = [0]*32
	for t in range(16):
		a = [np.random.random()*0.5 + 2.7, np.random.random()*3.8-1.9, np.random.random()*20-10]
		if np.random.randint(4)==0 and t>4:
			a[0] = np.random.random()*7+3
		s_next,_,_,_ = env.step(a)
		s_next = s_next[1:]
		s_fix = s_next.copy().tolist()
		s_fix[t*2], s_fix[t*2+1] = 0, 0
		s_fix.append(s_next[t*2])
		s_fix.append(s_next[t*2+1])
		state = list(np.append(s, a))
		if t&1:
			for k in range(8):
				state[k*4], state[k*4+1], state[k*4+2], state[k*4+3] = state[k*4+2], state[k*4+3], state[k*4], state[k*4+1]
				s_fix[k*4], s_fix[k*4+1], s_fix[k*4+2], s_fix[k*4+3] = s_fix[k*4+2], s_fix[k*4+3], s_fix[k*4], s_fix[k*4+1]
		value = env.GetReward(t&1)
		'''pred = physical_model.predict([state])[0]
		predValue = value_model.predict([state])[0]
		value_real.append(value)
		value_pred.append(predValue)
		value_loss.append((value-predValue)**2)
		plt.subplot(4,1,1).cla()
		plt.subplot(4,1,1).plot(pred)
		plt.subplot(4,1,1).plot(s_fix)
		plt.subplot(4,1,1).plot(state)
		plt.pause(0.1)
		loss.append(np.sum((pred - s_fix)**2))'''
		physical_memory[0].append(state)
		physical_memory[1].append(s_fix)
		physical_memory[2].append(value)
		np.save("memory0.npy", physical_memory[0])
		np.save("memory1.npy", physical_memory[1])
		np.save("memory2.npy", physical_memory[2])
		print(len(physical_memory[0]))
		print(i,t)
		print(state)
		print(s_fix)
		print(value, env.GetReward(0, position = np.reshape(s_fix+[0,0],(18, 2))))
		'''
		plt.subplot(4,1,2).cla()
		plt.subplot(4,1,2).plot(loss)
		plt.subplot(4,1,3).cla()
		plt.subplot(4,1,3).plot(value_real)
		plt.subplot(4,1,3).plot(value_pred)
		plt.subplot(4,1,4).cla()
		plt.subplot(4,1,4).plot(value_loss)
		plt.pause(0.1)
		if len(physical_memory[0]) > 32:
			physical_model.fit(physical_memory[0], physical_memory[1], epochs = 50)
			value_model.fit(physical_memory[0], physical_memory[2], epochs = 50)
		'''
		s = s_next
	physical_model.save('physical.h5')
	#value_model.save('value2.h5')