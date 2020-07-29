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

void Platform::modelWork(int input[4], int output[4]) {
	int hidden[8];
	for (int j = 0; j < 8; j++) {
		hidden[j] = collisionModel_b0[j];
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 8; j++) {
			hidden[j] += collisionModel_W0[i][j] * input[i];
		}
	}
	for (int j = 0; j < 8; j++) {
		if (hidden[j] < 0) {
			hidden[j] = 0;
		}
	}
	for (int j = 0; j < 4; j++) {
		output[j] = collisionModel_b1[j];
	}
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 4; j++) {
			output[j] += collisionModel_W1[i][j] * hidden[i];
		}
	}
}
void Platform::Run() {
	bool moveFlag;
	int count[16][16] = {};
	int round = 0;
	record = 0;
	do {
		round++;
		double delta_time = DELTA_TIME;
		moveFlag = false;
		int collisionCount = 1;
		while (collisionCount > 0){
			collisionCount--;
			bool collisionFlag = false;
			for (int i = 0; i < Balls.size(); i++){
				if (std::abs(Balls[i].coordinate) > eps) {
					for (int j = i + 1; j < Balls.size(); j++) {
						if (std::abs(Balls[j].coordinate) > eps && count[i][j] <= round - 5) {
							std::complex<double> deltaC = Balls[i].coordinate - Balls[j].coordinate;
							std::complex<double> deltaV = Balls[i].velocity - Balls[j].velocity;
							if (std::abs(deltaV) < MIN_VELOCITY) {
								continue;
							}
							std::complex<double> tmp = -deltaC * std::conj(deltaV) / std::abs(deltaV);
							double dist = std::abs(tmp.imag());
							if (dist < 2 * STONE_R && tmp.real() > -eps) {
								double time_cost = (tmp.real() - std::sqrt(4*STONE_R*STONE_R - dist * dist)) / std::abs(deltaV);
								if (std::abs(time_cost) > 0.001) {
									time_cost -= eps;
									if (time_cost > 0 && delta_time > time_cost) {
										delta_time = time_cost;
									}
								}else{
									collisionFlag = true;
									count[i][j] = round;
									deltaC /= std::abs(deltaC);
									std::complex<double> F = std::conj(deltaC) * deltaV;
									double angleCos = std::abs(F.real()) / std::abs(deltaV);
									if (std::abs(F.real()) > 2) {
										record -= 0.5;
										F.imag(0);
										Balls[i].velocity -= F * deltaC;
										Balls[j].velocity += F * deltaC;
									}
									else {
										record -= 2;
										F.imag(F.imag() * std::pow(angleCos, 0.9));
										F *= 0.5;
										Balls[i].velocity -= F * deltaC;
										Balls[j].velocity += F * deltaC;
										/*
										int x = i, y = j;
										if (std::abs(Balls[x].velocity) < std::abs(Balls[y].velocity)) {
											std::swap(x, y);
										}
										int input[4], output[4];
										input[0] = Balls[x].velocity.real(); input[1] = Balls[x].velocity.imag();
										input[2] = Balls[y].velocity.real(); input[3] = Balls[y].velocity.imag();
										modelWork(input, output);

										Balls[x].velocity.real(output[0]); Balls[x].velocity.imag(output[1]);
										Balls[y].velocity.real(output[2]); Balls[y].velocity.imag(output[3]);
										*/
									}

								}
							}
						}
					}
				}
			}
			if (!collisionFlag) {
				collisionCount = 0;
			}
		}

		for (int i = 0; i < Balls.size(); i++){ 
			if (std::abs(Balls[i].coordinate) > eps) {
				//printf("%lf %lf\n", Balls[i].coordinate.real(), Balls[i].coordinate.imag());
				Ball &ball = Balls[i];
				std::complex<double> _velocity = Balls[i].velocity;
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
					if (Abs < delta_time * FRICTION[stage]) {
						ball.velocity = 0;
						ball.angle = 0;
					}
					else {
						ball.velocity -= ball.velocity / Abs * delta_time * FRICTION[stage];
						ball.velocity -= ball.velocity * Abs * AIR_DRAG[stage] * delta_time;
					}

					ball.velocity *= std::pow((1 - VELOCITY_LOSS_ANGLE[stage] * std::abs(ball.angle)), delta_time);
					std::complex<double> Rot = std::pow(std::complex<double>(1, ball.angle * VELOCITY_ANGLE[stage]), delta_time);
					ball.velocity *= Rot / std::abs(Rot);
					ball.angle *= std::pow((1 - ANGLE_LOSS[stage]), delta_time);
				}
				else {
					ball.velocity = 0;
					ball.angle = 0;
				}
				ball.angle += (1 - std::abs(ball.velocity)) * ANGLE_INCRESS_VELOCITY[stage] * delta_time;
				ball.coordinate += (Balls[i].velocity + _velocity) * 0.5 * delta_time;
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
bool Platform::InHouseLoose(std::complex<double> position) {
	if (std::abs(position - TEE) < HOUSE_R + STONE_R + 0.1) {
		return true;
	}
	return false;
}
bool Platform::OutLoose(std::complex<double> position)
{
	if (position.imag() < BACK_LINE + 0.3 || position.real() < 0.2 || position.real() > MAX_POINT[0] - 0.2) {
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
bool Platform::InDefendAreaLoose(std::complex<double> position) {
	if (InHouseLoose(position)) {
		return false;
	}
	if (position.imag() > TEE_Y - 0.05 && position.imag() < FRONT_LINE + 0.05) {
		return true;
	}
	return false;
}
double Platform::Evaluation(const Platform& const oldPlatform) {
	int N = Balls.size();
	//自由防守区规则判断
	if (N == 1 && !InDefendAreaLoose(Balls[0].coordinate)) {
		return -INF;
	}
	if (N <= 5) {
		for (int i = N - 2; i >= 0; i -= 2) {
			if (InDefendArea(oldPlatform.Balls[i].coordinate) && OutLoose(Balls[i].coordinate)) {
				return -INF;
			}
		}
	}

	//估价
	double value = 0;
	double minDist[2] = { HOUSE_R + STONE_R, HOUSE_R + STONE_R };

	if (N <= 4) {
		for (int i = N - 1, j = 1; i >= 0; i--, j = -j) {
			if (InDefendArea(Balls[i].coordinate)) {
				value += 4 * j;
			}
		}
	}
	//printf("%d\n", N);

	//我方壶尽可能分开
	double tmpSum = 0;
	int tmpCount = 0;
	for (int i = N - 1; i >= 0; i -= 2) {
		if (std::abs(Balls[i].coordinate) > eps) {
			for (int j = i - 2; j >= 0; j -= 2) {
				if (std::abs(Balls[j].coordinate) > eps) {
					std::complex<double> dist = Balls[i].coordinate - Balls[j].coordinate;
					tmpSum += max(min(std::abs(dist.real()), 1) * 1.5, min(std::abs(dist.imag()), 1.5));
					tmpCount++;
				}
			}
		}
	}
	if (tmpCount) {
		value += 2 * tmpSum / tmpCount;
	}

	double WeightForY = 0.6;
	//我方冰壶离中心越近越好
	for (int i = N - 1, j = 1; i >= 0; i--, j = -j) {
		if (std::abs(Balls[i].coordinate) > eps) {
			double dist = std::abs(Balls[i].coordinate - TEE);
			//printf("%d %lf\n", i, dist);
			if (dist < HOUSE_R + STONE_R) {
				value += j * (std::pow(2 - dist, 2) / 2 + WeightForY * Balls[i].coordinate.imag());
				int side = j < 0;
				if (minDist[side] > dist) {
					minDist[side] = dist;
				}
			}
			else {
				value += j / (100 + dist + std::abs(Balls[i].coordinate.real() - TEE_X) * 2);
			}
		}
	}


	//场上对方壶数量越少越好
	double tmpWeight = 1;
	if (N & 1) {
		tmpWeight = 10000;
	}
	//printf("%d\n", N);
	for (int i = N - 2; i >= 0; i -= 2) {
		//printf("%d (%lf,%lf) %d %d\n", i, Balls[i].coordinate.real(), Balls[i].coordinate.imag(), InHouse(std::abs(Balls[i].coordinate)), InDefendArea(std::abs(Balls[i].coordinate)));

		double dist = std::abs(Balls[i].coordinate - TEE);
		if (dist < HOUSE_R + STONE_R + 0.05) {
			value -= tmpWeight * (0.5 + 1. / (1 + dist));
		}
	}
	//场上自己壶数量越多越好
	tmpWeight = 1;
	//printf("%d\n", N);
	for (int i = N - 1; i >= 0; i -= 2) {
		//printf("%d (%lf,%lf) %d %d\n", i, Balls[i].coordinate.real(), Balls[i].coordinate.imag(), InHouse(std::abs(Balls[i].coordinate)), InDefendArea(std::abs(Balls[i].coordinate)));

		double dist = std::abs(Balls[i].coordinate - TEE);
		if (dist < HOUSE_R + STONE_R - 0.05) {
			value += tmpWeight * (0.5 + 1. / (1 + dist));
		}
	}


	//根据离中心最近的壶做判断
	minDist[0] += 0.1;
	int winSide = minDist[1] < minDist[0];
	int flag = winSide == 0 ? 1 : -1;
	tmpWeight = 1;
	if (N == 16) {
		value /= 1e5;
		tmpWeight = 10000;
	}
	if (minDist[winSide] < HOUSE_R + STONE_R) {
		for (int i = N - 1 - winSide; i >= 0; i -= 2) {
			double dist = std::abs(Balls[i].coordinate - TEE);
			if (dist < minDist[winSide ^ 1]) {
				//判断保护球
				for (int j = 0; j < N; j++) {
					std::complex<double> dist = Balls[j].coordinate - Balls[i].coordinate;
					if (i != j && Balls[j].coordinate.imag() > 0 && std::abs(Balls[j].coordinate.real()) < STONE_R) {
						if (Balls[j].coordinate.imag() < 0.6) {
							tmpWeight += 0.5;
						}
						else tmpWeight += 0.1;
					}
				}
				value += flag * (1 / (dist + 2) + tmpWeight);
			}
		}
	}
	return value + record;
}