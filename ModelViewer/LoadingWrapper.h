#pragma once
#include <future>

class LoadingWrapper
{
public:
	LoadingWrapper(std::function<void()> ctor, std::function<void()> dtor);

	future<void> Schedule(std::function<void()> fn);

	~LoadingWrapper();

private:
	std::function<void()> _dtor;
};

