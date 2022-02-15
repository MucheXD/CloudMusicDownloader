#include "workWidgets.h"
//###############################加载器工作组件###############################
workWidget_load::workWidget_load(QWidget* parent)
    : QWidget(parent)
{
    ui_wLoad.setupUi(this);
    //connect(ui.spinBox_MaxRange, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DRN::ChangeMaxRange_SpinBox);
    connect(ui_wLoad.loadMode, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &workWidget_load::searchModeChanged);
    connect(ui_wLoad.btn_search, &QPushButton::clicked, this, &workWidget_load::getSearchInfo);
    connect(ui_wLoad.searchText, &QLineEdit::textChanged, this, &workWidget_load::searchTextChanged);

    p_searchResultTable_layout = new QVBoxLayout;
    p_searchResultTable_widget = new QWidget(this);
    workWidget_load::reUi();
}
void workWidget_load::reUi(void)
{
    searchModeChanged(0);//设置默认模式
    p_searchResultTable_widget->setFixedWidth(600);
    p_searchResultTable_widget->setObjectName("searchResultTable_widget");
    p_searchResultTable_widget->setLayout(p_searchResultTable_layout);
    ui_wLoad.searchResultTable->setWidget(p_searchResultTable_widget);

}
void workWidget_load::searchModeChanged(int mode)
{
    if (mode < 0)
        QMessageBox::critical(this, "LUD-意外错误",
            "一个意外的错误发生在workWidgets->load->loadModeChanged()中, mode不可能小于0\n建议您重启程序, 如果仍无法解决, 请反馈给开发者");
    if (ui_wLoad.loadMode->currentIndex() != mode)//如果传入的mode值与现行不一致, 则以传入为主, 可能是其他函数的调用
        ui_wLoad.loadMode->setCurrentIndex(mode);
    //UI 如果更改了UI排序方式, 则此处需要一并修改
    if (mode == 0)//搜索单曲
    {
        ui_wLoad.searchText->setPlaceholderText("输入要搜索的单曲名称");
    }
    if (mode == 1)//搜索专辑
    {
        ui_wLoad.searchText->setPlaceholderText("输入要搜索的专辑名称");
    }
    if (mode == 2)//搜索歌单
    {
        ui_wLoad.searchText->setPlaceholderText("输入要搜索的歌单名称");
    }
    if (mode == 3)//本地缓存
    {
        ui_wLoad.searchText->setPlaceholderText("输入自定义的本地缓存文件夹位置");
    }
    ui_wLoad.btn_search->setText("搜索");
    clearSearchResult();
}
void workWidget_load::searchTextChanged(void)
{
    ui_wLoad.btn_search->setText("搜索");
    clearSearchResult();
}
void workWidget_load::getSearchInfo(void)
{
    ui_wLoad.btn_search->setEnabled(false);
    short mode = -1;
    if (ui_wLoad.loadMode->currentIndex() == 0)
        mode = 1;
    else if (ui_wLoad.loadMode->currentIndex() == 1)
        mode = 10;
    else if (ui_wLoad.loadMode->currentIndex() == 2)
        mode = 1000;
    else if (ui_wLoad.loadMode->currentIndex() == 3)
        mode = 0;
    else
        QMessageBox::critical(this, "LUD-意外错误",
            "一个意外的错误发生在workWidgets->load->getSearchInfo()中, 没有匹配的模式\n建议您重启程序, 如果仍无法解决, 请反馈给开发者");
    if (mode == 1000 || mode == 1)//DEBUG 此处限制了未适配的搜索类型
    {
        bool isAdd = false;
        short offset = 0;
        if (ui_wLoad.btn_search->text() == "更多结果")
        {
            offset = resultCount;
            isAdd = true;
        }
        const QString searchResultText = qNetwork_getHttpText("GET", QString("http://music.163.com/api/search/get/web?s=%1&type=%2&offset=%3&total=true&limit=10")
            .arg(ui_wLoad.searchText->text().toUtf8().toPercentEncoding())
            .arg(mode)
            .arg(offset), false);
        if (mode == 1)
            printSearchResult(1, searchInfoParser_Songs(searchResultText), isAdd);
        if (mode == 1000)
            printSearchResult(1000, searchInfoParser_PlayList(searchResultText), isAdd);
        ui_wLoad.btn_search->setText("更多结果");
    }
    else if (mode == 0)
    {
        if (ui_wLoad.searchText->text() == "")
            ui_wLoad.searchText->setText(QDir::homePath() + "/AppData/Local/Netease/CloudMusic/Cache/Cache");
        printSearchResult(0);
    }
    else
    {
        QMessageBox::critical(this, "LUD-意外错误",
            "一个意外的错误发生在workWidgets->load->getSearchInfo()中, 当前mode值不合法\n建议您重启程序, 如果仍无法解决, 请反馈给开发者");
        ui_wLoad.btn_search->setEnabled(true);
        return;
    }
    ui_wLoad.btn_search->setEnabled(true);
}
void workWidget_load::clearSearchResult(void)
{
    while (p_searchResultTable_layout->count() != 0)//这里不需要计数器 因为count会随child的减少而减少
    {
        searchResultW* nowDelItem = (searchResultW*)p_searchResultTable_layout->layout()->itemAt(0)->widget();//获取第一个childWidget
        delete nowDelItem;//删除该child以释放内存
    }
    resultCount = 0;//重置计数器
    p_searchResultTable_widget->setFixedHeight(0);//将列表长度置零(否则会有滚动条)
}
void workWidget_load::printSearchResult(int mode,std::vector<SONGINFO> sr_songInfo, bool isAdd)
{
    const int scrollBarPosHistory = ui_wLoad.searchResultTable->verticalScrollBar()->value();//记录当前进度条位置, 这样在增加内容时用户不会被打断
    if (isAdd == false)
        clearSearchResult();
    if (mode == 1)//单曲搜索
    {
        short nowPrtItem = 0;
        while (nowPrtItem < sr_songInfo.size())
        {
            searchResultW* widget = new searchResultW;
            widget->ui_searchResultW.text_title->setText(sr_songInfo.at(nowPrtItem).name);
            widget->ui_searchResultW.text_title->adjustSize();
            widget->ui_searchResultW.text_subTitle->setGeometry(widget->ui_searchResultW.text_title->x() + widget->ui_searchResultW.text_title->width(),
                widget->ui_searchResultW.text_subTitle->y(),
                widget->ui_searchResultW.text_subTitle->width(),
                widget->ui_searchResultW.text_subTitle->height());
            TIME songDuration = qTime_formatFromMs(sr_songInfo.at(nowPrtItem).duration.toInt(), 2);
            widget->ui_searchResultW.text_subTitle->setText(QString("%1'%2''").arg(songDuration.m).arg(songDuration.s));
            widget->ui_searchResultW.text_creator->setText(QString("%1").arg(qDID_artistsStrIn_songInfo(sr_songInfo.at(nowPrtItem).artists, 36)));
            widget->ui_searchResultW.text_creator->adjustSize();
            if (widget->ui_searchResultW.text_creator->width() > 350)
                widget->ui_searchResultW.text_creator->setFixedWidth(350);
            widget->ui_searchResultW.text_data->setGeometry(widget->ui_searchResultW.text_creator->x() + widget->ui_searchResultW.text_creator->width() + 10,
                widget->ui_searchResultW.text_data->y(),
                widget->ui_searchResultW.text_data->width(),
                widget->ui_searchResultW.text_data->height());
            if (sr_songInfo.at(nowPrtItem).mvid != "0")
                widget->ui_searchResultW.text_data->setText("有MV");
            else
                widget->ui_searchResultW.text_data->setText("");
            widget->ui_searchResultW.text_data->adjustSize();
            widget->ui_searchResultW.text_ID->setText(QString("#%1").arg(sr_songInfo.at(nowPrtItem).id));
            widget->type = mode;
            widget->songInfo = sr_songInfo.at(nowPrtItem);
            connect(widget->ui_searchResultW.btn_choose, &QPushButton::clicked, this, &workWidget_load::searchResultChoosed);
            widget->setAttribute(Qt::WA_DeleteOnClose);
            p_searchResultTable_layout->addWidget(widget);
            nowPrtItem += 1;
        }
        resultCount += sr_songInfo.size();
        p_searchResultTable_widget->setFixedHeight(resultCount * 60 + 10);
        if (isAdd == true)
            ui_wLoad.searchResultTable->verticalScrollBar()->setValue(scrollBarPosHistory + 1);//如果内容增加, 恢复用户之前浏览的位置
    }
    else
    {
        QMessageBox::critical(this, "LUD-意外错误",
            "一个意外的错误发生在workWidgets->load->printSearchResult()中, 指定的模式不被当前重载接受\n建议您重启程序, 如果仍无法解决, 请反馈给开发者");
    }
}
void workWidget_load::printSearchResult(int mode, std::vector<PLAYLISTINFO> sr_playList, bool isAdd)
{
    const int scrollBarPosHistory = ui_wLoad.searchResultTable->verticalScrollBar()->value();//记录当前进度条位置, 这样在增加内容时用户不会被打断
    if (isAdd == false)
        clearSearchResult();
    if (mode == 1000)//歌单模式
    {
        short nowPrtItem = 0;
        while (nowPrtItem < sr_playList.size())
        {
            searchResultW* widget = new searchResultW;
            widget->ui_searchResultW.text_title->setText(sr_playList.at(nowPrtItem).name);
            widget->ui_searchResultW.text_title->adjustSize();
            widget->ui_searchResultW.text_subTitle->setGeometry(widget->ui_searchResultW.text_title->x()+ widget->ui_searchResultW.text_title->width()+5,
                widget->ui_searchResultW.text_subTitle->y(),
                widget->ui_searchResultW.text_subTitle->width(),
                widget->ui_searchResultW.text_subTitle->height());
            widget->ui_searchResultW.text_subTitle->setText(QString("%1 首").arg(sr_playList.at(nowPrtItem).trackCount));
            widget->ui_searchResultW.text_creator->setText(QString("By %1").arg(sr_playList.at(nowPrtItem).creatorInfo.name));
            widget->ui_searchResultW.text_creator->adjustSize();
            if (widget->ui_searchResultW.text_creator->width() > 200)
                widget->ui_searchResultW.text_creator->setFixedWidth(200);
            widget->ui_searchResultW.text_data->setGeometry(widget->ui_searchResultW.text_creator->x() + widget->ui_searchResultW.text_creator->width() + 10,
                widget->ui_searchResultW.text_data->y(),
                widget->ui_searchResultW.text_data->width(),
                widget->ui_searchResultW.text_data->height());
            widget->ui_searchResultW.text_data->setText(QString("播放 %1 | 订阅 %2").arg(sr_playList.at(nowPrtItem).playCount).arg(sr_playList.at(nowPrtItem).bookCount));
            widget->ui_searchResultW.text_data->adjustSize();
            widget->ui_searchResultW.text_ID->setText(QString("#%1").arg(sr_playList.at(nowPrtItem).id));
            widget->type = mode;
            widget->playListInfo = sr_playList.at(nowPrtItem);
            connect(widget->ui_searchResultW.btn_choose, &QPushButton::clicked, this, &workWidget_load::searchResultChoosed);
            widget->setAttribute(Qt::WA_DeleteOnClose);
            p_searchResultTable_layout->addWidget(widget);
            nowPrtItem += 1;
        }
        resultCount += sr_playList.size();
        p_searchResultTable_widget->setFixedHeight(resultCount * 60 + 10);
        if (isAdd == true)
            ui_wLoad.searchResultTable->verticalScrollBar()->setValue(scrollBarPosHistory + 1);//如果内容增加, 恢复用户之前浏览的位置
    }
    else
    {
        QMessageBox::critical(this, "LUD-意外错误",
            "一个意外的错误发生在workWidgets->load->printSearchResult()中, 指定的模式不被当前重载接受\n建议您重启程序, 如果仍无法解决, 请反馈给开发者");
    }
}
void workWidget_load::printSearchResult(int mode)
{
    if (mode != 0)//搜索缓存文件
    {
        QMessageBox::critical(this, "LUD-意外错误",
            "一个意外的错误发生在workWidgets->load->printSearchResult()中, 指定的模式不被当前重载接受\n建议您重启程序, 如果仍无法解决, 请反馈给开发者");
    }
    
    const QStringList ucFileFilter = {"*.uc", "*.UC", "*.uc!", "*.UC!"};
    QDir searchDir;
    searchDir.setPath(ui_wLoad.searchText->text());
    int ucFileCount = searchDir.entryList(ucFileFilter, QDir::Files | QDir::Readable, QDir::Name).count();

    searchResultW* widget = new searchResultW;
    widget->ui_searchResultW.text_title->setText("本地网易云音乐缓存文件");
    widget->ui_searchResultW.text_title->adjustSize();
    widget->ui_searchResultW.text_subTitle->setGeometry(widget->ui_searchResultW.text_title->x() + widget->ui_searchResultW.text_title->width() + 5,
        widget->ui_searchResultW.text_subTitle->y(),
        widget->ui_searchResultW.text_subTitle->width(),
        widget->ui_searchResultW.text_subTitle->height());
    widget->ui_searchResultW.text_subTitle->setText(QString("%1 首").arg(ucFileCount));
    widget->ui_searchResultW.text_creator->setText(QString("%1").arg(searchDir.path()));
    widget->ui_searchResultW.text_creator->adjustSize();
    if (widget->ui_searchResultW.text_creator->width() > 400)
        widget->ui_searchResultW.text_creator->setFixedWidth(400);
    widget->ui_searchResultW.text_data->hide();
    widget->ui_searchResultW.text_ID->hide();
    widget->type = mode;
    widget->localDirPath = searchDir.path();
    connect(widget->ui_searchResultW.btn_choose, &QPushButton::clicked, this, &workWidget_load::searchResultChoosed);
    widget->setAttribute(Qt::WA_DeleteOnClose);
    p_searchResultTable_layout->addWidget(widget);
    p_searchResultTable_widget->setFixedHeight(80);
}
void workWidget_load::searchResultChoosed(void)
{
    QPushButton* senderCopy_choose = qobject_cast<QPushButton*>(sender());
    searchResultW* senderCopy_searchResultW = static_cast<searchResultW*>(senderCopy_choose->parent()->parent());
    std::vector<SONGINFO> append_songs;
    if (senderCopy_searchResultW->type == 1)
    {
        append_songs.push_back(senderCopy_searchResultW->songInfo);
    }
    if (senderCopy_searchResultW->type == 0)
    {
        const QStringList ucFileFilter = { "*.uc", "*.UC", "*.uc!", "*.UC!" };
        QDir searchDir;
        searchDir.setPath(senderCopy_searchResultW->localDirPath);
        QStringList ucFileList = searchDir.entryList(ucFileFilter, QDir::Files | QDir::Readable, QDir::Name);
        int nAdd = 0;
        while (nAdd < ucFileList.count())
        {
            SONGINFO song;
            song.isLocalOnly = true;
            song.isHaveLocal = true;
            song.localPath = searchDir.path() + "/" + ucFileList.at(nAdd);
            song.name = ucFileList.at(nAdd);
            if (ucFileList.at(nAdd).left(ucFileList.at(nAdd).indexOf("-")).toULongLong() != 0)
                song.id = QString::number(ucFileList.at(nAdd).left(ucFileList.at(nAdd).indexOf("-")).toULongLong());
            else
                song.id = "";
            append_songs.push_back(song);
            nAdd += 1;
        }
    }
    workWidget_list* listWidget = this->parent()->findChild<workWidget_list*>();
    if (listWidget == NULL)
    {
        listWidget = new workWidget_list;
        listWidget->setParent((QWidget*)this->parent());
    }
    listWidget->appendSongList(append_songs);
}
searchResultW::searchResultW(QWidget* parent)
{
    ui_searchResultW.setupUi(this);
    type = 0;
    songInfo = {};
    playListInfo = {};
}
//###############################待选列表工作组件###############################
workWidget_list::workWidget_list(QWidget* parent)
    : QWidget(parent)
{
    ui_wList.setupUi(this);

    p_songListTable_layout = new QVBoxLayout;
    p_songListTable_widget = new QWidget(this);
    p_songListTable_layout->setContentsMargins(QMargins(10, 0, 0, 0));
    p_songListTable_layout->setSpacing(0);
    p_songListTable_widget->setFixedWidth(600);
    p_songListTable_widget->setObjectName("songListTable_widget");
    p_songListTable_widget->setLayout(p_songListTable_layout);
    ui_wList.songListTable->setWidget(p_songListTable_widget);
    ui_wList.checkBox_chooseAll->hide();

    connect(ui_wList.btn_clearTable, &QPushButton::clicked, this, &workWidget_list::clearSongList);
    connect(ui_wList.checkBox_chooseAll, &QCheckBox::clicked, this, &workWidget_list::chooseAllBoxClicked);
    connect(ui_wList.btn_getSongInfo, &QPushButton::clicked, this, &workWidget_list::getLocalSongInfo);
    connect(ui_wList.btn_startDownload, &QCheckBox::clicked, this, &workWidget_list::addToDownload);
}
void workWidget_list::appendSongList(std::vector<SONGINFO> nAppend_songList)
{
    songList.insert(songList.end(), nAppend_songList.begin(), nAppend_songList.end());
    printSongListTable(nAppend_songList, true);
    scanList();
}
void workWidget_list::clearSongList(void)
{
    while (p_songListTable_layout->count() != 0)//这里不需要计数器 因为count会随child的减少而减少
    {
        songListW* nowDelItem = (songListW*)p_songListTable_layout->layout()->itemAt(0)->widget();//获取第一个childWidget
        delete nowDelItem;//删除该child以释放内存
    }
    songList.clear();
    songList.resize(0);
    songCount = 0;//重置计数器
    scanList();
    p_songListTable_widget->setFixedHeight(0);//将列表长度置零(否则会有滚动条)
}
void workWidget_list::scanList(void)
{
    int nCheck = 0;
    bool isHaveLocalOnly = false;//是否存在仅本地的文件
    bool isAllChecked = true;//是否已经全选
    //猜想法: 猜想已经全选, 但若遇到未选中的项打破该猜想
    while (nCheck < p_songListTable_layout->count())
    {
        songListW* nowCheckItem = (songListW*)p_songListTable_layout->layout()->itemAt(nCheck)->widget();
        if (nowCheckItem->ui_songListW.checkBox_choose->isChecked() == false)
            isAllChecked = false;
        if (nowCheckItem->songInfo.isLocalOnly == true)
            isHaveLocalOnly = true;
        nCheck += 1;
    }
    if (isHaveLocalOnly == true)
        ui_wList.btn_getSongInfo->show();
    else
        ui_wList.btn_getSongInfo->hide();
    if (songCount > 1)
    {
        ui_wList.checkBox_chooseAll->show();
        if (isAllChecked == true)
        {
            ui_wList.checkBox_chooseAll->setChecked(true);
            ui_wList.checkBox_chooseAll->setText("全不选");
        }
        else
        {
            ui_wList.checkBox_chooseAll->setChecked(false);
            ui_wList.checkBox_chooseAll->setText("全选");
        }
    }
    else
    {
        ui_wList.checkBox_chooseAll->hide();
    }
}

