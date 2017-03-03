#Vulture: A game engine using the Vulkan API

##Table of Contents
* [Requirements](#requirements)
* [Environment](#environment)
* [Building and Running](#building-and-running)
* [Vulkan Coding Tips](#vulkan-coding-tips)
* [Known Issues](#known-issues)
  
##Requirements

###Linux:

1. CMake
2. LunarG Vulkan SDK for Linux (64-bit)
3. g++ (haven't tested clang)
4. Xlib or XCB (both libraries and header files)

###Windows:

1. CMake
2. LunarG Vulkan SDK for Windows (64-bit)
3. Visual Studio (tested for 2015 only)


##Environment

###Linux

`VULKAN_SDK=<path_to_lunarg_vulkan_sdk>/x86_64`

`VK_LAYER_PATH=$VULKAN_SDK/etc/explicit_layer.d`

###Windows

`VULKAN_SDK=<path_to_lunarg_vulkan_sdk>`

Update registry to point to Vulkan SDK layer manifests (the Windows installer
  should take care of this for you)


##Building and Running

There are 2 demos you can run: 

**compute** will create the compute pipeline and execute the sample compute shader. You can check the logs
for output (you should see a ton of 'LMAO's and 'XDXD's).

**graphics** is the more interesting of the 2. It will trigger the graphics pipeline that will 
render graphics to your screen.

###Linux

#####From the Terminal:
1. cd into project root (same level as CMakeLists.txt)
2. cmake -H. -Bbuild
3. cd build
4. make
5. ./\<demo>

###Windows (using Visual Studio 2015)

#####From Developer Command Prompt (_Not_ the Windows command prompt):
1. cd into project root (same level as CMakeLists.txt)
2. cmake -H. -Bbuild -G"Visual Studio 14 2015 Win64"
3. cd build
4. msbuild Vulture.sln
5. Debug\\\<demo>.exe


##Vulkan coding tips
Enabling the standard validation layer is a great way to uncover bugs in
   your code during development.


##Known Issues
As of SDK version 1.0.37.0 the VK_KHR_display extension is not supported,
   which means true fullscreen is not supported yet.
