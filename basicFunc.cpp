#include "basicFunc.h"

QString qNetwork_getHttpText(QString method, QString url, bool autoAlert)
{
	Network network;
	int code = network.sendHttpRequest(method, url);
	if (code == 200)
	{
		QString responseText = network.getHttpData();
		return responseText.toUtf8();
	}
	else
		return QString("network_error:%1").arg(code);
}

QByteArray qNetwork_getHttpData(QString method, QString url, bool autoAlert)
{
	Network network;
	int code = network.sendHttpRequest(method, url);
	if (code == 200)
		return network.getHttpData();
	else
		return NULL;
}

QString qNetwork_getHttpHeaderText( QString url, QString header, bool autoAlert)
{
	Network network;
	int code = network.sendHttpRequest("HEAD", url);
	if (code == 200)
		return network.getHttpHeader(header);
	else
		return QString("network_error:%1").arg(code);
}

int qText_indexOfEnd(QString text, QString indexText, int p)
{
	if (text.indexOf(indexText, p) != -1)
		return text.indexOf(indexText, p) + indexText.length();
	else
		return -1;
}

QString qText_Between(QString text, QString left, QString right, int from)
{
	const int from_left = text.indexOf(left, from) + left.length();
	const int n = text.indexOf(right, from_left) - from_left;
	return text.mid(from_left, n);
}

int qJson_findCurrentEnd(QString text, int nPos, QString sMark, QString eMark)
{
	int markw = 0;
	while (nPos < text.length() && markw >= 0)
	{
		if (text.at(nPos) == sMark)
			markw += 1;
		if (text.at(nPos) == eMark)
			markw -= 1;
		nPos += 1;
	}
	return nPos;
}

bool qFile_createDir(QString path)
{
	QDir dir;
	return dir.mkdir(path);
}

QString qConfig_readKey( QString key, QString configFileName)
{
	QFile configFile;
	configFile.setFileName(configFileName);
	if (configFile.size() > 1048576)
		return "ERROR_OOM";//如果目标配置文件大于1MB直接返回失败
	configFile.open(QIODevice::ReadOnly);
	QString configData = configFile.readAll();
	return qText_Between(configData, "\"", "\"", qText_indexOfEnd(configData, key));
}

bool qConfig_writeKey(QString configFileName, QString key, QString data)
{
	QFile configFile;
	configFile.setFileName(configFileName);
	if (configFile.size() > 1048576)
		return false;//如果目标配置文件大于1MB直接返回失败
	configFile.open(QIODevice::ReadWrite);
	QString configData = configFile.readAll();
	return true;
}

TIME qTime_formatFromMs(int ms, short msb)
{
	TIME result = {};
	if (msb == 1)
	{
		result.ms = ms % 1000;
		ms = (ms - ms % 1000) / 1000;
		result.s = ms;
	}
	if (msb == 2)
	{
		result.ms = ms % 1000;
		ms = (ms - ms % 1000) / 1000;
		result.s = ms % 60;
		ms = (ms - ms % 60) / 60;
		result.m = ms;
	}
	if (msb == 3)
	{
		result.ms = ms % 1000;
		ms = (ms - ms % 1000) / 1000;
		result.s = ms % 60;
		ms = (ms - ms % 60) / 60;
		result.m = ms % 60;
		ms = (ms - ms % 60) / 60;
		result.h = ms;
	}
	return result;
}

QString qDID_artistsStrIn_songInfo(std::vector<USERINFO> artistList, int maxLength)
{
	QString result = "";
	int nAdd = 0;
	while (nAdd < artistList.size() && (result.toLocal8Bit().length() + artistList.at(nAdd).name.toLocal8Bit().length() <= maxLength || nAdd<1))
	{
		result.append(artistList.at(nAdd).name + ", ");
		nAdd += 1;
	}
	result = result.left(result.length() - 2);
	if (artistList.size() - nAdd > 0)//如果还有更多作者但已经超过长度
		result.append(QString(" 等%1位").arg(artistList.size()));
	return result;
}

//void qMsgbox_info(void)
//{
//	msgBox* mb = new msgBox;
//	mb->setModal(true);
//	mb->type = 1;
//	mb->exec();
//}

