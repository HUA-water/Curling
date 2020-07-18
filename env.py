# -*- coding: utf-8 -*-
import socket 
import threading
import time
import pyautogui
import gym
import numpy as np

#快速模式、准备、开始对局、返回主菜单
CLICK_POSITION = [[700, 575], [1010, 610], [1010, 710], [30, 150]]
TEE = [2.375, 4.88]
SLOW_MODE = 0

host='127.0.0.1'
port=7788

    
def list_to_str(list):
    res = str(list)[1:-1].replace(',','')
    return res
class Player:
	def __init__(self, name):
		self.Socker = socket.socket()
		self.Socker.connect((host,port))
		self.name = name
		self.state = [[0,0]]*16
		self.x_coordinate = -1
		self.y_coordinate = -1
		self.x_velocity = -1
		self.y_velocity = -1
		self.angular_velocity = -1
		self.go = False
		self.score = -1
		self.getNewState = True
		
		self.thread = threading.Thread(target=self.receive, args=())
		self.thread.start()
		
	def receive(self):
		while True:
			ret=str(self.Socker.recv(1024), encoding="utf-8")
			#print(self.name + " recv:" + ret)
			messageList = ret.split(" ")
			if messageList[0] == "NAME":
				self.order = messageList[1]
			
			if messageList[0]=="ISREADY":
				self.Socker.send(bytes("READYOK", encoding="utf-8"))
				self.Socker.send(bytes("NAME " + self.name, encoding="utf-8"))
				
			if messageList[0]=="POSITION":
				if self.state:
					self.state = []
				tmp = ret.split(" ")
				for i in range(16):
					self.state.append([float(tmp[1 + i*2]), float(tmp[2 + i*2])])
					self.getNewState = True
				
			if messageList[0]=="MOTIONINFO":
				self.x_coordinate = float(messageList[1])
				self.y_coordinate = float(messageList[2])
				self.x_velocity = float(messageList[3])
				self.y_velocity = float(messageList[4])
				self.angular_velocity = float(messageList[5])
				self.Socker.send(bytes("SWEEP " + str(self.sweep),encoding="utf-8"))
				self.sweep = 0
			if messageList[0]=="GO":
				self.go = True
			if messageList[0]=="SCORE":
				self.score = int(messageList[1])
				break
			if messageList[0]=="GAMEOVER":
				break
				
	def sendStrategy(self, shot, sweep = 0):
		self.sweep = sweep
		while (self.go == False):
			time.sleep(0.5)
		self.Socker.send(bytes("BESTSHOT " + list_to_str(shot), encoding="utf-8"))
		self.go = False
		self.getNewState = False
	
class Env:
	observation_space = gym.spaces.box.Box(np.array([0] + [0]*32), np.array([16] + [(4.75 if (i&1)==0 else 11) for i in range(32)]))
	action_space = gym.spaces.box.Box(np.array([2.6, -2.23, -10]), np.array([10, 2.23, 10]))
	def __init__(self):
		self.player = []
	def Start(self):
		self.shotNum = 0
		self.reward = 0
		#单击快速模式
		if SLOW_MODE:
			pyautogui.moveTo(CLICK_POSITION[0][0], CLICK_POSITION[0][1])
			time.sleep(1)
		time.sleep(1)
		pyautogui.click(CLICK_POSITION[0][0], CLICK_POSITION[0][1])
		time.sleep(0.3)
		pyautogui.click(CLICK_POSITION[0][0], CLICK_POSITION[0][1])
		time.sleep(0.3)
		pyautogui.click(CLICK_POSITION[0][0], CLICK_POSITION[0][1])
		
		time.sleep(1.5)
		
		#连接两个AI
		self.player = [Player("First")]
		time.sleep(0.3)
		if SLOW_MODE:
			time.sleep(1)
		self.player.append(Player("Second"))
		time.sleep(0.5)
		
		
		#单击准备、开始对局
		if SLOW_MODE:
			pyautogui.moveTo(CLICK_POSITION[1][0], CLICK_POSITION[1][1])
			time.sleep(1)
		pyautogui.click(CLICK_POSITION[1][0], CLICK_POSITION[1][1])
		time.sleep(0.3)
		pyautogui.click(CLICK_POSITION[1][0], CLICK_POSITION[1][1])
		time.sleep(0.3)
		pyautogui.click(CLICK_POSITION[1][0], CLICK_POSITION[1][1])
		
		time.sleep(0.3)
		
		if SLOW_MODE:
			pyautogui.moveTo(CLICK_POSITION[2][0], CLICK_POSITION[2][1])
			time.sleep(1)
		pyautogui.click(CLICK_POSITION[2][0], CLICK_POSITION[2][1])
		time.sleep(0.3)
		pyautogui.click(CLICK_POSITION[2][0], CLICK_POSITION[2][1])
	
	def GiveStrategy(self, shot, sweep = 0):
		side = self.shotNum&1
		self.player[side].sendStrategy(shot, sweep)
		self.shotNum += 1
	
	def End(self):
		if self.player:
			self.player[0].thread.join(0.1)
			self.player[1].thread.join(0.1)
			
		if SLOW_MODE:
			pyautogui.moveTo(CLICK_POSITION[3][0], CLICK_POSITION[3][1])
			time.sleep(1)
		pyautogui.click(CLICK_POSITION[3][0], CLICK_POSITION[3][1])
		time.sleep(0.7)
		pyautogui.click(CLICK_POSITION[3][0], CLICK_POSITION[3][1])
		time.sleep(0.5)
		pyautogui.click(CLICK_POSITION[3][0], CLICK_POSITION[3][1])
	
	def GetPosition(self):
		side = self.shotNum&1^1
		count = 0
		while self.player[side].getNewState == False:
			count+=1
			if count > 300:
				self.reset()
				count = 0
			time.sleep(0.1)
		position = self.player[side].state
		return position
	
	def GetState(self):
		state = np.array(self.GetPosition()).ravel()
		state = np.insert(state, 0, self.shotNum)
		#print(state)
		state /= np.array(self.observation_space.high)
		return state
	
	def reset(self):
		self.End()
		self.Start()
		return self.GetState()
	
	#局面估价函数
	def GetReward(self, side):
		position = self.GetPosition()
		#return np.sum(np.linalg.norm(position[side::2]) > 0.1) - np.sum(np.linalg.norm(position[side^1::2]) > 0.1)
		dist = np.linalg.norm(position - np.array(TEE), axis = 1)
		distPerSide = [dist[0::2], dist[1::2]]
		res = 0
		if np.min(distPerSide[side]) < np.min(distPerSide[side^1]):
			res = np.sum((distPerSide[side] < np.min(distPerSide[side^1])) * (distPerSide[side] < 1.975))
		else:
			res = -np.sum((distPerSide[side^1] < np.min(distPerSide[side])) * (distPerSide[side^1] < 1.975))
		
		#如果已经结束
		if self.shotNum == 16:
			res *= 30
		
		res += (np.sum(distPerSide[side^1]) - np.sum(distPerSide[side]))
		return res
	#增加对动作的约束方便收敛
	def GetAdditionReward(self, action):
		return -np.abs(action[0]-3) - np.abs(action[2])/10
		
		
	def step(self, action):
		side = self.shotNum&1
		self.GiveStrategy(action)
		#print(action, side, self.GetReward(side))
		return self.GetState(), self.GetReward(side), self.shotNum==16, ""
		
if __name__ == "__main__":
	tmp = Env()
	tmp.Start()
	for i in range(16):
		tmp.GiveStrategy([3,1,1], 1)
	tmp.End()