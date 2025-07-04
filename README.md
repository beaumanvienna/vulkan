# Lucre
A Vulkan Render Engine<br/>
<br/>
<br/>
<p align="center">
  <img src="resources/images/reservedScene.png">
</p>
<br/>
<br/>

Watch my game engine dev videos on my <a href="https://www.youtube.com/channel/UCb7glBOnGGHDYI4CIKYBV2A">YouTube channel</a>

<br/>
<br/>

Features:

- A Vulkan renderer with support for 2D and 3D scenes
- Support for 3D file formats Obj Wavefront (<a href="https://github.com/tinyobjloader/tinyobjloader">tinyobj</a>), glTF (<a href="https://github.com/spnda/fastgltf">fastgltf</a>), and FBX (<a href="https://github.com/ufbx/ufbx">ufbx</a> and <a href="https://github.com/assimp/assimp">asset importer</a>)
- Physically-based rendering (PBR), materials based on Blender's roughness-metallic workflow, normal mapping
- Point lights, directional lights with cascaded shadows, and deferred shading, instancing of models on the GPU
- Post-processing (Bloom), skeletal animations
- Scene management with scene descriptions, multi-threaded background loading, and scene saving based on <a href="https://github.com/simdjson/simdjson">simdjson</a>
- Hotplug gamepad support based on <a href="https://github.com/libsdl-org/SDL">SDL2</a> and a <a href="https://github.com/mdqinc/SDL_GameControllerDB">controller database</a>
- Cross-platform support for Linux, macOS, Windows
- Particle system
- Water shader with reflection and refraction cameras based on the <a href="https://www.youtube.com/playlist?list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh">tutorial from ThinMatrix</a>
- Mesh loading for height maps, including a color map and grass rendering
- Mesh loading for grass placement
- Collision mesh loading
- Event system for mouse, keyboard, controller, window, and game events
- Settings manager and a message logger
- Entity component system based on <a href="https://github.com/skypjack/entt">EnTT</a>
- Physics simulator based on <a href="https://github.com/jrouwe/JoltPhysics">Jolt</a>
- Physics simulator based on <a href="https://github.com/erincatto/box2d">Box2D</a>
- In-game GUI with two themes and debug GUI based on <a href="https://github.com/ocornut/imgui">ImGUI</a> and  <a href="https://github.com/CedricGuillemet/ImGuizmo">ImGuizmo</a>
- Sprite sheets, a sprite sheet generator, and sprite sheet animations
- Thread Profiling with <a href="https://github.com/wolfpld/tracy">Tracy</a>
- Profiling JSON output file for chrome://tracing
- Resource system for images and shaders to be compiled into the app
- Sound support and desktop volume settings (Linux only)
- Render-API abstraction and engine/application separation
- multi-threaded shader compilation with <a href="https://github.com/google/shaderc">shaderc</a>
- <a href="https://github.com/bshoshany/thread-pool">thread pool</a><br/>

Usage:<br/>


- Press "f" to toggle fullscreen and "m" to toggle the debug imgui window.
- Use the left and right sticks on the controller to move some objects around and scale them.
- Use the AWSD keys to move the camera. Q and E are for up and down
- Use the arrow keys to look around.
- Use the mouse wheel to zoom.
- The A button plays a test sound.
- Press ESC or the guide button on the controller to exit.
- Press "g" to fire the volcano and "r" to reset the scene.
- Use a gamepad to drive the vehicles and walk with the character animations
<br/>
To blacklist a GPU, enter its name or a substring in engine.cfg.<br/>
<br/>
Contributions: Please use https://en.wikipedia.org/wiki/Indentation_style#Allman_style and four spaces to indent.<br/>
<br/>

## How to clone the repository<br/>
<br/>
Run:<br/>

```
git clone --recurse-submodules https://github.com/beaumanvienna/vulkan
```

## Build Instructions<br/>
### Ubuntu Build Instructions<br/>
<br/>
Open a terminal
<br/><br/>
(Info: ppa:beauman/marley provides premake5)