Network::Network()
{
	sendBody = NULL;
	HRESULT isSuccessedCheck = CoInitialize(NULL);
	if (FAILED(isSuccessedCheck))
	{
		QMessageBox::critical(NULL, "LUD-意外错误",
			"一个意外的错误发生在basicFunc->::Network()->CoInitialize中, 无法初始化COM组件\n建议您重启程序, 如果仍无法解决, 请反馈给开发者");
	}
	pHttpWork.CreateInstance(__uuidof(WinHttp::WinHttpRequest));
	if (FAILED(isSuccessedCheck))
	{
		QMessageBox::critical(NULL, "LUD-意外错误",
			"一个意外的错误发生在basicFunc->::Network()->CreateInstance中, 无法初始化COM组件\n建议您重启程序, 如果仍无法解决, 请反馈给开发者");
	}
}

int Network::sendHttpRequest(QString method, QString url)
{
	HRESULT isSuccessedCheck;
	//DEBUG 此处的代理仅供调试使用
	//pHttpWork->SetProxy(2,L"127.0.0.1:8888");
	isSuccessedCheck = pHttpWork->Open(method.toStdWString().c_str(), url.toStdWString().c_str());
	if (FAILED(isSuccessedCheck))
	{
		return -3;
	}
	int nAddHeader = 0;
	while (headers[0].size() > nAddHeader)
	{
		pHttpWork->SetRequestHeader(headers[0].at(nAddHeader).toStdWString().c_str(), headers[1].at(nAddHeader).toStdWString().c_str());;
		nAddHeader += 1;
	}
	try
	{
		isSuccessedCheck = pHttpWork->Send(sendBody.toStdWString().c_str());
	}
	catch (...)
	{
		return -4;
	}
	return pHttpWork->Status;
}

QByteArray Network::getHttpData()
{
	_variant_t body = pHttpWork->GetResponseBody();
	ULONG dataLen = body.parray->rgsabound[0].cElements;
	QByteArray data((char*)body.parray->pvData, dataLen);
	return data;
}

QString Network::getHttpHeader(QString header)
{
	if (header == "")
	{
		_variant_t header = pHttpWork->GetAllResponseHeaders();
		ULONG dataLen = header.parray->rgsabound[0].cElements;
		QByteArray headerData((char*)header.parray->pvData, dataLen);
		return headerData;
	}
	else
	{
		return (LPCSTR)pHttpWork->GetResponseHeader(header.toStdWString().c_str());
	}
}

std::vector<PLAYLISTINFO> searchInfoParser_PlayList(QString rawResult)
{
	std::vector<PLAYLISTINFO> retResult;
	int nPos = 0;
	nPos = qText_indexOfEnd(rawResult, "playlists", 0);
	nPos = qText_indexOfEnd(rawResult, "[", nPos);
	while (nPos<rawResult.indexOf("]",nPos)-1)
	{
		nPos = qText_indexOfEnd(rawResult, "{", nPos);//找到开始的括号 (分段开始)
		int subcEndPos = qText_indexOfEnd(rawResult, "}", nPos);
		subcEndPos = rawResult.indexOf("}", subcEndPos);//找到向后第二个括回 (分段结束)
		QString subRawData = rawResult.mid(nPos, subcEndPos - nPos);//提取当前分段
		PLAYLISTINFO playListInfo;//开辟子存储
		playListInfo.id = qText_Between(subRawData,"\"id\":",",",0);
		playListInfo.name = qText_Between(subRawData, "\"name\":\"", "\",", 0);
		playListInfo.coverImgUrl = qText_Between(subRawData, "\"coverImgUrl\":\"", "\",", 0);
		int sub_creatorInfoStartPos = qText_indexOfEnd(subRawData, "\"creator\":{", 0);//获取创建者信息起始位置
		USERINFO creatorInfo;//开辟子存储的子存储 (创建者信息)
		creatorInfo.id = qText_Between(subRawData, "\"userId\":", ",", sub_creatorInfoStartPos);
		creatorInfo.name = qText_Between(subRawData, "\"nickname\":\"", "\",", sub_creatorInfoStartPos);
		playListInfo.creatorInfo = creatorInfo;
		int sub_creatorInfoEndPos = qText_indexOfEnd(subRawData, "},", sub_creatorInfoStartPos);//获取创建者信息结束位置
		playListInfo.trackCount = qText_Between(subRawData, "\"trackCount\":", ",", sub_creatorInfoEndPos);
		playListInfo.playCount = qText_Between(subRawData, "\"playCount\":", ",", sub_creatorInfoEndPos);
		playListInfo.bookCount = qText_Between(subRawData, "\"bookCount\":", ",", sub_creatorInfoEndPos);
		playListInfo.description = qText_Between(subRawData, "\"description\":\"", "\",", sub_creatorInfoEndPos);
		nPos = subcEndPos;
		retResult.push_back(playListInfo);//压入当前子存储
	}
	return retResult;//返回格式化后的数据
}

