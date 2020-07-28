#include "strategy.h"
#include "simulator.h"
#include <ctime>
using namespace std;


// make your decision here
const int MAX_TIME = 55 * CLOCKS_PER_SEC;
double search(int deep, Platform oldPlatform, SHOTINFO* action = NULL) {
	int time_limit = clock() + MAX_TIME;
	if (oldPlatform.Balls.size() == 16) {
		return 0;
	}
	double maxValue = -INF;
	SHOTINFO shotRes(0,0,0);
	SHOTINFO space(0.1, 0.1, 10);
	SHOTINFO range(2.6, 1.7, 10);
	if (action == NULL) {
		space = SHOTINFO(3.5, INF, 10);
		range = SHOTINFO(3, 0, 10);
	}
	if (action != NULL && oldPlatform.Balls.size() == 15) {
		space = SHOTINFO(0.05, 0.05, 5);
		range = SHOTINFO(2.6, 1.8, 10);
	}

	std::vector <double> DX;
	for (double dx = -range.h_x; dx <= range.h_x; dx += space.h_x) {
		DX.push_back(dx);
	}
	int normalNumber = DX.size();
	double minDist = 1e9;
	for (int i = oldPlatform.Balls.size() - 1; i >= 0; i -= 2) {
		double tmp = std::abs(oldPlatform.Balls[i].coordinate - oldPlatform.TEE);
		if (tmp < minDist) {
			minDist = tmp;
		}
	}
	for (int i = oldPlatform.Balls.size() - 1; i >= 0; i -= 2) {
		if (std::abs(oldPlatform.Balls[i].coordinate) > eps && std::abs(oldPlatform.Balls[i].coordinate - oldPlatform.TEE) <= minDist + 0.1) {
			DX.push_back(oldPlatform.Balls[i].coordinate.real() - 2.3506);
		}
	}

	Platform test(oldPlatform);
	test.AddBall(0,-10,0);
	test.Run();
	double valueLeast = test.Evaluation(oldPlatform, action != NULL);

	for (int i = 0; i < DX.size(); i++) {
		double dx = DX[i];
		for (double angle = -range.angle; angle <= range.angle; angle += space.angle) {
			double oldValue = -INF - 1;
			if (angle != 0 && i >= normalNumber) {
				continue;
			}
			if (action == NULL && i < normalNumber && ((dx < -eps && angle < eps) || (dx > eps && angle > -eps))) {
				continue;
			}
			for (double vy = range.speed; vy <= 7; vy += vy < 4 ? space.speed : 2) {
				if (clock() > time_limit && action != NULL){
					break;
				}
				Platform platform(oldPlatform);
				platform.AddBall(vy, dx, angle);
				platform.Run();
				double tmp = platform.Evaluation(oldPlatform, action != NULL);
				if (tmp == valueLeast) break;
				if (deep) {
					tmp = tmp * 0.3 - search(deep - 1, platform) * 0.7;
				}
				if (tmp == oldValue) {
					break;
				}
				oldValue = tmp;
				if (tmp > maxValue) {
					maxValue = tmp;

					shotRes = SHOTINFO(vy, dx, angle);
				}
				//如果价值高于一定值就输出，用于调试，设置INF表示不输出
				if (deep == 1 && tmp > INF) {
					/*for (int j = 0; j < platform.Balls.size(); j++) {
						printf("%d: (%lf, %lf)\n", j, platform.Balls[j].coordinate.real(), platform.Balls[j].coordinate.imag());
					}*/
					printf("%d : %lf %lf %lf : %lf\n\n", deep, vy, dx, angle, tmp);
				}
			}
		}
	}

	if (action != NULL) {
		SHOTINFO tmpShot(shotRes);
		double rangeDx = 0.05;
		double rangeVy = 0.05;
		double rangeAngle = 4;
		for (double dx = tmpShot.h_x - rangeDx; dx <= tmpShot.h_x + rangeDx; dx += 0.01) {
			for (double vy = tmpShot.speed - rangeVy; vy <= tmpShot.speed + rangeVy; vy += 0.01) {
				for (double angle = tmpShot.angle - rangeAngle; angle <= tmpShot.angle + rangeAngle; angle += 2) {
					if (std::abs(dx) <= 2 && std::abs(angle) <= 10 && vy < 10) {
						if (clock() > time_limit && action != NULL) {
							break;
						}
						Platform platform(oldPlatform);
						platform.AddBall(vy, dx, angle);
						platform.Run();
						double tmp = platform.Evaluation(oldPlatform);
						if (deep) {
							tmp = tmp * 0.3 - search(deep - 1, platform) * 0.7;
						}
						if (tmp > maxValue) {
							maxValue = tmp;
							shotRes = SHOTINFO(vy, dx, angle);
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
	}
	if (action != NULL) {
		*action = shotRes;
	}
	return maxValue;
}
void getBestShot(const GAMESTATE* const gs, SHOTINFO* vec_ret)
{
	int startTime = clock();

	double value = search(1, gs, vec_ret);

	
	printf("COST: %lfs  %lf\n", 1.*(clock() - startTime)/CLOCKS_PER_SEC, value);
	return;
}