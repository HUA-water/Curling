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
	void AddBall(double vy, double dx, double angle);
	void Run();
	double Evaluation(const GAMESTATE* const gs);
	bool InDefendArea(std::complex<double> position);
	bool InHouse(std::complex<double> position);

	const std::complex<double> TEE = std::complex<double>(TEE_X, TEE_Y);
	const double START_POINT[2] = { 2.3506, 27.6 + TEE_Y };
	const double BACK_LINE = TEE_Y - HOUSE_R;
	const double FRONT_LINE = BACK_LINE + 7.56;
	const double MAX_POINT[2] = { 4.75, 44.5 };
	const double AIR_DRAG[3] = { 0.034, 0.034, 0.01 }; //��������orʪĦ�������ٶ��й�
	const double FRICTION[3] = { 0.045, 0.045, 0.0825 }; //Ħ���������ٶ��޹�
	const double MIN_VELOCITY = 1e-2;
	const double MIN_ANGLE = 1e-2;
	const double ANGLE_LOSS[3] = { 0.21, 0.21, 0.21 }; //ת�����
	const double VELOCITY_LOSS_ANGLE[3] = { 0.000737, 0, 0 }; //ת�ǵĴ��ڵ��µ��ٶ����
	const double VELOCITY_ANGLE[3] = { 0.00196, 0, 0 }; //ת�ǵĴ��ڵ��µ��ٶȷ���ı�
	const double DELTA_TIEM = 0.1; //��ɢʱ����
	const double COLLISION[2] = { 0.5 , 0.5*0.45}; //��ײ������ģ�ʵ�����鲿��
	const double COLLISION_LOSS = 0; //��ײ�����ٶ�����


	std::vector<Ball> Balls;
	double record;
};