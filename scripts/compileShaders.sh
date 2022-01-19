#/bin/bash
mkdir -p bin
glslc engine/platform/Vulkan/shaders/simpleShader.vert -o bin/simpleShader.vert.spv
glslc engine/platform/Vulkan/shaders/simpleShader.frag -o bin/simpleShader.frag.spv
