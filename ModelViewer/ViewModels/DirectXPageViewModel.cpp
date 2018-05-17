#include "pch.h"
#include "DirectXPageViewModel.h"

using namespace ViewModels;

DirectXPageViewModel::DirectXPageViewModel() :
	_proxy(this)
{
	_data = Container::Instance().ResolveDirectXPageViewModelData();
}

void DirectXPageViewModel::NotifySelectionChanged(shared_ptr<GraphNode> node)
{
	SelectedTransform = ref new TransformViewModel(node);
}

float DirectXPageViewModel::LightScale::get()
{
	return _data->LightScale();
}

void DirectXPageViewModel::LightScale::set(float val)
{
	if (_data->LightScale() == val)
		return;
	OnPropertyChanged(getCallerName(__FUNCTION__));
	_data->SetLightScale(val);
}

float DirectXPageViewModel::LightRotation::get()
{
	return _data->LightRotation();
}

void DirectXPageViewModel::LightRotation::set(float val)
{
	if (_data->LightRotation() == val)
		return;
	_data->SetLightRotation(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
	_data->SetLightDirection(ConvertDirection(LightRotation, LightPitch,
		const_cast<float *>(_data->LightDirection())));
}

float DirectXPageViewModel::LightPitch::get()
{
	return _data->LightPitch();
}

void DirectXPageViewModel::LightPitch::set(float val)
{
	if (_data->LightPitch() == val)
		return;
	_data->SetLightPitch(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
	_data->SetLightDirection(ConvertDirection(LightRotation, LightPitch,
		const_cast<float *>(_data->LightDirection())));
}

float DirectXPageViewModel::Ibl::get()
{
	return _data->Ibl();
}

void DirectXPageViewModel::Ibl::set(float val)
{
	if (_data->Ibl() == val)
		return;
	_data->SetIbl(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

Color DirectXPageViewModel::LightColour::get()
{
	return ConvertColor(_data->LightColour());
}

void DirectXPageViewModel::LightColour::set(Color val)
{
	if (_data->LightColour()[0] == val.R &&
		_data->LightColour()[1] == val.G &&
		_data->LightColour()[2] == val.B)
		return;
	unsigned char col[3] = { val.R, val.G, val.B };
	_data->SetLightColour(col);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

bool DirectXPageViewModel::BaseColour::get()
{
	return _data->BaseColour();
}

void DirectXPageViewModel::BaseColour::set(bool val)
{
	if (_data->BaseColour() == val)
		return;
	_data->SetBaseColour(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

bool DirectXPageViewModel::Metallic::get()
{
	return _data->Metallic();
}

void DirectXPageViewModel::Metallic::set(bool val)
{
	if (_data->Metallic() == val)
		return;
	_data->SetMetallic(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

bool DirectXPageViewModel::Roughness::get()
{
	return _data->Roughness();
}

void DirectXPageViewModel::Roughness::set(bool val)
{
	if (_data->Roughness() == val)
		return;
	_data->SetRoughness(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

bool DirectXPageViewModel::Diffuse::get()
{
	return _data->Diffuse();
}

void DirectXPageViewModel::Diffuse::set(bool val)
{
	if (_data->Diffuse() == val)
		return;
	_data->SetDiffuse(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

bool DirectXPageViewModel::Specular::get()
{
	return _data->Specular();
}

void DirectXPageViewModel::Specular::set(bool val)
{
	if (_data->Specular() == val)
		return;
	_data->SetSpecular(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

bool DirectXPageViewModel::F::get()
{
	return _data->F();
}

void DirectXPageViewModel::F::set(bool val)
{
	if (_data->F() == val)
		return;
	_data->SetF(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

bool DirectXPageViewModel::G::get()
{
	return _data->G();
}

void DirectXPageViewModel::G::set(bool val)
{
	if (_data->G() == val)
		return;
	_data->SetG(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

bool DirectXPageViewModel::D::get()
{
	return _data->D();
}

void DirectXPageViewModel::D::set(bool val)
{
	if (_data->D() == val)
		return;
	_data->SetD(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

TransformViewModel^ DirectXPageViewModel::SelectedTransform::get()
{
	return _selectedTransform;
}

void DirectXPageViewModel::SelectedTransform::set(TransformViewModel^ val)
{
	if (_selectedTransform == val)
		return;
	_selectedTransform = val;
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

Color DirectXPageViewModel::ConvertColor(const unsigned char col[3])
{
	auto ret = new Color();
	ret->R = col[0];
	ret->G = col[1];
	ret->B = col[2];
	return *ret;
}

float *DirectXPageViewModel::ConvertDirection(float rotation, float pitch, float *data)
{
	auto rot = rotation * M_PI / 180;
	auto ptch = pitch * M_PI / 180;
	data[0] = static_cast<float>(sin(rot) * cos(ptch));
	data[1] = static_cast<float>(sin(ptch));
	data[2] = static_cast<float>(cos(rot) * cos(ptch));
	return data;
}
