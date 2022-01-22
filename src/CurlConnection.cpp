#include "CurlConnection.h"
#include <cassert>

#define  SKIP_PEER_VERIFICATION 1  

static CurlInit  curlInitInstance;
//#define  SKIP_HOSTNAME_VERFICATION 1  

/*
ptr是指向存储数据的指针，
size是每个块的大小，
nmemb是指块的数目，
stream是用户参数。
所以根据以上这些参数的信息可以知道，ptr中的数据的总长度是size*nmemb
*/
size_t call_wirte_func(const char *ptr, size_t size, size_t nmemb, std::string *stream)
{
	assert(stream != NULL);
	size_t len = size * nmemb;
	stream->append(ptr, len);
	return len;
}
// 返回http header回调函数    
size_t header_callback(const char  *ptr, size_t size, size_t nmemb, CurlConnection * conn)
{
	assert(conn != NULL);
	size_t len = size * nmemb;
	conn->appendHeaderLine(ptr, len);


	/*string str;
	str.assign(ptr, len);
	printf("%s\n", str.c_str());*/
	return len;
}


CurlConnection::CurlConnection(): headerList(nullptr)
{
	curl = curl_easy_init();
}
CurlConnection::~CurlConnection()
{
	if (curl != nullptr)
	{
		curl_easy_cleanup(curl);
		curl = nullptr;
	}

	if (headerList) //last free the header list
		curl_slist_free_all(headerList); /* free the header list */

}

void CurlConnection::appendHeaderLine(const char  *ptr, size_t size)
{
	if (status == 0)
	{
		CURLcode ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	}
	headerStr.append(ptr, size);
}
void CurlConnection::addHeader(const string& key, const string& value)
{
	
	//headers = curl_slist_append(headers, "Content-Type: text/xml");
	//headers = curl_slist_append(headers, "Accept: text/html, */*;q=0.01");
	//...
	string str = key + ":" + value;
	headerList = curl_slist_append(headerList, str.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

	
}

void CurlConnection::init()
{
	if (curl == nullptr)
		return ;


	curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
	// 官方下载的DLL并不支持GZIP，Accept-Encoding:deflate, gzip  
	curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip, deflate");
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);//调试信息打开  

	//https 访问专用：start  
#ifdef SKIP_PEER_VERIFICATION  
		//跳过服务器SSL验证，不使用CA证书  
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	//如果不跳过SSL验证，则可指定一个CA证书目录  
	//curl_easy_setopt(curl, CURLOPT_CAPATH, "this is ca ceat");  
#endif  

#ifdef SKIP_HOSTNAME_VERFICATION  
		//验证服务器端发送的证书，默认是 2(高)，1（中），0（禁用）  
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif  
	//https 访问专用：end  


	//发送cookie值给服务器  
	//curl_easy_setopt(curl, CURLOPT_COOKIE, "name1=var1; name2=var2;");   
	/* 与服务器通信交互cookie，默认在内存中，可以是不存在磁盘中的文件或留空 */
	//curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "./cookie.txt");
	/* 与多个CURL或浏览器交互cookie，会在释放内存后写入磁盘文件 */
	//curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "./cookie.txt");

	/* POST 数据 */
	// curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");  
	//设置重定向的最大次数  
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
	//设置301、302跳转跟随location  
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	//抓取内容后，回调函数  
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_wirte_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
	//抓取头信息，回调函数  
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

	/* enable TCP keep-alive for this transfer */
	curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

	/* keep-alive idle time to 120 seconds */
	curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);

	/* interval time between keep-alive probes: 6 seconds */
	curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 6L);

	//addHeader("Content-Type", "text/plain");
	addHeader("Content-Type", "application/json");
	
}

// 当不调用CURLOPT_POST，和CURLOPT_POSTFIELDS，默认使用GET方式
int CurlConnection::getUrl(const string& url)
{
	if (curl == nullptr)
		return false;

	body = "";
	headerStr = "";
	status = 0;
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

	/*
	CURLE_OK    任务完成一切都好
	CURLE_UNSUPPORTED_PROTOCOL  不支持的协议，由URL的头部指定
	CURLE_COULDNT_CONNECT   不能连接到remote 主机或者代理
	CURLE_REMOTE_ACCESS_DENIED  访问被拒绝
	CURLE_HTTP_RETURNED_ERROR   Http返回错误
	CURLE_READ_ERROR    读本地文件错误
	CURLE_SSL_CACERT    访问HTTPS时需要CA证书路径
	*/
	code = curl_easy_perform(curl);
	if (CURLE_OK == code)
	{
		if (status == 0)
		{
			CURLcode ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
		}
	}

	return code;

}

int  CurlConnection::postUrl(const string& url, const string & data)
{
	int timeout = 3000;
	status = 0;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	// 设置请求超时时间
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);
	// 设置请求体
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);//打印调试信息

	code = curl_easy_perform(curl);
	if (CURLE_OK == code)
	{
		if (status == 0)
		{
			CURLcode ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
		}
	}

	return 0;
}

void CurlConnection::setTask(const std::shared_ptr<ITask> & task)
{ 
	reset();
	this->curTask = task; 
	curl_easy_setopt(curl, CURLOPT_URL, task->url.c_str());
	if (task->isGet == false)
	{
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, task->body.c_str());
		curl_easy_setopt(curl, CURLOPT_POST, 1);
	}
	
}