```
sudo add-apt-repository ppa:beauman/marley
sudo add-apt-repository universe
sudo apt-get update
sudo apt install premake5 git build-essential xorg-dev libxrandr-dev libvulkan-dev libpulse-dev
sudo apt install libibus-1.0-dev libglib2.0-dev libsamplerate0-dev libasound2-dev libudev-dev
```

Get the Vulkan SDK from here:<br>
https://vulkan.lunarg.com/doc/view/latest/linux/getting_started_ubuntu.html
<br>
<br>
Install the source code:
```
git clone --recurse-submodules https://github.com/beaumanvienna/vulkan
cd vulkan
```

Create project files for gcc:
```
premake5 gmake2
```

<br/>
<br/>
Define the number of CPU cores to be used for compiling<br/>
e.g. "-j4" <br/>
To use all available CPU cores, say:

```
export MAKEFLAGS=-j$(nproc)
```

You may want to add the `export MAKEFLAGS=-j$(nproc)` command to ~/.bashrc to always use multiple CPU cores for compiling a makefile.
<br/>
<br/>
<br/>
Compile and run debug target:

```
make verbose=1 && ./bin/Debug/lucre
```

Compile and run release target:
```
make config=release verbose=1 && ./bin/Release/lucre
```

<br/>


### MacOSX Build Instructions<br/>
#### Install Dependencies

[Download and install MacOS Vulkan sdk](https://vulkan.lunarg.com/)<br/>
[Download and install Homebrew](https://brew.sh/)<br/>
<br/>
Open a terminal
<br/><br/>
To install Xcode, type `git`<br/>
<br/>
Install dependencies (note that premake5 is installed as `premake`, but called in the terminal as `premake5`):

```
brew install premake glib gtk+ sfml sdl2 sdl2_mixer libvorbis libogg glfw
```

<br/>
Install the source code:

```
git clone --recurse-submodules https://github.com/beaumanvienna/vulkan
cd vulkan
```

<br/>
Create project files for clang:

```
premake5 gmake2
```

<br/>
<br/>
Define the number of CPU cores to be used for compiling<br/>
e.g. "-j8" to use eight CPU cores<br/>
To use all available CPU cores, say:

```
export MAKEFLAGS=-j$(sysctl -n hw.ncpu)
```

To find out, how many cores your Mac has, say:

```
sysctl -n hw.ncpu
```

You may want to add the `export MAKEFLAGS=-j8` command to ~/.zshrc to always use multiple CPU cores for compiling a makefile.
<br/>
<br/>
<br/>
Compile and run debug target:

```
make verbose=1 && ./bin/Debug/lucre
```

Compile and run release target:
```
make config=release verbose=1 && ./bin/Release/lucre
```

<br/>



### Windows Build Instructions<br/>
<br/>
Dependencies: gitbash, premake5, Visual Studio<br/>
<br/>

Download the Vulkan SDK from [lunarg.com](https://vulkan.lunarg.com/), install it, then copy "C:\VulkanSDK\1.3.204.1" (path and version may differ) to vendor/VulkanSDK. The version number is omitted in the path.<br/>
<br/><br/>
Open gitbash
<br/><br/>
<br/>
Install the source code:

```
git clone --recurse-submodules https://github.com/beaumanvienna/vulkan
cd vulkan
```

If you have Visual Studio 2022:

```
premake5.exe vs2022
```

<br/>
If you have Visual Studio 2022, use instead:

```
premake5.exe vs2022
```

<br/>
Open the solution for Vulkan, switch to Release, and hit F5<br/>
<br/>

### Clean Instruction<br/>
<br/>
To clean all temporary files (including the spv-files, engine.cfg, imgui.ini, and the scene files in the top directory), use:

```
premake5 clean
```

<br/>

### Update Instruction<br/>
<br/>
To pull in the latest changes for the repository, use:

```
git pull && git submodule update --init --recursive
```

<br/>
