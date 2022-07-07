#pragma once
#include <QtWidgets/QWidget>
#include <QPixmap>
#include <QFile>
#include <QDataStream>
#include "workWidgets.h"
#include "basicFunc.h"
#include "ui_mainWindow.h"
#pragma execution_character_set("utf-8")
class mainWindow : public QWidget
{
    Q_OBJECT

public:
    mainWindow(QWidget *parent = Q_NULLPTR);
    void test(void);
private:
    Ui::mainWindowClass ui_mainWindow;
    //QLabel* p_tag_list;
    void reUi(void);
    void mainMenuChanged(void);
};
