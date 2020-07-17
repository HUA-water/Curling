from Throw import Throw

DEBUG = True


class Curling:
    def __init__(self, x, y, my_flag):
        self.x = x
        self.y = y
        self.my = my_flag
        self.marker = 'unknown'

        """
        Curling.marker
        unknown: 未知
        score: 大本营里的得分球
        non_score: 大本营里的非得分球
        camp: 位于大本营里的球
        defend: 自由防守区的防守球
        """

    def get_dist(self):
        return (self.x - 2.375) ** 2 + (self.y - 4.88) ** 2

    def definite_dist(self, x, y):
        return (self.x - x) ** 2 + (self.y - y) ** 2

    def __lt__(self, other):
        return self.get_dist() < other.get_dist()

    def __str__(self):
        string = 'Mine' if self.my else 'Enemy'
        return string + ' {.3f}, {.3f}'.format(self.x, self.y)

    def is_in_house(self):
        House_R = 1.870
        Stone_R = 0.145
        if self.get_dist() < (House_R + Stone_R) ** 2:
            return 1
        else:
            return 0

    def is_mine(self):
        return self.my

    def mark(self, state):
        self.marker = state


class Parser:         # 场景解析
    def __init__(self, state_list, order):
        self.positions = state_list[0]
        self.current_shotnum = int(state_list[1])
        self.player = order
        self.curlings = None

    def base_parse(self):
        curlings = []

        res = []
        i = 0
        while i < 30:  # 过滤掉空冰壶
            if float(self.positions[i])**2 + float(self.positions[i + 1])**2 > 0.1:
                res.append(i)
            i += 2

        for i in res:
            if self.player == str("Player1"):      # 判断冰壶归属
                my_flag = (i % 4 == 0)
            else:
                my_flag = (i % 4 == 2)

            curling_i = Curling(float(self.positions[i]), float(self.positions[i+1]), my_flag)
            curlings.append(curling_i)
            curlings = sorted(curlings)

        for curling in curlings:
            if curling.is_in_house():
                curling.mark('non_score')          # 大本营内先统一标注为未得分球
            else:
                if 3 > curling.x > 1.75:
                    curling.mark('defend')
                else:
                    curling.mark('non_defend')

        self.curlings = curlings

        return curlings

    def score(self):        # 分析得分球
        score = [0, 0]  # index0: my_score  index1: enemy_score
        curlings = self.base_parse()

        if curlings:
            nonzero_score_index = 0 if curlings[0].is_mine() else 1
            for curling in curlings:
                index = 0 if curling.is_mine() else 1
                if index == nonzero_score_index:
                    score[index] += 1
                    curling.mark('score')
                else:
                    break

        return score

    def curlings_statistics(self):         # 统计各个类型的冰壶数量
        my_curlings = {'score': 0, 'non_score': 0, 'defend': 0}
        enemy_curlings = {'score': 0, 'non_score': 0, 'defend': 0}
        for curling in self.curlings:
            if curling.is_mine():
                if curling.marker in my_curlings.keys():
                    my_curlings[curling.marker] += 1
                else:
                    my_curlings[curling.marker] = 1
            else:
                if curling.marker in enemy_curlings.keys():
                    enemy_curlings[curling.marker] += 1
                else:
                    enemy_curlings[curling.marker] = 1
        return my_curlings, enemy_curlings

    def find_curlings(self, my_flag, marker=None):       # 找到符合要求的curling(按距离排好)
        curlings = []
        if marker:
            if type(marker) != list:
                marker = [marker]

        for curling in self.curlings:
            if not marker:
                if curling.is_mine() == my_flag:
                    curlings.append(curling)
            else:
                if (curling.is_mine() == my_flag) and (curling.marker in marker):
                    curlings.append(curling)
        if DEBUG:
            print('查找' + '我的' if my_flag else '敌方的' + '冰壶', marker)
            print(curlings)
        return curlings

    def have_defend(self, curling):          # 检测某冰壶是否有防守冰壶
        for other_curling in self.curlings:
            if other_curling.is_in_house():
                continue
            offset = abs(float(curling.x) - float(other_curling.x))
            if offset < 0.2:
                # print('Current score curling have defend.')
                return True, other_curling
        return False, curling

    def have_my_curling(self, x, y, r):    # 对一定范围内进行搜索是否存在自己的冰壶
        for curling in self.curlings:
            if curling.definite_dist(x, y) < r**2:
                if curling.is_mine():
                    return True
        return False

    def offense_position(self):
        # 先检测中心是否有我们的壶
        center_flag = self.have_my_curling(2.375, 4.88, 0.4)
        if not center_flag:
            return 2.375, 4.88         # 对中心处进行占位

        my_center_curling = self.find_curlings(True)
        if not my_center_curling:
            return 2.375, 4.88

        my_center_curling = my_center_curling[0]  # 取最靠近中心的冰壶为基础
        if not self.have_my_curling(my_center_curling.x + 0.6, my_center_curling.y, 0.4):
            return my_center_curling.x + 0.6, my_center_curling.y
        elif not self.have_my_curling(my_center_curling.x - 0.6, my_center_curling.y, 0.4):
            return my_center_curling.x - 0.6, my_center_curling.y
        elif not self.have_my_curling(my_center_curling.x + 1.2, my_center_curling.y, 0.4):
            return my_center_curling.x + 1.2, my_center_curling.y
        else:
            return my_center_curling.x - 1.2, my_center_curling.y

    def defense_analyze(self, target_x):     # 分析某大本营投掷目标被遮挡的情况
        defense_direction = [0, 0, 0]    # 0:middle, 1:left, 2:right
        for other_curling in self.curlings:
            if other_curling.is_in_house():
                continue
            offset = float(target_x) - float(other_curling.x)     # 目标横坐标-冰壶横坐标
            if abs(offset) < 0.2:
                # print('Current score curling have defend.')
                defense_direction[0] += 1
            elif 0.6 < offset < 0.2:
                defense_direction[1] += 1
            elif -0.6 < offset < -0.2:
                defense_direction[2] += 1
            else:
                pass
        return defense_direction


