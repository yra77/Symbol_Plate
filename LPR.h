#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include "Face.h"
#include "Symbol_Recog.h"
#include "Add_Mouse_Region.h"
#include "View_Result.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <array>
#include <mutex>
#include <Commdlg.h>
#include <deque>
#include <condition_variable>
#include <thread>
#include <future>

#include <QDialog>
#include <QLabel>
#include <QMetaObject>
#include <QArgument>
#include <QtCore>
#include <QLineEdit>
#include <QLayout>
#include <QTextCursor>
#include <QtWidgets/QMainWindow>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect.hpp>

using namespace std;
using namespace cv;
using namespace cv::dnn;

class LPR
{
	//Q_OBJECT

public:
	bool state = false;

	void Thread_Video()
	{

		Mat frame3;
		int period = 0;
		
		while (1)
		{
			capture.read(frame);

			if (!state || waitKey(30) >= 0 || frame.empty() || !resultWind.state)
			{
				state = false;
				if (this->ui.LPR_Button->text() == "Dis LPR")
				{
					this->ui.LPR_Button->setText("Video LPR");
					this->ui.LPR_Button->setStyleSheet("QPushButton {border-color: cyan;}");
				}
				if (this->ui.Camera_LPR_Button->text() == "Pause LPR")
				{
					this->ui.Camera_LPR_Button->setText("Camera LPR");
					this->ui.Camera_LPR_Button->setStyleSheet("QPushButton {border-color: cyan;}");
				}
				capture.release();
				break;
			}

			// передача frame в основной поток для вывода на экран
			qRegisterMetaType<cv::Mat>("cv::Mat");
			QMetaObject::invokeMethod
			(
				&resultWind,
				"View_Video",
				Qt::QueuedConnection,
				Q_ARG(cv::Mat, frame)
			);
			///////

			cv::resize(frame, frame3, Size(siZe, siZe), InterpolationFlags::INTER_CUBIC);

			if (period == 3)// каждый n кадр записываем в очередь и передаём на проверку
			{

				Mat a = frame3(roi).clone();
				Mat b = frame.clone();
				queueData.push_back(make_pair(a, b));

				period = 0;
			}
			period++;
		}
		onOff = true;
	}

	void Start(bool flag, string path, string pathEXE, string desckTop, Ui::FaceClass ui)// true - camera, false - mp4
	{
		this->ui = ui;
		this->pathEXE = pathEXE;
		this->desckTop = desckTop;
		onOff = false;
		queueData.clear();
		arrNum.clear();
		//string path;
		siZe = 350;
		int deviceID = 0;  // 0 = open default camera

		if (flag)
		{
			if (path == "0" || path == "1" || path == "2" || path == "3" || path == "4" || path == "5")
			{		
				deviceID = atoi(path.c_str());
				capture.open(deviceID);
			}
			else
			{
				capture.open(path);// "rtsp://admin:a1234567@91.228.236.8:50121/cam/realmonitor?channel=1&subtype=0");
			}
		}
		else
		{
			path = Open_File_Dialog();
			if (path.empty())
				return;
			capture.open(path);
		}
	
		// Выбираем и рисуем Регион
		Add_Mouse_Region aMR;
		
		roi = aMR.Start(path, siZe, x11, x22, y11, y22);//задаём регион
		//

		if (roi.empty())
			return;
		if (!capture.isOpened())
			MessageBox(0, L"Error when reading steam_avi", L"", MB_OK | MB_ICONERROR | MB_APPLMODAL);

		capture >> frame;

		resultWind.show();
		resultWind.Clear();
		resultWind.state = true;

		float w = (float)frame.cols / siZe;
		float h = (float)frame.rows / siZe;

		std::thread reader([h, w, this]()
			{
				Queue_Controller(w, h);
			});
		reader.detach();

		std::thread thrVid([this]()
			{
				Thread_Video();
			});
		thrVid.detach();
		
	}

	bool IsVisible_LPR()
	{
		return resultWind.isVisible();
	}

	void Close()
	{
		state = false;
		queueData.clear();
		arrNum.clear();
		resultWind.Close();
		//this->close();
	}

private:

	Symbol_Recog sR;
	View_Result resultWind;
	Ui::FaceClass ui;
	VideoCapture capture;
	Mat frame;
	string desckTop;
	string pathEXE;
	int x11, x22, y11, y22;
	bool onOff;
	int siZe;
	Rect roi;
	std::deque<std::pair<Mat, Mat>> queueData;
	//std::condition_variable reader_action;
	//mutex mutex_lock;
	map<std::string, QTime> arrNum;