std::vector<SONGINFO> searchInfoParser_Songs(QString rawResult)
{
	std::vector<SONGINFO> result;
	int nPos = 0;
	nPos = qText_indexOfEnd(rawResult, "\"songs\":", 0);
	nPos = qText_indexOfEnd(rawResult, "[", nPos);
	while (rawResult.indexOf("]", nPos)-nPos != 0)
	{
		nPos += 1;
		SONGINFO songInfo;
		songInfo.isHaveLocal = false;
		songInfo.isLocalOnly = false;
		songInfo.localPath.clear();
		songInfo.id = qText_Between(rawResult, "\"id\":", ",", nPos);
		songInfo.name = qText_Between(rawResult, "\"name\":\"", "\",", nPos);
		nPos = qText_indexOfEnd(rawResult, "\"artists\":", nPos);
		std::vector<USERINFO> artists;
		while (rawResult.indexOf("]", nPos) - nPos != 0)
		{
			nPos += 1;
			USERINFO artist;
			int subStr_artist_endPos = qText_indexOfEnd(rawResult, "}", nPos);
			QString subStr_artist = qText_Between(rawResult, "{", "}", nPos);
			artist.name = qText_Between(subStr_artist, "\"name\":\"", "\",", 0);
			artist.id = qText_Between(subStr_artist, "\"id\":", ",", 0);
			artists.push_back(artist);
			nPos = subStr_artist_endPos;
		}
		songInfo.artists = artists;
		songInfo.duration = qText_Between(rawResult, "\"duration\":", ",", nPos);
		nPos = qText_indexOfEnd(rawResult, "\"duration\":", nPos);
		songInfo.mvid = qText_Between(rawResult, "\"mvid\":", ",", nPos);
		result.push_back(songInfo);
		nPos = qText_indexOfEnd(rawResult, "\"mvid\":", nPos);
		nPos = qJson_findCurrentEnd(rawResult, nPos);
		//nPos = qText_indexOfEnd(rawResult, "}", nPos);
		/*if (type == 1)
			nPos = qText_indexOfEnd(rawResult, "\"mark\":", nPos);
		else if (type == 2)
			nPos = qText_indexOfEnd(rawResult, "\"volumeDelta\":", nPos);*/
		
	}
	return result;//返回格式化后的数据
}

std::vector<ALBUMINFO> searchInfoParser_Album(QString rawAlbumInfo)
{
	std::vector<ALBUMINFO> result;
	if (rawAlbumInfo.indexOf("\"albumCount\":0") != -1)
		return result;
	int nPos = 0;
	nPos = qText_indexOfEnd(rawAlbumInfo,"\"albums\"",0);//定位到第一个括回 跳过创建者及以前的内容
	nPos = qText_indexOfEnd(rawAlbumInfo, "[", nPos);
	while (true)
	{
		nPos += 1;
		ALBUMINFO albumInfo;
		albumInfo.name = qText_Between(rawAlbumInfo, "\"name\":\"", "\",", nPos);
		albumInfo.id = qText_Between(rawAlbumInfo, "\"id\":", ",", nPos);
		albumInfo.blurPicUrl = qText_Between(rawAlbumInfo, "\"blurPicUrl\":\"", "\",", nPos);
		albumInfo.description = qText_Between(rawAlbumInfo, "\"description\":\"", "\",", nPos);
		nPos = qText_indexOfEnd(rawAlbumInfo, "\"artists\":", nPos);
		std::vector<USERINFO> artists;
		while (rawAlbumInfo.indexOf("]", nPos)-nPos != 0)
		{
			nPos += 1;
			USERINFO artist;
			int subStr_artist_endPos = qText_indexOfEnd(rawAlbumInfo, "}", nPos);
			QString subStr_artist = qText_Between(rawAlbumInfo, "{", "}", nPos);
			artist.name = qText_Between(subStr_artist, "\"name\":\"", "\",", 0);
			artist.id = qText_Between(subStr_artist, "\"id\":", ",", 0);
			artists.push_back(artist);
			nPos = subStr_artist_endPos;
		}
		albumInfo.artists = artists;
		nPos = qText_indexOfEnd(rawAlbumInfo, "},", nPos);
		result.push_back(albumInfo);
		if (qText_indexOfEnd(rawAlbumInfo, "code", nPos) - nPos <= 10)
			break;
	}
	return result;
}

