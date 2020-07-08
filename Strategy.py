# 与大本营中心距离
def get_dist(x, y):
    return (x-2.375)**2+(y-4.88)**2


# 大本营内是否有球
def is_in_house(x, y):
    House_R = 1.830
    Stone_R = 0.145
    if get_dist(x, y) < (House_R+Stone_R)**2:
        return 1
    else:
        return 0


def list_to_str(list_):
    tmp = str(list_)[1:-1].replace(',', '')
    res = "BESTSHOT "+tmp
    return res


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

    def __lt__(self, other):
        return get_dist(self.x, self.y) < get_dist(other.x, other.y)

    def is_in_house(self):
        return is_in_house(self.x, self.y)

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
                curling.mark('defend')

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

    def find_curling(self, my_flag, marker):
        for curling in self.curlings:
            if curling.is_mine() == my_flag and marker == curling.marker:
                return curling

        return None

    def have_defend(self, curling):
        for other_curling in self.curlings:
            if other_curling.is_in_house():
                continue
            offset = abs(float(curling.x) - float(other_curling.x))
            if offset < 0.2:
                print('Current score curling have defend.')
                return True
        return False


class Throw:
    @staticmethod
    def center():
        return 3.05, 1.65, -10

    @staticmethod
    def defense(curling):
        return 2.45, float(curling.x - 2.375), 0

    @staticmethod
    def defense2(curling):
        return 2.45, float(curling.x - 2.375 + 0.5), 0

    @staticmethod
    def free_defense_center_1():
        return 2.5, -0.8, 0

    @staticmethod
    def free_defense_center_2():
        return 2.5, 0, 0

    @staticmethod
    def hit(curling):
        return float(3.613 - 0.12234 * curling.y + 1), float(curling.x - 2.375), 0


class Decision:
    def __init__(self, parser):
        self.parser = parser

    def raw_decide(self):
        curlings = self.parser.base_parse()
        if not curlings:
            v = 3.05
            h_x = 1.65
            w = -10
        else:
            if curlings[0].is_mine():  # 中心球属于我方，防守球
                v = float(3.613 - 0.12234 * curlings[0].y)
                h_x = float(curlings[0].x - 2.375)
                w = 0
            else:       # 撞击球
                v = float(3.613 - 0.12234 * curlings[0].y + 1)
                h_x = float(curlings[0].x - 2.375)
                w = 0

        bestshot = [v, h_x, w]
        bestshot = list_to_str(bestshot)
        return bestshot

    def decide(self):
        self.parser.base_parse()
        current_score = self.parser.score()
        my_curlings_num, enemy_curlings_num = self.parser.curlings_statistics()
        shotnum = self.parser.current_shotnum
        protect_flag = (shotnum < 5)   # 自由防守区生效flag
        offensive_flag = (shotnum % 2 == 0)   # 先手flag
        final_flag = shotnum >= 14     # 最后一球flag

        if protect_flag and offensive_flag:   # 前5球，2球占位
            if shotnum / 2 == 0:
                v, h_x, w = Throw.free_defense_center_1()
            elif shotnum / 2 == 1:
                v, h_x, w = Throw.free_defense_center_2()
            else:
                v, h_x, w = Throw.center()

        elif protect_flag and not offensive_flag:  # 前5球，2球占位
            if shotnum / 2 == 0:
                v, h_x, w = Throw.free_defense_center_1()
            elif shotnum / 2 == 1:
                v, h_x, w = Throw.free_defense_center_2()
            else:
                v, h_x, w = Throw.center()

        elif final_flag:
            if offensive_flag:
                v, h_x, w = Throw.center()
            else:
                v, h_x, w = Throw.center()

        elif enemy_curlings_num['defend'] > my_curlings_num['defend'] and my_curlings_num['non_score'] > 0:
            target = self.parser.find_curling(False, 'defend')
            v, h_x, w = Throw.hit(target)
        elif enemy_curlings_num['score'] > 1:
            target = self.parser.find_curling(False, 'score')
            v, h_x, w = Throw.hit(target)
        elif enemy_curlings_num['non_score'] > 1:
            target = self.parser.find_curling(False, 'non_score')
            v, h_x, w = Throw.hit(target)
        elif current_score[0] != 0:
            target = self.parser.find_curling(True, 'score')
            if self.parser.have_defend(target):
                v, h_x, w = Throw.center()
            else:
                v, h_x, w = Throw.defense(target)
        else:
            v, h_x, w = Throw.center()

        bestshot = [v, h_x, w]
        bestshot = list_to_str(bestshot)
        return bestshot