void workWidget_list::printSongListTable(std::vector<SONGINFO> songsInfo, bool isAdd)
{
    const int scrollBarPosHistory = ui_wList.songListTable->verticalScrollBar()->value();//记录当前进度条位置, 这样在增加内容时用户不会被打断
    if (isAdd == false)
        clearSongList();
    short nowPrtItem = 0;
    while (nowPrtItem < songsInfo.size())
    {
        songListW* widget = new songListW;
        widget->songInfo = songsInfo.at(nowPrtItem);
        widget->reUiInfo();
        widget->setAttribute(Qt::WA_DeleteOnClose);
        p_songListTable_layout->addWidget(widget);
        nowPrtItem += 1;
    }
    songCount += songsInfo.size();
    scanList();
    p_songListTable_widget->setFixedHeight(songCount * 35 + 12);
}
void workWidget_list::chooseAllBoxClicked(void)
{
    if (ui_wList.checkBox_chooseAll->isChecked() == true)
    {
        int nCheck = 0;
        while (nCheck < p_songListTable_layout->count())
        {
            songListW* nowCheckItem = (songListW*)p_songListTable_layout->layout()->itemAt(nCheck)->widget();
            if(nowCheckItem->ui_songListW.checkBox_choose->isVisible() == true)
                nowCheckItem->ui_songListW.checkBox_choose->setChecked(true);
            nCheck += 1;
        }
        ui_wList.checkBox_chooseAll->setText("全不选");
        return;
    }
    if (ui_wList.checkBox_chooseAll->isChecked() == false)
    {
        int nCheck = 0;
        while (nCheck < p_songListTable_layout->count())
        {
            songListW* nowCheckItem = (songListW*)p_songListTable_layout->layout()->itemAt(nCheck)->widget();
            nowCheckItem->ui_songListW.checkBox_choose->setChecked(false);
            nCheck += 1;
        }
        ui_wList.checkBox_chooseAll->setText("全选");
        return;
    }
}
void workWidget_list::getLocalSongInfo(void)
{
    //获取缓存文件的歌曲信息
    //其实可以一个循环搞定, 但不想麻烦服务器(不能发太多请求), 只能委屈一下程序了
    int nCheck = 0;
    std::vector<SONGINFO> songInfo;
    while (nCheck < p_songListTable_layout->count())//提取所有songInfo
    {
        songListW* nowItem = (songListW*)p_songListTable_layout->layout()->itemAt(nCheck)->widget();
        songInfo.push_back(nowItem->songInfo);
        nCheck += 1;
    }
    songInfo = completeSongsInfo(songInfo);
    nCheck = 0;
    while (nCheck < p_songListTable_layout->count())//置入获取的信息, 刷新Ui
    {
        songListW* nowItem = (songListW*)p_songListTable_layout->layout()->itemAt(nCheck)->widget();
        nowItem->songInfo = songInfo.at(nCheck);
        nowItem->reUiInfo();
        nCheck += 1;
    }
    scanList();
}
void workWidget_list::addToDownload(void)
{
    int nAdd = 0;
    workWidget_workList* workListWidget = this->parent()->findChild<workWidget_workList*>();
    if (workListWidget == NULL)
    {
        workListWidget = new workWidget_workList;
        workListWidget->setParent((QWidget*)this->parent());
    }
	while (nAdd < p_songListTable_layout->count())
	{
		songListW* nowAddItem = (songListW*)p_songListTable_layout->layout()->itemAt(nAdd)->widget();
		if (nowAddItem->ui_songListW.checkBox_choose->isChecked() == true && nowAddItem->ui_songListW.checkBox_choose->isVisible() == true)
		{
			WORKINFO workInfo;
			workInfo.type = 1;
			workInfo.songInfo = nowAddItem->songInfo;
            if (workInfo.songInfo.isHaveLocal == true)//TODO 此处默认优先操作缓存, 可以允许用户自定义
                workInfo.workStatus.type = 2;
            else
                workInfo.workStatus.type = 1;
			workListWidget->appendWork(workInfo);
			nowAddItem->ui_songListW.checkBox_choose->hide();//防止重复下载
            nowAddItem->ui_songListW.checkBox_choose->setChecked(false);
		}
		nAdd += 1;
	}
	scanList();
}