std::vector<SONGINFO> songsIn_Album(ALBUMINFO from)
{
	QString albumInfo = qNetwork_getHttpText("GET", QString("http://music.163.com/api/album/%1").arg(from.id), false);
	return searchInfoParser_Songs(qText_Between(albumInfo, "\"album\":{", "}],\"paid\""));
}

void fillSongInfo(SONGINFO *rawSongInfo)
{
	QString getBlurPicUrl = QString("http://music.163.com/api/song/detail/?ids=[%1]").arg(rawSongInfo->id);
	QString fullInfoText = qNetwork_getHttpText("GET", getBlurPicUrl, false);
	rawSongInfo->blurPicUrl = qText_Between(fullInfoText, "blurPicUrl\":\"", "\"", 0);
	QString getLyricsUrl = QString("https://music.163.com/api/song/lyric?id=%1&lv=1&tv=1&rv=1").arg(rawSongInfo->id);
	QString fullLyricsText = qNetwork_getHttpText("GET", getLyricsUrl, false);
	fullLyricsText.replace("\\n", "\n");
	int nRp = 0;
	nRp = qText_indexOfEnd(fullLyricsText, "\"lrc\"", 0);
	if (nRp != -1)
		rawSongInfo->lyrics.lver = qText_Between(fullLyricsText, "\"lyric\":\"", "\"", nRp);
	else
		rawSongInfo->lyrics.isHaveLver = false;
	nRp = qText_indexOfEnd(fullLyricsText, "\"tlyric\"", 0);
	if (nRp != -1)
		rawSongInfo->lyrics.tver = qText_Between(fullLyricsText, "\"lyric\":\"", "\"", nRp);
	else
		rawSongInfo->lyrics.isHaveTver = false;
	nRp = qText_indexOfEnd(fullLyricsText, "\"romalrc\"", 0);
	if (nRp != -1)
		rawSongInfo->lyrics.rver = qText_Between(fullLyricsText, "\"lyric\":\"", "\"", nRp);
	else
		rawSongInfo->lyrics.isHaveRver = false;
	if (rawSongInfo->mvid != "0")
	{
		QString mvInfoText = qNetwork_getHttpText("GET", QString("http://music.163.com/api/mv/detail?id=%1&type=mp4").arg(rawSongInfo->mvid), false);
		rawSongInfo->mv.title = qText_Between(mvInfoText, "\"name\":\"", "\"", 0);
		if (rawSongInfo->mv.title == "")
			rawSongInfo->mv.title = "点击前往MV";
		nRp = qText_indexOfEnd(mvInfoText, "\"brs\":", 0);
		if (nRp != -1)
		{
			if (mvInfoText.indexOf("\"1080\":\"") != -1)
				rawSongInfo->mv.url = qText_Between(mvInfoText, "\"1080\":\"", "\"", nRp);
			else if (mvInfoText.indexOf("\"720\":\"") != -1)
				rawSongInfo->mv.url = qText_Between(mvInfoText, "\"720\":\"", "\"", nRp);
			else if (mvInfoText.indexOf("\"480\":\"") != -1)
				rawSongInfo->mv.url = qText_Between(mvInfoText, "\"480\":\"", "\"", nRp);
			else if (mvInfoText.indexOf("\"360\":\"") != -1)
				rawSongInfo->mv.url = qText_Between(mvInfoText, "\"360\":\"", "\"", nRp);
			else if (mvInfoText.indexOf("\"240\":\"") != -1)
				rawSongInfo->mv.url = qText_Between(mvInfoText, "\"240\":\"", "\"", nRp);
			else
				rawSongInfo->mv.isAvailable = false;
		}
		else
			rawSongInfo->mv.isAvailable = false;
	}
	else
		rawSongInfo->mv.isAvailable = false;
}

