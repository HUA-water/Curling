#include "strategy.h"

using namespace std;

void DefenseStrategy(const GAMESTATE* const gs, SHOTINFO* vec_ret);
void TestStrategy(const GAMESTATE* const gs, SHOTINFO* vec_ret);


double target[2];
void Regular(double* target, SHOTINFO* vec_ret)
{
	vec_ret->angle = 0;
	vec_ret->h_x = target[0] - TEE_X;
	vec_ret->speed = sqrt((44.5 - target[1]) * 2 * 0.1134);
}

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

// get distance^2 from a stone to the center of House
float get_dist(const float body[2])
{
	return get_dist(body[0], body[1]);
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

bool is_in_House(const float body[2])
{
	if (is_in_House(body[0], body[1]))
	{
		return true;
	}
	else return false;
}

int num_in_House(const GAMESTATE* const gs)
{
	int num = 0;
	for (int i = 0; i < gs->ShotNum; i++)
	{
		if (is_in_House(gs->body[i]))
		{
			num++;
		}
	}
	return num;
}

//  sort Shot number (rank[] = {0, 1, 2 ... 15})
//  by distance from center of House (TEEX, TEEY)
void get_ranking(int* rank, const GAMESTATE* const gs)
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
	//// ranking of Shot number
	//// rank[n] = x;
	////   n : the n th Stone from the center of House
	////   x : the x th Shot in this End (corresponding to the number of GAMESTATE->body[x])
	//int rank[16];

	//// sort by distance from center
	//get_ranking(rank, gs);

	//// create Shot according to condition of No.1 Stone
	//if (is_in_House(gs->body[rank[0]][0], gs->body[rank[0]][1]))
	//{
	//	if (rank[0] % 2 != gs->ShotNum % 2) {
	//		// choose Shot 1. this case your opponent's curling is in the house
	//		vec_ret->speed = 6.0f;
	//		vec_ret->h_x = -0.1f;
	//		vec_ret->angle = 3.0f;
	//	}
	//	else {
	//		// choose Shot 2.
	//		// this case your curling is in the house
	//		vec_ret->speed = 2.9f;
	//		vec_ret->h_x = -0.1f;
	//		vec_ret->angle = 3.4f;
	//	}
	//}
	//else {
	//	// choose Shot 3.
	//	// this case no curling is in the house
	//	
	//	vec_ret->speed = 3.0f;
	//	vec_ret->h_x = -0.2f;
	//	vec_ret->angle = 3.0f;
	//}
	////  all bellow code is just for test
	////  you need to make your good logic or model
	//if (gs->ShotNum > 10)
	//{
	//	if (gs->ShotNum % 2 == 0)
	//	{
	//		vec_ret->speed = 3.0f;
	//		vec_ret->h_x = 1.5f;
	//		vec_ret->angle = -5.0f;
	//	}
	//	if (gs->ShotNum % 2 == 1)
	//	{
	//		vec_ret->speed = 3.0f;
	//		vec_ret->h_x = -1.5f;
	//		vec_ret->angle = 5.0f;
	//	}
	//}
	//// last shot
	//if (gs->ShotNum > 14)
	//{
	//	vec_ret->speed = 3.0f;
	//	vec_ret->h_x = -1.0f;
	//	vec_ret->angle = 4.0f;
	//}
	//// presentation for free defense zone rule
	//if (gs->ShotNum < 5)
	//{
	//	if (gs->ShotNum % 2 == 0)
	//	{
	//		vec_ret->speed = 2.5f;
	//		vec_ret->h_x = 0.0f;
	//		vec_ret->angle = 0.0f;
	//	}
	//	else
	//	{
	//		vec_ret->speed = 5.0f;
	//		vec_ret->h_x = 0.1f;
	//		vec_ret->angle = -1.0f;
	//	}
	//}
	TestStrategy(gs, vec_ret);
	//if (gs->LastEnd == 4) //初赛4局
	//{
	//	DefenseStrategy(gs, vec_ret);
	//}
	//else if (gs->LastEnd == 8) //决赛8局
	//{
	//	DefenseStrategy(gs, vec_ret);
	//}
	//else //单局测试
	//{
	//	DefenseStrategy(gs, vec_ret);
	//}
}

//防守策略
void DefenseStrategy(const GAMESTATE* const gs, SHOTINFO* vec_ret)
{
	double target[2];
	int rank[16];
	get_ranking(rank, gs);

	if (gs->ShotNum == 0)
	{
		target[0] = TEE_X;
		target[1] = TEE_Y;
	}
	else if (gs->ShotNum % 2 != rank[0] % 2)
	{
		if (is_in_House(gs->body[rank[0]][0], gs->body[rank[0]][1]))
		{
			target[0] = gs->body[rank[0]][0];
			target[1] = gs->body[rank[0]][1] - 15.0;
			//printf("state1, target[0]: %f, target[1]: %f\n", target[0], target[1]);
		}
		else if (gs->body[rank[0]][1] > TEE_Y)
		{
			if (abs(gs->body[rank[0]][0] - TEE_X) > 2.03 * STONE_R)
			{
				target[0] = TEE_X;
				target[1] = TEE_Y;
				//printf("state2-1\n");
			}
			else
			{
				if (gs->body[rank[0]][0] > TEE_X)
				{
					target[0] = gs->body[rank[0]][0] - 2.03 * STONE_R;
				}
				else
				{
					target[0] = gs->body[rank[0]][0] + 2.03 * STONE_R;
				}
				target[1] = TEE_Y;
				//printf("state22\n");
			}
		}
		else
		{
			target[0] = TEE_X;
			target[1] = TEE_Y;
			//printf("state3");
		}

	}
	else
	{
		target[0] = TEE_X;
		target[1] = TEE_Y;
	}
	Regular(target, vec_ret);
}


