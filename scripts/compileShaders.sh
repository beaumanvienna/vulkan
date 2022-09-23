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

glslc engine/platform/Vulkan/shaders/spriteRenderer2D.vert                  -o bin/spriteRenderer2D.vert.spv
glslc engine/platform/Vulkan/shaders/spriteRenderer2D.frag                  -o bin/spriteRenderer2D.frag.spv

glslc engine/platform/Vulkan/shaders/guiShader.vert                         -o bin/guiShader.vert.spv
glslc engine/platform/Vulkan/shaders/guiShader.frag                         -o bin/guiShader.frag.spv

glslc engine/platform/Vulkan/shaders/guiShader2.vert                        -o bin/guiShader2.vert.spv
glslc engine/platform/Vulkan/shaders/guiShader2.frag                        -o bin/guiShader2.frag.spv

glslc engine/platform/Vulkan/shaders/skybox.vert                            -o bin/skybox.vert.spv
glslc engine/platform/Vulkan/shaders/skybox.frag                            -o bin/skybox.frag.spv

glslc engine/platform/Vulkan/shaders/shadowShader.vert                      -o bin/shadowShader.vert.spv
glslc engine/platform/Vulkan/shaders/shadowShader.frag                      -o bin/shadowShader.frag.spv

glslc engine/platform/Vulkan/shaders/debug.vert                             -o bin/debug.vert.spv
glslc engine/platform/Vulkan/shaders/debug.frag                             -o bin/debug.frag.spv