std::vector<SONGINFO> completeSongsInfo(std::vector<SONGINFO> rawSongInfoList)
{
	std::vector<SONGINFO> result = rawSongInfoList;
	int nGetPos = 0;
	while (nGetPos < result.size())
	{
		int getGroupEndPos = nGetPos + 20;
		QString getUrl = "http://music.163.com/api/song/detail/?ids=[";
		int urlWritePos = nGetPos;
		while (urlWritePos < result.size() && urlWritePos < getGroupEndPos)
		{
			if (result.at(urlWritePos).id != "" && result.at(urlWritePos).isLocalOnly == true)
				getUrl += result.at(urlWritePos).id + ",";
			urlWritePos += 1;
		}
		getUrl = getUrl.left(getUrl.length() - 1);//去除最后一个逗号
		getUrl += "]";
		std::vector<SONGINFO> getResult = searchInfoParser_Songs(qNetwork_getHttpText("GET", getUrl, false));
		//将获取到的信息(不止一条)与传入的ID比对, 匹配时为该项填充
		while (nGetPos < result.size() && nGetPos < getGroupEndPos)
		{
			int nSearchPos = 0;
			if (result.at(nGetPos).isLocalOnly == true)
			{
				while (nSearchPos < getResult.size())//枚举所有结果, 直到找到匹配项
				{
					if (result.at(nGetPos).id == getResult.at(nSearchPos).id)//如果找到匹配项(ID相同)
					{
						result.at(nGetPos).isLocalOnly = false;
						//填充并补全信息
						result.at(nGetPos).name = getResult.at(nSearchPos).name;
						result.at(nGetPos).duration = getResult.at(nSearchPos).duration;
						result.at(nGetPos).mvid = getResult.at(nSearchPos).mvid;
						result.at(nGetPos).artists = getResult.at(nSearchPos).artists;
						break;
					}
					nSearchPos += 1;
				}
			}
			nGetPos+=1;
		}
	}
	return result;
}

thread_downloader::thread_downloader()
{
	//初始化内存数据
	downloadUrl = NULL;
	savePath = NULL;
	bufferSize = 409600;//默认缓存大小400KB
	index = -1;
}

void thread_downloader::run(void)
{
	const QString totalDownloadSize_text = qNetwork_getHttpHeaderText(downloadUrl, "Content-Length", false);
	if (totalDownloadSize_text.indexOf("network_error") != -1)
	{
		emit finishWork(index, -1);//发射失败信号
		return;
	}
	long long totalDownloadSize = totalDownloadSize_text.toLong();//获取下载数据总量
	emit reProgress(index, totalDownloadSize, -1);//发送首次数据, 该数据记录总大小
	long long nowDownloadedSize = 0;//已下载大小
	QFile outputFile;
	outputFile.setFileName(savePath);
	if (outputFile.exists() == true)//如果已存在
	{
		if (outputFile.size() != totalDownloadSize)//使用文件大小简单验证是否为同一首歌, 同名又同大小的歌大概是不存在的吧(我想)
			outputFile.setFileName(savePath.remove(".mp3") + "#" + id + ".mp3");
		else
		{
			emit finishWork(index, 304);//发射已存在特征信号
			return;
		}
	}
	if (outputFile.open(QIODevice::ReadWrite) == false)
	{
		emit finishWork(index, -2);//发射失败信号
		return;
	}
	Network downloader;
	while (nowDownloadedSize < totalDownloadSize)
	{
		long startTick = GetTickCount64();//记录开始时间(ms)
		downloader.headers[0].clear();
		downloader.headers[1].clear();
		downloader.headers[0].push_back("Range");
		downloader.headers[1].push_back(QString("bytes=%1-%2").arg(nowDownloadedSize).arg(nowDownloadedSize + bufferSize - 1));
		int code = downloader.sendHttpRequest("GET", downloadUrl);
		if (code != 206 && code != 200)
		{
			emit finishWork(index, -3);//发射失败信号
			return;
		}
		outputFile.write(downloader.getHttpData());
		nowDownloadedSize += bufferSize;
		const int speed = bufferSize / (GetTickCount64() - startTick);
		emit reProgress(index, nowDownloadedSize,speed);//成功完成分块后发射更新进度信号
	}
	outputFile.close();//写入文件并关闭文件
	emit finishWork(index, 200);//发射完成信号
}

thread_converter::thread_converter()
{
	//初始化内存数据
	filePath = NULL;
	savePath = NULL;
	bufferSize = 1048576;//默认缓存大小1MB
	index = -1;
}

