#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN 
#pragma once
#include "Symbol_Recog.h"
#include "Face.h"
#include "View_Image.h"

#include <fstream>
#include <algorithm>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <Commdlg.h>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/opencv_modules.hpp"
#include <opencv2/ml.hpp>
#include <opencv2/flann.hpp>

using namespace std;
using namespace cv;
using namespace cv::dnn;

class Image_LPR
{

public:
	void Start(string pathEXE)
	{
		this->pathEXE = pathEXE;
		string caffeConfig_Proto = pathEXE + "models/LPR_Detect_6000.prototxt";
		string caffeWeight_Model = pathEXE + "models/LPR_Detect_6000.caffemodel";

		Net net = cv::dnn::readNetFromCaffe(caffeConfig_Proto, caffeWeight_Model);

		string fileName = Open_File_Dialog();
		if (fileName.empty())
			return;
		vector < pair<Point, Mat>> plate;
		Mat frame = imread(fileName, IMREAD_COLOR);

		if (!frame.empty())
		{

			Detecting_LPR(net, frame, plate);
			sR.Start(this->pathEXE, 0.35f);
			std::pair<string, float> result;
			string res;
			//Point x;
			for (int i = 0; i < plate.size(); i++)
			{
				//x = plate[i].first;
				result = sR.Check_Symb(plate[i].second);
				res += "Номер на фото - " + result.first + " ( " + std::to_string((int)(result.second / .08)) + " %)" + "\n\n";
				//putText(frame, result, Point(x.x - 50, x.y), FONT_HERSHEY_SIMPLEX, 0.8f, Scalar(40, 40, 255), 2);				
			}
			vI.View(frame);
			vI.View_Text(res);
			vI.show();

		}
		else
			MessageBox(0, L"ERROR file read", L"", MB_OK | MB_ICONERROR | MB_APPLMODAL);
	}

	void Close()
	{
		vI.close();
	}

private:
	string pathEXE;
	Symbol_Recog sR;
	View_Image vI;

	void Detecting_LPR(Net& net, Mat& frame3, vector <pair<Point, Mat>>& plate)
	{

		Mat frame;
		int siZe = 300;
		float w = siZe / (float)frame3.cols;
		float h = siZe / (float)frame3.rows;

		if (frame3.cols < siZe || frame3.rows < siZe)
		{
			cv::resize(frame3, frame, Size((float)frame3.cols * w, (float)frame3.rows * h));
		}
		else
		{
			frame = frame3;
		}

		int frameHeight = frame.rows;
		int frameWidth = frame.cols;

		Mat inputBlob = blobFromImage(frame, 1.0 / 127.5, Size(), Scalar::all(127.5), true, false);

		net.setInput(inputBlob, "data");
		cv::Mat detection = net.forward("detection_out");

		cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>(0));

		for (int i = 0; i < detectionMat.rows; i++)
		{
			float confidence = detectionMat.at<float>(i, 2);

			if (confidence > 0.15)
			{
				int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * frameWidth);// -13);
				int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * frameHeight);// - 4);
				int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * frameWidth); //- 2);
				int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * frameHeight);

				Rect t;

				if (frame3.cols < siZe || frame3.rows < siZe)
				{
					x1 = x1 / w;
					y1 = y1 / h;
					x2 = x2 / w;
					y2 = y2 / h;
					t = Rect(Point(x1, y1), Point(x2, y2));
				   rectangle(frame3, t, Scalar(0, 255, 0), 2, 4);
				}
				else
				{
					x1 = x1 - 4;
					y1 = y1 - 3;
					t = Rect(Point(x1, y1), Point(x2 + 7, y2));
					rectangle(frame3, t, Scalar(0, 255, 0), 2, 4);
				}
				//imwrite("C:/Users/User/Desktop/License Plate/auto2/NEW/nnn/" + to_string(t.x) + ".jpg", frame3(t));
				if (0 <= t.x && 0 <= t.width && t.x + t.width <= frame3.cols && 0 <= t.y && 0 <= t.height && t.y + t.height <= frame3.rows)
					plate.push_back(make_pair(Point(t.x, t.y), frame3(t)));
			}
		}
	}

	string Open_File_Dialog()
	{
		OPENFILENAME ofn = { 0 };
		TCHAR szFile[260] = { 0 };
		// Initialize remaining fields of OPENFILENAME structure
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = L"Image Files (*.png, *.jpg, *.jpeg)\0*.png;*.jpg;*.jpeg\0;";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		std::string fileName;
		if (GetOpenFileName(&ofn) == TRUE)
		{
			std::wstring arr_w(szFile);
			fileName = std::string(arr_w.begin(), arr_w.end());
		}
		else
		{
			MessageBox(0, L"ERROR file read", L"", MB_OK | MB_ICONERROR | MB_APPLMODAL);
		}

		return fileName;
	}

};

