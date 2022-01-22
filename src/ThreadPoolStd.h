#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <deque>
#include <iostream>
#include <condition_variable>
#include <chrono>
#include <atomic>

//#include "CommitDataHttp.h"
#include "ITask.h"
#include "HttpClientWorker.h"
using namespace std;

namespace robin
{

	class ThreadPoolStd
	{
	public:
		static ThreadPoolStd * Instance()
		{
			static ThreadPoolStd pool;
			return &pool;
		}

		// 线程的工作函数
		static void ThreadWorkFun(ThreadPoolStd * lp, size_t  n);
		void DoWork(size_t  index);

		void Init(size_t n, size_t m);
		void start(const CmdCallBack& cb);
		void stop();

		// 将任务压入队列
		size_t pushToQue(TaskPtr & task);
		bool   getFromQue(TaskPtr & task);
		
		// 获取当前队列大小
		size_t getQueSize();
		inline void incCount(size_t index)
		{
			vecNums[index] ++;
		}
		inline int64_t getNum(size_t index)
		{
			if (index >= vecNums.size())
				return 0;
			return vecNums[index];
		}

		inline int64_t getCount()
		{
			int64_t sum = 0;
			for (size_t i=0; i< vecNums.size(); i++)
			{
				sum += vecNums[i];
			}
			return sum;
		}

	private:
		std::vector<std::thread> vecThreads;  // threads
		std::vector<int64_t> vecNums;         // count num

		std::deque<TaskPtr> queTasks;
		std::mutex queMutex;
		size_t nThreads;
		size_t nSocketsPerThread;
		static std::atomic_bool bStop;

		std::mutex  eventMutex;
		std::condition_variable condEvent;
	private:
		//CommitDataHttp  workerHttp;
		std::shared_ptr<HttpClientWorker> curlWorker;
	};
}