//测试策略
void TestStrategy(const GAMESTATE* const gs, SHOTINFO* vec_ret)
{
	/*7_08 Afternoon*/
	double target[2];
	//先手1：直接发中心
	if (gs->ShotNum == 0)
	{
		target[0] = TEE_X;
		target[1] = TEE_Y;
		Regular(target, vec_ret);
	}

	//后手1：
	else if (gs->ShotNum == 1)
	{
		//对手壶在圈内
		if (is_in_House(gs->body[0][0], gs->body[0][1]))
		{
			if (get_dist(gs->body[0][0], gs->body[0][0]) < 2.5 * STONE_R) //近似认为对方壶在圆心附近，直接撞击
			{
				vec_ret->speed = 4.0f;
				vec_ret->h_x = gs->body[0][0];
				vec_ret->angle = 0;
			}
			else if (abs(gs->body[0][0] - TEE_X) < 0.5 * STONE_R) //对方壶在中心道路上，用旋转方式绕行
			{
				vec_ret->speed = 3;
				if (gs->body[0][0] < TEE_X)
				{
					vec_ret->h_x = 0.5;
					vec_ret->angle = -2.5;
				}
				else
				{
					vec_ret->h_x = -0.5;
					vec_ret->angle = 2.5;
				}
			}
			else
			{
				target[0] = TEE_X;
				target[1] = TEE_Y;
				Regular(target, vec_ret);
			}
		}
		//对手壶在自由防守区
		else if (gs->body[0][1] > TEE_Y)
		{
			if (abs(gs->body[0][0] - TEE_X) < 0.5 * STONE_R) //在中路，旋进
			{
				vec_ret->speed = 3;
				if (gs->body[0][0] < TEE_X)
				{
					vec_ret->h_x = 0.5;
					vec_ret->angle = -2.5;
				}
				else
				{
					vec_ret->h_x = -0.5;
					vec_ret->angle = 2.5;
				}
			}
			else
			{
				target[0] = TEE_X;
				target[1] = TEE_Y;
				Regular(target, vec_ret);
			}
		}

	}
	//先手2
	else if (gs->ShotNum == 2)
	{
		//第一个壶还在圈内
		if (is_in_House(gs->body[0]))
		{
			if (get_dist(gs->body[0]) > get_dist(gs->body[1])) //对手壶比自己近
			{
				if (gs->body[0][0] < gs->body[1][0])
				{
					target[0] = gs->body[1][0] - 0.5 * STONE_R;
					target[1] = gs->body[1][1] - 3.0;
					Regular(target, vec_ret);
				}
				else
				{
					target[0] = gs->body[1][0] + 0.5 * STONE_R;
					target[1] = gs->body[1][1] - 3.0;
					Regular(target, vec_ret);

				}
			}
			else  //自己壶近，保护
			{
				target[0] = gs->body[0][0];
				target[1] = gs->body[0][1] + 3.5;
				Regular(target, vec_ret);
			}
		}
		else if (is_in_House(gs->body[1])) //只剩对方壶
		{
			target[0] = gs->body[1][0];
			target[1] = gs->body[1][1] - 10;
			Regular(target, vec_ret);
		}
		else if (num_in_House(gs) == 0) //圈内无壶
		{
			target[0] = TEE_X;
			target[1] = TEE_Y;
			Regular(target, vec_ret);
		}
	}

	//后手2
	else
	{
		int rank[16];
		get_ranking(rank, gs);
		if (!is_in_House(gs->body[rank[0]]))
		{
			target[0] = TEE_X;
			target[1] = TEE_Y;
			Regular(target, vec_ret);
		}
		else
		{
			if (rank[0] % 2 != gs->ShotNum % 2)
			{
				int flag = 0;
				int i;
				for (i = 1; i < gs->ShotNum; i++)
				{
					if (abs(gs->body[rank[i]][0] - gs->body[rank[0]][0]) < 2 * STONE_R && gs->body[rank[i]][1] > gs->body[rank[0]][1])
					{
						flag = 1;
						break;
					}
				}
				if (flag == 0)
				{
					target[0] = gs->body[rank[0]][0];
					target[1] = gs->body[rank[0]][1] - 10;
					Regular(target, vec_ret);
				}
				else
				{
					vec_ret->speed = 3.2;
					if (gs->body[rank[i]][0] < gs->body[rank[0]][0])
					{
						vec_ret->h_x = gs->body[rank[0]][0] + 0.5*STONE_R;
						vec_ret->angle = -2.5;
					}
					else
					{
						vec_ret->h_x = gs->body[rank[0]][0] - 0.5*STONE_R;
						vec_ret->angle = 2.5;
					}
				}
			}
			else
			{
				target[0] = gs->body[rank[0]][0] - 0.5 * STONE_R;
				target[1] = gs->body[rank[0]][1] + 4;
				Regular(target, vec_ret);
			}
		}
	}
}