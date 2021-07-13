#pragma once
#include <QDialog>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QTextCursor>
#include <QtWidgets/QMainWindow>
#include <QBoxLayout>
#include <QTableWidget>
#include <QTranslator>
#include <QTextCodec>
#include <QTextStream>
#include <QTime>
#include <QDate>

#include <windows.h>
#include <Commdlg.h>

#include <vector>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

class View_Image : public QDialog
{
	Q_OBJECT

public:

	View_Image() : QDialog()
	{
		codec = QTextCodec::codecForName("CP1251");
		QTextCodec::setCodecForLocale(codec);

		layout = new QGridLayout(this);
		mylabel = new QLabel();
		textlabel = new QLabel();

		this->resize(600, 400);
		this->setWindowTitle("LPR Image");
	}

	void View(Mat frame)
	{
		Mat img;
		cv::resize(frame, img, Size(580, 320), 0, 0, INTER_CUBIC);
		cv::cvtColor(img, img, COLOR_BGR2RGB);

		QImage qimg = QImage((uchar*)img.data, img.cols, img.rows, img.step, QImage::Format_RGB888);
		mylabel->setPixmap(QPixmap::fromImage(qimg));
		//  mylabel->QWidget::setGeometry(QRect(400, 0, 380, 400));
		layout->addWidget(mylabel);
		setLayout(layout);
	}

	void View_Text(string str)
	{
		QFont qFont = textlabel->font();
		qFont.setBold(false);
		qFont.setPointSize(12);
		textlabel->setFont(qFont);
		setStyleSheet("QLabel{ background-color : rgba( 160, 160, 160, 255); border-radius : 7px;  }");
		QPalette pa;
		pa.setColor(QPalette::WindowText, Qt::red);
		textlabel->setPalette(pa);
		textlabel->setMargin(40);
		textlabel->setText(codec->toUnicode(str.c_str()));
		layout->addWidget(textlabel);
	}

	virtual ~View_Image()
	{
		delete layout;
		delete mylabel;
		delete textlabel;
	}
private:
	QGridLayout* layout;
	QTextCodec* codec;
	QLabel* mylabel;
	QLabel* textlabel;

};

