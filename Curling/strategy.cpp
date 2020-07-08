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
bool is_in_House(const float x, const float y)
{
	if (get_dist(x, y) < pow(HOUSE_R + STONE_R, 2)) {
		return true;
	}
	else {
		return false;
	}
}

bool is_in_House(const float* coordinate)
{
	if (get_dist(coordinate[0], coordinate[1]) < pow(HOUSE_R + STONE_R, 2)) {
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

void setShot(SHOTINFO* vec_ret, float speed, float h_x, float angle)
{
	vec_ret->speed = speed;
	vec_ret->h_x = h_x;
	vec_ret->angle = angle;
}

bool isFreeZone(const float* coordinate)
{
	bool flag = (coordinate[1] > TEE_Y && !is_in_House(coordinate)) ? 1 : 0;
	return flag;
}

bool isAlive(const float* coordinate)
{
	return isFreeZone(coordinate) || is_in_House(coordinate);
}

// firsthand best shot
void getFirstBestShot(const GAMESTATE* const gs, SHOTINFO* vec_ret, int* rank)
{
	switch (int(gs->ShotNum / 2))
	{
	case 0: 
		setShot(vec_ret, 2.5f, 0.0f, 0.0f);
		printf("0");
		break;
	case 1: 
		setShot(vec_ret, 2.5f, gs->body[0][0] - TEE_X + 0.5f, 0.0f); 
		printf("1");
		break;
	case 2:
		setShot(vec_ret, 2.5f, gs->body[0][0] - TEE_X - 0.5f, 0.0f); 
		printf("2");
		break;
	case 3: 
	case 4:
		if (is_in_House(gs->body[rank[0]][0], gs->body[rank[0]][1])) {
			// this case your opponent's curling is in the house
			if (rank[0] % 2 != gs->ShotNum % 2) setShot(vec_ret, 6.0f, -0.1f, 3.0f);
			// this case your curling is in the house
			else setShot(vec_ret, 2.9f, -0.1f, 3.4f);
		}
		// this case no curling is in the house
		else setShot(vec_ret, 3.0f, -0.2f, 3.0f);
		printf("3 & 4");
		break;
	case 5:
	case 6:
		if (is_in_House(gs->body[rank[0]][0], gs->body[rank[0]][1])) {
			// this case your opponent's curling is in the house
			if (rank[0] % 2 != gs->ShotNum % 2) setShot(vec_ret, 6.0f, gs->body[rank[0]][0] - TEE_X - 0.1f, 3.0f);
			// this case your curling is in the house
			else setShot(vec_ret, 3.0f, 1.5f, -5.0f);
		}
		else setShot(vec_ret, 3.0f, -1.5f, 5.0f);
		printf("5 & 6");
		break;
	case 7: 
		if (is_in_House(gs->body[rank[0]][0], gs->body[rank[0]][1])) {
			// this case your opponent's curling is in the house
			if (rank[0] % 2 != gs->ShotNum % 2) setShot(vec_ret, 6.0f, gs->body[rank[0]][0] - TEE_X, 0.0f);
			// this case your curling is in the house
			else setShot(vec_ret, 3.0f, 1.5f, -5.0f);
		}
		else setShot(vec_ret, 3.0f, -1.0f, 4.0f); 
		printf("7");
		break;
	default: break;
	}
}

// backhand best shot
void getBackBestShot(const GAMESTATE* const gs, SHOTINFO* vec_ret, int* rank)
{
	switch (int(gs->ShotNum / 2))
	{
	case 0: 
		if (isFreeZone(gs->body[0])) setShot(vec_ret, 3.0f, -1.5f, 8.0f);
		else if (is_in_House(gs->body[0])) setShot(vec_ret, 6.0f, gs->body[0][0] - TEE_X, 0.0f);
		else {
			if (rand() > 0.5) setShot(vec_ret, 2.7f, -0.5f, -5.0f);
			else setShot(vec_ret, 2.7f, 0.5f, 5.0f);
		}
		printf("0");
		break;
	case 1: 
		if (isFreeZone(gs->body[0]) || isFreeZone(gs->body[2])) {
			if (is_in_House(gs->body[1])) setShot(vec_ret, 2.8f, -1.5f, 8.0f);
			else setShot(vec_ret, 3.0f, -1.5f, 8.0f);
		}
		else {
			if (rand() > 0.5) setShot(vec_ret, 2.7f, -0.5f, -5.0f);
			else setShot(vec_ret, 2.7f, 0.5f, 5.0f);
		}
		printf("1");
		break;
	case 2:
	case 3:
	case 4:
		if (is_in_House(gs->body[rank[0]][0], gs->body[rank[0]][1])) {
			// this case your opponent's curling is in the house
			if (rank[0] % 2 != gs->ShotNum % 2) setShot(vec_ret, 6.0f, -0.1f, 3.0f);
			// this case your curling is in the house
			else setShot(vec_ret, 2.9f, -0.1f, 3.4f);
		}
		// this case no curling is in the house
		else setShot(vec_ret, 3.0f, -0.2f, 3.0f);
		printf("2 & 3 & 4");
		break;
	case 5:
	case 6: 
		if (is_in_House(gs->body[rank[0]][0], gs->body[rank[0]][1])) {
			// this case your opponent's curling is in the house
			if (rank[0] % 2 != gs->ShotNum % 2) setShot(vec_ret, 6.0f, gs->body[rank[0]][0] - TEE_X - 0.1f, 3.0f);
			// this case your curling is in the house
			else setShot(vec_ret, 3.0f, 1.5f, -5.0f);
		}
		else setShot(vec_ret, 3.0f, -1.5f, 5.0f);
		printf("5 & 6");
		break;
	case 7: 
		if (is_in_House(gs->body[rank[0]][0], gs->body[rank[0]][1])) {
			// this case your opponent's curling is in the house
			if (rank[0] % 2 != gs->ShotNum % 2) setShot(vec_ret, 6.0f, gs->body[rank[0]][0] - TEE_X - 0.1f, 3.0f);
			// this case your curling is in the house
			else setShot(vec_ret, 3.0f, 1.5f, -5.0f);
		}
		else setShot(vec_ret, 3.0f, -1.0f, 4.0f);
		printf("7");
		break;
	default: break;
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

	// my strategy
	bool isFirst = gs->ShotNum % 2 ? 0 : 1;
	if (isFirst) {
		getFirstBestShot(gs, vec_ret, rank);
	}
	if (!isFirst) {
		getBackBestShot(gs, vec_ret, rank);
	}
}
