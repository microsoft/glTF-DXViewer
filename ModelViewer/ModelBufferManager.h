#pragma once
#include "BufferCache.h"

class ModelBufferManager : public Singleton<ModelBufferManager>
{
	friend class Singleton<ModelBufferManager>;

public:
	BufferCache<ID3D11BufferWrapper> *CurrentBufferCache()
	{
		if (_currBufferCache == nullptr)
		{
			_currBufferCache = make_unique<BufferCache<ID3D11BufferWrapper>>();
		}
		return _currBufferCache.get();
	}

protected:
	ModelBufferManager();

private:
	unique_ptr<BufferCache<ID3D11BufferWrapper>> _currBufferCache = nullptr;
};

