#/bin/bash
mkdir -p bin
glslc engine/platform/Vulkan/shader/simpleShader.vert -o bin/simpleShader.vert.spv
glslc engine/platform/Vulkan/shader/simpleShader.frag -o bin/simpleShader.frag.spv