songListW::songListW(QWidget* parent)
{
    ui_songListW.setupUi(this);
}
void songListW::reUiInfo(void)
{
    ui_songListW.text_title->setText(songInfo.name);
    ui_songListW.text_title->adjustSize();
    if (ui_songListW.text_title->width() > 250)
        ui_songListW.text_title->setFixedWidth(250);
    ui_songListW.text_artists->setGeometry(ui_songListW.text_title->x() + ui_songListW.text_title->width() + 10,
        ui_songListW.text_artists->y(),
        ui_songListW.text_artists->width(),
        ui_songListW.text_artists->height());
    const int spaceLeftTo_text_artists = ui_songListW.rootWidget->width()
        - ui_songListW.text_title->width()
        - ui_songListW.text_title->x() - 10
        - ui_songListW.btn_choose->width() - 70;
    if (songInfo.isLocalOnly == true)
        ui_songListW.text_artists->setText("本地缓存文件");
    else
        ui_songListW.text_artists->setText(QString("%1").arg(qDID_artistsStrIn_songInfo(songInfo.artists, spaceLeftTo_text_artists / 10)));
    ui_songListW.text_artists->adjustSize();
    if (ui_songListW.text_artists->width() > spaceLeftTo_text_artists)
        ui_songListW.text_artists->setFixedWidth(spaceLeftTo_text_artists);
    TIME songDuration = qTime_formatFromMs(songInfo.duration.toInt(), 2);
    ui_songListW.text_subTitle->setText(QString("%1'%2''").arg(songDuration.m).arg(songDuration.s));
    ui_songListW.text_subTitle->adjustSize();
    if (songInfo.isLocalOnly == true)
        ui_songListW.text_subTitle->hide();
    if (ui_songListW.text_subTitle->width() > 60)
        ui_songListW.text_subTitle->setFixedWidth(60);
}
//###############################工作列表工作组件###############################
workWidget_workList::workWidget_workList(QWidget* parent)
    : QWidget(parent)
{
    ui_wWorkList.setupUi(this);

    nowDownloading = 0;
    p_workInfoTable_layout = new QVBoxLayout;
    p_workInfoTable_widget = new QWidget(this);
    p_workInfoTable_layout->setContentsMargins(QMargins(10, 0, 0, 0));
    p_workInfoTable_layout->setSpacing(0);
    p_workInfoTable_widget->setFixedWidth(600);
    p_workInfoTable_widget->setObjectName("workListTable_widget");
    p_workInfoTable_widget->setLayout(p_workInfoTable_layout);
    ui_wWorkList.workInfoTable->setWidget(p_workInfoTable_widget);

    connect(ui_wWorkList.btn_openDir, &QPushButton::clicked, this, &workWidget_workList::openSaveLocation);
}
void workWidget_workList::addPrintWorkTable(WORKINFO workInfo)
{
    workInfoW* widget = new workInfoW;
    widget->workInfo = workInfo;
    if (widget->workInfo.type == 1)
    {
        widget->ui_workInfoW.text_title->setText(widget->workInfo.songInfo.name);
        widget->ui_workInfoW.text_title->adjustSize();
    }
    else
    {
        QMessageBox::critical(this, "LUD-意外错误",
            "一个意外的错误发生在workWidgets->workList->rePrintWorkListTable()中, 指定的模式不被接受\n建议您重启程序, 如果仍无法解决, 请反馈给开发者");
    }
    widget->setAttribute(Qt::WA_DeleteOnClose);
    p_workInfoTable_layout->addWidget(widget);
    p_workInfoTable_widget->setFixedHeight(p_workInfoTable_layout->count() * 35 + 15);
    startWork(-1);
}
int workWidget_workList::findNextWorkItemIndex()
{
    int nFind = 0;
    while (nFind < p_workInfoTable_layout->count())
    {
        workInfoW* nowFindItem = (workInfoW*)p_workInfoTable_layout->layout()->itemAt(nFind)->widget();
        if (nowFindItem->workInfo.workStatus.code==0)
        {
            return nFind;
        }
        nFind += 1;
    }
    return -1;
}
void workWidget_workList::openSaveLocation(void)
{
    QString runCommand = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/Result");
    QProcess::startDetached("explorer.exe",QStringList(runCommand));
}
bool workWidget_workList::createDir(QString dirPath)
{
    QDir dirWorker;
    //dirWorker.setCurrent(dirPath);
    dirWorker.setPath(dirPath);
    if (dirWorker.exists() == false)
    {
        if (dirWorker.mkpath(dirPath) == false)
            return false;
    }
    return true;
}
void workWidget_workList::startWork(int index)
{
    if (nowWorking >= 2)//TODO 此处可以允许用户自定义最大同时工作数
        return;
    if (index == -1)//如果未定义指定工作编号那么自动查找下一个工作
        index = findNextWorkItemIndex();
    if (index == -1)//如果找不到下一个工作, 可能已经全部完成, 直接返回
        return;
    workInfoW* workInfoItem = (workInfoW*)p_workInfoTable_layout->layout()->itemAt(index)->widget();
    if (workInfoItem->workInfo.workStatus.type == 1)
    {
        if (nowDownloading >= 1)//TODO 此处可以允许用户自定义最大下载数
            return;
        QString downloadUrl;
        qFile_createDir(QApplication::applicationDirPath() + "/Result");//TODO 此处应该允许用户自定义保存路径
        //生成下载URL
        if (workInfoItem->workInfo.type = 1)//下载单曲
            downloadUrl = QString("http://music.163.com/song/media/outer/url?id=%1.mp3")
            .arg(workInfoItem->workInfo.songInfo.id);
        thread_downloader* downloader = new thread_downloader;//新线程
        downloader->index = index;
        downloader->downloadUrl = downloadUrl;
        //TODO 这里默认了分块大小(1MB) 可以允许用户自定义
        createDir(QApplication::applicationDirPath() + "/Result");//TODO 这里应该允许用户自定义保存位置
        downloader->savePath = QApplication::applicationDirPath() + "/Result/" + workInfoItem->workInfo.songInfo.name + ".mp3";
        connect(downloader, &thread_downloader::finishWork, this, &workWidget_workList::workFinished);
        connect(downloader, &thread_downloader::reProgress, this, &workWidget_workList::reWorkProgress);
        downloader->start();
        workInfoItem->workInfo.workStatus.code = 1;//设置状态码
        workInfoItem->reUiProgress();
        nowDownloading += 1;
        nowWorking += 1;
    }
    if (workInfoItem->workInfo.workStatus.type == 2)
    {
        if (nowConverting >= 1)//TODO 此处可以允许用户自定义最大同时转换数
            return;
        thread_converter* converter = new thread_converter;//新线程
        converter->index = index;
        converter->filePath = workInfoItem->workInfo.songInfo.localPath;
        createDir(QApplication::applicationDirPath() + "/Result");//TODO 这里应该允许用户自定义保存位置
        converter->savePath = QApplication::applicationDirPath() + "/Result/" + workInfoItem->workInfo.songInfo.name + ".mp3";
        connect(converter, &thread_converter::finishWork, this, &workWidget_workList::workFinished);
        connect(converter, &thread_converter::reProgress, this, &workWidget_workList::reWorkProgress);
        converter->start();
        workInfoItem->workInfo.workStatus.code = 1;//设置状态码
        nowConverting += 1;
        nowWorking += 1;
    }
}
void workWidget_workList::reWorkProgress(int index, int size, int speed)
{
    workInfoW* workInfoItem = (workInfoW*)p_workInfoTable_layout->layout()->itemAt(index)->widget();
    if (speed == -1)//speed=-1声明此次信号传递总大小 同时说明下载已经开始
    {
        workInfoItem->workInfo.workStatus.totalSize = size;
        workInfoItem->workInfo.workStatus.speed = 0;
        workInfoItem->workInfo.workStatus.speed_ain = 0;
        workInfoItem->workInfo.workStatus.code = 2;
        workInfoItem->reUiProgress();
        return;
    }
    else
    {
        workInfoItem->workInfo.workStatus.finishedSize = size;
        workInfoItem->workInfo.workStatus.speed = (workInfoItem->workInfo.workStatus.speed * workInfoItem->workInfo.workStatus.speed_ain + speed) /
            (workInfoItem->workInfo.workStatus.speed_ain + 1);
        workInfoItem->workInfo.workStatus.speed_ain += 1;
        if (workInfoItem->workInfo.workStatus.speed_ain > 5)//最大五次取平均
            workInfoItem->workInfo.workStatus.speed_ain = 5;
        workInfoItem->workInfo.workStatus.code = 3;
        workInfoItem->reUiProgress();
        return;
    }
}
void workWidget_workList::workFinished(int index , int code)
{
    workInfoW* workInfoItem = (workInfoW*)p_workInfoTable_layout->layout()->itemAt(index)->widget();
    workInfoItem->workInfo.workStatus.code = code;
    if (workInfoItem->workInfo.workStatus.type == 1)
        nowDownloading -= 1;
    if (workInfoItem->workInfo.workStatus.type == 2)
        nowConverting -= 1;
    nowWorking -= 1;
    startWork(-1);//自动开始下一个下载
    workInfoItem->reUiProgress();
}

