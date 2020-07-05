#include"simulator.h"
#include "strategy.h"

Platform::Platform(const GAMESTATE* const gs) {
	Balls.clear();
	for (int i = 0; i < gs->ShotNum; i++) {
		Balls.push_back(Ball(std::complex<double>(gs->body[i][0], gs->body[i][1])));
	}
	record = 0;
}

void Platform::AddBall(double vy, double dx, double angle) {
	Balls.push_back(Ball(std::complex<double>(START_POINT[0] + dx, START_POINT[1]), std::complex<double>(0, -vy), angle));
}

void Platform::Run() {
	bool moveFlag;
	do {
		moveFlag = false;
		int collisionCount = 4;
		while (collisionCount > 0){
			collisionCount--;
			bool collisionFlag = true;
			for (int i = 0; i < Balls.size(); i++){
				if (std::abs(Balls[i].coordinate) > eps) {
					for (int j = i + 1; j < Balls.size(); j++) {
						if (std::abs(Balls[j].coordinate) > eps) {
							std::complex<double> deltaC = Balls[i].coordinate - Balls[j].coordinate;
							std::complex<double> deltaV = Balls[i].velocity - Balls[j].velocity;
							if (std::abs(deltaC + deltaV * DELTA_TIEM) <= 2 * STONE_R) {
								collisionFlag = false;

								deltaC /= std::abs(deltaC);
								std::complex<double> F = std::conj(deltaC) * deltaV;
								if (F.real() > 2) {
									F.imag(0);
								}
								else {
									F.real(F.real() * COLLISION[0]);
									F.imag(F.imag() * COLLISION[1]);
								}

								Balls[i].velocity -= F * deltaC;
								Balls[j].velocity += F * deltaC;
							}
						}
					}
				}
			}
			if (collisionFlag) {
				collisionCount = 0;
			}
		}

		for (int i = 0; i < Balls.size(); i++){ 
			if (std::abs(Balls[i].coordinate) > eps) {
				//printf("%lf %lf\n", Balls[i].coordinate.real(), Balls[i].coordinate.imag());
				Ball &ball = Balls[i];
				ball.coordinate += Balls[i].velocity * DELTA_TIEM;
				if (ball.coordinate.real() < STONE_R || ball.coordinate.real() > MAX_POINT[0] - STONE_R || ball.coordinate.imag() < BACK_LINE - STONE_R || ball.coordinate.imag() > MAX_POINT[1] - STONE_R) {
					ball.coordinate = ball.velocity = ball.angle = 0;
				}
				double Abs = std::abs(ball.velocity);
				int stage;
				if (Abs > 1.5) stage = 0;
				else if (Abs > 1) stage = 1;
				else stage = 2;

				if (Abs > MIN_VELOCITY || ball.angle > MIN_ANGLE) {
					moveFlag = true;
					ball.velocity -= ball.velocity / Abs * DELTA_TIEM * FRICTION[stage];
					ball.velocity -= ball.velocity * Abs * AIR_DRAG[stage] * DELTA_TIEM;

					ball.velocity *= std::pow((1 - VELOCITY_LOSS_ANGLE[stage] * std::abs(ball.angle)), DELTA_TIEM);
					std::complex<double> Rot = std::pow(std::complex<double>(1, ball.angle * VELOCITY_ANGLE[stage]), DELTA_TIEM);
					ball.velocity *= Rot / std::abs(Rot);
					ball.angle *= std::pow((1 - ANGLE_LOSS[stage]), DELTA_TIEM);
				}
				else {
					ball.velocity = 0;
					ball.angle = 0;
				}
			}
		}
	} while (moveFlag);
}
bool Platform::InHouse(std::complex<double> position) {
	if (std::abs(position - TEE) < HOUSE_R + STONE_R) {
		return true;
	}
	return false;
}
bool Platform::InDefendArea(std::complex<double> position) {
	if (InHouse(position)) {
		return false;
	}
	if (position.imag() > TEE_Y && position.imag() < FRONT_LINE) {
		return true;
	}
	return false;
}
double Platform::Evaluation(const GAMESTATE* const gs) {
	if (gs->ShotNum == 0 && !InDefendArea(Balls[0].coordinate)) {
		return -INF;
	}
	//自由防守区规则判断
	if (gs->ShotNum <= 4) {
		for (int i = gs->ShotNum - 1; i >= 0; i -= 2) {
			if (InDefendArea(std::complex<double>(gs->body[i][0], gs->body[i][1])) && std::abs(Balls[i].coordinate) == 0) {
				return -INF;
			}
		}
	}

	//估价
	double value = 0;
	double minDist[2] = { HOUSE_R + STONE_R, HOUSE_R + STONE_R };

	//我方冰壶离中心越近越好
	for (int i = gs->ShotNum, j = 1; i >= 0; i--, j = -j) {
		if (std::abs(Balls[i].coordinate) > eps) {
			double dist = std::abs(Balls[i].coordinate - TEE);
			//printf("%d %lf\n", i, dist);
			if (dist < HOUSE_R + STONE_R) {
				value += j / (std::pow(1 + dist, 2)) - 0.1;
				int side = j == -1;
				if (minDist[side] > dist) {
					minDist[side] = dist;
				}
			}
			else {
				value += j / (1 + dist + std::abs(Balls[i].coordinate.real() - TEE_X) * 2);
			}
		}
	}
	minDist[0] += 0.05;
	int winSide = minDist[1] < minDist[0], flag = winSide == 0? 1 : -1;

	//根据离中心最近的壶做判断
	if (minDist[winSide] < HOUSE_R + STONE_R) {
		for (int i = gs->ShotNum - winSide; i >= 0; i -= 2) {
			double dist = std::abs(Balls[i].coordinate - TEE);
			if (dist < minDist[winSide ^ 1]) {
				value += flag * (3 / dist + 0.5);
			}
		}
	}

	//场上对方壶数量越少越好
	for (int i = gs->ShotNum - 1; i >= 0; i -= 2) {
		if (std::abs(Balls[i].coordinate) > eps) {
			value -= 2;
		}
	}
	return value + record;
}