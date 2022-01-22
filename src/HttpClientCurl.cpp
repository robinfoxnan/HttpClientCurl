#include "HttpClientCurl.h"

void HttpClientCurl::init(size_t n)
{
	if (n < 1)
		n = 1;
	if (n > 1024)
		n = 1024;

	nConnections = n;
	if (vecCurl.size() == 0)
	{
		for (size_t i = 0; i < n; i++)
		{
			CurlConnection * conn = new CurlConnection();
			conn->init();
			size_t index = (size_t)vecCurl.size();
			conn->setIndex(index);
			mapCurl[conn->getHandle()] = index;
			idleQue.emplace_back(index);
			vecCurl.emplace_back(std::unique_ptr<CurlConnection>(conn));
		}

	}
}

bool HttpClientCurl::loop()
{
	int still_running = 0;
	int n = 0;
	do
	{
		tryLoadTask();

		CURLMcode code = curl_multi_perform(cm, &still_running);

		if ((CURLMcode::CURLM_OK != code))
		{
			return false;
		}
		//int m = nConnections - idleQue.size();
		//if (still_running < m)
		checkFinish();

		/* wait for activity, timeout or "nothing" */
		while (n == 0)
		{
			//code = curl_multi_poll(cm, NULL, 0, 0, &n);
			code = curl_multi_wait(cm, NULL, 0, 0, &n);
			if (code != CURLcode::CURLE_OK)
			{
				fprintf(stderr, "curl_multi_poll() failed, code %d.\n", (int)code);
				return false;
			}

			if (getIdleCount() > 0)
				break;
		}

		checkFinish();


		/* if there are still transfers, loop! */
	} while (still_running);

	return true;
}

int HttpClientCurl::checkFinish()
{
	CURLMsg *msg = NULL;
	int msgs_left = 0;
	int count = 0;
	do
	{
		msg = curl_multi_info_read(cm, &msgs_left);
		if (msg && (msg->msg == CURLMSG_DONE))
		{
			count++;

			CURL *curl = msg->easy_handle;
			handleFinished(curl);
		}
	} while (msg);

	return count;
}

void HttpClientCurl::handleFinished(CURL * curl)
{
	size_t index = mapCurl[curl];
	CurlConnection * conn = vecCurl[index].get();
	if (sendCallback)
	{
		// 由用户处理失败的情况
		sendCallback(conn->getTask(), conn);
	}
	conn->reset();

	std::lock_guard<mutex> guard(mutexQue);
	if (taskQue.size() > 0)
	{
		conn->setTask(taskQue[0]);
		taskQue.pop_front();
		curl_multi_remove_handle(cm, curl);
		curl_multi_add_handle(cm, curl);
	}
	else
	{
		curl_multi_remove_handle(cm, curl);
		idleQue.push_back(conn->getIndex());
	}
}

bool HttpClientCurl::tryLoadTask()
{
	std::lock_guard<mutex> guard(mutexQue);
	if (idleQue.size() == 0)
	{
		return false;
	}

	if (taskQue.size() == 0)
		return false;

	size_t index = idleQue[0];
	idleQue.pop_front();
	vecCurl[index]->setTask(taskQue[0]);
	taskQue.pop_front();

	curl_multi_add_handle(cm, vecCurl[index]->getHandle());

	return true;
}


size_t HttpClientCurl::pushTask(const std::shared_ptr<ITask> & task)
{
	std::lock_guard<mutex> guard(mutexQue);
	taskQue.emplace_back(task);
	return taskQue.size();
}
//////////////////////////////////////////////////////////////////////
// using in thread pool
bool HttpClientCurl::setTask(const std::shared_ptr<ITask> & task)
{
	std::lock_guard<mutex> guard(mutexQue);
	if (idleQue.size() == 0)
	{
		return false;
	}

	size_t index = idleQue[0];
	idleQue.pop_front();
	vecCurl[index]->setTask(task);

	curl_multi_add_handle(cm, vecCurl[index]->getHandle());

	return true;
}

bool HttpClientCurl::loopOnce()
{
	int still_running = 0;
	int n = 0;


	CURLMcode code = curl_multi_perform(cm, &still_running);

	if ((CURLMcode::CURLM_OK != code))
	{
		return false;
	}
	//int m = nConnections - idleQue.size();
	//if (still_running < m)
	checkFinishOnce();

	/* wait for activity, timeout or "nothing" */
	while (n == 0)
	{
		//code = curl_multi_poll(cm, NULL, 0, 0, &n);
		code = curl_multi_wait(cm, NULL, 0, 0, &n);
		if (code != CURLcode::CURLE_OK)
		{
			fprintf(stderr, "curl_multi_poll() failed, code %d.\n", (int)code);
			return false;
		}
		if (getIdleCount() > 0)
			break;
	}

	checkFinishOnce();


	return true;
}

int HttpClientCurl::checkFinishOnce()
{
	CURLMsg *msg = NULL;
	int msgs_left = 0;
	int count = 0;
	do
	{
		msg = curl_multi_info_read(cm, &msgs_left);
		if (msg && (msg->msg == CURLMSG_DONE))
		{
			count++;

			CURL *curl = msg->easy_handle;
			handleFinishedOnce(curl);
		}
	} while (msg);

	return count;
}

void HttpClientCurl::handleFinishedOnce(CURL * curl)
{
	size_t index = mapCurl[curl];
	CurlConnection * conn = vecCurl[index].get();
	if (sendCallback)
	{
		// 由用户处理失败的情况
		sendCallback(conn->getTask(), conn);
	}
	conn->reset();

	std::lock_guard<mutex> guard(mutexQue);
	curl_multi_remove_handle(cm, curl);
	idleQue.push_back(conn->getIndex());
}