class Decision:
    def __init__(self, parser):
        self.parser = parser
        self.decision = None
        self.parser.base_parse()
        self.current_score = self.parser.score()

    def decide_w(self, target_x):     # 利用目标横坐标决定投掷时左旋还是右旋
        defense_direction = self.parser.defense_analyze(target_x)
        if not defense_direction[0]:
            return 0
        elif not defense_direction[1]:
            return 10
        elif not defense_direction[2]:
            return -10
        return 0

    def offense(self):  # 投掷冰壶的默认模式，在大本营中占位
        x, y = self.parser.offense_position()
        v, h_x, w = Throw.rotate_throw(x, y, self.decide_w(x))
        return v, h_x, w

    def policy(self):
        policy_prob = [0, 0, 0, 0]        # 'hit_center', 'hit_defense', 'defense', 'offense'
        policy = ['hit_center', 'hit_defense', 'defense', 'offense']
        # 决定hit_center的概率(撞击中心其他冰壶)
        center_curlings = [0, 0]     # [0]我方 [1]敌方
        for curling in self.parser.curlings:
            if curling.get_dist() < 0.5**2:            # 中心区域
                if curling.is_mine():
                    center_curlings[0] += 1
                else:
                    center_curlings[1] += 1
        # prob决定为敌方中心所占比例
        if sum(center_curlings) != 0:
            policy_prob[0] = center_curlings[1] / float(sum(center_curlings))
        # 如果是我方得分，禁止撞击
        if self.current_score != 0:
            policy_prob[0] = 0

        my_curlings_num, enemy_curlings_num = self.parser.curlings_statistics()
        # 决定hit_defense的概率
        base_defense_prob = (5 - my_curlings_num['defend'] - enemy_curlings_num['defend']) / 5
        policy_prob[1] = 1 - base_defense_prob
        policy_prob[1] = (2*policy_prob[1] + enemy_curlings_num['score']/2)/3

        # 决定防守defense的概率(占位防守)
        policy_prob[2] = base_defense_prob
        if enemy_curlings_num['score'] != 0:
            policy_prob[2] = 0
        elif my_curlings_num['score'] != 0:
            policy_prob[2] = 1
        else:
            policy_prob[2] = policy_prob[2] * my_curlings_num['score']

        # 决定进攻offense的概率(占大本营)
        enemy_num = enemy_curlings_num['score'] + enemy_curlings_num['non_score']
        my_num = my_curlings_num['score'] + my_curlings_num['non_score']

        center_prob = (center_curlings[1] + center_curlings[0]) / 3
        policy_prob[3] = 1 - center_prob

        print('Policy: ', policy_prob)

        return policy[policy_prob.index(max(policy_prob))]  # 返回策略概率最大的方法

    def decide(self):

        # my_curlings_num, enemy_curlings_num = self.parser.curlings_statistics()
        shotnum = self.parser.current_shotnum
        protect_flag = (shotnum < 5)   # 自由防守区生效flag
        offensive_flag = (shotnum % 2 == 0)   # 先手flag
        final_flag = shotnum >= 14     # 最后一球flag

        if protect_flag and offensive_flag:   # 前5球，2球占位
            print('Decide: 自由防守决策.')
            if shotnum / 2 == 0:
                v, h_x, w = Throw.free_defense_center_1()
            elif shotnum / 2 == 1:
                v, h_x, w = Throw.free_defense_center_2()
            else:
                v, h_x, w = Throw.center()

        elif protect_flag and not offensive_flag:  # 前5球，2球占位
            print('Decide: 自由防守决策.')
            if shotnum / 2 == 0:
                v, h_x, w = Throw.free_defense_center_1()
            elif shotnum / 2 == 1:
                v, h_x, w = Throw.free_defense_center_2()
            else:
                v, h_x, w = Throw.center()

        elif final_flag:

            if offensive_flag:      # 如果先手，判断得分球是否有防御
                my_score_curlings = self.parser.find_curlings(True, 'score')
                print('Decide: 最后一球决策，先手决策. 目前得分球{:.0f}个.'.format(len(my_score_curlings)))
                if my_score_curlings:
                    flag, curling = self.parser.have_defend(my_score_curlings[0])
                    if flag:
                        v, h_x, w = Throw.center()
                    else:          # 缺少防御时添加防御
                        print('Decide: 最后一球决策，添加防御.')
                        v, h_x, w = Throw.defense(my_score_curlings[0])
                else:
                    v, h_x, w = self.offense()
            else:
                v, h_x, w = Throw.center()

        else:
            policy = self.policy()
            if policy == 'hit_center':     # 击打中心区域冰壶
                enemy_curlings = self.parser.find_curlings(False, ['score', 'non_score'])
                if enemy_curlings:
                    print('Decide: 击打中心区域敌方冰壶，找到目标.')
                    target_curling = enemy_curlings[0]
                    if self.decide_w(target_curling.x) == 0:
                        v, h_x, w = Throw.hit(target_curling)
                    elif self.decide_w(target_curling.x) > 0:
                        v, h_x, w = Throw.left_hit(target_curling)
                    elif self.decide_w(target_curling.x) < 0:
                        v, h_x, w = Throw.right_hit(target_curling)
                    else:
                        v, h_x, w = Throw.hit(target_curling)

                else:
                    print('Decide: 击打中心区域敌方冰壶，未找到目标.')
                    v, h_x, w = self.offense()

            elif policy == 'hit_defense':     # 击打防守冰壶
                curlings = self.parser.find_curlings(False, 'defend')
                curlings.extend(self.parser.find_curlings(True, 'defend'))

                if curlings:        # 找到需要打掉的防守壶
                    v, h_x, w = self.offense()     # 默认投掷中心防止错误
                    print('Decide: 击打防守冰壶.')
                    enemy_curlings = self.parser.find_curlings(False, ['score', 'non_score'])
                    for enemy_curling in enemy_curlings:
                        have_defend, defend_curling = self.parser.have_defend(enemy_curling)
                        if have_defend:
                            print('Decide: 击打防守冰壶，找到目标.')
                            v, h_x, w = Throw.hit(defend_curling)
                            break
                else:
                    print('Decide: 击打防守冰壶，未找到目标.')
                    v, h_x, w = self.offense()

            elif policy == 'defense':   # 防守模式
                curlings = self.parser.find_curlings(True, 'score')
                if curlings:
                    print('Decide: 防守，找到需要被防守的冰壶并防守.')
                    v, h_x, w = Throw.defense(curlings[0])
                else:
                    print('Decide: 防守，未找到目标.')
                    v, h_x, w = self.offense()
            elif policy == 'offense':   # 进攻（占位）模式
                print('Decide: 进攻.')
                x, y = self.parser.offense_position()
                v, h_x, w = Throw.rotate_throw(x, y, self.decide_w(x))
            else:
                print('Decide: 默认.')
                v, h_x, w = self.offense()

        # 整理并返回最终结果
        bestshot = [v, h_x, w]
        tmp = str(bestshot)[1:-1].replace(',', '')
        res = "BESTSHOT " + tmp
        return res
