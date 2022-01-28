#/bin/bash
mkdir -p bin
glslc engine/platform/Vulkan/shaders/simpleShader.vert -o bin/simpleShader.vert.spv
glslc engine/platform/Vulkan/shaders/simpleShader.frag -o bin/simpleShader.frag.spv

glslc engine/platform/Vulkan/shaders/pointLight.vert -o bin/pointLight.vert.spv
glslc engine/platform/Vulkan/shaders/pointLight.frag -o bin/pointLight.frag.spv
