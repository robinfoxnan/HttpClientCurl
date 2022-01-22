// multiCurl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "CurlConnection.h"
#include "HttpClientCurl.h"
#include "MyTimer.h"
#include "ThreadPoolStd.h"

using namespace robin;

void test1()
{
	std::cout << "Hello World!\n";

	CurlConnection conn;
	conn.init();

	string body = "12345678910";
	Timer timer;
	timer.start();
	for (int i = 0; i < 1; i++)
	{
		int ret;
		ret = conn.getUrl("http://127.0.0.1/1.php");
		//ret = conn.postUrl("http://127.0.0.1/1.php", body);
		printf("code = %d\n", conn.getCode());

		printf("hreader= %s\n", conn.getHeader().c_str());
		printf("body = %s\n", conn.getBody().c_str());
	}
	double delta = timer.stop_delta_ms();
	printf("cost = %f\n", delta);
}

void onSendEnd(ITask * task, CurlConnection * conn)
{
	if (task == nullptr)
	{
		printf("error meet task * is null\n");
		return;
	}
	printf("id=%lld, index= %lld, code=%d\n", task->id, conn->getIndex(),  conn->getCode());
}


void test2()
{
	HttpClientCurl httpClient;

	httpClient.init(100);


	for (int i = 0; i < 1000; i++)
	{
		std::shared_ptr<ITask> task = std::make_shared<ITask>();
		// 天气预报接口
		task->url = "http://apis.juhe.cn/simpleWeather/query?city=%E8%8B%8F%E5%B7%9E&key=";
		task->body = "123457890";
		task->id = i;
		task->isGet = true;

		httpClient.pushTask(task);
	}
	httpClient.setSendCallback(onSendEnd);

	Timer timer;
	timer.start();

	bool ret = httpClient.loop();
	while (ret && httpClient.getQueLen() > 0)
	{
		ret = httpClient.loop();
	}

	double delta = timer.stop_delta_ms();
	printf("cost = %f\n", delta);
}


void test3()
{

}

int main()
{
	//test1();
	//test2();
}
