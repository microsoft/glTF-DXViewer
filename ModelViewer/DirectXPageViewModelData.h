#pragma once
#include <boost\signals2\signal.hpp>

using namespace std;
namespace signals = boost::signals2;

class DirectXPageViewModelData
{
public:
	DirectXPageViewModelData();

	signals::connection RegisterForUpdates(signals::signal<void(DirectXPageViewModelData const& data)>::slot_type slot)
	{
		return DataChanged.connect(slot);
	}

	void UnregisterForUpdates(signals::connection conn)
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
		DataChanged(*this);
	}

	float LightPitch() const { return _lightPitch; }
	void SetLightPitch(float lp)
	{
		if (lp == _lightPitch)
			return;
		_lightPitch = lp;
		DataChanged(*this);
	}

	float LightRotation() const { return _lightRotation; }
	void SetLightRotation(float lr)
	{
		if (lr == _lightRotation)
			return;
		_lightRotation = lr;
		DataChanged(*this);
	}

	float LightScale() const { return _lightScale; }
	void SetLightScale(float ls)
	{
		if (ls == _lightScale)
			return;
		_lightScale = ls;
		DataChanged(*this);
	}

	float Ibl() const { return _ibl; }
	void SetIbl(float i)
	{
		if (i == _ibl)
			return;
		_ibl = i;
		DataChanged(*this);
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
		DataChanged(*this);
	}

	bool Metallic() const { return _metallic; }
	void SetMetallic(bool m)
	{
		if (m == _metallic)
			return;
		_metallic = m;
		DataChanged(*this);
	}

	bool Roughness() const { return _roughness; }
	void SetRoughness(bool r)
	{
		if (r == _roughness)
			return;
		_roughness = r;
		DataChanged(*this);
	}

	bool BaseColour() const { return _baseColour; }
	void SetBaseColour(bool bc)
	{
		if (bc == _baseColour)
			return;
		_baseColour = bc;
		DataChanged(*this);
	}

	bool Diffuse() const { return _diffuse; }
	void SetDiffuse(bool d)
	{
		if (d == _diffuse)
			return;
		_diffuse = d;
		DataChanged(*this);
	}

	bool Specular() const { return _specular; }
	void SetSpecular(bool s)
	{
		if (s == _specular)
			return;
		_specular = s;
		DataChanged(*this);
	}

	bool F() const { return _f; }
	void SetF(bool f)
	{
		if (f == _f)
			return;
		_f = f;
		DataChanged(*this);
	}

	bool G() const { return _g; }
	void SetG(bool g)
	{
		if (g == _g)
			return;
		_g = g;
		DataChanged(*this);
	}

	bool D()const { return _d; }
	void SetD(bool d)
	{
		if (d == _d)
			return;
		_d = d;
		DataChanged(*this);
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

	signals::signal<void(DirectXPageViewModelData const& data)> DataChanged;
};


