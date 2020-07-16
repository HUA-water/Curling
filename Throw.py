from random import randint


class Throw:          # 投掷速度生成函数集合

    @staticmethod
    def non_rotate_throw(x, y):
        return 3.491674 - 0.114903 * y, x - 2.375, 0

    @staticmethod
    def rotate_throw(x, y, w=10):
        # 建议角速度为10或-10
        v = 3.56836 - 0.11606 * y        # 利用y决定投掷速度

        # 观测得w为正时右偏，运动全程x偏移量为正，以下公式目前仅w=10或-10时相近
        x_offset = (0.74370 - 0.72993 * v) * w / 10
        h_x = x - 2.375 - x_offset

        # 一般情况
        # ...

        return v, h_x, w

    @staticmethod
    def center():
        if randint(0, 100) % 2 == 0:
            return 3.05, 1.65, -10
        else:
            return 3.05, -1.65, 10

    @staticmethod
    def direct_center():
        return Throw.non_rotate_throw(2.375, 4.88)

    @staticmethod
    def defense(curling):
        return 2.45, float(curling.x - 2.375), 0

    @staticmethod
    def free_defense_center_1():
        return 2.5, -0.8, 0

    @staticmethod
    def free_defense_center_2():
        return 2.5, 0, 0

    @staticmethod
    def free_defense_center_3():
        return 2.5, 0.8, 0

    @staticmethod
    def hit(curling):
        return float(3.613 - 0.12234 * curling.y + 3), float(curling.x - 2.375), 0

    @staticmethod
    def left_hit(curling):
        v, h_x, w = Throw.rotate_throw(curling.x, curling.y, 10)
        return v*2, h_x, w

    @staticmethod
    def right_hit(curling):
        v, h_x, w = Throw.rotate_throw(curling.x, curling.y, -10)
        return v*2, h_x, w
