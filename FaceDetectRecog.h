#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN 
#pragma once
#include "Face.h"
#include "ui_Face.h"
//#include "View_Result.h"
#include "View_Face.h"

#include <string>
#include <fstream>
#include <thread>
#include <vector>
#include <filesystem>
#include <regex>
#include <map>
#include "conio.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <Commdlg.h>
#include <windows.h>

#include <QLabel>
#include <QMetaObject>
#include <QArgument>
#include <QtCore>
#include <QTextCursor>
#include <QtWidgets/QMainWindow>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/face.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/utility.hpp>

using namespace cv;
using namespace std;
using namespace cv::dnn;
using namespace cv::face;

class FaceDetectRecog
{

private:
	
	Ui::FaceClass ui;
	View_Face vF;

	Mat frame;
	VideoCapture cap;
	int kadr;
	bool state = false;
	string desctopPath;
	string path_EXE;	
	vector<std::string> data_To_Humans;
	map<std::string, QTime> data_local;
	int countName;
	Ptr<LBPHFaceRecognizer> model;

	void PreStart_FaceRecog(string pathEXE, string desctop_Path, Ui::FaceClass ui)
	{

		this->ui = ui;
		this->path_EXE = pathEXE;
		this->desctopPath = desctop_Path;

		if (!std::filesystem::exists(desctopPath + "Foto Base Recognize"))
		{
			//std::filesystem::create_directories(desctopPath + "Foto Base Recognize/Recognize Base");
			//std::filesystem::create_directories(desctopPath + "Foto Base Recognize/Foto Base");
			std::filesystem::create_directories(desctopPath + "Foto Base Recognize/Human Base");
			std::filesystem::create_directories(desctopPath + "Foto Base Recognize/Data_Base_Text/Base.txt");
		}

		//for recognizer
		vector<Mat> images;
		vector<int> labels;
		if (!Read_csv(images, labels))
		{
			MessageBox(0, L"Read file ERROR", L"", MB_OK | MB_ICONERROR | MB_APPLMODAL);
			exit(1);
		}

		if (images.size() <= 1)
		{
			MessageBox(0, L"Нет данных в файле для поиска !", L"", MB_OK | MB_ICONERROR | MB_APPLMODAL);
		}

		model = LBPHFaceRecognizer::create();
		model->train(images, labels);
		// View
		vF.Start(desctopPath, countName);
	}

	void Thread_Video()
	{
	
		string tensorflowConfigFile = path_EXE + "models/opencv_face_detector.pbtxt";
		string tensorflowWeightFile = path_EXE + "models/opencv_face_detector_uint8.pb";
		Net net = cv::dnn::readNetFromTensorflow(tensorflowWeightFile, tensorflowConfigFile);

		while (1)
		{
			cap.read(frame);

			if (!state || !vF.state || GetAsyncKeyState(VK_ESCAPE) || waitKey(30) >= 0 || frame.empty())
			{
				state = false;
				ui.startButton->setText("Start");
				ui.startButton->setStyleSheet("QPushButton {border-color: cyan;}");
				cap.release();
				break;
			}

			if (kadr == 3)
			{
				DetectFaces_Dnn(net, frame);
				kadr = 0;
			}
	
			// передача frame в основной поток для вывода на экран
			qRegisterMetaType<cv::Mat>("cv::Mat");
			QMetaObject::invokeMethod
			(
				&vF,
				"View_Video",
				Qt::QueuedConnection,
				Q_ARG(cv::Mat, frame)
			);
			///////			

			kadr++;
		}
	}

	void Start(string path)
	{
		countName = 0;
		kadr = 0;
		View_Result::time_Krug = 0;
		int deviceID;     // 0 = autodetect default API

		if (path == "0" || path == "1" || path == "2" || path == "3" || path == "4" || path == "5")
		{
			deviceID = atoi(path.c_str());
			cap.open(deviceID);
		}
		else
		{
			cap.open(path);// "rtsp://admin:a1234567@91.228.236.8:50121/cam/realmonitor?channel=1&subtype=0");
		}

		if (!cap.isOpened())
		{
			MessageBox(0, L"Camera not working", L"", MB_OK | MB_ICONERROR | MB_APPLMODAL);
			return;
		}

		cap >> frame;

		vF.show();
		vF.Clear();
		vF.state = true;

		std::thread th([this] ()
		{
			Thread_Video();
		});
		th.detach();

	}

