// multiCurl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "../src/CurlConnection.h"
#include "../src/HttpClientCurl.h"
#include "../src/MyTimer.h"
#include "../src/ThreadPoolStd.h"

using namespace robin;



static inline  void sleepForMs(int msecs)
{
#if defined (_WIN32) || defined (_WIN64)
	std::this_thread::sleep_for(std::chrono::milliseconds(msecs));
#else
	static struct timespec req;
	req.tv_sec = msecs / 1000;
	req.tv_nsec = msecs * 1000000 % 1000000000;   // ns
	int ret = nanosleep(&req, NULL);
	//printf("--------nano-----------\n");
#endif
}

int run_id = 0;
std::atomic_bool bRun(true);
ThreadPoolStd threadPool;
int64_t taskId = 0;
using ms = std::ratio<1, 1000>;

// push task to pool
void generateThreadFunc(uint32_t nSpeed, int64_t nPackets)
{
	printf("generate thread start here.\n");


	uint32_t count = 0;
	time_point<high_resolution_clock> tpStart = high_resolution_clock::now();

	while (bRun && nPackets > 0)
	{
		TaskPtr task = std::make_shared<ITask>();
		task->isGet = true;
		task->body = "123456789";
		// 天气预报接口
		//task->url = "http://apis.juhe.cn/simpleWeather/query?city=%E8%8B%8F%E5%B7%9E&key=";
		task->url = "http://127.0.0.1/hi";
		task->id = ++taskId;
		threadPool.pushToQue(task);

		nPackets--;
		count++;


		if (count >= nSpeed)
		{
			count = 0;
			time_point<high_resolution_clock> tpStop = high_resolution_clock::now();
			
			duration<double, ms> tmDelta = duration<double, ms>(tpStop - tpStart);
			int leftMs = 1000 - (int)tmDelta.count();
			if (leftMs > 1)
				sleepForMs(leftMs);
			printf("thread left %lld to send \n",  nPackets);

			tpStart = high_resolution_clock::now();
		}
	}


	printf("generate thread exit ...\n");
}

int64_t sendCount = 0;
void onSendEnd(ITask * task, CurlConnection * conn)
{
	if (task == nullptr)
	{
		printf("error meet task * is null\n");
		return;
	}

	sendCount++;

	//printf("id=%lld, index= %lld, code=%d\n", task->id, conn->getIndex(), conn->getCode());
}

int main()
{
	threadPool.Init(4, 10);
	threadPool.start(onSendEnd);
	int64_t total = 1000000;
	std::thread generateThread = std::thread(generateThreadFunc, 40000, total);
	
	// print speed
	int64_t lastSend = 0;
	while (sendCount < total)
	{
		sleepForMs(1000);
		int speed = sendCount - lastSend;
		lastSend = sendCount;
		printf("==========> speed = %d \n", speed);
	}

	if (generateThread.joinable())
	{
		generateThread.join();
	}
	threadPool.stop();

}
