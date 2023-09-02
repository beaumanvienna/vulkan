# Lucre
A Vulkan Render Engine<br/>
<br/>
<br/>
<p align="center">
  <img src="resources/images/meme.png">
</p>
<br/>
<br/>

Features:

- A Vulkan renderer with support for 2D and 3D scenes
- Support for the 3D file formats Obj Wavefront and glTF
- Physically-based rendering (PBR), materials based on Blender's roughness-metallic workflow, normal mapping
- Point lights, directional lights with shadows, and deferred shading
- Scene management with scene descriptions, background loading, scene saving, and attachable scripts
- Sound support and desktop volume settings (Linux only)
- Hotplug gamepad support based on SDL2 and a controller database
- Cross-platform resource system (Windows/Linux)
- Particle system
- Event system for mouse, keyboard, controller, window and game events
- Settings manager and a message logger
- An entity component system based on EnTT
- Physics simulator based on Box2D
- In-game GUI with two themes and debug GUI based on imgui 
- Sprite sheets, a sprite sheet generator, sprite sheet animations
- Render-API abstraction and engine/application separation
- Profiling JSON output file for chrome://tracing<br/>

Usage:<br/>


- Press "f" to toggle fullscreen and "m" to toggle the debug imgui window.
- Use the left and right sticks on the controller to move some objects around and scale them.
- Use the AWSD keys to move the camera. Q and E are for up and down
- Use the arrow keys to look around.
- Use the mouse wheel to zoom.
- The A button plays a test sound.
- Press ESC or the guide button on the controller to exit.
- Press "g" to fire the volcano and "r" to reset the scene.
<br/>
To blacklist a GPU, enter its name or a substring in engine.cfg.<br/>
<br/>
Contributions: Please use https://en.wikipedia.org/wiki/Indentation_style#Allman_style and four spaces to indent.<br/>
<br/>

## Build Instructions<br/>

Ubuntu:<br/>
(Info: ppa:beauman/marley provides premake5)<br/>
<br/>
<br/>
sudo add-apt-repository ppa:beauman/marley<br/>
sudo add-apt-repository universe<br/>
sudo apt-get update<br/>
sudo apt install premake5 git build-essential xorg-dev libxrandr-dev libvulkan-dev libpulse-dev libibus-1.0-dev libglib2.0-dev libsamplerate0-dev libasound2-dev libudev-dev <br/>
<br>
<br>
Get the Vulkan SDK from here:<br>
https://vulkan.lunarg.com/doc/sdk/1.2.198.1/linux/getting_started_ubuntu.html
<br>
<br>
Install the source code: <br/>
git clone --recurse-submodules https://github.com/beaumanvienna/vulkan<br/>
cd vulkan<br/>
<br/>
<br/>
Create project files for gcc: <br/>
premake5 gmake2<br/>
<br/>
<br />
#define the number of CPU cores to be used for compiling<br />
#e.g. "-j4" <br />
#To use all available CPU cores, say:<br />
export MAKEFLAGS=-j$(nproc)<br />
<br />
<br />
Compile and run debug target: make verbose=1 && ./bin/Debug/lucre <br/>
Compile and run release target: make config=release verbose=1 && ./bin/Release/lucre<br/>
<br/>
<br/>

### Windows Build Instructions<br/>
<br/>
Dependencies: premake5, Visual Studio, Vulkan SDK<br/>
<br/>
Download the Vulkan SDK from lunarg.com, install it, then copy "C:\VulkanSDK\1.3.204.1" (path and version may differ) to vendor/Vulkan SDK. The version number is omitted in the path.<br/>
<br/>
In a terminal, starting from the root folder vulkan, if you have Visual Studio 2019:<br/>
premake5.exe vs2019<br/>
<br/>
If you have Visual Studio 2022, use instead:<br/>
premake5.exe vs2022<br/>
<br/>
Open the solution for Vulkan, switch to Release, and hit F5<br/>
<br/>
<br/>

### Update Instruction<br/>
<br/>
To pull in the latest changes for the repository, use:<br/>
<br/>
git pull && git submodule update --init --recursive
