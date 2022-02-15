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
	return text.indexOf(indexText, p) + indexText.length();
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
		result.append(artistList.at(nAdd).name + "  ");
		nAdd += 1;
	}
	if (artistList.size() - nAdd > 0)//如果还有更多作者但已经超过长度
		result.append(QString(" 等%1位").arg(artistList.size()));
	return result;
}

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
	std::vector<PLAYLISTINFO> result;
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
		result.push_back(playListInfo);//压入当前子存储
	}
	return result;//返回格式化后的数据
}

std::vector<SONGINFO> searchInfoParser_Songs(QString rawResult)
{
	std::vector<SONGINFO> result;
	int nPos = 0;
	nPos = qText_indexOfEnd(rawResult, "\"songs\":", 0);
	nPos = qText_indexOfEnd(rawResult, "[", nPos);
	while (rawResult.indexOf(",{", nPos) != -1)
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
		/*if (type == 1)
			nPos = qText_indexOfEnd(rawResult, "\"mark\":", nPos);
		else if (type == 2)
			nPos = qText_indexOfEnd(rawResult, "\"volumeDelta\":", nPos);*/
		nPos = qJson_findCurrentEnd(rawResult, nPos);
	}
	return result;//返回格式化后的数据
}

std::vector<SONGINFO> songsIn_PlayList(QString rawPlayListInfo)
{
	//TODO 此处可以增加容错性 正常情况code=200 但经常性服务器繁忙
	std::vector<SONGINFO> result;
	int nPos = 0;
	nPos = qText_indexOfEnd(rawPlayListInfo,"}",0);//定位到第一个括回 跳过创建者及以前的内容
	nPos = qText_indexOfEnd(rawPlayListInfo, "\"tracks\":", nPos);
	while (rawPlayListInfo.indexOf("]",nPos)-nPos!=0)
	{
		nPos += 1;
		SONGINFO songInfo;
		songInfo.isHaveLocal = false;
		songInfo.isLocalOnly = false;
		songInfo.name = qText_Between(rawPlayListInfo, "\"name\":\"", "\",", nPos);
		songInfo.id = qText_Between(rawPlayListInfo, "\"id\":", ",", nPos);
		//TODO 此处代码与searchInfoParser_Songs中重复, 考虑封装一个函数
		nPos = qText_indexOfEnd(rawPlayListInfo, "\"artists\":", nPos);
		std::vector<USERINFO> artists;
		while (rawPlayListInfo.indexOf("]", nPos)-nPos != 0)
		{
			nPos += 1;
			USERINFO artist;
			int subStr_artist_endPos = qText_indexOfEnd(rawPlayListInfo, "}", nPos);
			QString subStr_artist = qText_Between(rawPlayListInfo, "{", "}", nPos);
			artist.name = qText_Between(subStr_artist, "\"name\":\"", "\",", 0);
			artist.id = qText_Between(subStr_artist, "\"id\":", ",", 0);
			artists.push_back(artist);
			nPos = subStr_artist_endPos;
		}
		songInfo.artists = artists;
		//nPos = qText_indexOfEnd(rawPlayListInfo, "\"album\":", nPos);
		//nPos = qText_indexOfEnd(rawPlayListInfo, "}", nPos);//跳过所属专辑->作者名
		//nPos = qText_indexOfEnd(rawPlayListInfo, "\"artists\":[", nPos);
		//nPos = qText_indexOfEnd(rawPlayListInfo, "],", nPos);//跳过所属专辑->作者列表
		//nPos = qText_indexOfEnd(rawPlayListInfo, "}", nPos);//跳过所属专辑(对上面几行全部是为了跳过这个)
		//因为发现全文 "duration": 只有一个所以直接定位到那里, 这个叫隔空tp(bs
		songInfo.duration = qText_Between(rawPlayListInfo, "\"duration\":", ",", nPos);
		nPos = qText_indexOfEnd(rawPlayListInfo, "\"duration\":", nPos);
		songInfo.mvid = qText_Between(rawPlayListInfo, "\"mvid\":", ",", nPos);
		result.push_back(songInfo);
		nPos = qText_indexOfEnd(rawPlayListInfo, "\"mvid\":", nPos);
		nPos = qText_indexOfEnd(rawPlayListInfo, "\"lMusic\":", nPos);
		nPos = qText_indexOfEnd(rawPlayListInfo, "}", nPos);
		nPos = qText_indexOfEnd(rawPlayListInfo, "}", nPos);
	}
	return result;
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
	bufferSize = 1048576;//默认缓存大小1MB
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
	bufferSize = 102400;//默认缓存大小100kb
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