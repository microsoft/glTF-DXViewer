#include "stdafx.h"
#include "GLTF_Parser.h"
#include <fstream>

enum _SHGDNF
{
	SHGDN_NORMAL = 0,
	SHGDN_INFOLDER = 0x1,
	SHGDN_FOREDITING = 0x1000,
	SHGDN_FORADDRESSBAR = 0x4000,
	SHGDN_FORPARSING = 0x8000
};
typedef DWORD SHGDNF;

#include "Windowsstoragecom.h"
#include <fcntl.h>
#include <corecrt_io.h>
#include <fstream>
#include <wrl.h>

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		// Set a breakpoint on this line to catch Win32 API errors.
		throw Platform::Exception::CreateException(hr);
	}
}

using namespace Platform;
using namespace WinRTGLTFParser;
using namespace Microsoft::WRL;

GLTF_Parser::GLTF_Parser()
{
}

String^ WinRTGLTFParser::ToStringHat(char* ch)
{
	std::string s_str = std::string(ch);
	std::wstring wid_str = std::wstring(s_str.begin(), s_str.end());
	const wchar_t* w_char = wid_str.c_str();
	String^ p_string = ref new String(w_char);
	return p_string;
}

void GLTF_Parser::ParseFile(StorageFile^ storageFile)
{
	// Convert the StorageFile into an istream..
	// try to get an IStorageItemHandleAccess interface from the StorageFile 
	ComPtr<IUnknown> unknown(reinterpret_cast<IUnknown*>(storageFile));
	ComPtr<IStorageItemHandleAccess> fileAccessor;
	ThrowIfFailed(unknown.As(&fileAccessor));

	shared_ptr<void> fileHandle;
	HANDLE file = nullptr;
	ThrowIfFailed(fileAccessor->Create(HANDLE_ACCESS_OPTIONS::HAO_READ,
		HANDLE_SHARING_OPTIONS::HSO_SHARE_NONE,
		HANDLE_OPTIONS::HO_RANDOM_ACCESS,
		nullptr,
		&file));

	bool closed = false;
	fileHandle.reset(file, [&closed](HANDLE f) { if (!closed) { CloseHandle(f); closed = true; }});

	int fd = _open_osfhandle((intptr_t)file, _O_RDONLY);
	if (fd == -1)
	{
		throw std::exception("Unable to open file descriptor!");
	}
	unique_ptr<int, function<int(int*)>> osHandle(&fd,
		[&closed](int *fd)
	{
		int ret = -1;
		if (!closed)
		{
			ret = _close(*fd);
			closed = true;
		}
		return ret;
	});

	unique_ptr<FILE, function<int(FILE *)>> fileDescriptor(_fdopen(fd, "r"),
		[&closed](FILE *fp)
	{
		int ret = -1;
		if (!closed)
		{
			ret = fclose(fp);
			closed = true;
		}
		return ret;
	});

	ifstream ifs(fileDescriptor.get());
	auto str = make_shared<istream>(ifs.rdbuf());

	::ParseFile(str, 
		[this](const BufferData& data)
		{
			auto bd = ref new GLTF_BufferData(data);
			OnBufferEvent(this, bd);
		},
		[this](const MaterialData& data)
		{
			auto md = ref new GLTF_MaterialData(data);
			OnMaterialEvent(this, md);
		},
		[this](const TextureData& data)
		{
			auto td = ref new GLTF_TextureData(data);
			OnTextureEvent(this, td);
		},
		[this](const TransformData& data)
		{
			auto td = ref new GLTF_TransformData(data);
			OnTransformEvent(this, td);
		},
		[this](const SceneNodeData& data)
		{
			auto td = ref new GLTF_SceneNodeData(data);
			OnSceneNodeEvent(this, td);
		});
}
