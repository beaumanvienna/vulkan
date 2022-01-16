# vulkan
A Vulkan Render Engine


## Build Instructions<br/>

Ubuntu:<br/>
<br/>
sudo add-apt-repository ppa:beauman/marley<br/>
sudo add-apt-repository universe<br/>
sudo apt-get update<br/>
sudo apt install premake5 git build-essential xorg-dev libxrandr-dev libvulkan-dev <br/>
<br/>
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
