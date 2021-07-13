#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include <Windows.h>
#include <Commdlg.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <string>
#include <filesystem>

class Write_To_File
{
public:
	void To_File(std::string& str, std::string desckTop_Path)
	{
		if (!std::filesystem::exists(desckTop_Path + "Auto Base"))
		{
			std::filesystem::create_directories(desckTop_Path + "Auto Base");
		}

			std::wofstream writeFile;

			time_t now = time(0);
			tm* timeNow = localtime(&now);
			std::string tm = std::to_string(timeNow->tm_mday) + "." + std::to_string(timeNow->tm_mon + 1) + " " + std::to_string(timeNow->tm_hour) + "." + std::to_string(timeNow->tm_min) + "." + std::to_string(timeNow->tm_sec);
			
			std::string fileName = desckTop_Path + "Auto Base/" + tm + ".csv";

			writeFile.open(fileName);
			if (writeFile.is_open())
			{
				writeFile << str.c_str();
				writeFile.close();

				MessageBox(0, L"File Write!", L"", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
			}
			else
				MessageBox(0, L"File not write!", L"", MB_OK | MB_ICONERROR | MB_APPLMODAL);
	}

};
