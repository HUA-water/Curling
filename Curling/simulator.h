#pragma once
#include "strategy.h"
#include <vector>
#include <complex>

const double INF = 1e9;
const double eps = 1e-6;

struct Ball {
	Ball(std::complex<double> _coordinate, std::complex<double> _velocity = std::complex<double>(0, 0), double _angle = 0)
		:coordinate(_coordinate), velocity(_velocity), angle(_angle){}
	std::complex<double> coordinate;
	std::complex<double> velocity;
	double angle;
};

class Platform {
public:
	Platform(const GAMESTATE* const gs);
	void setCollsionWeight(double weight);
	void AddBall(double vy, double dx, double angle);
	void Run();
	double Evaluation(const Platform& const oldPlatform);
	bool InDefendArea(std::complex<double> position);
	bool InDefendAreaLoose(std::complex<double> position);
	bool InHouse(std::complex<double> position);
	bool InHouseLoose(std::complex<double> position);
	bool OutLoose(std::complex<double> position);
	void modelWork(int input[4], int output[4]);

	const std::complex<double> TEE = std::complex<double>(TEE_X, TEE_Y);
	const double START_POINT[2] = { 2.3506, 27.6 + TEE_Y };
	const double BACK_LINE = TEE_Y - HOUSE_R;
	const double FRONT_LINE = BACK_LINE + 7.56;
	const double MAX_POINT[2] = { 4.75, 44.5 };
	const double AIR_DRAG[3] = { 0.0326, 0.0329, 0.03 }; //空气阻力or湿摩擦，与速度有关
	const double FRICTION[3] = { 0.05, 0.05, 0.0689 }; //摩擦力，与速度无关
	const double MIN_VELOCITY = 1e-2;
	const double MIN_ANGLE = 10;
	const double ANGLE_LOSS[3] = { 0.2127, 0.214, 0.2127 }; //转角损耗
	const double VELOCITY_LOSS_ANGLE[3] = { 0.00065, 0, 0 }; //转角的存在导致的速度损耗
	const double VELOCITY_ANGLE[3] = { 0.00190, 0.0, 0.00182 }; //转角的存在导致的速度方向改变
	const double ANGLE_INCRESS_VELOCITY[3] = { 0, 0, 0 };
	const double DELTA_TIME = 0.2; //离散时间间隔
	const double COLLISION[2] = { 0.5 , 0.5*0.45}; //碰撞力的损耗（实部和虚部）
	const double COLLISION_LOSS = 0; //碰撞产生速度削减

	const double collisionModel_W0[4][8] = {
		{4.1131996e-02, -1.0417538e+00, -5.9740597e-01, -1.0872648e+00, -8.3225191e-01, -6.0967201e-01, -4.9793854e-01, 6.6255634e-03},
		{-1.1378649e-03, 3.3654135e-01, -2.1296076e-01, -1.3018285e-02, -8.6532021e-03, -7.0109017e-02, -2.5112593e-01, 9.2475920e-04},
		{-2.1793677e-02, 4.2148504e-02, 1.8028688e-02, -3.8841985e-02, -3.2606758e-02, 6.6548474e-03, 4.5798589e-02, 2.4229083e-02},
		{2.1343712e-02, 2.1966431e-02, -2.7804971e-03, -3.6419608e-02, -4.1233204e-02, -2.2620607e-02, 3.1847347e-02, 3.2174680e-02}
	};
	const double collisionModel_W1[8][4] = {
		{4.0737279e-02, -1.9420493e-02, 7.4601336e-04, -2.7470462e-02},
		{-1.9666934e-01, 1.3615680e+00, -1.6374318e-01, 6.0069494e-02},
		{-1.0264250e-01, -9.1514587e-01, -1.6832444e-01, -1.4787009e-03},
		{2.7077484e+00, -2.3846289e-02, -2.6073647e+00, 3.1547647e-02},
		{-2.9004893e+00, 3.1473950e-02, 2.8288262e+00, -3.7404153e-02},
		{-3.5082957e-01, -6.3679028e-01, -3.8595623e-01, -6.7250505e-02},
		{-8.9578532e-02, -9.6864384e-01, 3.3726458e-02, -4.9371529e-02},
		{-5.7815138e-02, 3.4212465e-03, 2.1068506e-02, -2.7996741e-02}
	};
	const double collisionModel_b0[8] = { -0.01198078, 1.6158131, 1.0908095, -2.1039994, -1.9520979, 0.42485985, 1.2549162, -0.01099465 };
	const double collisionModel_b1[4] = { 0.72483873, 0.27127874, 0.60260564, 0.00277396 };

	std::vector<Ball> Balls;
	double record;
	double collsionWeight;
};