#pragma once
#include <vector>

using namespace Microsoft::WRL;

class ImgUtils
{
public:
	static std::vector<unsigned char> LoadRGBAImage(void *imgFileData, size_t imgFileDataSize, uint32_t& width, uint32_t& height, 
		bool jpg = false, const wchar_t *filename = nullptr);
};

