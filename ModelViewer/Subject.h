#pragma once

#include <map>
#include <functional>
#include "sub_token.h"

using namespace std::placeholders;

template <typename... Args>
class subject
{
public:
	void notify(Args... arg)
	{
		for (auto& kv : _map)
		{
			kv.second(arg...);
		}
	}

	sub_token subscribe(std::function<void(Args...)> fn)
	{
		sub_token tk;
		_map[tk] = fn;
		return tk;
	}

	//sub_token subscribe(const function<void(Args...)> fn) const
	//{
	//	sub_token tk;
	//	_map[tk] = fn;
	//	return tk;
	//}

	void unsubscribe(sub_token tk)
	{
		_map.erase(_map.find(tk));
	}

private:
	map<sub_token, std::function<void(Args...)>> _map;
};