void workWidget_workList::rePrintWorkListTable()
{
    while (p_workInfoTable_layout->count() != 0)//这里不需要计数器 因为count会随child的减少而减少
    {
        workInfoW* nowDelItem = (workInfoW*)p_workInfoTable_layout->layout()->itemAt(0)->widget();//获取第一个childWidget
        delete nowDelItem;//删除该child以释放内存
    }
    p_workInfoTable_widget->setFixedHeight(0);//将列表长度置零(否则会有滚动条)
    short nowPrtItem = 0;
    while (nowPrtItem < workList.size())
    {
        addPrintWorkTable(workList.at(nowPrtItem));
        nowPrtItem += 1;
    }
}

void workWidget_workList::appendWork(WORKINFO nAppend_work)
{
    nAppend_work.workStatus.code = 0;
    workList.push_back(nAppend_work);
    addPrintWorkTable(nAppend_work);
}
workInfoW::workInfoW(QWidget* parent)
{
    ui_workInfoW.setupUi(this);
}
void workInfoW::reUiProgress()
{
    if (workInfo.workStatus.type == 1)
    {
        if (workInfo.workStatus.code == 1)
        {
            ui_workInfoW.progressBar_workProgress->setValue(0);
            ui_workInfoW.text_workInfo->setText("正在获取总大小...");
        }
        if (workInfo.workStatus.code == 2)
        {
            ui_workInfoW.text_workInfo->setText("正在开始下载...");
        }
        if (workInfo.workStatus.code == 3)
        {
            ui_workInfoW.progressBar_workProgress->setValue(workInfo.workStatus.finishedSize * ui_workInfoW.progressBar_workProgress->maximum() / workInfo.workStatus.totalSize);
            ui_workInfoW.text_workInfo->setText(QString("%1kb/s 正在下载-%2%")
                .arg(workInfo.workStatus.speed)
                .arg(workInfo.workStatus.finishedSize * 100 / workInfo.workStatus.totalSize));
        }
        if (workInfo.workStatus.code == 200)
        {
            ui_workInfoW.progressBar_workProgress->setValue(0);
            ui_workInfoW.text_workInfo->setText("已完成");
        }
        if (workInfo.workStatus.code < 0)//出错的情况 (错误代码全部小于0)
        {
            ui_workInfoW.progressBar_workProgress->setValue(0);
            if (workInfo.workStatus.code == -1)
                ui_workInfoW.text_workInfo->setText("无法获取下载内容大小");
            if (workInfo.workStatus.code == -2)
                ui_workInfoW.text_workInfo->setText("无法写入文件");
            if (workInfo.workStatus.code == -3)
                ui_workInfoW.text_workInfo->setText("下载中断");
            ui_workInfoW.rootWidget->setProperty("style", "error");
            ui_workInfoW.rootWidget->setStyleSheet(ui_workInfoW.rootWidget->styleSheet());//刷新样式表使更改得以应用
            ui_workInfoW.text_workInfo->setProperty("style", "error");
            ui_workInfoW.text_workInfo->setStyleSheet(ui_workInfoW.text_workInfo->styleSheet());//刷新样式表使更改得以应用
        }
    }
    if (workInfo.workStatus.type == 2)
    {
        //if (workInfo.workStatus.code == 1) 这个时间非常短, 用户几乎不会觉察, 所以不再浪费时间刷新Ui
        if (workInfo.workStatus.code == 3)
        {
            ui_workInfoW.progressBar_workProgress->setValue(workInfo.workStatus.finishedSize * ui_workInfoW.progressBar_workProgress->maximum() / workInfo.workStatus.totalSize);
            ui_workInfoW.text_workInfo->setText(QString("正在转换-%1%")
                .arg(workInfo.workStatus.finishedSize * 100 / workInfo.workStatus.totalSize));
        }
        if (workInfo.workStatus.code == 200)
        {
            ui_workInfoW.progressBar_workProgress->setValue(0);
            ui_workInfoW.text_workInfo->setText("已完成");
        }
        if (workInfo.workStatus.code < 0)//出错的情况 (错误代码全部小于0)
        {
            ui_workInfoW.progressBar_workProgress->setValue(0);
            if (workInfo.workStatus.code == -1)
                ui_workInfoW.text_workInfo->setText("无法打开源文件");
            if (workInfo.workStatus.code == -2)
                ui_workInfoW.text_workInfo->setText("无法写入文件");
            ui_workInfoW.rootWidget->setProperty("style", "error");
            ui_workInfoW.rootWidget->setStyleSheet(ui_workInfoW.rootWidget->styleSheet());//刷新样式表使更改得以应用
            ui_workInfoW.text_workInfo->setProperty("style", "error");
            ui_workInfoW.text_workInfo->setStyleSheet(ui_workInfoW.text_workInfo->styleSheet());//刷新样式表使更改得以应用
        }
    }
    
}

//###############################设置项目工作组件###############################
workWidget_settings::workWidget_settings(QWidget* parent)
    : QWidget(parent)
{
    ui_wSettings.setupUi(this);
}