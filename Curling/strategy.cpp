#include "strategy.h"
#include "simulator.h"
using namespace std;


// make your decision here
void getBestShot(const GAMESTATE* const gs, SHOTINFO* vec_ret)
{
	double maxValue = -INF;
	for (double vy = 2.6; vy <= 10; vy += vy < 4 ? 0.1 : 1) {
		for (double dx = -2.0; dx <= 2.0; dx += 0.1) {
			for (double angle = -10; angle <= 10; angle += 10) {
				Platform platform(gs);
				platform.AddBall(vy, dx, angle);
				platform.Run();
				double tmp = platform.Evaluation(gs);
				if (tmp > maxValue) {
					maxValue = tmp;

					vec_ret->speed = vy;
					vec_ret->h_x = dx;
					vec_ret->angle = angle;
					/*
					for (int i = 0; i < platform.Balls.size(); i++) {
						printf("%lf %lf\n", platform.Balls[i].coordinate.real(), platform.Balls[i].coordinate.imag());
						printf("%lf %lf\n", platform.Balls[i].velocity.real(), platform.Balls[i].velocity.imag());
					}
					*/
				}
				//printf("%lf %lf %lf : %lf\n", vy, dx, angle, tmp);
			}
		}
	}
	return;
}