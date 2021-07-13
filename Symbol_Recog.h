#pragma once
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <array>
#include <mutex>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn_superres.hpp>

using namespace std;
using namespace cv;
using namespace cv::dnn;
using namespace dnn_superres;

class Symbol_Recog
{
public:
	
	Symbol_Recog() = default;

	void Start(string path, float set_Confid)
	{
		this->set_Confid = set_Confid;
		pathEXE = path;
		string caffeConfig_Proto = pathEXE + "models/Short_9324.prototxt";
		string caffeWeight_Model = pathEXE + "models/Short_2000.caffemodel";

		net = cv::dnn::readNetFromCaffe(caffeConfig_Proto, caffeWeight_Model);
		
	}

	std::pair<string, float> Check_Symb(Mat plate)
	{
		return Detect_LPR(plate);
	}

private:
	string pathEXE;
	float set_Confid;
	Net net;
	/*char CLASSES[39] = {
						 '0','0', '1', '2', '3', '4' , '5', '6' , '7', '8' , '9', 'A' , 'B', 'C' , 'D', 'E',
						 'F', 'G' , 'H', 'I' , 'J', 'K' , 'L', 'M' ,
						 'N', 'O' , 'P', 'Q' , 'R', 'S' , 'T', 'U' , 'V', 'W', 'X', 'Y', 'Z', 'П', 'У'
	                   };*/

	char CLASSES[23] = { 
		                 '0','0', '1', '2', '3', '4' , '5', '6' , '7', '8' , '9', 'A' , 'B', 'C', 'E',
						 'H', 'K' , 'M' , 'P', 'T', 'X', 'I', 'O' 
	                   };

	Mat SuperResize(Mat img_Original, int scale)
	{
		Mat img_new;
		DnnSuperResImpl sr;
		const std::string model = pathEXE + "models/Resize/FSRCNN_x4.pb";

		sr.readModel(model);
		sr.setModel("fsrcnn", scale);
		sr.upsample(img_Original, img_new);

		return img_new.clone();
	}

	std::pair<string, float> Detect_LPR(Mat plate)
	{
		    float conf = 0;
			std::pair<string, float> res;
			Mat frame;
			Mat frame3 = plate.clone();
			int siZe = 400;
			//float w = siZe / (float)frame3.cols;
			//float h = siZe / (float)frame3.rows;
			//if (frame3.cols < siZe || frame3.rows < siZe)
			//{
			//	//Mat tmp;
			////	tmp = SuperResize(frame3.clone(), 4);
			//	//resize(tmp, frame, Size((float)frame3.cols * w, (float)frame3.rows * h), InterpolationFlags::INTER_CUBIC);
			//}
			//else
			//{
			//	frame = frame3.clone();
			//}

			frame = plate.clone();

			int frameHeight = frame.rows;
			int frameWidth = frame.cols;

			Mat inputBlob = blobFromImage(frame, 1./127.5, Size(siZe, siZe), Scalar::all(127.5), true, false);//104, 117, 123    1./127.5
			
			net.setInput(inputBlob, "data");
			cv::Mat detection = net.forward("detection_out");
			
			cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>(0));
			vector <pair<int, char>> str;

			for (int i = 0; i < detectionMat.rows; i++)
			{
				float confidence = detectionMat.at<float>(i, 2);

				if (confidence > set_Confid)
				{
					int class_Index = (int)detectionMat.at<float>(i, 1);
					int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * frameWidth);
					int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * frameHeight);
					int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * frameWidth);
					int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * frameHeight);

					Rect t = Rect(Point((x1), (y1)), Point((x2), (y2)));//for region
					if (0 <= t.x && 0 <= t.width && t.x + t.width <= frame.cols && 0 <= t.y && 0 <= t.height && t.y + t.height <= frame.rows)
						if (class_Index != 0)
						{
							str.push_back(make_pair(x1, CLASSES[class_Index]));
							conf += confidence;
						}
				}
				else
					break;
			}

			string s{};
			if (!str.empty())// здесь переставляем символы по порядку слева на право 
			{
				for (size_t f = 0; f < str.size(); f++)
				{
					for (int h = f; h < str.size(); h++)
					{
						if (str[f].first > str[h].first)
						{
							int temp = str[f].first;
							char buf = str[f].second;
							str[f].first = str[h].first;
							str[f].second = str[h].second;
							str[h].first = temp;
							str[h].second = buf;
						}
					}
					s += str[f].second;
				}

				 res = std::make_pair(s, conf);
			}
			
			return res;
	}

};

