#pragma once
#include <QString>

const uint PROGRAMBUILDVER = 0xD0000052;
const QString PROGRAMVERSION = "0.5.0";


struct USERINFO
{
	QString id;
	QString name;
};
struct PLAYLISTINFO
{
	QString id;
	QString name;
	QString coverImgUrl;
	QString trackCount;
	QString playCount;
	QString bookCount;
	QString description;
	USERINFO creatorInfo;
};
struct ALBUMINFO
{
	QString id;
	QString name;
	QString description;
	std::vector<USERINFO> artists;
	QString blurPicUrl;
};

struct LYRIC
{
	bool isHaveLver = true;
	bool isHaveTver = true;
	bool isHaveRver = true;

	QString lver;
	QString tver;
	QString rver;

	int needDownload = 0;//0=不附加下载,1/2/3=附加下载l/t/r,4=附加下载全部
};

struct MVINFO
{
	bool isAvailable = true;
	QString title;
	QString url;
};

struct SONGINFO
{
	bool isLocalOnly;
	bool isHaveLocal;
	QString localPath;

	QString id;
	QString name;
	QString duration;
	QString mvid;
	MVINFO mv;
	QString blurPicUrl;
	LYRIC lyrics;

	std::vector<USERINFO> artists;
};
struct TIME
{
	int h;
	short m;
	short s;
	short ms;
};

struct WORKSTATUS
{
	//code=-3 无法下载文件主体或下载中断
	//code=-2 无法打开保存文件
	//code=-1 无法获取总大小/无法打开源文件
	//code=0 等待开始
	//code=1 已经调用下载/已经开始转换
	//code=2 已经开始下载(已经获取到总大小)
	//code=3 正在下载进程(已经下载了一部分)/正在转换
	//code=200 成功完成
	int code;
	//type=1 下载
	//type=2 转换
	int type;
	long long totalSize;
	long long finishedSize;
	int speed;//平均速度
	int speed_ain;//参与取平均的项数
};
struct WORKINFO
{
	//type=1 单曲
	int type;
	QString savePath;
	SONGINFO songInfo;
	WORKSTATUS workStatus;
};
