#pragma once
#include "DirectXMath.h"

using namespace DirectX;

template<class T>
class BoundingBox
{
public:
	BoundingBox() :
		MinX((numeric_limits<T>::max)()),
		MaxX((numeric_limits<T>::min)()),
		MinY((numeric_limits<T>::max)()),
		MaxY((numeric_limits<T>::min)()),
		MinZ((numeric_limits<T>::max)()),
		MaxZ((numeric_limits<T>::min)())
	{
	}

	XMFLOAT3 Centre()
	{
		XMFLOAT3 ret;
		ret.x = bbox.MinX + (bbox.MaxX - bbox.MinX) * 0.5f;
		ret.y = bbox.MinY + (bbox.MaxY - bbox.MinY) * 0.5f;
		ret.z = bbox.MinZ + (bbox.MaxZ - bbox.MinZ) * 0.5f;
		return ret;
	}

	void Grow(BoundingBox<T>& other)
	{
		MinX = min<T>(MinX, other.MinX);
		MinY = min<T>(MinY, other.MinY);
		MinZ = min<T>(MinZ, other.MinZ);

		MaxX = max<T>(MaxX, other.MaxX);
		MaxY = max<T>(MaxY, other.MaxY);
		MaxZ = max<T>(MaxZ, other.MaxZ);
	}

	static BoundingBox<T> CreateBoundingBoxFromVertexBuffer(void *buffer, size_t bufferSize)
	{
		XMFLOAT3 *buffPtr = (XMFLOAT3 *)buffer;
		BoundingBox<T> bbox;
		for (size_t i = 0; i < bufferSize / (3 * sizeof(float)); i++, buffPtr++)
		{
			bbox.MaxX = max<T>(buffPtr->x, bbox.MaxX);
			bbox.MaxY = max<T>(buffPtr->y, bbox.MaxY);
			bbox.MaxZ = max<T>(buffPtr->z, bbox.MaxZ);
			bbox.MinX = min<T>(buffPtr->x, bbox.MinX);
			bbox.MinY = min<T>(buffPtr->y, bbox.MinY);
			bbox.MinZ = min<T>(buffPtr->z, bbox.MinZ);
		}
		return bbox;
	}

	T MinX;
	T MaxX;
	T MinY;
	T MaxY;
	T MinZ;
	T MaxZ;
};

