#include "strategy.h"

using namespace std;

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
//先手策略
void offensiveWork(const GAMESTATE* const gs, SHOTINFO* vec_ret) {
}
//后手策略
void defensiveWork(const GAMESTATE* const gs, SHOTINFO* vec_ret) {
}
// make your decision here
void getBestShot(const GAMESTATE* const gs, SHOTINFO* vec_ret)
{
	int rank[16];
	get_ranking(rank, gs, cmpY);
	double aim = TEE_X;
	bool flag[3] = { 0, 0, 0 };
	for (int i = ((gs->ShotNum) & 1); i < gs->ShotNum; i += 2) {
		if (get_dist(gs->body[i][0], gs->body[i][1], TEE_X, TEE_Y) < 0.2) {
			flag[0] = 1;
		}
		else if (get_dist(gs->body[i][0], gs->body[i][1], TEE_X - 0.3, TEE_Y) < 0.1) {
			flag[1] = 1;
		}
		else if (get_dist(gs->body[i][0], gs->body[i][1], TEE_X + 0.3, TEE_Y) < 0.1) {
			flag[2] = 1;
		}
	}
	if (!flag[0]) {
		aim = TEE_X;
	}
	else if (!flag[1]) {
		aim = TEE_X - 0.3;
	}
	else if (!flag[2]) {
		aim = TEE_X + 0.3;
	}
	for (int i = 0; i < gs->ShotNum; i++) {
		int j = rank[i];
		if ((j & 1) != (gs->ShotNum & 1) && gs->body[j][1] > 4.5 && fabs(gs->body[j][0] - aim) < STONE_R) {
			if (gs->ShotNum < 4) {
				vec_ret->speed = 3.2f;
			}
			else {
				vec_ret->speed = 4.0f;
			}
			vec_ret->h_x = (gs->body[i][0] - TEE_X) / 2;
			vec_ret->angle = 0.0f;
			return;
		}
	}
	vec_ret->speed = 3.f;
	vec_ret->h_x = aim - TEE_X;
	vec_ret->angle = 0.0f;

	return;

	if ((gs->ShotNum & 1) == 0) {
		offensiveWork(gs, vec_ret);
	}
	else {
		defensiveWork(gs, vec_ret);
	}
	
	return;
	// ranking of Shot number
	// rank[n] = x;
	//   n : the n th Stone from the center of House
	//   x : the x th Shot in this End (corresponding to the number of GAMESTATE->body[x])
	//int rank[16];

	// sort by distance from center
	get_ranking(rank, gs);

	// create Shot according to condition of No.1 Stone
	if (is_in_House(gs->body[rank[0]][0], gs->body[rank[0]][1]))
	{
		if (rank[0] % 2 != gs->ShotNum % 2) {
			// choose Shot 1. this case your opponent's curling is in the house
			vec_ret->speed = 6.0f;
			vec_ret->h_x = -0.1f;
			vec_ret->angle = 3.0f;
		}
		else {
			// choose Shot 2.
			// this case your curling is in the house
			vec_ret->speed = 2.9f;
			vec_ret->h_x = -0.1f;
			vec_ret->angle = 3.4f;
		}
	}
	else {
		// choose Shot 3.
		// this case no curling is in the house
		
		vec_ret->speed = 3.0f;
		vec_ret->h_x = -0.2f;
		vec_ret->angle = 3.0f;
	}
	//  all bellow code is just for test
	//  you need to make your good logic or model
	if (gs->ShotNum > 10)
	{
		if (gs->ShotNum % 2 == 0)
		{
			vec_ret->speed = 3.0f;
			vec_ret->h_x = 1.5f;
			vec_ret->angle = -5.0f;
		}
		if (gs->ShotNum % 2 == 1)
		{
			vec_ret->speed = 3.0f;
			vec_ret->h_x = -1.5f;
			vec_ret->angle = 5.0f;
		}
	}
	// last shot
	if (gs->ShotNum > 14)
	{
		vec_ret->speed = 3.0f;
		vec_ret->h_x = -1.0f;
		vec_ret->angle = 4.0f;
	}
	// presentation for free defense zone rule
	if (gs->ShotNum < 5)
	{
		if (gs->ShotNum % 2 == 0)
		{
			vec_ret->speed = 2.5f;
			vec_ret->h_x = 0.0f;
			vec_ret->angle = 0.0f;
		}
		else
		{
			vec_ret->speed = 5.0f;
			vec_ret->h_x = 0.1f;
			vec_ret->angle = -1.0f;
		}
	}
}