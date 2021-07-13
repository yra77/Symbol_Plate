#pragma once
#include "Write_To_File.h"

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

class View_Result : public QDialog
{
	Q_OBJECT

 public slots:
    void View_Video(cv::Mat frame)
    {
        Mat img;
        cv::resize(frame, img, Size(580, 290), 0, 0, INTER_CUBIC);
        cv::cvtColor(img, img, COLOR_BGR2RGB);

        QImage qimg = QImage((uchar*)img.data, img.cols, img.rows, img.step, QImage::Format_RGB888);
        mylabel->setPixmap(QPixmap::fromImage(qimg));
        //  mylabel->QWidget::setGeometry(QRect(400, 0, 380, 400));
        hlayout->addWidget(mylabel);
    }

public:
    bool state = true;
    static inline int time_Krug = 0;

	View_Result() : QDialog()
	{    
        id = 0;
        codec = QTextCodec::codecForName("CP1251");
        QTextCodec::setCodecForLocale(codec);
        
		layout = new QVBoxLayout(this);
        hlayout = new QHBoxLayout();
        tableWidget = new QTableWidget();
        mylabel = new QLabel();

        QWidget::setStyleSheet("QWidget{background-color: gray}");
        
        this->resize(600, 600);
        this->setWindowTitle("Detect LPR");
        layout->addLayout(hlayout);
        Create_TableWidget();

	}

    virtual void RefreshTable(std::string num, QTime time_Now, int sizes, QTime prevTime)
    {
        QTableWidgetItem* item = nullptr;
        QFont font("Times", 9, QFont::Bold);

        QString str = QString::fromUtf8(num.c_str());
        QString str1 = QString::fromUtf8(std::to_string(sizes).c_str());

        this->tableWidget->setRowCount(this->tableWidget->rowCount());
       // int n = tableWidget->rowCount();
     
            tableWidget->insertRow(0);
           
            tableWidget->setItem(0, 0, new QTableWidgetItem(to_string(id).c_str()));
            tableWidget->setItem(0, 1, item = new QTableWidgetItem((str)));
            item->setTextAlignment(Qt::AlignCenter);
            
            tableWidget->setItem(0, 2, item = new QTableWidgetItem(prevTime.toString()));
            item->setTextAlignment(Qt::AlignCenter);

            tableWidget->setItem(0, 3, item = new QTableWidgetItem(time_Now.toString()));

            // Если круг больше чем time_Krug , то выделяем красным
            if (sizes > time_Krug)
            {
                item->setFont(font);
                item->setTextColor(QColor(Qt::red));
            }
            item->setTextAlignment(Qt::AlignCenter);

            tableWidget->setItem(0, 4, item = new QTableWidgetItem(str1));
            item->setTextAlignment(Qt::AlignCenter);
            tableWidget->setRowHeight(0, 20);
            id++;

    }
    
    void WriteToFile(string desckTop_Path)
    {
        std::string str;

        for (int row = tableWidget->rowCount() - 1; row >= 0; row--)
        {
            for (int col = 0; col < tableWidget->columnCount(); col++)
            {
                str += (tableWidget->item(row, col)->text() + ";").toStdString();
            }
            str += '\n';
        }

        Write_To_File toFile;
        toFile.To_File(str, desckTop_Path);
    }

    void Clear()
    {
        for (int i = 0; i < this->tableWidget->rowCount(); ++i)
                this->tableWidget->removeRow(i);
                this->tableWidget->setRowCount(0);
      // this->tableWidget->clear();
    }

    virtual ~View_Result()
    {
        //  this->tableWidget->close();
      /*  delete hlayout;
        delete layout;
        delete tableWidget;
        delete mylabel;*/
    }

    virtual void Close()
    {
        state = false;
        this->QWidget::close();
    }

protected:
    QVBoxLayout* layout;
    QTableWidget* tableWidget;
    QTextCodec* codec;
    QLabel* mylabel;
    QHBoxLayout* hlayout;
    int id;

    void closeEvent(QCloseEvent* ev)
    {
        state = false;
       // QWidget::closeEvent(ev);
       // ev->accept();
    }

private:

    void Create_TableWidget()
    {
       // tableWidget->QWidget::setGeometry(QRect(0, 0, 400, 500));

        /*QFont font("Times", 28, QFont::Bold);
        tableWidget->setFont(font);*/
        tableWidget->setColumnCount(5);
        tableWidget->setColumnWidth(0, 20);
        tableWidget->setColumnWidth(1, 150);
        tableWidget->setColumnWidth(2, 140);
        tableWidget->setColumnWidth(3, 140);
        tableWidget->setColumnWidth(4, 110);
        tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(codec->toUnicode(("ID"))));
        tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(codec->toUnicode("Машина")));
        tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(codec->toUnicode("Время prev")));
        tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(codec->toUnicode("Время now")));
        tableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem(codec->toUnicode("Врямя круга")));
        tableWidget->setShowGrid(true);
        tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
         tableWidget->setColumnHidden(0, true);

         tableWidget->setStyleSheet("QTableWidget {background-color: lightgray;}" "QHeaderView::section { background-color: gray; color: black;}");
         
        layout->addWidget(tableWidget);
        setLayout(layout);
    }

};

