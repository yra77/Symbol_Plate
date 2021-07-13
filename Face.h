#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN 
#pragma once
#include "ui_Face.h"
#include "LPR.h"
#include "Image_LPR.h"
#include "FaceDetectRecog.h"

#include "conio.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <thread>
#include <vector>
#include <filesystem>
#include <regex>

#include <QLabel>
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


class Face : public QMainWindow
{
	Q_OBJECT

private:
	string path_EXE;
	string desctopPath;

	//Face	
	QLabel* myLabel;
	Ui::FaceClass ui;
	FaceDetectRecog fdr;

	// Всё для авто номеров video
	LPR lpr;

// Всё для авто номеров image
	Image_LPR imgLPR;
	bool imageLPrbool = false;
	QString styleSheet;

public:
	int boolFaceCamera = 0;

	Face(string path_EXE, string desctop_Path, QWidget* parent = Q_NULLPTR) : path_EXE(path_EXE), QMainWindow(parent)
	{
		ui.setupUi(this);
		desctopPath = desctop_Path;
		styleSheet = "QPushButton{ background-color: gray; color: black; border-color: cyan;}"
			         "QPushButton:hover{ border: 2px solid white; background-color: red; color: white}"
			         "QPushButton:pressed{ border: 2px solid red; background-color: red; color: white}";
		
		connect(ui.startButton, SIGNAL(released()), this, SLOT(Face_Cam_Button_Click()));
		connect(ui.LPR_Button, SIGNAL(released()), this, SLOT(Start_LPR_Click()));
		connect(ui.img_LPR_Button, SIGNAL(released()), this, SLOT(Img_LPR_Button_Click()));
		connect(ui.Camera_LPR_Button, SIGNAL(released()), this, SLOT(Camera_LPR_Button_Click()));
		connect(ui.Close_Button, SIGNAL(released()), this, SLOT(Close_Button_Click()));
		
		ui.startButton->setStyleSheet(styleSheet);
		ui.img_LPR_Button->setStyleSheet(styleSheet);
		ui.LPR_Button->setStyleSheet(styleSheet);
		ui.Camera_LPR_Button->setStyleSheet(styleSheet);
		ui.Close_Button->setStyleSheet("QPushButton{ background-color: #9B001C; color: white; border-color: #9B001C;}"
			                           "QPushButton:hover{ border: 2px solid white; background-color: red; color: white}"
			                           "QPushButton:pressed{ border: 2px solid red; background-color: red; color: white}");
		
		ui.centralWidget->setStyleSheet("QWidget{border: 2px solid Cyan; background-color: gray}");
		ui.menuBar->hide();
		ui.mainToolBar->hide();
		ui.statusBar->hide();
		
		//setWindowFlags(Qt::FramelessWindowHint);// Отключение title bar
	}

private slots:

	void Face_Cam_Button_Click()
	{
		if (boolFaceCamera == 0)
		{
			fdr.PreStart_FaceRecog(path_EXE, desctopPath, ui);
			boolFaceCamera = 1;
		}
		if (!fdr.state && (ui.startButton->text() == "Start" || ui.startButton->text() == "Face Camera"))
		{
			auto s = Rtsp_Dialog(false);
			if (s != "-")
			{
				ui.startButton->setText("Pause");
				ui.startButton->setStyleSheet("QPushButton {border-color: red;}");
				fdr.state = true;
				fdr.Start(s);
			}
		}
		else
		{
			ui.startButton->setText("Start");
			ui.startButton->setStyleSheet("QPushButton {border-color: cyan;}");
			fdr.state = false;
		}
	}

	void Start_LPR_Click()
	{
		if (lpr.state == false && ui.LPR_Button->text() == "Video LPR")
		{
			auto s = Rtsp_Dialog(true);
			lpr.state = true;
			ui.LPR_Button->setText("Dis LPR");
			ui.LPR_Button->setStyleSheet("QPushButton {border-color: red;}");
			lpr.Start(false, " ", path_EXE, desctopPath, ui);
		}
		else
		{
			lpr.state = false;
			ui.LPR_Button->setText("Video LPR");
			ui.LPR_Button->setStyleSheet("QPushButton {border-color: cyan;}");
		}
	}

	void Camera_LPR_Button_Click()
	{
		if (lpr.state == false && ui.Camera_LPR_Button->text() == "Camera LPR")
		{
			auto s = Rtsp_Dialog(false);
			if (s != "-")
			{
				ui.Camera_LPR_Button->setText("Pause LPR");
				ui.Camera_LPR_Button->setStyleSheet("QPushButton {border-color: red;}");
				lpr.state = true;
			    lpr.Start(true, s, path_EXE, desctopPath, ui);
			}
		}
		else
		{
			lpr.state = false;
			ui.Camera_LPR_Button->setText("Camera LPR");
			ui.Camera_LPR_Button->setStyleSheet("QPushButton {border-color: cyan;}");
		}
	}

