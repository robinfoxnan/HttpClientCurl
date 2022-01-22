#include "ThreadPoolStd.h"




#define DEBUG_PRINT(...)  printf(__VA_ARGS__)
//#define DEBUG_PRINT(...)



using namespace std;
using namespace robin;

std::atomic_bool ThreadPoolStd::bStop = {false};


void ThreadPoolStd::Init(size_t n, size_t m)
{
	nThreads = n;
	if (nThreads < 1)
		nThreads = 1;
	if (m < 2)
		m = 2;
	nSocketsPerThread = m;

	queTasks.clear();
}

void ThreadPoolStd::start(const CmdCallBack& cb)
{
	if (vecThreads.size() > 0)
		return;

	curlWorker = make_shared<HttpClientWorker>();
	
	curlWorker->init(nThreads, nSocketsPerThread);
	curlWorker->setSendCallback(cb);

	bStop = false;
	for (size_t i = 0; i < nThreads; i++)
	{
		vecThreads.push_back(std::thread(ThreadWorkFun, this, i));
		
	}
	vecNums = std::vector<int64_t >(nThreads, 0);
	/*std::string host = "http://localhost:80";
	workerHttp.Init(nThreads, host);*/
}

void ThreadPoolStd::stop()
{
	bStop = true;
	// 
	for (int i = 0; i < vecThreads.size(); i++)
	{
		condEvent.notify_all();
	}
	for (int i = 0; i < vecThreads.size(); i++)
	{
		if (vecThreads[i].joinable())
			vecThreads[i].join();
	}
}

//
size_t ThreadPoolStd::pushToQue(TaskPtr & task)
{
	size_t n = 0;
	// 
	{
		std::lock_guard<mutex> guard(queMutex);
		queTasks.emplace_back(task);
		n = queTasks.size();
	}
	// 
	condEvent.notify_all();

	return n;

}
bool   ThreadPoolStd::getFromQue(TaskPtr & task)
{
	// 
	{
		std::lock_guard<mutex> guard(queMutex);
		if (queTasks.size() < 1)
			return false;

		task = queTasks[0];
		queTasks.pop_front();
		return true;
	}
	return false;
}

// 
size_t ThreadPoolStd::getQueSize()
{
	std::lock_guard<mutex> guard(queMutex);
	return queTasks.size();
}


void ThreadPoolStd::ThreadWorkFun(ThreadPoolStd * lp, size_t  n)
{
	ThreadPoolStd * pool = (ThreadPoolStd * )lp;
	while (bStop != true)
	{
		{
			std::unique_lock<std::mutex> lock(pool->eventMutex);
			//DEBUG_PRINT("thread index = %zu wait \n", n);
			pool->condEvent.wait(lock);
		}	

		
		if (bStop)
		{
			//DEBUG_PRINT("thread index = %zu active and exit \n", n);
			break;
		}
		else
		{
			//DEBUG_PRINT("thread index = %zu active \n", n);
		}

		pool->DoWork(n);
	}
}

void ThreadPoolStd::DoWork(size_t index)
{
	TaskPtr taskptr;
	bool ret;
	while (bStop == false)
	{
		uint64_t idleCount = curlWorker->getIdleSlots(index);
		for (uint64_t i=0; i<idleCount; i++)
		{
			ret = getFromQue(taskptr);
			if (false == ret)
				break;

			curlWorker->setTask(index, taskptr);
		}
		
		curlWorker->loopOnce(index);

		// task queue is empty, and jobs are finished
		if (curlWorker->getIdleSlots(index) == 20 && getQueSize() == 0)
		{
			printf("loopOnce return");
			return;
		}
	}// end of whiles
}
