#pragma once

struct ID3D11DeviceContext2;

class SceneContext
{
public:
	SceneContext(ID3D11DeviceContext3 *context);
	~SceneContext();

	ID3D11DeviceContext3& context() { return *_context; }
	XMFLOAT4X4& model() { return _model; }
	void SetModel(XMFLOAT4X4& model) { _model = model; }

private:
	ID3D11DeviceContext3 *_context;
	XMFLOAT4X4 _model;
};

