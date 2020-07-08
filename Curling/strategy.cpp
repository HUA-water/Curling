#include "strategy.h"
#include "simulator.h"
#include <ctime>
using namespace std;


// make your decision here
void getBestShot(const GAMESTATE* const gs, SHOTINFO* vec_ret)
{
	int startTime = clock();
	double maxValue = -INF;
	
	for (double dx = -1.5; dx <= 1.5; dx += 0.1) {
		for (double angle = -10; angle <= 10; angle += 10) {
			double oldValue = -INF - 1;
			for (double vy = 2.6; vy <= 10; vy += vy < 4 ? 0.1 : 1) {
				Platform platform(gs);
				platform.AddBall(vy, dx, angle);
				platform.Run();
				double tmp = platform.Evaluation(gs);
				if (tmp == oldValue) {
					break;
				}
				oldValue = tmp;
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
	printf("COST: %lf s\n", 1.*(clock() - startTime)/CLOCKS_PER_SEC);
	return;
}