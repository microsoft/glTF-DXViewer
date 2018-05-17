#pragma once

class DXUtils
{
public:
	static HRESULT CompileShader(_In_ LPCWSTR srcFile, const D3D_SHADER_MACRO *defines, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob);
};

