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


# 策略
def strategy(state_list, order):
    parser = Parser(state_list, order)
    decision = Decision(parser=parser)
    return decision.decide()


def handle_strategy():
    v, h_x, w = map(float, input().split())
    bestshot = list_to_str([v, h_x, w])
    return bestshot


def sample_strategy(state_list, order):
    res = []
    sorted_res = []
    i = 0
    while i < 30:
        if is_in_house(float(state_list[0][i]), float(state_list[0][i+1])):
            res.append(i)
        i += 2
    # 大本营内没有球，向大本营中心打球
    if not res:
        bestshot = str("BESTSHOT 3.05 1.65 -10")
        return bestshot
    # 大本营有球
    else:
        for i in res:
            sorted_res.append([get_dist(float(state_list[0][i]), float(state_list[0][i+1])), i])
            sorted_res = sorted(sorted_res)
        # 先手
        if order == str("Player1"):
            # 离大本营中心最近的球是自己的，防守
            if sorted_res[0][1] % 4 == 0:
                # print("防守球")
                target = [float(state_list[0][sorted_res[0][1]]), float(state_list[0][sorted_res[0][1]+1])+2]
                v = float(3.613-0.12234*target[1])
                h_x = float(target[0]-2.375)
            # 大本营离中心最近的球是自对方的，撞飞
            else:
                # print("撞飞")
                target = [float(state_list[0][sorted_res[0][1]]), float(state_list[0][sorted_res[0][1]+1])]
                v = float(3.613-0.12234*target[1]+1)
                h_x = float(target[0]-2.375)
        # 后手
        else:
            if sorted_res[0][1] % 4 == 2:
                # print("防守球")
                target = [float(state_list[0][sorted_res[0][1]]), float(state_list[0][sorted_res[0][1]+1])+2]
                v = float(3.613-0.12234*target[1])
                h_x = float(target[0]-2.375)
            # 大本营离中心最近的球是自对方的，撞飞
            else:
                # print("撞飞")
                target = [float(state_list[0][sorted_res[0][1]]), float(state_list[0][sorted_res[0][1]+1])]
                v = float(3.613-0.12234*target[1]+1)
                h_x = float(target[0]-2.375)
        
        bestshot = [v, h_x, 0]
        bestshot = list_to_str(bestshot)
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
        shot = strategy(state, order)
        obj.send(bytes(shot, encoding="utf-8"))
    if messageList[0] == "MOTIONINFO":
        x_coordinate = float(messageList[1])
        y_coordinate = float(messageList[2])
        x_velocity = float(messageList[3])
        y_velocity = float(messageList[4])
        angular_velocity = float(messageList[5])
        obj.send(bytes("SWEEP 4.0", encoding="utf-8"))
