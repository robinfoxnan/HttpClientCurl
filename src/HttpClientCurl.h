#pragma once
#include <vector>
#include <deque>
#include <mutex>
#include <thread>
#include <map>
#include "CurlConnection.h"



using CmdCallBack = std::function<void(ITask *, CurlConnection *)>;

class HttpClientCurl
{
public:
	HttpClientCurl(): cm(nullptr) 
	{ 
		cm = curl_multi_init(); 
	}

	~HttpClientCurl() 
	{
		if (cm) 
			curl_multi_cleanup(cm);

		vecCurl.clear();
	}

	void init(size_t n);

	size_t pushTask(const std::shared_ptr<ITask> & task);
	void setSendCallback(const CmdCallBack& cb) { this->sendCallback = cb; }
	size_t getQueLen() 
	{
		std::lock_guard<mutex> guard(mutexQue);
		return taskQue.size();
	}
	bool loop();

public:  // used in thread pool
	bool loopOnce();
	size_t getIdleCount()
	{
		std::lock_guard<mutex> guard(mutexQue);
		return idleQue.size();
	}
	bool setTask(const std::shared_ptr<ITask> & task);
	
private:
	bool tryLoadTask();
	int checkFinish();
	void handleFinished(CURL * curl);

	int checkFinishOnce();
	void handleFinishedOnce(CURL * curl);
private:
	CURLM *cm = NULL;
	CURLMsg *msg = NULL;
	CURLcode return_code = CURLcode::CURLE_OK;



	size_t  nConnections;
	using CurlConnectionPtr = std::unique_ptr<CurlConnection>;
	std::vector<CurlConnectionPtr> vecCurl;
	std::map<CURL *, size_t> mapCurl;

	std::deque<size_t> idleQue;
	//////////////////////////////////////////////////////////////////////////
	CmdCallBack sendCallback;
	std::deque<std::shared_ptr<ITask>> taskQue;
	std::mutex mutexQue;
};

