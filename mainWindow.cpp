#include "mainWindow.h"

mainWindow::mainWindow(QWidget *parent)
    : QWidget(parent)
{
    ui_mainWindow.setupUi(this);
    connect(ui_mainWindow.btn_mainMenu_load, &QPushButton::clicked, this, &mainWindow::mainMenuChanged);
    connect(ui_mainWindow.btn_mainMenu_list, &QPushButton::clicked, this, &mainWindow::mainMenuChanged);
    connect(ui_mainWindow.btn_mainMenu_workList, &QPushButton::clicked, this, &mainWindow::mainMenuChanged);
    connect(ui_mainWindow.btn_mainMenu_settings, &QPushButton::clicked, this, &mainWindow::mainMenuChanged);

    p_tag_list = new QLabel(ui_mainWindow.btn_mainMenu_list);
    mainWindow::reUi();
    //test();
    //test();
}
void mainWindow::reUi(void)
{
    ui_mainWindow.btn_mainMenu_load->setChecked(true);//设置默认菜单
    mainWindow::mainMenuChanged();//加载工作区
}
void mainWindow::mainMenuChanged(void)
{
    workWidget_load* loadWidget = ui_mainWindow.widget_workWidget->findChild<workWidget_load*>();
    workWidget_list* listWidget =  ui_mainWindow.widget_workWidget->findChild<workWidget_list*>();
    workWidget_workList* workListWidget = ui_mainWindow.widget_workWidget->findChild<workWidget_workList*>();
    workWidget_settings* settingsWidget = ui_mainWindow.widget_workWidget->findChild<workWidget_settings*>();
    if (loadWidget != NULL)
        loadWidget->hide();
    if (listWidget != NULL)
        listWidget->hide();
    if (workListWidget != NULL)
        workListWidget->hide();
    if (settingsWidget != NULL)
        settingsWidget->hide();
    if (ui_mainWindow.widget_workWidget->children().count() > 4)
        QMessageBox::critical(this,"LUD-意外错误",
            "一个意外的错误发生在mainWindow->mainMenuChanged()中, 子对象值不可能大于4\n建议您重启程序, 如果仍无法解决, 请反馈给开发者");
    if (ui_mainWindow.btn_mainMenu_load->isChecked() == true)
    {
        if (loadWidget == NULL)
        {
            loadWidget = new workWidget_load;
        }
        loadWidget->setParent(ui_mainWindow.widget_workWidget);
        loadWidget->show();
        return;
    }
    if (ui_mainWindow.btn_mainMenu_list->isChecked() == true)
    {
        if (listWidget == NULL)
        {
            listWidget = new workWidget_list;
        }
        listWidget->setParent(ui_mainWindow.widget_workWidget);
        listWidget->show();
        return;
    }
    if (ui_mainWindow.btn_mainMenu_workList->isChecked() == true)
    {
        if (workListWidget == NULL)
        {
            workListWidget = new workWidget_workList;
        }
        workListWidget->setParent(ui_mainWindow.widget_workWidget);
        workListWidget->show();
        return;
    }
    if (ui_mainWindow.btn_mainMenu_settings->isChecked() == true)
    {
        if (settingsWidget == NULL)
        {
            settingsWidget = new workWidget_settings;
        }
        settingsWidget->setParent(ui_mainWindow.widget_workWidget);
        settingsWidget->show();
        return;
    }
}
void mainWindow::test(void)
{
    QByteArray data;
    data = qNetwork_getHttpData("GET", "https://p1.music.126.net/Mw7z4vn0EMrWlitsiYgzPQ==/109951163606870393.jpg", false);
    QFile file("E:/Programming/Workspace/repos/LUD/LUD/test.png");
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
    QPixmap pixmap;
    pixmap.loadFromData(data);
    pixmap = pixmap.scaled(QSize(256, 256), Qt::KeepAspectRatio);
    //ui.testLable->setPixmap(pixmap);
}