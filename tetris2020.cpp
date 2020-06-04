// Tetris2020.cpp : This file contains the 'main' function. Program execution begins and ends there.

//TODO handle screen resize

#include "pch.h"
#include <iostream>
#include <vector>
#include <thread>
#include <ctime>
#include <string>
#include <fstream>
#include <Windows.h>

using namespace std;

wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr;

int nScreenWidth = 80; //80
int nScreenHeight = 30; //30

int rotate(int px, int py, int r) {
	switch (r%4) {
	case 0:
		return py * 4 + px;
	case 1:
		return 12 + py - (px * 4);
	case 2:
		return 15 - (py * 4) - px;
	case 3:
		return 3 - py + (px * 4);

	default:
		return 0;
	}

}

bool doesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
	for (int px = 0; px < 4; px++) {
		for (int py = 0; py < 4; py++) {
			// get index into piece
			int pi = rotate(px, py, nRotation);

			//get index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			if (nPosX + px >= 0 && nPosX + px < nFieldHeight) {
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
					if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0) {
						return false;
					}
				}
			}
		}
	}

	return true;
}

int main() {
	//create assets
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");

	tetromino[1].append(L"..X.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L".X..");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X..");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"..X.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"....");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L"....");

	tetromino[4].append(L"..X.");
	tetromino[4].append(L".XX.");
	tetromino[4].append(L"..X.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"....");
	tetromino[5].append(L".XX.");
	tetromino[5].append(L"..X.");
	tetromino[5].append(L"..X.");

	tetromino[6].append(L"....");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L".X..");
	tetromino[6].append(L".X..");


	/* Create playing field */

	pField = new unsigned char[nFieldWidth * nFieldHeight];

	//board boundary
	for (int i = 0; i < nFieldWidth; i++) {
		for (int j = 0; j < nFieldHeight; j++) {
			pField[j*nFieldWidth + i] = (i == 0 || i == nFieldWidth - 1 || j == nFieldHeight - 1) ? 9 : 0;
		}
	}

	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
	for (int i = 0; i < nScreenWidth*nScreenHeight; i++) {
		screen[i] = L' ';
	}
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;


	/* game logic */

	srand(time(NULL));
	bool bGameOver = false;
	int nCurrentPiece = rand()%7;
	int nNextPiece = rand() % 7;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	bool bKey[4];
	bool bRotateHold = false;

	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;
	int nPieceCount = 0;
	int nScore = 0;
	int nHighestScore = 0;
	vector<string> vTop5Names;
	vector<int> vTop5Scores;

	vector<int> vLines;
	int nLinesCounter = 0;


	/* Get high scores */

	ifstream fLeaderboardIn("leaderboard.txt");
	if (fLeaderboardIn.is_open()) {
		string sLine;
		while (getline(fLeaderboardIn, sLine)) {
			vTop5Names.push_back(sLine);
			getline(fLeaderboardIn, sLine);
			vTop5Scores.push_back(stoi(sLine));
		}

		fLeaderboardIn.close();
		if (!vTop5Scores.empty()) {
			nHighestScore = vTop5Scores[0];
		}
	}


	/* main game loop */

	while (!bGameOver) {
		/* GAME TIMING */

		this_thread::sleep_for(50ms);
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);


		/* INPUT */

		for (int k = 0; k < 4; k++) {
			//checking for input from right key, left key, down key, Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}


		/* GAME LOGIC */

		// input is right key
		nCurrentX += (bKey[0] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY));
		// input is left key
		nCurrentX -= (bKey[1] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY));
		// input is down key
		nCurrentY += (bKey[2] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1));
		// input is Z
		if (bKey[3]) {
			nCurrentRotation += (!bRotateHold && bKey[3] && doesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY));
			bRotateHold = true;
		}
		else {
			bRotateHold = false;
		}

		if (bForceDown) {
			if (doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
				nCurrentY++;
			}
			else {
				//lock piece
				for (int px = 0; px < 4; px++) {
					for (int py = 0; py < 4; py++) {
						if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X') {
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
						}
					}
				}

				nPieceCount++;

				if (nPieceCount % 10 == 0) {
					if (nSpeed >= 10) {
						nSpeed--;
					}
				}

				//check if there are any lines
				for (int py = 0; py < 4; py++) {
					if (nCurrentY + py < nFieldHeight - 1) {
						bool bLine = true;

						for (int px = 1; px < nFieldWidth - 1; px++) {
							bLine &= (pField[(nCurrentY + py) *nFieldWidth + px]) != 0;
						}

						if (bLine) {
							// remove line, set to =
							for (int px = 1; px < nFieldWidth - 1; px++) {
								pField[(nCurrentY + py)* nFieldWidth + px] = 8;
							}

							vLines.push_back(nCurrentY + py);
							nLinesCounter++;
						}
					}
				}

				nScore += 25;
				if (!vLines.empty()) {
					nScore += pow(vLines.size(),2)*100;
				}

				// next piece
				nCurrentPiece = nNextPiece;
				nNextPiece = rand()%7;
				nCurrentRotation = 0;
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;

				//Check for Game Over
				bGameOver = !doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}

			nSpeedCounter = 0;
		}


		/* RENDER OUTPUT */

		//Draw field
		for (int i = 0; i < nFieldWidth; i++) {
			for (int j = 0; j < nFieldHeight; j++) {
				screen[(j + 2)*nScreenWidth + (i + 2)] = L" ABCDEFG=#"[pField[j*nFieldWidth + i]];
			}
		}

		//Draw current piece
		for (int px = 0; px < 4; px++) {
			for (int py = 0; py < 4; py++) {
				if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X') {
					screen[(nCurrentY + py + 2)*nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;
				}
			}
		}

		//Draw next piece
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 12, L"NEXT PIECE:");

		for (int px = 0; px < 4; px++) {
			for (int py = 0; py < 4; py++) {
				if (tetromino[nNextPiece][rotate(px, py, 0)] == L'X') {
					screen[(py+3)*nScreenWidth + (nFieldWidth + px + 6)] = nNextPiece + 65;
				}
				else {
					screen[(py + 3)*nScreenWidth + (nFieldWidth + px + 6)] = 0;
				}
			}
		}

		//Draw Score
		swprintf_s(&screen[8*nScreenWidth + nFieldWidth + 6], 24, L"SCORE: %16d", nScore);

		//Draw lines counter
		swprintf_s(&screen[9 * nScreenWidth + nFieldWidth + 6], 24, L"LINES: %16d", nLinesCounter);

		//Draw highest score
		swprintf_s(&screen[11 * nScreenWidth + nFieldWidth + 6], 24, L"HIGHEST SCORE: %8d", nHighestScore);

		//make full lines disappear
		if (!vLines.empty()) {
			WriteConsoleOutputCharacter(hConsole, screen, nScreenHeight * nScreenWidth, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);

			for (auto &v : vLines) {
				for (int px = 1; px < nFieldWidth-1; px++) {
					for (int py = v; py > 0; py--) {
						pField[py*nFieldWidth + px] = pField[(py - 1)*nFieldWidth + px];
					}
					pField[px] = 0;
				}
			}

			vLines.clear();
		}

		//Display frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth*nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	// Game Over
	CloseHandle(hConsole);
	cout << "GAME OVER" << endl << "Score: " << nScore << endl<<endl;

	/* check if score is in top 5 */
	int nLeaderboardSize = vTop5Scores.size();
	string sName;
	if (nLeaderboardSize < 5 || nScore > vTop5Scores.back()) {
		cout << "High Score! Enter your name below" << endl;
		FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
		cin >> sName;
		cout << endl;

		for (int i = 0; i < nLeaderboardSize; i++) {
			if (nScore > vTop5Scores[i]) {
				vTop5Names.insert(vTop5Names.begin() + i, sName);
				vTop5Scores.insert(vTop5Scores.begin() + i, nScore);
				break;
			}
		}

		if (nLeaderboardSize == vTop5Names.size()) {
			vTop5Names.push_back(sName);
			vTop5Scores.push_back(nScore);
		}
		else if (vTop5Names.size() > 5) {
			vTop5Names.pop_back();
			vTop5Scores.pop_back();
		}
	}

	/* print and save leaderboard */
	ofstream fLeaderboardOut("leaderboard.txt");
	cout << "LEADERBOARD:" << endl;
	for (unsigned i = 0; i < vTop5Names.size(); i++) {
		cout << i + 1 << ". " << vTop5Names[i] << ": " << vTop5Scores[i] << endl;
		fLeaderboardOut << vTop5Names[i] << endl;
		fLeaderboardOut << vTop5Scores[i] << endl;
	}
	cout << endl;

	fLeaderboardOut.close();

	system("pause");

	return 0;
}
