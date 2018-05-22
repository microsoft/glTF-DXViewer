#include "stdafx.h"
#include "GLTF_Parser.h"
#include <fstream>
#include <locale>
#include <codecvt>

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
#include <GLTFSDK\IStreamReader.h>
#include <filesystem>
#include <wrl/event.h>
#include <concrt.h>
#include <ppl.h>
#include <ppltasks.h>

using namespace std::experimental::filesystem::v1;
using namespace Microsoft::glTF;
using namespace Platform;
using namespace WinRTGLTFParser;
using namespace Microsoft::WRL;
using namespace Concurrency;

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		// Set a breakpoint on this line to catch Win32 API errors.
		throw Exception::CreateException(hr);
	}
}

class GLTFStreamReader : public IStreamReader
{
public:
	GLTFStreamReader(shared_ptr<istream> wrapped, const string& baseUri) :
		m_stream(wrapped)
	{
		path pth(baseUri);
		auto buri = pth.remove_filename().c_str();
		auto base = wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(buri);
		auto sep = wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(path::preferred_separator);
		_baseUri = base + sep;
	}

	shared_ptr<istream> GetInputStream(const string& uri) const override
	{
		// Here we need to translate the string passed into a stream and that is where 
		// we can't progress as we would need arbitrary access to the file system...
		// So, would expect this to fail with file system access.
		// Construct the path using the relative uri passed in and the baseUri member
		auto path = _baseUri + uri;
		auto pathwsrtring = wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(path.c_str());
		auto storageFileAction = StorageFile::GetFileFromPathAsync(ref new Platform::String(pathwsrtring.c_str()));

		// Make a blocking call to get hold of the StorageFile
		StorageFile^ storageFile;
		create_task(storageFileAction).then([&storageFile](StorageFile^ file)
		{
			storageFile = file;

		}).wait();

		// Retrieve the IStorageItemHandleAccess interface from the StorageFile
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
		fileHandle.reset(file, [&closed](HANDLE f) { });

		int fd = _open_osfhandle((intptr_t)file, _O_RDONLY);
		if (fd == -1)
		{
			throw std::exception("Unable to open file descriptor!");
		}

		unique_ptr<FILE, function<void(FILE *)>> fileDescriptor(_fdopen(fd, "r"),
			[&closed](FILE *fp)
		{
		});

		auto ifs = make_shared<ifstream>(fileDescriptor.get());
		if (ifs->fail())
		{
			throw exception("failed to open file");
		}
		return ifs;
	}

private:
	shared_ptr<istream> m_stream;
	string _baseUri;
};

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

	// convert from wide string to string...
	auto baseUri = wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(storageFile->Path->Data());

	auto gltfStreamReader = std::make_unique<GLTFStreamReader>(str, baseUri);

	::ParseFile(str, baseUri, *(gltfStreamReader.get()),
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
