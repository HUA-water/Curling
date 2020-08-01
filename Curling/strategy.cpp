#include "strategy.h"

using namespace std;

extern int TotalScore;
//  get distance from center of House
double get_dist(double x, double y)
{
	return pow(pow(x - TEE_X, 2) + pow(y - TEE_Y, 2), 0.5);
}

//  get distance of two cordinates
double get_dist(double x1, double y1, double x2, double y2)
{
	return pow(pow(x1 - x2, 2) + pow(y1 - y2, 2), 0.5);
}

//  is a Stone in House
bool is_in_House(double x, double y)
{
	if (get_dist(x, y) < HOUSE_R + STONE_R) {
		return true;
	}
	else {
		return false;
	}
}
double cmpX(double x, double y) {
	return x;
}
double cmpY(double x, double y) {
	return y;
}
double cmpDisHouse(double x, double y) {
	return get_dist(x, y);
}

//  sort Shot number (rank[] = {0, 1, 2 ... 15})
//  by distance from center of House (TEEX, TEEY)
void get_ranking(int *rank, const GAMESTATE* const gs, double cmp(double, double) = cmpDisHouse)
{
	// init array
	for (int i = 0; i < 16; i++) {
		rank[i] = i;
	}

	// sort
	int tmp;
	for (int i = 1; i < gs->ShotNum; i++) {
		for (int j = i; j > 0; j--) {
			if (cmp(gs->body[rank[j]][0], gs->body[rank[j]][1]) < cmp(gs->body[rank[j - 1]][0], gs->body[rank[j - 1]][1])) {
				// swap
				tmp = rank[j];
				rank[j] = rank[j - 1];
				rank[j - 1] = tmp;
			}
			else {
				break;
			}
		}
	}
}
// make your decision here
void getBestShot(const GAMESTATE* const gs, SHOTINFO* vec_ret)
{
	if (gs->ShotNum == 0) {
		vec_ret->speed = 10.0f;
		vec_ret->h_x = 0.0f;
		vec_ret->angle = 0.0f;
		return;
	}
	else if (gs->ShotNum < 4){
		for (int i = gs->ShotNum-1; i>=0; i-=2)
		if (gs->body[i][1] > 1) {
			double x = gs->body[i][1];
			vec_ret->speed = x*x*(-0.03560838)+0.70997966*x+1.26361588;
			vec_ret->h_x = gs->body[i][0] - 2.3506;
			vec_ret->angle = 0.0f;
			return;
		}
		vec_ret->speed = 10.0f;
		vec_ret->h_x = 0.0f;
		vec_ret->angle = 0.0f;
		return;
	}
	else if (gs->ShotNum < 15){
		for (int i = gs->ShotNum - 1; i >= 0; i -= 2) {
			if (gs->body[i][1] > 1) {
				double x = gs->body[i][1];
				vec_ret->speed = 8.f;
				vec_ret->h_x = gs->body[i][0] - 2.3506;
				if (vec_ret->h_x > 0) {
					vec_ret->h_x -= 0.08;
				}
				else {
					vec_ret->h_x -= 0.08;
				}
				vec_ret->angle = 0.0f;
				return;
			}
		}
		vec_ret->speed = 10.0f;
		vec_ret->h_x = 0.0f;
		vec_ret->angle = 0.0f;
		return;
	}
	else {
		if (gs->LastEnd == gs->CurEnd + 1) {
			for (int i = gs->ShotNum - 1; i >= 0; i -= 2) {
				if (get_dist(gs->body[i][0], gs->body[i][1]) < 0.8) {
					double x = gs->body[i][1];
					vec_ret->speed = 8.f;
					vec_ret->h_x = gs->body[i][0] - 2.3506;
					vec_ret->angle = 0.0f;
					return;
				}
			}
			for (double dx = 0;;) {
				bool flag = true;
				for (int i = gs->ShotNum - 1; i >= 0; i --) {
					if (std::abs(gs->body[i][0] - dx - 2.3506) < 0.3&& gs->body[i][1]>4.6) {
						flag = false;
						break;
					}
				}
				if (flag) {
					vec_ret->speed = 3.f;
					vec_ret->h_x = dx;
					vec_ret->angle = 0.0f;
					return;
				}
				if (dx > 1e-9) {
					dx = -dx;
				}
				else {
					dx = -dx + 0.1;
				}
			}
		}
		else {
			for (int i = gs->ShotNum - 1; i >= 0; i -= 2) {
				if (gs->body[i][1] > 1) {
					double x = gs->body[i][1];
					vec_ret->speed = 8.f;
					vec_ret->h_x = gs->body[i][0] - 2.3506;
					if (vec_ret->h_x > 0) {
						vec_ret->h_x -= 0.08;
					}
					else {
						vec_ret->h_x -= 0.08;
					}
					vec_ret->angle = 0.0f;
					return;
				}
			}
			vec_ret->speed = 10.0f;
			vec_ret->h_x = 0.0f;
			vec_ret->angle = 0.0f;
			return;
		}
	}
	
	return;
}