# vulkan
A Vulkan Render Engine<br/>
<br/>
<br/>
Features: Vulkan support for 3D rendering, hotplug gamepad support, sound support, profiling, resource system, render API abstraction, engine/application separation, event system, settings manager<br/>
<br/>
* Press "f" to toggle fullscreen.<br/>
* Use the left and right sticks on the controller to move the top left quad around and scale it.<br/>
* Use the mouse wheel to zoom.<br/>
* The A button plays a test sound.<br/>
* Press ESC or the guide button on the controller to exit.<br/>
<br/>
To blacklist a GPU, enter its name or a substring in engine.cfg.
<br/>
## Build Instructions<br/>

Ubuntu:<br/>
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
Compile and run debug target: make verbose=1 && ./bin/Debug/engine <br/>
Compile and run release target: make config=release verbose=1 && ./bin/Release/engine<br/>
<br/>
<br/>

### Windows Build Instructions<br/>
<br/>
Dependencies: premake5, VS2019<br/>
In a terminal, starting from the root folder vulkan:<br/>
premake5.exe vs2019<br/>
<br/>
Open the solution for vulkan, switch to Release, and hit F5<br/>
<br/>
