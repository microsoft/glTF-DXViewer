#pragma once

#include <wrl.h>
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#include <d2d1effects_2.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <memory>
#include <agile.h>
#include <concrt.h>
#include <collection.h>
#include "App.xaml.h"
#include "Utility.h"

#include <future>
#include <experimental/resumable>
#include <pplawait.h>
#include <map>
#include <string>
#include <memory>
#include "./Common/DirectXHelper.h"

#include "ViewModels/RootPageViewModel.h"
#include "ViewModels/DirectXPageViewModel.h"
#include "ViewModels/ConnectPageViewModel.h"
#include "GlyphConverter.h"
#include "BooleanToVisibilityConverter.h"
#include "FileSystemData.h"
#include "DecimalPlacesConverter.h"