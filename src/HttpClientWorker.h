#pragma once
#include "ITask.h"
#include "IWork.h"
#include "HttpClientCurl.h"
#include <assert.h>


namespace robin
{
	class HttpClientWorker : public IWorker
	{
	public:
		HttpClientWorker()  
		{
			name = "HttpClientWorkerCurl";
		}

		virtual ~HttpClientWorker()
		{
			vecClient.clear();
		}

		void init(size_t nThread, size_t nSocksPerThread)
		{
			for (size_t i=0; i<nThread; i++)
			{
				vecClient.emplace_back(std::make_shared<HttpClientCurl>());
				vecClient[i]->init(nSocksPerThread);
			}
		}

		void setSendCallback(const CmdCallBack& cb)
		{
			for (size_t i = 0; i < vecClient.size(); i++)
			{
				vecClient[i]->setSendCallback(cb);
			}
		}

	public:
		virtual size_t getIdleSlots(size_t index) override
		{
			assert(index < vecClient.size());
			return vecClient[index]->getIdleCount();
		}
		virtual bool setTask(size_t index, TaskPtr & task)  override
		{
			assert(index < vecClient.size());
			return vecClient[index]->setTask(task);
		}
		virtual bool loopOnce(size_t index)  override
		{
			assert(index < vecClient.size());
			return vecClient[index]->loopOnce();
		}
	private:
		std::vector<std::shared_ptr<HttpClientCurl> > vecClient;
	};
}



