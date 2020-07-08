# -*- coding: utf-8 -*-
import socket 
import time
from Strategy import *

# python 与客户端连接
host = '127.0.0.1'
port = 7788
obj = socket.socket()
obj.connect((host, port))

# 初始化
shotnum = str("0")
order = str("Player1")   # 先后手
state = []
MANUAL = False


# 策略
def strategy(state_list, order):
    parser = Parser(state_list, order)
    decision = Decision(parser=parser)
    return decision.decide()


def handle_strategy():
    v, h_x, w = map(float, input().split())
    bestshot = list_to_str([v, h_x, w])
    return bestshot


while True:
    ret = str(obj.recv(1024), encoding="utf-8")
    print("recv:" + ret)
    messageList = ret.split(" ")
    if messageList[0] == "NAME":
        order = messageList[1]
        if order == str("Player1"):
            print("玩家1，首局先手")
        else:
            print("玩家2，首局后手")
    if messageList[0] == "ISREADY":
        time.sleep(0.5)
        obj.send(bytes("READYOK", encoding="utf-8"))
        print("send READYOK")
        obj.send(bytes("NAME SampleAI", encoding="utf-8"))
        print("send NAME SampleAI")
    if messageList[0] == "POSITION":
        if state:
            state = []
        state.append(ret.split(" ")[1:31])
    if messageList[0] == "SETSTATE":
        shotnum = ret.split(" ")[1]
        state.append(shotnum)
        print(state)
    if messageList[0] == "GO":
        if MANUAL:
            shot = handle_strategy()
        else:
            shot = strategy(state, order)
        obj.send(bytes(shot, encoding="utf-8"))
    if messageList[0] == "MOTIONINFO":
        x_coordinate = float(messageList[1])
        y_coordinate = float(messageList[2])
        x_velocity = float(messageList[3])
        y_velocity = float(messageList[4])
        angular_velocity = float(messageList[5])
        obj.send(bytes("SWEEP 4.0", encoding="utf-8"))
