#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN 
#pragma once
//#include "FaceDetectRecog.h"
#include "View_Result.h"

class View_Face : public View_Result
{

public:
    vector<Mat> img_Face;

    View_Face() : View_Result() {};

    void Start(string desctopPath, int countName)
    {
        this->desctopPath = desctopPath;
        countImg = 0;
        this->countName = countName;
        this->resize(1000, 600);
        this->setWindowTitle("Detect Face");

        lineEdit = new QLineEdit();
        faceLabel = new QLabel();
        vbox = new  QVBoxLayout();
        hbox = new  QHBoxLayout();
        vbox2 = new  QVBoxLayout();
        button = new QPushButton("Save");
        button1 = new QPushButton("Prev");
        button2 = new QPushButton("Next");

        this->connect(button, &QPushButton::clicked, this, &View_Face::Save_Button_Click);
        this->connect(button1, &QPushButton::clicked, this, &View_Face::Prev_Button_Click);
        this->connect(button2, &QPushButton::clicked, this, &View_Face::Next_Button_Click);

        this->Create_BaseFaces();
        this->Create_TableWidget();
    }

    void RefreshTable(std::string name, QTime time_Now, int sizes, QTime time_prev) override
    {

        QTableWidgetItem* item = nullptr;
        QFont font("Times", 9, QFont::Bold);

       // if (this->tableWidget->rowCount() == 0)
            ViewFace();
        QString str = QString::fromUtf8(name.c_str());
        QString str1 = QString::fromUtf8(std::to_string(time_Krug).c_str());
        this->tableWidget->setRowCount(this->tableWidget->rowCount());

        tableWidget->insertRow(0);

        tableWidget->setItem(0, 0, new QTableWidgetItem(to_string(id).c_str()));
        tableWidget->setItem(0, 1, item = new QTableWidgetItem((str)));
        item->setTextAlignment(Qt::AlignCenter);

        tableWidget->setItem(0, 2, item = new QTableWidgetItem(time_prev.toString()));
        item->setTextAlignment(Qt::AlignCenter);

        tableWidget->setItem(0, 3, item = new QTableWidgetItem(time_Now.toString()));

        // Если круг меньше чем time_Krug, то выделяем красным
        if (sizes < time_Krug)
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

    void Close() override
    {
        state = false;
        this->QWidget::close();
    }

    virtual ~View_Face()
    {
        /*delete layout;
        delete tableWidget;
        delete mylabel;
        delete lineEdit;
        delete vbox;
        delete vbox2;
        delete hbox;
        delete faceLabel;
        delete button;
        delete button1;
        delete button2;*/
    }

private:
    QVBoxLayout* vbox;
    QVBoxLayout* vbox2;
    QHBoxLayout* hbox;

    QLineEdit* lineEdit;
    QLabel* faceLabel;
    QImage qimg;
    QPushButton* button;
    QPushButton* button1;
    QPushButton* button2;

    int countImg;
    string desctopPath;
    int countName;

    void Create_BaseFaces()
    {
       
        hlayout->addLayout(vbox);
        hlayout->addLayout(vbox2);

        vbox->addWidget(faceLabel);
        vbox->addLayout(hbox);

        hbox->addWidget(lineEdit);
        hbox->addWidget(button);

        vbox2->addWidget(button1);
        vbox2->addWidget(button2);

        button->setMaximumWidth(70);
        button1->setMaximumWidth(80);
        button2->setMaximumWidth(80);

        QString styleSheet = "QPushButton{ background-color: gray; color: black; border: 2px solid cyan;}"
                             "QPushButton:hover{ border: 2px solid red; background-color: red; color: white}" 
                             "QPushButton:pressed{ border: 2px solid red; background-color: red; color: white}";
        button->setStyleSheet(styleSheet);
        button1->setStyleSheet(styleSheet);
        button2->setStyleSheet(styleSheet);

        faceLabel->setMinimumWidth(280);

        lineEdit->setMaximumWidth(100);
        lineEdit->setPlaceholderText("Input the name");
        lineEdit->setClearButtonEnabled(true);
        lineEdit->setFocusPolicy(Qt::ClickFocus);
        lineEdit->setStyleSheet("QLineEdit {background-color: white; border: 2px solid black}" "QLineEdit:hover {background-color: lightgray; border: 2px solid cyan}");
    }

    void ViewFace()
    {
        Mat img_QT;
        cv::cvtColor(img_Face[countImg].clone(), img_QT, COLOR_BGR2RGB);
        cv::resize(img_QT, img_QT, Size(275, 260), 0, 0, INTER_LINEAR_EXACT);
        qimg = QImage((uchar*)img_QT.data, img_QT.cols, img_QT.rows, img_QT.step, QImage::Format_RGB888);
        faceLabel->setPixmap(QPixmap::fromImage(qimg));
    }

    void Create_TableWidget()
    {

        tableWidget->setColumnCount(5);
        tableWidget->setColumnWidth(0, 20);
        tableWidget->setColumnWidth(1, 150);
        tableWidget->setColumnWidth(2, 140);
        tableWidget->setColumnWidth(3, 140);
        tableWidget->setColumnWidth(4, 110);
        tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(codec->toUnicode(("ID"))));
        tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(codec->toUnicode("ФИО")));
        tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(codec->toUnicode("Время prev")));
        tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(codec->toUnicode("Время now")));
        tableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem(codec->toUnicode("Врямя круга")));
        tableWidget->setShowGrid(true);
        tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        // tableWidget->setColumnHidden(0, true);
        layout->addWidget(tableWidget);
        setLayout(layout);
    }

    void Save_Button_Click()
    {
       string nameStr = lineEdit->text().toStdString();
        wofstream fileWrite;
        fileWrite.open(desctopPath + "Foto Base Recognize/Data_Base_Text/Base.txt", std::ios_base::app);
        string rassh = ".png";
        string num0 = to_string(countName - 1);

        if (fileWrite.is_open())
        {
            string probel = " ";
            string n = "\n";
            string buf = nameStr + n;
            fileWrite << buf.c_str();

            fileWrite.close();

            string filename = desctopPath + "Foto Base Recognize/Human Base/" + num0 + rassh;
            Mat greyImg;
            cvtColor(img_Face[countImg].clone(), greyImg, COLOR_BGR2GRAY);
            imwrite(filename, greyImg);
            countName++;
        }
        else
        {
            MessageBox(0, L"Write file ERROR", L"", MB_OK | MB_ICONERROR | MB_APPLMODAL);
        }
      
       lineEdit->clear();
    }

    void Prev_Button_Click()
    {
        if (img_Face.size() > 0 && countImg > 0)
        {
            countImg--;
            ViewFace();
        }
    }

    void Next_Button_Click()
    {
        if (img_Face.size() > 0 && countImg < img_Face.size() - 1)
        {
            countImg++;
            ViewFace();
        }
    }

};

