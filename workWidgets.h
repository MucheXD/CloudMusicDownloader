#pragma once
#include <QtWidgets/QWidget>
#include <QMessageBox>
#include <QTimer>
#include <QLayout>
#include <QScrollBar>
#include <QProcess>
#include <QPixmap>
#include <QRegularExpression> 
#include <QClipboard>
#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include "ui_workWidget_load.h"
#include "ui_workWidget_list.h"
#include "ui_workWidget_workList.h"
#include "ui_workWidget_settings.h"
#include "ui_searchResultW.h"
#include "ui_songInfoW.h"
#include "ui_songListW.h"
#include "ui_workInfoW.h"
#include "ui_settingsAreaW.h"
#include "basicFunc.h"
#include "global.h"

class workWidget_load : public QWidget
{
	Q_OBJECT

public:
	workWidget_load(QWidget* parent = Q_NULLPTR);

	
private:
	QVBoxLayout* p_searchResultTable_layout;
	QWidget* p_searchResultTable_widget;
	Ui::workWidget_load_Class ui_wLoad;
	//std::vector<PLAYLISTINFO> sr_playList;//搜索结果: 歌单存放位置
	int resultCount = 0;

	void reUi(void);
	void searchModeChanged(int mode);
	void searchTextChanged(void);
	void getSearchInfo(void);
	void clearSearchResult(void);
	void printSearchResult(int mode, std::vector<SONGINFO> sr_songInfo, bool isAdd);
	void printSearchResult(int mode, std::vector<PLAYLISTINFO> sr_playList, bool isAdd);
	void printSearchResult(int mode = 0);
	void showSearchResultDetailInfo(void);
	void searchResultChoosed(void);
};

class searchResultW : public QWidget
{
	Q_OBJECT
public:
	searchResultW(QWidget* parent = Q_NULLPTR);
	int type;
	SONGINFO songInfo;
	PLAYLISTINFO playListInfo;
	QString localDirPath;
	Ui::searchResultWClass ui_searchResultW;
};

class songInfoW : public QWidget
{
	Q_OBJECT
public:
	songInfoW(QWidget* parent = Q_NULLPTR);
	void initUi(void);
	void switchLyrics(void);
	void quitInfoW(void);
	void copyLyrics(void);
	void u_setDownloadLyricBtn(int mode);
	void downloadLyricBtnClicked(void);
	void downloadBlurPicToFile(void);
	int index;
	int type;
	SONGINFO* p_songInfo;
	SONGINFO songInfo;
	Ui::songInfoW_Class ui_songInfoW;
signals:
	void songInfoWExit(int index);
	//PLAYLISTINFO* playListInfo;
};

class workWidget_list : public QWidget
{
	Q_OBJECT

public:
	workWidget_list(QWidget* parent = Q_NULLPTR);
	void appendSongList(std::vector<SONGINFO> nAppend_songList);
	void clearSongList(void);
	void scanList(void);
	void printSongListTable(std::vector<SONGINFO> songsInfo, bool isAdd);
	void chooseAllBoxClicked(void);
	void getLocalSongInfo(void);
	void addToDownload(void);
	void showSearchResultDetailInfo(void);
	void songInfoWindowExited(int index);
	std::vector<SONGINFO> songList;
private:
	Ui::workWidget_list_Class ui_wList;
	QVBoxLayout* p_songListTable_layout;
	QWidget* p_songListTable_widget;

	int songCount = 0;
signals:

};

class songListW : public QWidget
{
	Q_OBJECT
public:
	songListW(QWidget* parent = Q_NULLPTR);
	void reUiInfo(void);//根据songInfo刷新Ui界面的信息
	int numInSongList;
	SONGINFO songInfo;
	Ui::songListWClass ui_songListW;
};

class workWidget_workList : public QWidget
{
	Q_OBJECT

public:
	workWidget_workList(QWidget* parent = Q_NULLPTR);
	void appendWork(WORKINFO nAppend_work);
	void workFinished(int index, int code);
	void reWorkProgress(int index, int size, int speed);
private:
	void addPrintWorkTable(WORKINFO workInfo);
	void rePrintWorkListTable(void);
	int findNextWorkItemIndex(void);
	void openSaveLocation(void);
	bool createDir(QString dirPath);//确认并自动创建目录 返回是否成功
	void startWork(int index);//如果index=-1自动寻找下一个
	void downloadLyrics(int mode, QString saveFileName, LYRIC lyrics);

	int nowWorking = 0;//当前正在运行的工作线程数(nowConverting+nowDownloading)
	int nowConverting = 0;//当前正在转换的任务数
	int nowDownloading = 0;//当前正在下载的任务数
	std::vector<WORKINFO> workList;
	Ui::workWidget_workList_Class ui_wWorkList;
	QVBoxLayout* p_workInfoTable_layout;
	QWidget* p_workInfoTable_widget;
};

class workInfoW : public QWidget
{
	Q_OBJECT
public:
	workInfoW(QWidget* parent = Q_NULLPTR);
	void reUiProgress();
	WORKINFO workInfo;
	Ui::workInfoWClass ui_workInfoW;
};

class workWidget_settings : public QWidget
{
	Q_OBJECT

public:
	workWidget_settings(QWidget* parent = Q_NULLPTR);
private:
	Ui::workWidget_settings_Class ui_wSettings;

	void applyChanges(void);
};

class settingsAreaW : public QWidget
{
	Q_OBJECT
public:
	settingsAreaW(QWidget* parent = Q_NULLPTR);
	Ui::settingsAreaW ui_sAW;
private:
	
};