	void DetectFaces_Dnn(Net& net, Mat& frameOpenCVDNN)
	{
		const Scalar meanVal(104.0, 177.0, 123.0);
		int frameHeight = frameOpenCVDNN.rows;
		int frameWidth = frameOpenCVDNN.cols;
		size_t inWidth = 0;
		size_t inHeight = 0;
		double inScaleFactor = 1.0;
		float confidenceThreshold = 0.9;// для изменения чувствительности

		Mat inputBlob = blobFromImage(frameOpenCVDNN, inScaleFactor, Size(inWidth, inHeight), meanVal, true, false);

		net.setInput(inputBlob, "data");
		cv::Mat detection = net.forward("detection_out");

		cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

		for (int i = 0; i < detectionMat.rows; i++)
		{
			float confidence = detectionMat.at<float>(i, 2);

			if (confidence > confidenceThreshold)
			{
				int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * frameWidth + 5);
				int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * frameHeight - 20);
				int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * frameWidth + 30);
				int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * frameHeight + 35);

				//прямоугольник вокруг лица
				//rectangle(frameOpenCVDNN, Point(x1, y1), Point(x2, y2), Scalar(0, 255, 0), 2, 4);				
				Rect t = Rect(Point(x1, y1), Point(x2, y2));
             if(!t.empty())
				if (0 <= t.x && 0 <= t.width && t.x + t.width <= frameOpenCVDNN.cols && 0 <= t.y && 0 <= t.height && t.y + t.height <= frameOpenCVDNN.rows)
				{
					Mat img_Check = frameOpenCVDNN(t);
					Mat grayscale;
					cvtColor(img_Check.clone(), grayscale, COLOR_BGR2GRAY);
					cv::resize(grayscale, grayscale, Size(100, 140), 0, 0, INTER_LINEAR_EXACT);

					if (i >= vF.img_Face.size())
					{
						vF.img_Face.push_back(img_Check);
					}

					std::string result_message = Face_Recognize(grayscale, std::ref(frameOpenCVDNN));
						auto times = QTime::currentTime();
						
						To_View(img_Check, result_message, times);
						//putText(frameOpenCVDNN, result_message, Point(x1 + 15, y1 - 2), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 2);
				}
			}
		}
	}

	void To_View(Mat& img_Check, string res, QTime times)
	{
		
		QTime prevTime;
		map<string, QTime>::iterator it;
		int krug = 0;
		int t = (times.hour() * 3600) + (times.minute() * 60) + times.second();

		if (it = data_local.find(res); it == data_local.end())
		{
			data_local.insert(make_pair(res, times));
			vF.img_Face.push_back(img_Check);
			vF.RefreshTable(res, times, krug, prevTime);
		}
		else if (int a = (it->second.hour() * 3600) + (it->second.minute() * 60) + it->second.second(); krug = (t - a) > 60)
		{
			prevTime = it->second;
			it->second = times;
			krug /= 60;
			vF.img_Face.push_back(img_Check);
			vF.RefreshTable(res, times, krug, prevTime);
		}
	}

	std::string Face_Recognize(Mat inputImage, Mat& frameOpenCVDNN)
	{

		int predictedLabel = -1;
		double predicted_confidence = 0.0;

		model->predict(inputImage, predictedLabel, predicted_confidence);

		string result_message;
		if (predicted_confidence >= 85.0)
			 result_message = data_To_Humans[predictedLabel];
		else
			result_message = "undefined";

		return result_message;
	}

	bool Read_csv(vector<Mat>& images, vector<int>& labels)
	{

		string regExprStr("([0-9]+)\\.png");
		regex rgx(regExprStr);

		std::ifstream fileRead;
		fileRead.open(desctopPath + "Foto Base Recognize/Data_Base_Text/Base.txt");

		string folder(desctopPath + "Foto Base Recognize/Human Base/*.png");
		vector<string> filenames;

		cv::glob(folder, filenames, false);

		if (filenames.empty() || !fileRead.is_open())
		{
			return false;
		}

		for (int i = 0; i < filenames.size(); i++)
		{
			images.push_back(imread(filenames[i], IMREAD_GRAYSCALE));
			smatch smatch;

			if (regex_search(filenames[i], smatch, rgx))
			{
				string ss = smatch[0];

				//if (ss[0] == '0')
				//	ss = ss[1];
				//else
				//	ss = ss[0] + ss[1];

				labels.push_back(stoi(ss));
			}
		}

		while (!fileRead.eof())
		{
			data_To_Humans.resize(data_To_Humans.size() + 1);
			fileRead >> data_To_Humans[countName];
			countName++;
		}

		fileRead.close();

		return true;
	}

	void Close()
	{
		state = false;
		ui.startButton->setText("Start");
		ui.startButton->setStyleSheet("QPushButton {border-color: cyan;}");
		cap.release();
		vF.close();
	}

	friend class Face;

};

