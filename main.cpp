#pragma once
#include "Face.h"
#include <QtWidgets/QApplication>
#include <iostream>


string Path_To_Folder()
{
	char dir_Path[MAX_PATH];
	string path_EXE;
	string bad_Path;
	//»щем папку с ехе и обрезаем сам ехе
	GetModuleFileNameA(NULL, dir_Path, MAX_PATH);
	path_EXE = dir_Path;
	string::size_type iLastSlash = path_EXE.find_last_of("\\/");

	if (string::npos != iLastSlash)
	{
		path_EXE.erase(iLastSlash, numeric_limits<string::size_type>::max());
		path_EXE = path_EXE + "/";

		string copyFolder = path_EXE + "install_OpenCV/";

		if (!std::filesystem::exists("C:/install_OpenCV"))
		filesystem::copy(copyFolder, "C:/install_OpenCV/", filesystem::copy_options::overwrite_existing | filesystem::copy_options::recursive);

		return path_EXE;
	}

	return bad_Path;
}

string Get_Desctop_Path()
{
	string desctopPath;
	string bad_Path;
	string userprofile = std::getenv("USERPROFILE");
	if (!userprofile.empty())
	{
		return desctopPath = userprofile + "/Desktop/";
	}

	return bad_Path;
}

int main(int argc, char *argv[])
{
	//setlocale(LC_CTYPE, "rus");
	//system("chcp 1251 > text");
	QLocale curLocale(QLocale("chcp 1251"));
	QLocale::setDefault(curLocale);

	string path_EXE = Path_To_Folder();
	string desctopPath = Get_Desctop_Path();
	if (!path_EXE.empty())
	{
		QApplication a(argc, argv);
		Face w(path_EXE, desctopPath);
		w.setWindowTitle("Face and LPR");

		w.show();
	
	return a.exec();
	}

	return 0;
}
