#/bin/bash
mkdir -p bin
glslc engine/platform/Vulkan/shaders/litShader.vert -o bin/litShader.vert.spv
glslc engine/platform/Vulkan/shaders/litShader.frag -o bin/litShader.frag.spv

glslc engine/platform/Vulkan/shaders/pointLight.vert -o bin/pointLight.vert.spv
glslc engine/platform/Vulkan/shaders/pointLight.frag -o bin/pointLight.frag.spv

glslc engine/platform/Vulkan/shaders/normalMapping.vert -o bin/normalMapping.vert.spv
glslc engine/platform/Vulkan/shaders/normalMapping.frag -o bin/normalMapping.frag.spv