	void Img_LPR_Button_Click()
	{
		imageLPrbool = true;
		imgLPR.Start(path_EXE);
	}

	void Close_Button_Click()
	{
		if (imageLPrbool)
			 imgLPR.Close();
		if (lpr.IsVisible_LPR())
		{
			lpr.state = false;
			lpr.Close();
		}

		if (boolFaceCamera == 1)
		{
			fdr.state = false; 
			fdr.Close();
		}

		connect(ui.Close_Button, &QPushButton::clicked, this, &QWidget::close);
	}

	void closeEvent(QCloseEvent* ev)
	{
		Close_Button_Click();
		/*QWidget::closeEvent(ev);*/
	}

	string Rtsp_Dialog(bool isVideo)
	{	
		QTextCodec* codec = QTextCodec::codecForName("CP1251");
		QTextCodec::setCodecForLocale(codec);

		QDialog* dialog = new QDialog();
		QBoxLayout* layout = new QVBoxLayout();
		QLineEdit* lineEdit = new QLineEdit();
		QLineEdit* lineEdit_Krug = new QLineEdit();
		QDialogButtonBox* rtspButton = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		dialog->resize(500, 150);
		dialog->setWindowTitle(tr("Settings"));

		layout->addWidget(lineEdit);
		layout->addWidget(lineEdit_Krug);
		layout->addWidget(rtspButton);

		rtspButton->setCenterButtons(true);
		rtspButton->setStyleSheet("QPushButton{ width: 70; height: 20; background-color: gray; color: black; border: 2px solid cyan;}"
			                      "QPushButton:hover{ border: 2px solid white; background-color: red; color: white}"
			                      "QPushButton:pressed{ border: 2px solid red; background-color: red; color: white}");

		lineEdit->setPlaceholderText("rtsp://admin:a1234567@91.228.236.8:50121/cam/realmonitor?channel=1&subtype=0");
		lineEdit->setClearButtonEnabled(true);
		lineEdit->setFocusPolicy(Qt::ClickFocus);
		lineEdit->setStyleSheet("QLineEdit{ background-color: white; border: 2px solid black;}" 
			                    "QLineEdit:hover{ background-color: white; border: 2px solid cyan;}");
		
		lineEdit_Krug->setPlaceholderText(codec->toUnicode("Введите время круга в мин."));
		lineEdit_Krug->setClearButtonEnabled(true);
		lineEdit_Krug->setStyleSheet("QLineEdit{ background-color: white; border: 2px solid black;}"
			                         "QLineEdit:hover{ background-color: white; border: 2px solid cyan;}");
		lineEdit_Krug->setFixedSize(170, 20);

		if (isVideo)
		{
			MessageBox(0, L"Установите допустимое время круга в мин.", L"", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
			lineEdit->hide();
		}
		else
		{
			MessageBox(0, L"Установите номер веб usb камеры (0 или 1 или ...10) или адрес Rtsp\n\tУстановите допустимое время круга в мин.", L"", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
		}

		dialog->setStyleSheet("QWidget{ background-color: grey}");
		
		dialog->setLayout(layout);	
		dialog->show();
		
		QDialog::connect(rtspButton, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
		QDialog::connect(rtspButton, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

		QDialogButtonBox::StandardButton reply;

		if (dialog->exec() == QDialog::Accepted)
		{
			QString str;
			QString strKrug;

			strKrug = lineEdit_Krug->text();			
			str = lineEdit->text();

			if (isVideo)
			     str = "1";
			if(!strKrug.isEmpty())
				View_Result::time_Krug = atoi((strKrug).toStdString().c_str());

			if (!str.isEmpty())
			{
				if(dialog)
				    delete dialog;

				return str.toUtf8().constData();
			}
			else
			{
				if(!isVideo)
				    MessageBox(0, L"Error \nУстановите номер веб usb камеры (0 или 1 или ...10) или адрес Rtsp", L"", MB_OK | MB_ICONERROR | MB_APPLMODAL);
				else
					MessageBox(0, L"Error \nУстановите допустимое время круга в мин", L"", MB_OK | MB_ICONERROR | MB_APPLMODAL);

				dialog->close();
			}
		}

		if (dialog)
		     delete dialog;

		return "-";
	}

};
