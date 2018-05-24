#pragma once

#include "Subject.h"

using namespace std;

class DirectXPageViewModelData
{
public:
	DirectXPageViewModelData();

	sub_token RegisterForUpdates(function<void(DirectXPageViewModelData&)> slot)
	{
		return DataChanged.subscribe(slot);
	}

	void UnregisterForUpdates(sub_token conn)
	{
		conn.disconnect();
	}

	const float *LightDirection() const { return _lightDirection; }
	void SetLightDirection(float ld[3])
	{
		if (ld[0] == _lightDirection[0] && 
			ld[0] == _lightDirection[1] &&
			ld[0] == _lightDirection[2])
			return;
		_lightDirection[0] = ld[0];
		_lightDirection[1] = ld[1];
		_lightDirection[2] = ld[2];
		DataChanged.notify(*this);
	}

	float LightPitch() const { return _lightPitch; }
	void SetLightPitch(float lp)
	{
		if (lp == _lightPitch)
			return;
		_lightPitch = lp;
		DataChanged.notify(*this);
	}

	float LightRotation() const { return _lightRotation; }
	void SetLightRotation(float lr)
	{
		if (lr == _lightRotation)
			return;
		_lightRotation = lr;
		DataChanged.notify(*this);
	}

	float LightScale() const { return _lightScale; }
	void SetLightScale(float ls)
	{
		if (ls == _lightScale)
			return;
		_lightScale = ls;
		DataChanged.notify(*this);
	}

	float Ibl() const { return _ibl; }
	void SetIbl(float i)
	{
		if (i == _ibl)
			return;
		_ibl = i;
		DataChanged.notify(*this);
	}

	const unsigned char *LightColour() const { return _lightColour; }
	void SetLightColour(unsigned char lc[3])
	{
		if (lc[0] == _lightColour[0] &&
			lc[0] == _lightColour[1] &&
			lc[0] == _lightColour[2])
			return;
		_lightColour[0] = lc[0];
		_lightColour[1] = lc[1];
		_lightColour[2] = lc[2];
		DataChanged.notify(*this);
	}

	bool Metallic() const { return _metallic; }
	void SetMetallic(bool m)
	{
		if (m == _metallic)
			return;
		_metallic = m;
		DataChanged.notify(*this);
	}

	bool Roughness() const { return _roughness; }
	void SetRoughness(bool r)
	{
		if (r == _roughness)
			return;
		_roughness = r;
		DataChanged.notify(*this);
	}

	bool BaseColour() const { return _baseColour; }
	void SetBaseColour(bool bc)
	{
		if (bc == _baseColour)
			return;
		_baseColour = bc;
		DataChanged.notify(*this);
	}

	bool Diffuse() const { return _diffuse; }
	void SetDiffuse(bool d)
	{
		if (d == _diffuse)
			return;
		_diffuse = d;
		DataChanged.notify(*this);
	}

	bool Specular() const { return _specular; }
	void SetSpecular(bool s)
	{
		if (s == _specular)
			return;
		_specular = s;
		DataChanged.notify(*this);
	}

	bool F() const { return _f; }
	void SetF(bool f)
	{
		if (f == _f)
			return;
		_f = f;
		DataChanged.notify(*this);
	}

	bool G() const { return _g; }
	void SetG(bool g)
	{
		if (g == _g)
			return;
		_g = g;
		DataChanged.notify(*this);
	}

	bool D()const { return _d; }
	void SetD(bool d)
	{
		if (d == _d)
			return;
		_d = d;
		DataChanged.notify(*this);
	}

private:
	float _lightDirection[3] = { 0.5f, 0.5f, 0.0f };

	float _lightPitch = 0.0f;
	float _lightRotation = 0.0f;
	float _lightScale = 0.0f;
	float _ibl = 1.0f;
	unsigned char _lightColour[3] = {255, 255, 255};

	bool _metallic = false;
	bool _roughness = false;
	bool _baseColour = false;
	bool _diffuse = false;
	bool _specular = false;
	bool _f = false;
	bool _g = false;
	bool _d = false;

	subject<DirectXPageViewModelData&> DataChanged;
};


