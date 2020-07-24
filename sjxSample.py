# -*- coding: utf-8 -*-
import socket
import time
import time
# python 与客户端连接
host = '127.0.0.1'
port = 7788
obj = socket.socket()
obj.connect((host, port))
# 初始化
shotnum = str("0")
order = str("Player1")  # 先后手
state = []
flag = 0
offset = 1.07
v_coo = -0.5464
v_y = 8.7978

# 与大本营中心距离
def get_dist(x, y):
    return (x - 2.375) ** 2 + (y - 4.88) ** 2


# 大本营内是否有球
def is_in_house(x, y):
    House_R = 1.830
    Stone_R = 0.145
    if get_dist(x, y) < (House_R + Stone_R) ** 2:
        return 1
    else:
        return 0


def list_to_str(list):
    tmp = str(list)[1:-1].replace(',', '')
    res = "BESTSHOT " + tmp
    return res


def my_sort_y(num_list, state_list):
    sorted_y = []
    print('begin to sort y')
    for i in num_list:
        sorted_y.append([float(state_list[0][i + 1]), i])
        sorted_y = sorted(sorted_y, reverse=True)
    print('sorted y:')
    print(sorted_y)
    return sorted_y


def my_sort_res(num_list, state_list):
    sorted_res = []
    print('begin to sort res')
    for i in num_list:
        sorted_res.append([get_dist(float(state_list[0][i]), float(state_list[0][i + 1])), i])
        sorted_res = sorted(sorted_res)
    print('sorted res:')
    print(sorted_res)
    return sorted_res


