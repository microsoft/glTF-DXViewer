#include "pch.h"
#include "ShaderCache.h"

int ShaderDescriptor::Hash()
{
	if (!_hashCalculated)
	{
		hash<ShaderDescriptor> shaderHash;
		_hash = shaderHash(*this);
	}
	return _hash;
}