#pragma once
#include <memory>
#include <string>
#include "ITask.h"

using namespace std;
namespace robin
{

	class IWorker
	{
	public:
		IWorker() {}
		virtual ~IWorker() {}

		virtual size_t getIdleSlots(size_t index) { return 0;  }
		virtual bool setTask(size_t index, TaskPtr & task) { return false; }
		virtual bool loopOnce(size_t index ) { return false; }

		//virtual void doWork(TaskPtr& task, size_t threadIndex) {}
		//virtual void afterWork(TaskPtr & task) {}

		string getName() { return name;  }

	protected:
		string name = "ITask";

	};
	using WorkPtr = std::shared_ptr<IWorker>;
}