void thread_converter::run(void)
{
	QFile ucFile;
	ucFile.setFileName(filePath);
	if (!ucFile.open(QIODevice::ReadOnly))
	{
		emit finishWork(index, -1);//发射失败信号
		return;
	}
	QFile resultFile;
	resultFile.setFileName(savePath);
	if (resultFile.exists() == true)//如果已存在
	{
		if (resultFile.size() != ucFile.size())//使用文件大小简单验证是否为同一首歌, 同名又同大小的歌大概是不存在的吧(我想)
			resultFile.setFileName(savePath.remove(".mp3") + "-" + QString::number(GetTickCount64() % 100000) + ".mp3");
		else
		{
			emit finishWork(index, 304);//发射已存在特征信号
			return;
		}
	}
	if (!resultFile.open(QIODevice::ReadWrite))
	{
		emit finishWork(index, -2);//发射失败信号
		return;
	}
	long long ucFileSize = ucFile.size();//获取缓存文件大小
	emit reProgress(index, ucFileSize, -1);//发送首次数据, 该数据记录总大小
	long long nConvertedSize = 0;
	//long long nConvertGroupBeg = 0;
	//long long nConvertGroupEnd = 0;
	while (nConvertedSize<ucFileSize)
	{
		QByteArray ucBuf;
		QByteArray resultBuf;
		ucFile.seek(nConvertedSize);
		ucBuf = ucFile.read(bufferSize);
		//nConvertGroupEnd = nConvertGroupBeg + bufferSize;
		int nGroupConvertPos = 0;
		while (nGroupConvertPos<ucBuf.size())
		{
			resultBuf += ucBuf.at(nGroupConvertPos) ^ 163;
			nGroupConvertPos+=1;
		}
		resultFile.write(resultBuf);
		nConvertedSize += bufferSize;
		emit reProgress(index, nConvertedSize, 0);//成功完成分块后发射更新进度信号
	}
	resultFile.close();//写入文件并关闭文件
	emit finishWork(index, 200);//发射完成信号
}

//msgBox::msgBox(QDialog* parent)
//{
//	ui_mb.setupUi(this);
//	this->setWindowFlag(Qt::FramelessWindowHint);
//	setWindowStyle();
//
//	connect(ui_mb.btn_closeWindow, &QPushButton::clicked, this, this->close());
//}
//
//void msgBox::setWindowStyle()
//{
//	if (type == 1)
//	{
//		ui_mb.w_topbar->setProperty("style", "information");
//		ui_mb.w_topbar->setStyleSheet(ui_mb.w_topbar->styleSheet());
//		ui_mb.w_title->setProperty("style", "information");
//		ui_mb.w_title->setStyleSheet(ui_mb.w_title->styleSheet());
//	}
//	if (type == 2)
//	{
//		ui_mb.w_topbar->setProperty("style", "warning");
//		ui_mb.w_topbar->setStyleSheet(ui_mb.w_topbar->styleSheet());
//		ui_mb.w_title->setProperty("style", "warning");
//		ui_mb.w_title->setStyleSheet(ui_mb.w_title->styleSheet());
//	}
//	if (type == 3)
//	{
//		ui_mb.w_topbar->setProperty("style", "error");
//		ui_mb.w_topbar->setStyleSheet(ui_mb.w_topbar->styleSheet());
//		ui_mb.w_title->setProperty("style", "error");
//		ui_mb.w_title->setStyleSheet(ui_mb.w_title->styleSheet());
//	}
//}
//
//void msgBox::mouseMoveEvent(QMouseEvent* mouseEvent)//鼠标拖动事件
//{
//	if (mouseEvent->y() < 25 || slideWindowCenterX != -1 || slideWindowCenterY != -1)
//	{
//		if (slideWindowCenterX == -1 || slideWindowCenterY == -1)
//		{
//			slideWindowCenterX = mouseEvent->x();
//			slideWindowCenterY = mouseEvent->y();
//		}
//		if(slideWindowCenterX != -2 && slideWindowCenterY != -2)
//			this->setGeometry(mouseEvent->globalX() - slideWindowCenterX, mouseEvent->globalY() - slideWindowCenterY, this->width(), this->height());
//	}
//	else
//	{
//		slideWindowCenterX = -2;
//		slideWindowCenterY = -2;
//	}
//}
//void msgBox::mouseReleaseEvent(QMouseEvent* mouseEvent)
//{
//	slideWindowCenterX = -1;
//	slideWindowCenterY = -1;
//}