# 策略
def strategy(state_list, order):
    first_num = [0,4,8,12,16,20,24,28]#xianshoudexuhao
    second_num = [2,6,10,14,18,22,26]
    all_num = [0,2,4,6,8,10,12,14,16,18,20,22,24,26,28]
    global flag
    res = []
    i = 0
    while i < 30:
        if is_in_house(float(state_list[0][i]), float(state_list[0][i + 1])):
            res.append(i)
        i += 2
    # 先手
    if int(state_list[1]) % 2 == 0:
        if not res:
            bestshot = str("BESTSHOT 2.765 0.0 0.0")
            flag = 1
            return bestshot
        if my_sort_y(second_num, state_list)[0][0] == 0:
            bestshot = str("BESTSHOT 3.05 1.65 -10")
            flag = 0
            return bestshot
        if int(state_list[1]) == 0:
            v = 2.756
            h_x = float(0.0)
            angle = float(0.0)
            flag = 1
        elif int(state_list[1]) == 2:   #block in front of opponent
            v = float(3.613 - 0.12234 * float(my_sort_y(second_num, state_list)[0][0]))
            h_x = float(float(state_list[0][my_sort_y(second_num, state_list)[0][1]]) - 2.375)
            angle = float(0.0)
            flag = 0
        else:
            if int(state_list[1]) == 14:
                if my_sort_res(all_num, state_list)[0][1]%4 == 0:
                    target=[float(state_list[0][my_sort_res(all_num, state_list)[0][1]]),float(state_list[0][my_sort_res(all_num, state_list)[0][1]+1])+2]
                    v=float(3.613-0.12234*target[1])
                    h_x=float(target[0]-2.375)
                    angle = float(0.0)
                    flag = 0
                else:
                    target = [float(state_list[0][my_sort_res(second_num, state_list)[0][1]]), float(state_list[0][my_sort_res(second_num, state_list)[0][1] + 1])]
                    if (target[0] >= offset) and (target[0] <= 2.375):
                        v = float(v_coo * target[1] + v_y)
                        h_x = float(target[0] - offset - 2.375)
                        angle = float(10.0)
                        flag = 0
                    elif (target[0] <= 4.75-offset) and (target[0] >= 2.375):
                        v = float(v_coo * target[1] + v_y)
                        h_x = float(target[0] + offset - 2.375)
                        angle = float(-10.0)
                        flag = 0
                    else:
                        v = float(3.613 - 0.12234 * target[1] + 4)
                        h_x = float(target[0] - 2.375)
                        angle = float(0.0)
                        flag = 0
            else:
                target = [float(state_list[0][my_sort_res(second_num, state_list)[0][1]]), float(state_list[0][my_sort_res(second_num, state_list)[0][1] + 1])]
                if (target[0] >= offset) and (target[0] <= 2.375):
                    v = float(v_coo * target[1] + v_y)
                    h_x = float(target[0] - offset - 2.375)
                    angle = float(10.0)
                    flag = 0
                elif (target[0] <= 4.75-offset) and (target[0] >= 2.375):
                    v = float(v_coo * target[1] + v_y)
                    h_x = float(target[0] + offset - 2.375)
                    angle = float(-10.0)
                    flag = 0
                else:
                    v = float(3.613 - 0.12234 * target[1] + 4)
                    h_x = float(target[0] - 2.375)
                    angle = float(0.0)
                    flag = 0
        if (len(res) >= 2) and (my_sort_res(all_num, state_list)[0][1] % 4 == 0) and (my_sort_res(all_num, state_list)[1][1] % 4 == 0):
            target = [float(state_list[0][my_sort_res(all_num, state_list)[0][1]]), float(state_list[0][my_sort_res(all_num, state_list)[0][1] + 1]) + 2]
            v = float(3.613 - 0.12234 * target[1])
            h_x = float(target[0] - 2.375)
            angle = float(0.0)
            flag = 0
    #后手
    else:
        if not res:
            bestshot = str("BESTSHOT 2.765 0.0 0.0")
            flag = 1
            return bestshot
        if my_sort_y(first_num, state_list)[0][0] == 0:
            bestshot = str("BESTSHOT 3.05 1.65 -10")
            flag = 0
            return bestshot
        if int(state_list[1]) == 1:
            v = float(3.613 - 0.12234 * my_sort_y(first_num, state_list)[0][0])
            h_x = float(float(state_list[0][my_sort_y(first_num, state_list)[0][1]]) - 2.375)
            angle = float(0.0)
            flag = 0
        elif int(state_list[1]) == 3:
            v = float(3.613 - 0.12234 * my_sort_y(first_num, state_list)[0][0])
            h_x = float(float(state_list[0][my_sort_y(first_num, state_list)[0][1]]) - 2.375)
            angle = float(0.0)
            flag = 0
        else:
            if int(state_list[1]) == 13:
                if my_sort_res(all_num, state_list)[0][1] % 4 == 2:
                    target = [float(state_list[0][my_sort_res(all_num, state_list)[0][1]]), float(state_list[0][my_sort_res(all_num, state_list)[0][1] + 1]) + 2]
                    if (target[0] >= offset) and (target[0] <= 2.375):
                        v = float(v_coo * target[1] + v_y)
                        h_x = float(target[0] - offset - 2.375)
                        angle = float(10.0)
                        flag = 0
                    elif (target[0] <= 4.75-offset) and (target[0] >= 2.375):
                        v = float(v_coo * target[1] + v_y)
                        h_x = float(target[0] + offset - 2.375)
                        angle = float(-10.0)
                        flag = 0
                    else:
                        v = float(3.613 - 0.12234 * target[1] + 4)
                        h_x = float(target[0] - 2.375)
                        angle = float(0.0)
                        flag = 0
                else:
                    target = [float(state_list[0][my_sort_res(first_num, state_list)[0][1]]), float(state_list[0][my_sort_res(first_num, state_list)[0][1] + 1])]
                    if (target[0] >= offset) and (target[0] <= 2.375):
                        v = float(v_coo * target[1] + v_y)
                        h_x = float(target[0] - offset - 2.375)
                        angle = float(10.0)
                        flag = 0
                    elif (target[0] <= 4.75-offset) and (target[0] >= 2.375):
                        v = float(v_coo * target[1] + v_y)
                        h_x = float(target[0] + offset - 2.375)
                        angle = float(-10.0)
                        flag = 0
                    else:
                        v = float(3.613 - 0.12234 * target[1] + 4)
                        h_x = float(target[0] - 2.375)
                        angle = float(0.0)
                        flag = 0
            else:
                target = [float(state_list[0][my_sort_res(first_num, state_list)[0][1]]), float(state_list[0][my_sort_res(first_num, state_list)[0][1] + 1])]
                if (target[0] >= offset) and (target[0] <= 2.375):
                    v = float(v_coo * target[1] + v_y)
                    h_x = float(target[0] - offset - 2.375)
                    angle = float(10.0)
                    flag = 0
                elif (target[0] <= 4.75-offset) and (target[0] >= 2.375):
                    v = float(v_coo * target[1] + v_y)
                    h_x = float(target[0] + offset - 2.375)
                    angle = float(-10.0)
                    flag = 0
                else:
                    v = float(3.613 - 0.12234 * target[1] + 4)
                    h_x = float(target[0] - 2.375)
                    angle = float(0.0)
                    flag = 0
        if (len(res) >= 2) and (my_sort_res(all_num, state_list)[0][1] % 4 == 2) and (my_sort_res(all_num, state_list)[1][1] % 4 == 2):
                target = [float(state_list[0][my_sort_res(all_num, state_list)[0][1]]), float(state_list[0][my_sort_res(all_num, state_list)[0][1] + 1]) + 2]
                v = float(3.613 - 0.12234 * target[1])
                h_x = float(target[0] - 2.375)
                angle = float(0.0)
                flag = 0
    bestshot = [v,h_x,angle]
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
        obj.send(bytes("NAME sjxSample", encoding="utf-8"))
        print("send NAME sjxSample")
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
        print('flag = %d' % flag)
        if flag == 0:
            obj.send(bytes("SWEEP 9.0", encoding="utf-8"))
        else:
            obj.send(bytes("SWEEP 0.0", encoding="utf-8"))







