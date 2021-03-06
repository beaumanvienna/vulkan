#/bin/bash
mkdir -p bin

glslc engine/platform/Vulkan/shaders/pointLight.vert                        -o bin/pointLight.vert.spv
glslc engine/platform/Vulkan/shaders/pointLight.frag                        -o bin/pointLight.frag.spv
                                                                            
glslc engine/platform/Vulkan/shaders/spriteRenderer.vert                    -o bin/spriteRenderer.vert.spv
glslc engine/platform/Vulkan/shaders/spriteRenderer.frag                    -o bin/spriteRenderer.frag.spv
                                                                            
glslc engine/platform/Vulkan/shaders/pbrNoMap.vert                          -o bin/pbrNoMap.vert.spv
glslc engine/platform/Vulkan/shaders/pbrNoMap.frag                          -o bin/pbrNoMap.frag.spv
                                                                            
glslc engine/platform/Vulkan/shaders/pbrDiffuse.vert                        -o bin/pbrDiffuse.vert.spv
glslc engine/platform/Vulkan/shaders/pbrDiffuse.frag                        -o bin/pbrDiffuse.frag.spv
                                                                            
glslc engine/platform/Vulkan/shaders/pbrDiffuseNormal.vert                  -o bin/pbrDiffuseNormal.vert.spv
glslc engine/platform/Vulkan/shaders/pbrDiffuseNormal.frag                  -o bin/pbrDiffuseNormal.frag.spv

glslc engine/platform/Vulkan/shaders/pbrDiffuseNormalRoughnessMetallic.vert -o bin/pbrDiffuseNormalRoughnessMetallic.vert.spv
glslc engine/platform/Vulkan/shaders/pbrDiffuseNormalRoughnessMetallic.frag -o bin/pbrDiffuseNormalRoughnessMetallic.frag.spv

glslc engine/platform/Vulkan/shaders/deferredRendering.vert                 -o bin/deferredRendering.vert.spv
glslc engine/platform/Vulkan/shaders/deferredRendering.frag                 -o bin/deferredRendering.frag.spv

glslc engine/platform/Vulkan/shaders/atlasShader.vert                       -o bin/atlasShader.vert.spv
glslc engine/platform/Vulkan/shaders/atlasShader.frag                       -o bin/atlasShader.frag.spv

glslc engine/platform/Vulkan/shaders/guiShader.vert                       -o bin/guiShader.vert.spv
glslc engine/platform/Vulkan/shaders/guiShader.frag                       -o bin/guiShader.frag.spv
