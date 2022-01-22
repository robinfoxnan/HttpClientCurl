#include "CurlConnection.h"
#include <cassert>

#define  SKIP_PEER_VERIFICATION 1  

static CurlInit  curlInitInstance;
//#define  SKIP_HOSTNAME_VERFICATION 1  

/*
ptr��ָ��洢���ݵ�ָ�룬
size��ÿ����Ĵ�С��
nmemb��ָ�����Ŀ��
stream���û�������
���Ը���������Щ��������Ϣ����֪����ptr�е����ݵ��ܳ�����size*nmemb
*/
size_t call_wirte_func(const char *ptr, size_t size, size_t nmemb, std::string *stream)
{
	assert(stream != NULL);
	size_t len = size * nmemb;
	stream->append(ptr, len);
	return len;
}
// ����http header�ص�����    
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
	// �ٷ����ص�DLL����֧��GZIP��Accept-Encoding:deflate, gzip  
	curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip, deflate");
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);//������Ϣ��  

	//https ����ר�ã�start  
#ifdef SKIP_PEER_VERIFICATION  
		//����������SSL��֤����ʹ��CA֤��  
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	//���������SSL��֤�����ָ��һ��CA֤��Ŀ¼  
	//curl_easy_setopt(curl, CURLOPT_CAPATH, "this is ca ceat");  
#endif  

#ifdef SKIP_HOSTNAME_VERFICATION  
		//��֤�������˷��͵�֤�飬Ĭ���� 2(��)��1���У���0�����ã�  
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif  
	//https ����ר�ã�end  


	//����cookieֵ��������  
	//curl_easy_setopt(curl, CURLOPT_COOKIE, "name1=var1; name2=var2;");   
	/* �������ͨ�Ž���cookie��Ĭ�����ڴ��У������ǲ����ڴ����е��ļ������� */
	//curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "./cookie.txt");
	/* ����CURL�����������cookie�������ͷ��ڴ��д������ļ� */
	//curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "./cookie.txt");

	/* POST ���� */
	// curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");  
	//�����ض����������  
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
	//����301��302��ת����location  
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	//ץȡ���ݺ󣬻ص�����  
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_wirte_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
	//ץȡͷ��Ϣ���ص�����  
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

// ��������CURLOPT_POST����CURLOPT_POSTFIELDS��Ĭ��ʹ��GET��ʽ
int CurlConnection::getUrl(const string& url)
{
	if (curl == nullptr)
		return false;

	body = "";
	headerStr = "";
	status = 0;
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

	/*
	CURLE_OK    �������һ�ж���
	CURLE_UNSUPPORTED_PROTOCOL  ��֧�ֵ�Э�飬��URL��ͷ��ָ��
	CURLE_COULDNT_CONNECT   �������ӵ�remote �������ߴ���
	CURLE_REMOTE_ACCESS_DENIED  ���ʱ��ܾ�
	CURLE_HTTP_RETURNED_ERROR   Http���ش���
	CURLE_READ_ERROR    �������ļ�����
	CURLE_SSL_CACERT    ����HTTPSʱ��ҪCA֤��·��
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
	// ��������ʱʱ��
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);
	// ����������
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);//��ӡ������Ϣ

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