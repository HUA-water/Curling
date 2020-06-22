#pragma once
#include <string>
#include <cstring>
#include "main.h"

using namespace std;

// state of the game
GAMESTATE GameState;


//! initialize GAMESTATE
void initGameState(GAMESTATE *pgs) {
	memset(pgs->body, 0x00, sizeof(float) * 32);
	memset(pgs->Score, 0x00, sizeof(int) * 10);
	pgs->LastEnd = 0;
	pgs->CurEnd = 0;
	pgs->ShotNum = 0;
	pgs->WhiteToMove = 0;
}

//! send command
void sendCommand(const char *message)
{
	int send_len = send(m_server, message, strlen(message), 0);
	if (send_len < 0) {
		std::cout << "send failed" << std::endl;
	}
	std::cout << "send " + string(message) << std::endl;
}

//! receive command
void recvCommand(char *message, SOCKET s_server, size_t size)
{
	int length = recv(s_server, message, size, 0);
	DeleteNL(message);
	if (length < 0) {
		std::cout << "recv failed" << std::endl;
	}
	else if(length > 0) {
		std::cout << "recv message:" << message << std::endl;
	}
}


//! delete newline
void DeleteNL(char *Message)
{
	char *p;
	p = Message;

	while (*p != 0x00) {
		if (*p == '\n' || *p == '\r') {
			*p = 0x00;
			break;
		}
		p++;
	}
	return;
}

//! get argument from command
bool GetArgument(char *lpResult, size_t numberOfElements, char *Message, int n)
{
	char *p, *q;

	if (Message != NULL) {
		p = Message;
		while (*p == ' ') {
			p++;
		}

		for (int i = 0; i<n; i++) {
			while (*p != ' ') {
				if (*p == 0x00) {
					return false;
				}
				p++;
			}
			while (*p == ' ') {
				p++;
			}
		}

		q = strstr(p, " ");
		if (q == NULL) {
			strcpy(lpResult, p);
		}
		else {
			strncpy_s(lpResult, numberOfElements, p, q - p);
			if ((unsigned int)(q - p) < numberOfElements) {
				lpResult[q - p] = 0x00;
			}
		}
	}

	return true;
}

//! process command
bool processCommand(char *command)
{
	char cmd[BUFSIZE];
	char buffer[BUFSIZE];
	// delete newline
	DeleteNL(command);

	// get command
	if (!GetArgument(cmd, sizeof(cmd), command, 0)) {
		return false;
	}

	// process command
	if (_stricmp(cmd, "NEWGAME") == 0) {
	}
	else if (_stricmp(cmd, "ISREADY") == 0) {
		// initialize GameState
		initGameState(&GameState);
		sendCommand("READYOK");
	}
	else if (_stricmp(cmd, "POSITION") == 0) {
		for (int i = 0; i < 16; i++) {
			// get x cordinate
			if (!GetArgument(buffer, sizeof(buffer), command, 2 * i + 1)) {
				return false;
			}
			GameState.body[i][0] = (float)atof(buffer);

			// get y cordinate
			if (!GetArgument(buffer, sizeof(buffer), command, 2 * i + 2)) {
				return false;
			}
			GameState.body[i][1] = (float)atof(buffer);
		}
	}
	else if (_stricmp(cmd, "SETSTATE") == 0) {
		// number of current shots
		if (GetArgument(buffer, sizeof(buffer), command, 1) == FALSE) {
			return false;
		}
		GameState.ShotNum = atoi(buffer);

		// number of current ends
		if (GetArgument(buffer, sizeof(buffer), command, 2) == FALSE) {
			return false;
		}
		GameState.CurEnd = atoi(buffer);

		// number of last
		if (GetArgument(buffer, sizeof(buffer), command, 3) == FALSE) {
			return false;
		}
		GameState.LastEnd = atoi(buffer);
		if (GetArgument(buffer, sizeof(buffer), command, 4) == FALSE) {
			return false;
		}
		if (atoi(buffer) == 1) {
			GameState.WhiteToMove = true;
		}
		else {
			GameState.WhiteToMove = false;
		}
	}
	else if (_stricmp(cmd, "GO") == 0) {
		SHOTINFO shot(0.0f, 0.0f, 0.0f);

		// get a Shot by getBestShot (defined in 'strategy.cpp')
		getBestShot(&GameState, &shot);

		// create BESTSHOT command
		sprintf_s(buffer, sizeof(char) * BUFSIZE, "BESTSHOT %f %f %f", shot.speed, shot.h_x, shot.angle);

		// send BESTSHOT command
		sendCommand(buffer);
	}
	else if (_stricmp(cmd, "SCORE") == 0) {
		// get score of previous end
		if (GetArgument(buffer, sizeof(buffer), command, 1) == FALSE) {
			return false;
		}
		GameState.Score[GameState.CurEnd] = atoi(buffer);
	}
	else if (_stricmp(cmd, "MOTIONINFO") == 0)
	{
		MOTIONINFO motionInfo; 
		if (GetArgument(buffer, sizeof(buffer), command, 1) == FALSE) {
			return false;
		}
		motionInfo.x_coordinate = (float)atof(buffer);

		if (GetArgument(buffer, sizeof(buffer), command, 2) == FALSE) {
			return false;
		}
		motionInfo.y_coordinate = (float)atof(buffer);

		if (GetArgument(buffer, sizeof(buffer), command, 3) == FALSE) {
			return false;
		}
		motionInfo.x_velocity = (float)atof(buffer);

		if (GetArgument(buffer, sizeof(buffer), command, 4) == FALSE) {
			return false;
		}
		motionInfo.y_velocity = (float)atof(buffer);

		if (GetArgument(buffer, sizeof(buffer), command, 5) == FALSE) {
			return false;
		}
		motionInfo.angular_velocity = (float)atof(buffer);

		// TODO you need to make your sweep decision according to the motion information
		// if you don't want to sweep, just send nothing

		// create SWEEP command
		float sweepDistance = 4.0f; // you need to estimate the distance you want to sweep
		sprintf_s(buffer, sizeof(char) * BUFSIZE, "SWEEP %f", sweepDistance);
		// send SWEEP command
		sendCommand(buffer);
	}

	return true;
}

void initialization() {
	// initialize socket
	WORD w_req = MAKEWORD(2, 2);//version
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		std::cout << "initialization failed" << std::endl;
	}
	else {
		std::cout << "initialization succeed" << std::endl;
	}
	// check socket version
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		std::cout << "socket version is not correct" << std::endl;
		WSACleanup();
	}
	else {
		std::cout << "socket version is correct" << std::endl;
	}
}

int main()
{
	SOCKET s_server;
	SOCKADDR_IN server_addr;
	initialization();
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr(IP);//127.0.0.1
	server_addr.sin_port = htons(PORT);
	s_server = socket(AF_INET, SOCK_STREAM, 0);//7788
	if (connect(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		std::cout << "Server connection failed" << std::endl;
		WSACleanup();
	}
	else {
		std::cout << "Server connection success" << std::endl;
	}
	m_server = s_server;
	char message[BUFSIZE];

	// process command
	while (1) {
		memset(message, 0, sizeof(message));
		recvCommand(message, s_server, sizeof(message));
		// processing multiple instructions in one message buffer
		for (int i = 0, j = 0; i + j < sizeof(message); j++) {
			if (message[i + j] == 0x00) {
				processCommand(&message[i]);
				i = i + j + 1;
				j = 0;
			}
		}
	}

	return 0;
}