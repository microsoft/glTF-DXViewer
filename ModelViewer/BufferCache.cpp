#include "pch.h"
#include "BufferCache.h"

int BufferDescriptor::Hash()
{
	if (!_hashCalculated)
	{
		hash<BufferDescriptor> bufferHash;
		_hash = bufferHash(*this);
		_hashCalculated = true;
	}
	return _hash;
}