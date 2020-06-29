#pragma once
#include "main.h"

typedef struct _GAMESTATE {
	int		ShotNum;		// number of Shot
							// if ShotNum = n, next Shot is (n+1)th shot in this End
	int		CurEnd;			// (number of current end) - 1
	int		LastEnd;		// number of final End
	int		Score[8];		// score of each End (if Score < 0: First player in 1st End scored)
	bool	WhiteToMove;	// Which player will shot next
							// if WhiteToMove = 0: First player in 1st End will shot next, 
							//  else (WhiteToMove = 1) : Second player will shot next

	double	body[16][2];	// body[n][0] : x of coordinate of n th stone
							// body[n][1] : y of coordinate of n th stone

}GAMESTATE, * PGAMESTATE;


typedef struct  _ShotInfo
{
	_ShotInfo(double s, double h, double a)
	{
		speed = s;
		h_x = h;
		angle = a;
	};
	double speed;
	double h_x;
	double angle;
}SHOTINFO;

typedef struct _MOTIONINFO
{
	double x_coordinate;
	double y_coordinate;
	double x_velocity;
	double y_velocity;
	double angular_velocity;
}MOTIONINFO;

// positions on sheet
static const double TEE_X = (double)2.375;    // x of center of house
static const double TEE_Y = (double)4.880;    // y of center of house
static const double HOUSE_R = (double)1.870;  // radius of house
static const double STONE_R = (double)0.145;  // radius of stone

// coordinate (x, y) is in play-area if:
//   (PLAYAREA_X_MIN < x < PLAYAREA_X_MAX && PLAYAREA_Y_MIN < y < PLAYAREA_Y_MAX)
static const double PLAYAREA_X_MIN = (double)0.000 + STONE_R;
static const double PLAYAREA_X_MAX = (double)0.000 + (double)4.750 - STONE_R;
static const double PLAYAREA_Y_MIN = (double)2.650 + STONE_R;
static const double PLAYAREA_Y_MAX = (double)2.650 + (double)8.165 + STONE_R;


void getBestShot(const GAMESTATE* gs, SHOTINFO* vec_ret);