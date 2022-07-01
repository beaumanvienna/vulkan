#/bin/bash
mkdir -p bin

glslc engine/platform/Vulkan/shaders/pointLight.vert                        -o bin/pointLight.vert.spv
glslc engine/platform/Vulkan/shaders/pointLight.frag                        -o bin/pointLight.frag.spv
                                                                            
glslc engine/platform/Vulkan/shaders/defaultDiffuseMap.vert                 -o bin/defaultDiffuseMap.vert.spv
glslc engine/platform/Vulkan/shaders/defaultDiffuseMap.frag                 -o bin/defaultDiffuseMap.frag.spv
                                                                            
glslc engine/platform/Vulkan/shaders/pbrNoMap.vert                          -o bin/pbrNoMap.vert.spv
glslc engine/platform/Vulkan/shaders/pbrNoMap.frag                          -o bin/pbrNoMap.frag.spv
                                                                            
glslc engine/platform/Vulkan/shaders/pbrDiffuse.vert                        -o bin/pbrDiffuse.vert.spv
glslc engine/platform/Vulkan/shaders/pbrDiffuse.frag                        -o bin/pbrDiffuse.frag.spv
                                                                            
glslc engine/platform/Vulkan/shaders/pbrDiffuseNormal.vert                  -o bin/pbrDiffuseNormal.vert.spv
glslc engine/platform/Vulkan/shaders/pbrDiffuseNormal.frag                  -o bin/pbrDiffuseNormal.frag.spv

glslc engine/platform/Vulkan/shaders/pbrDiffuseNormalRoughnessMetallic.vert -o bin/pbrDiffuseNormalRoughnessMetallic.vert.spv
glslc engine/platform/Vulkan/shaders/pbrDiffuseNormalRoughnessMetallic.frag -o bin/pbrDiffuseNormalRoughnessMetallic.frag.spv

glslc engine/platform/Vulkan/shaders/gBuffer.vert                        -o bin/gBuffer.vert.spv
glslc engine/platform/Vulkan/shaders/gBuffer.frag                        -o bin/gBuffer.frag.spv
                                                                            
glslc engine/platform/Vulkan/shaders/deferredRendering.vert                 -o bin/deferredRendering.vert.spv
glslc engine/platform/Vulkan/shaders/deferredRendering.frag                 -o bin/deferredRendering.frag.spv
