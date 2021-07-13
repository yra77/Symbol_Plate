#define WIN32_LEAN_AND_MEAN 
#pragma once
#include "Face.h"
#include "ui_Face.h"
#include "LPR.h"
#include "Image_LPR.h"

#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

class Add_Mouse_Region
{
public:
	//выделение региона детекции
	static inline int init = 0;
	//начальные координаты
	static inline int initX = 0;
	static inline int initY = 0;
	//конечные координаты
	static inline int actualX = 0;
	static inline int actualY = 0;

	Rect Start(string path, int size, int& x1, int& x2, int& y1, int& y2)
	{
		Mat frame;
		VideoCapture capture;
		init = 0;
		Rect roi;
		
		if (path == "0" || path == "1" || path == "2" || path == "3" || path == "4" || path == "5")
		{
			int deviceID = atoi(path.c_str()); 
			capture.open(deviceID);
		}
		else
		{
			capture.open(path);
		}

		if (!capture.isOpened())
			throw "Error when reading steam_avi";

		cv::resizeWindow("Region", 1000, 500);
		namedWindow("Region", WINDOW_NORMAL);
		MessageBox(0, L"Выберите мышкой, регион для детектирования.", L"", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
		
		capture >> frame;
		float w = (float)frame.cols / size;
		float h = (float)frame.rows / size;

		setMouseCallback("Region", CallBackF, &frame);

		for (int g = 0; ; g++)
		{
			capture.read(frame);

			if (waitKey(30) >= 0 || frame.empty())
			{		
				break;
			}
			if (cv::getWindowProperty("Region", WINDOW_NORMAL) == -1)
			{
				break;
			}

			rectangle(frame, Rect(Add_Mouse_Region::initX, Add_Mouse_Region::initY, Add_Mouse_Region::actualX - Add_Mouse_Region::initX,
				             Add_Mouse_Region::actualY - Add_Mouse_Region::initY), Scalar(40, 40, 255), 2);
			if (init == 3)
			{				
				int i = MessageBox(0, L"Сохранить ?", L"", MB_OKCANCEL | MB_ICONINFORMATION | MB_APPLMODAL);
				if (i == IDCANCEL)
				{
					init = 0;
					continue;
				}
				if (i == IDOK)
				{
					//roi = Rect(Point(Add_Mouse_Region::initX, Add_Mouse_Region::initY), Point(Add_Mouse_Region::actualX, Add_Mouse_Region::actualY));
					cv::destroyAllWindows();
					y2 = actualY / h;
					x2 = actualX / w;//x2 = size;
					x1 = initX / w;// x1 = 0;
					y1 = initY / h;
					roi = Rect(Point(x1, y1), Point(x2, y2));
					initX = 0;
					initY = 0;
					actualX = 0;
					actualY = 0;
					return roi;
				}
			}
			imshow("Region", frame);
		}
		y2 = frame.rows / h;
		x2 = size;
		x1 = 0;
		y1 = frame.rows / 2 / h;
		roi = Rect(Point(x1, y1), Point(x2, y2));
		initX = 0;
		initY = 0;
		actualX = 0;
		actualY = 0;
		cv::destroyAllWindows();
		MessageBox(0, L"Регион не выбран. Детектируется нижняя половина", L"", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
		return roi;
	}

	static void CallBackF(int event, int x, int y, int flags, void* ptr)
	{

		if (event == EVENT_RBUTTONDOWN)
		{
			return;
		}
		if (event == EVENT_LBUTTONDOWN)
		{
			if (init == 0)
			{
				initX = x;
				initY = y;
				init++;
			}
			return;
		}
		if (event == EVENT_MOUSEMOVE)
		{
			if (init == 1)
			{
				actualX = x;
				actualY = y;
			}
			return;
		}

		if (event == EVENT_LBUTTONUP)
		{
			if (init == 1)
			{
				actualX = x;
				actualY = y;
				init = 3;
			}
			return;
		}
	}

};