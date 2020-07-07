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
        defend: 自由防守区的防守球
        """

    def __lt__(self, other):
        return get_dist(self.x, self.y) < get_dist(other.x, other.y)

    def is_in_house(self):
        return is_in_house(self.x, self.y)

    def is_mine(self):
        return self.my

    def use(self):
        pass


class Parser:         # 场景解析
    def __init__(self, state_list, order):
        self.positions = state_list[0]
        self.current_shotnum = state_list[1]
        self.player = order

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

        return curlings

    def score(self):
        score = [0, 0]  # index0: my_score  index1: enemy_score
        curlings = self.base_parse()

        if curlings:
            nonzero_score_index = 0 if curlings[0].is_mine() else 1
            for curling in curlings:
                index = 0 if curling.is_mine() else 1
                if index == nonzero_score_index:
                    score[index] += 1
                else:
                    break

        return score


class Decision:
    def __init__(self, parser):
        self.parser = parser

    def decide(self):
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

        bestshot = [v, h_x, 0]
        bestshot = list_to_str(bestshot)
        return bestshot

