# DirectX GLTF Viewer Sample
This project was motivated by a lack of sample code demonstrating the graphics API agnostic nature of the [glTF specification](https://www.google.com). The sample code is written using modern C++, DirectX 11 and the Universal Windows Platform (UWP) for the client application. The client application could have been written using any application development platform that supports DirectX 11 rendering. This sample is a port of the [Khronos PBR WebGL Sample](https://github.com/KhronosGroup/glTF-WebGL-PBR) and supports the same feature set.

![Main Sample App Screenshot](https://raw.github.com/peted70/dx-gltf-viewer/master/img/screenshot1.PNG)

The screenshot is showing the DamagedHelmet sample file  being rendered in the scene window, some controls to the right to adjust transforms and a Tree View control with the scene hierarchy.

# Features

* Physically Based Rendering (PBR)
* Buffer Management
* Specification Support
* Loader
* Environment Map
* Selective PBR Rendering

The selective PBR rendering allow you to turn on and off different parts of the PBR shader to provide a better understanding of the visual effect of each.

![Selective PBR Rendering](https://raw.github.com/peted70/dx-gltf-viewer/master/img/selective-rendering.PNG)

# Dependencies
* [boost-signals2](https://www.boost.org/doc/libs/1_67_0/doc/html/signals2.html) (header only observer/observable pattern)
* [Microsoft.glTF.cpp](https://www.nuget.org/packages/Microsoft.glTF.CPP/)

I used [vcpkg](https://github.com/Microsoft/vcpkg) for any source code dependencies and [Nuget](https://www.nuget.org/) and the Visual Studio integration to install binary dependencies. Neither of which are requirements.

# Building
This project can be built using Visual Studio 2017 Update 4 on Windows 10 Fall Creators Update (16299.0).

# Further Information
Please see this article for full details around features and coding for this sample. There is an offical sample [here] but I am retaining this repo as I would like to use it to experiment with new features - the official sample should get updated with features specific to glTF.
