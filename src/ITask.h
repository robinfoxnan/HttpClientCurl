
#pragma once
#include <string>
#include <inttypes.h>
#include <memory>

using namespace  std;
class ITask
{
public:
	int64_t id;
	string url;
	string body;
	bool isGet;
};
using TaskPtr = std::shared_ptr<ITask>;

