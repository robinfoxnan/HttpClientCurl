#pragma once
#include <string>
#include <sstream>
#include <curl/curl.h>  
#include "ITask.h"

using namespace std;

#pragma comment(lib, "libcurl.lib") 
#pragma comment(lib, "json-c.lib") 

class CurlInit
{
public:
	CurlInit()
	{
		curl_global_init(CURL_GLOBAL_ALL);
	}
	~CurlInit()
	{
		curl_global_cleanup();
	}
};


class CurlConnection
{
public:
	CurlConnection();
	~CurlConnection();
	void init();
	void appendHeaderLine(const char  *ptr, size_t size);

	void addHeader(const string& key, const string& value);
	int  getUrl(const string& url);
	int  postUrl(const string& url, const string & data);

	inline string& getBody()
	{
		return body;
	}

	inline string& getHeader()
	{
		return headerStr;
	}

	inline int getCode()
	{
		return status;
	}
	inline CURL * getHandle()
	{
		return curl;
	}

	inline void reset()
	{
		code = CURLE_OK;
		body = "";
		headerStr = "";
		status = 0;
		curTask = nullptr;
	}

	void setTask(const std::shared_ptr<ITask> & task);
	inline ITask* getTask() { if (curTask) return curTask.get(); return nullptr; }

	
	inline size_t getIndex() { return this->indexInVec;  }

protected:
	friend class HttpClientCurl;
	inline void setIndex(size_t n) { this->indexInVec = n; }
private:
	std::string body;
	std::string headerStr;

	size_t indexInVec;

	std::shared_ptr<ITask> curTask;

	CURL *curl = nullptr;
	struct curl_slist *headerList;
	CURLcode code = CURLE_OK;
	int status;    // http response code

	std::string useragent = "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:13.0) Gecko/20100101 Firefox/13.0.1";
private:
	CurlConnection(const CurlConnection &) = delete;
	CurlConnection operator=(const CurlConnection &) = delete;
};

