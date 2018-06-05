#include "pch.h"
#include "ShaderCache.h"

size_t ShaderDescriptor::Hash()
{
	if (!_hashCalculated)
	{
		hash<ShaderDescriptor> shaderHash;
		_hash = shaderHash(*this);
	}
	return _hash;
}