	void Queue_Controller(float w, float h)
	{
		int start = 0;
		int empty_img = 0;
		sR.Start(pathEXE, 0.7f);
		std::vector < std::pair<string, float>> res;
		string caffeConfig_Proto = pathEXE + "models/LPR_Detect_6000.prototxt";
		string caffeWeight_Model = pathEXE + "models/LPR_Detect_6000.caffemodel";
		Net net = cv::dnn::readNetFromCaffe(caffeConfig_Proto, caffeWeight_Model);

		while (1)
		{
			if (onOff && queueData.empty() == 1)
			{
				resultWind.WriteToFile(desckTop);
				MessageBox(0, L"Detection end", L"", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
				return;
			}
			if (!queueData.empty())
			{
				Rect roi;
				pair<Mat, Mat> tempPair;

				tempPair = queueData.front();
				queueData.pop_front();

				roi = Detect_LPR(net, tempPair.second, tempPair.first, w, h);
			
				if (!roi.empty())
				{		
					Mat img = tempPair.second(roi).clone();
					std::pair<string, float> temp = (sR.Check_Symb(img));
					if(temp.first.length() == 8)
					      res.push_back(temp);
					empty_img = 0;
				}
				else
				{
					if (empty_img >= 8)// если идут n пустых кадров подряд значит машина проехала - выбираем самый лучший номер
					{
						if (!res.empty())
						{
							Find_The_Best_Num(res);// в отсортированном векторе берём первый номер он с наивысшим коэффициентом
							res.clear();
						}
					}
					empty_img++;
				}
			}
			else
			{
				continue;
			}
		}	
	}

	void Find_The_Best_Num(std::vector <std::pair<string, float>> res)
	{
		sort(res.begin(), res.end(), [](const std::pair<string, float>& a, const pair<string, float>& b) { return a.second > b.second; });
		
		string s = res[0].first;// в отсортированном векторе берём первый номер он с наивысшим коэффициентом
		if (s.length() == 8)// длинна номера
		{
			auto times = QTime::currentTime();
			int krug = 0;
			QTime prevTime;
			if (Checking_Number(s, times, krug, prevTime))
			{
				s = s + " (" + std::to_string((int)(res[0].second / .08)) + " %)";
				resultWind.RefreshTable(s, times, krug, prevTime);
			}
		}
	}

	bool Checking_Number(string str, QTime& times, int& krug, QTime& prevTime)//ещё одна проверка номера
	{
		if (std::isdigit(str[0]) || std::isdigit(str[1]) || !std::isdigit(str[2]) || !std::isdigit(str[5]) || std::isdigit(str[6]) || std::isdigit(str[7]))
			return false;

		int t = (times.hour() * 3600) + (times.minute() * 60) + times.second();

		map<string, QTime>::iterator it;	

		if (it = arrNum.find(str); it == arrNum.end())
		{
			arrNum.insert(make_pair(str, times));
			return true;
		}
		else if (int a = (it->second.hour() * 3600) + (it->second.minute() * 60) + it->second.second(); (t - a) > 60)
		{
			prevTime = it->second;
			it->second = times;
			krug = (t - a) / 60;
			return true;
		}

		return false;
	}

	Rect Detect_LPR(Net net, Mat frame1, Mat frame, float w, float h)
	{

		int frameHeight = frame.rows;
		int frameWidth = frame.cols;

		Mat inputBlob = blobFromImage(frame, 1. / 127.5, Size(), Scalar::all(127.5), true, false);

		net.setInput(inputBlob, "data");
		cv::Mat detection = net.forward("detection_out");

		cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>(0));

		for (int i = 0; i < detectionMat.rows; i++)
		{
			float confidence = detectionMat.at<float>(i, 2);

			if (confidence > 0.35)
			{
				int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * frameWidth);
				int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * frameHeight);
				int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * frameWidth);
				int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * frameHeight);
				//Rect t = Rect(Point(x1 * w, frame1.rows/2 + y1 * h), Point(x2 * w, frame1.rows / 2 + y2 * h));//for polovina image
				Rect t = Rect(Point((x11 + x1) * w, (y11 + y1) * h), Point((x11 + x2) * w, (y11 + y2) * h));

				if (0 <= t.x && 0 <= t.width && t.x + t.width <= frame1.cols && 0 <= t.y && 0 <= t.height && t.y + t.height <= frame1.rows)
					return t;
			}
			else
				continue;
		}

		return Rect(0,0,0,0);
	}

	string Open_File_Dialog()
	{
		OPENFILENAME ofn = { 0 };
		TCHAR szFile[260] = { 0 };

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = L"Image Files (*.mp4)\0*.mp4;\0;";
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
			if (ui.LPR_Button->text() == "Dis LPR")
			{
				ui.LPR_Button->setText("Video LPR");
				ui.LPR_Button->setStyleSheet("QPushButton {border-color: cyan;}");
			}
			MessageBox(0, L"ERROR file read", L"", MB_OK | MB_ICONERROR | MB_APPLMODAL);
		}

		return fileName;
	}

};

