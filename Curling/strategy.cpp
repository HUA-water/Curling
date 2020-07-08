#include "strategy.h"
#include "simulator.h"
#include <ctime>
using namespace std;


// make your decision here
void getBestShot(const GAMESTATE* const gs, SHOTINFO* vec_ret)
{
	int startTime = clock();
	double maxValue = -INF;


	std::vector <double> DX;
	for (double dx = -1.5; dx <= 1.5; dx += 0.15) {
		DX.push_back(dx);
	}
	int normalNumber = DX.size();
	for (int i = gs->ShotNum - 1; i >= 0; i-=2) {
		if (gs->body[i][0] != 0) {
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
			for (double vy = 2.6; vy <= 6; vy += vy < 4.5 ? 0.1 : 1) {
				if (vy >= 5 && (angle != 0 || i >= normalNumber)) {
					break;
				}
				Platform platform(gs);
				platform.AddBall(vy, dx, angle);
				RecA -= clock();
				platform.Run();
				RecA += clock();
				double tmp = platform.Evaluation(gs);
				if (tmp == oldValue) {
					break;
				}
				oldValue = tmp;
				if (tmp > maxValue) {
					maxValue = tmp;

					tmpVy = vy;
					tmpDx = dx;
					tmpAngle = angle;
					vec_ret->speed = vy;
					vec_ret->h_x = dx;
					vec_ret->angle = angle;
				}
				printf("%lf %lf %lf : %lf\n", vy, dx, angle, tmp);
			}
		}
	}
	double rangeDx = 0.15;
	double rangeVy = 0.1;
	double rangeAngle = 0;
	for (double dx = tmpDx - rangeDx; dx <= tmpDx + rangeDx; dx += 0.03) {
		for (double vy = tmpVy - rangeVy; vy <= tmpVy + rangeVy; vy += 0.02) {
			for (double angle = tmpAngle - rangeAngle; angle <= tmpAngle + rangeAngle; angle += 0.2) {
				Platform platform(gs);
				platform.AddBall(vy, dx, tmpAngle);
				platform.Run();
				double tmp = platform.Evaluation(gs);
				if (tmp > maxValue) {
					maxValue = tmp;

					vec_ret->speed = vy;
					vec_ret->h_x = dx;
					vec_ret->angle = angle;
				}
				printf("%lf %lf %lf : %lf\n", vy, dx, angle, tmp);
			}
		}
	}
	printf("COST: %lf s  %lf\n", 1.*(clock() - startTime)/CLOCKS_PER_SEC, 1.*RecA/ CLOCKS_PER_SEC);
	return;
}