#include "strategy.h"
#include "simulator.h"
#include <ctime>
using namespace std;


// make your decision here
const double disturbV = 0.003;
const double disturbDx = 0.015;
const double disturbAngle = 0.015;
const double spaceColl = 0.33;
const double Weight = 0.7;
extern int totalScore;
void getBestShot(const GAMESTATE* const gs, SHOTINFO* vec_ret)
{
	bool defense = false;
	printf("Score:%d\n", totalScore);
	if (totalScore + ((gs->LastEnd - gs->CurEnd) & 1) * ((gs->ShotNum & 1) ? 1 : -1) > 0) {
		defense = true;
		printf("Defense on\n");
	}
	if (defense) {
		if (gs->ShotNum == 0) {
			vec_ret->speed = 3.1f;
			vec_ret->h_x = 0.0f;
			vec_ret->angle = 0.0f;
			return;
		}
		else if (gs->ShotNum < 4) {
			for (int i = gs->ShotNum - 1; i >= 0; i -= 2)
				if (gs->body[i][1] > 1) {
					double x = gs->body[i][1];
					vec_ret->speed = x * x*(-0.03560838) + 0.70997966*x + 1.26361588;
					vec_ret->h_x = gs->body[i][0] - 2.3506;
					vec_ret->angle = 0.0f;
					return;
				}
			vec_ret->speed = 3.1f;
			vec_ret->h_x = 0.0f;
			vec_ret->angle = 0.0f;
			return;
		}
	}
	int startTime = clock();
	double maxValue = -INF;

	Platform oldPlatform(gs);

	std::vector <double> DX;
	for (double dx = -1.7; dx <= 1.7; dx += 0.05) {
		DX.push_back(dx);
	}
	int normalNumber = DX.size();
	for (int i = gs->ShotNum - 1; i >= 0; i-=2) {
		if (gs->body[i][0] > 1) {
			//printf("%lf\n", gs->body[i][0] - 2.3506);
			DX.push_back(gs->body[i][0] - 2.3506);
		}
	}
	int RecA = 0;
	
	double tmpDx = 0, tmpVy = 0, tmpAngle = 0;
	for (int i = 0; i < DX.size(); i++) {
		double dx = DX[i];
		for (double angle = -10; angle <= 10; angle += 10) {
			double oldValue = -INF - 1;
			for (double vy = 2.6; vy <= 6.4; vy += vy < 4.5 ? 0.05 : 0.5) {
				if (angle != 0 && i >= normalNumber) {
					break;
				}
				Platform platform(oldPlatform);
				platform.AddBall(vy, dx, angle);
				platform.Run();
				double tmp = platform.Evaluation(oldPlatform, defense);
				if (tmp == oldValue) {
					break;
				}
				oldValue = tmp;
				for (double deltaColl = 0; deltaColl <= 1 && tmp > maxValue; deltaColl += spaceColl) {
					Platform platform(oldPlatform);
					platform.setCollsionWeight(deltaColl);
					platform.AddBall(vy, dx, angle);
					platform.Run();
					double value = platform.Evaluation(oldPlatform, defense);
					if (value < tmp) {
						tmp = tmp * Weight + value * (1 - Weight);
					}
				}

				if (tmp > maxValue) {
					maxValue = tmp;

					tmpVy = vy;
					tmpDx = dx;
					tmpAngle = angle;
					vec_ret->speed = vy;
					vec_ret->h_x = dx;
					vec_ret->angle = angle;
				}
				//如果价值高于一定值就输出，用于调试，设置INF表示不输出
				if (tmp > INF) {
					/*for (int j = 0; j < platform.Balls.size(); j++) {
						printf("%d: (%lf, %lf)\n", j, platform.Balls[j].coordinate.real(), platform.Balls[j].coordinate.imag());
					}*/
					printf("%lf %lf %lf : %lf\n\n", vy, dx, angle, tmp);
				}
			}
		}
	}
	
	double rangeDx = 0.03;
	double rangeVy = 0.03;
	double rangeAngle = 0.5;
	for (double dx = tmpDx - rangeDx; dx <= tmpDx + rangeDx; dx += 0.01) {
		for (double vy = tmpVy - rangeVy; vy <= tmpVy + rangeVy; vy += 0.01) {
			for (double angle = tmpAngle - rangeAngle; angle <= tmpAngle + rangeAngle; angle += 0.1) {
				if (std::abs(dx) <= 2 && std::abs(angle) <= 10 && vy < 10) {
					Platform platform(gs);
					platform.AddBall(vy, dx, angle);
					platform.Run();
					double tmp = platform.Evaluation(oldPlatform, defense);
					for (double deltaColl = 0; deltaColl <= 1 && tmp > maxValue; deltaColl += spaceColl/2) {
						for (int deltaAngle = -1; deltaAngle <= 1; deltaAngle += 2) {
							Platform platform(oldPlatform);
							platform.setCollsionWeight(deltaColl);
							platform.AddBall(vy, dx, angle * (1 + disturbAngle * deltaAngle));
							platform.Run();
							double value = platform.Evaluation(oldPlatform, defense);
							if (value < tmp) {
								tmp = tmp * Weight + value * (1 - Weight);
							}
						}
					}
					if (tmp > maxValue) {
						maxValue = tmp;

						vec_ret->speed = vy;
						vec_ret->h_x = dx;
						vec_ret->angle = angle;
					}
					//如果价值高于一定值就输出，用于调试，设置INF表示不输出
					if (tmp > INF) {
						for (int j = 0; j < platform.Balls.size(); j++) {
							printf("%d: (%lf, %lf)\n", j, platform.Balls[j].coordinate.real(), platform.Balls[j].coordinate.imag());
						}
						printf("%lf %lf %lf : %lf\n", vy, dx, angle, tmp);
					}
				}
			}
		}
	}
	
	printf("COST: %lf s  %lf\n", 1.*(clock() - startTime)/CLOCKS_PER_SEC, 1.*RecA/ CLOCKS_PER_SEC);
	return;
}