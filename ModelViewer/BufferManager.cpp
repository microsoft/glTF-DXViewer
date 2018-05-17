#include "pch.h"
#include "BufferManager.h"

BufferManager::BufferManager() :
	_cbPerObject(0),
	_cbPerFrame(1),
	_mvpBuffer(0)
{
}