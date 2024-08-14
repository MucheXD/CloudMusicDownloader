#pragma once
#include <QtWidgets/QDialog>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QMouseEvent>
#include <vector>
#include "ui_msgbox.h"
#include "global.h"
#import "winhttpcom.dll"

#pragma execution_character_set("utf-8")

QByteArray qNetwork_getHttpData(QString method, QString url, bool autoAlert);
QString qNetwork_getHttpText(QString method, QString url, bool autoAlert);
QString qNetwork_getHttpHeaderText(QString url, QString header, bool autoAlert);
int qText_indexOfEnd(QString text, QString indexText, int p = 0);
QString qText_Between(QString text, QString left, QString right, int from = 0);
int qJson_findCurrentEnd(QString text, int nPos, QString sMark = "{", QString eMark = "}");
int qJson_findCurrentEnd(QString text, int nPos, QString sMark, QString eMark);
bool qFile_createDir(QString path);
QString qConfig_readKey(QString key, QString configFileName = QCoreApplication::applicationDirPath() + "/LUD.dcf");
bool qConfig_writeKey(QString configFile, QString key, QString data);
TIME qTime_formatFromMs(int ms, short msb);//msb指定最高位, 1=秒 2=分 3=时
QString qDID_artistsStrIn_songInfo(std::vector<USERINFO> artistList, int maxLength);//将作者列表解析为最大为指定长度的字符串
//void qMsgbox_info(void);
std::vector<PLAYLISTINFO> searchInfoParser_PlayList(QString rawResult);
std::vector<SONGINFO> searchInfoParser_Songs(QString rawResult);//type指定兼容模板 =1搜索结果来源 =2信息获取来源(我也不知道为什么网易云两个模板不一样)
std::vector<ALBUMINFO> searchInfoParser_Album(QString rawAlbumInfo);
std::vector<SONGINFO> songsIn_Album(ALBUMINFO from);
void fillSongInfo(SONGINFO *rawSongInfo);
std::vector<SONGINFO> completeSongsInfo(std::vector<SONGINFO> rawSongInfoList);
bool compareByLocalCreateTimeSec(const SONGINFO& a, const SONGINFO& b);

class Network
{
public:
	WinHttp::IWinHttpRequestPtr pHttpWork;
	QString sendBody;
	QStringList headers[2];//0位存储标头 1位存储头内容
	Network();
	int sendHttpRequest(QString method, QString url);
	QByteArray getHttpData();//先发送请求
	QString getHttpHeader(QString header);//先发送请求
	/*结束代码
	* 1=成功
	* 0=未调用
	* -3=无法初始化连接
	* -4=无法获取数据
	*/
private:
};

//class msgBox : public QDialog
//{
//	Q_OBJECT
//public:
//	msgBox(QDialog* parent = Q_NULLPTR);
//
//	int type = 0;
//
//private:
//	
//	int slideWindowCenterX = 0;
//	int slideWindowCenterY = 0;
//	void setWindowStyle(void);
//	void mouseMoveEvent(QMouseEvent* mouseEvent);
//	void mouseReleaseEvent(QMouseEvent* mouseEvent);
//	Ui::msgBoxW ui_mb;
//};

class thread_downloader :
	public QThread
{
	Q_OBJECT
public:
	QString downloadUrl;//下载URL
	QString savePath;//文件保存位置
	int bufferSize;//缓存大小
	int index;//仅存储 不参与计算也不会被修改
	QString id;
	thread_downloader();//构造函数 用于初始化内存数据
	void run();
signals:
	void reProgress(int index , int size, int speed);//此信号第一次发送时size为总大小, 声明下载开始, 此后size发送为已下载大小
	void finishWork(int index, int code);
};

class thread_converter :
	public QThread
{
	Q_OBJECT
public:
	QString filePath;//缓存文件位置
	QString savePath;//文件保存位置
	int bufferSize;//缓存大小
	int index;//仅存储 不参与计算也不会被修改
	thread_converter();//构造函数 用于初始化内存数据
	void run();
signals:
	void reProgress(int index, int size, int speed);//此信号第一次发送时size为总大小, 声明转换, 此后size发送为已转换大小
	void finishWork(int index, int code);
};