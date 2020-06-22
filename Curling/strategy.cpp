#include "strategy.h"

using namespace std;

//  get distance^2 from center of House
float get_dist(float x, float y)
{
	return pow(x - TEE_X, 2) + pow(y - TEE_Y, 2);
}

//  get distance^2 of two cordinates
float get_dist(float x1, float y1, float x2, float y2)
{
	return pow(x1 - x2, 2) + pow(y1 - y2, 2);
}

//  is a Stone in House
bool is_in_House(float x, float y)
{
	if (get_dist(x, y) < pow(HOUSE_R + STONE_R, 2)) {
		return true;
	}
	else {
		return false;
	}
}

//  sort Shot number (rank[] = {0, 1, 2 ... 15})
//  by distance from center of House (TEEX, TEEY)
void get_ranking(int *rank, const GAMESTATE* const gs)
{
	// init array
	for (int i = 0; i < 16; i++) {
		rank[i] = i;
	}

	// sort
	int tmp;
	for (int i = 1; i < gs->ShotNum; i++) {
		for (int j = i; j > 0; j--) {
			if (get_dist(gs->body[rank[j]][0], gs->body[rank[j]][1]) < get_dist(gs->body[rank[j - 1]][0], gs->body[rank[j - 1]][1])) {
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
	// ranking of Shot number
	// rank[n] = x;
	//   n : the n th Stone from the center of House
	//   x : the x th Shot in this End (corresponding to the number of GAMESTATE->body[x])
	int rank[16];

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