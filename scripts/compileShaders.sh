#/bin/bash
mkdir -p bin

glslc engine/platform/Vulkan/shaders/pointLight.vert                        -I. -o bin/pointLight.vert.spv
glslc engine/platform/Vulkan/shaders/pointLight.frag                        -I. -o bin/pointLight.frag.spv

glslc engine/platform/Vulkan/shaders/spriteRenderer.vert                    -I. -o bin/spriteRenderer.vert.spv
glslc engine/platform/Vulkan/shaders/spriteRenderer.frag                    -I. -o bin/spriteRenderer.frag.spv

glslc engine/platform/Vulkan/shaders/pbrNoMap.vert                          -I. -o bin/pbrNoMap.vert.spv
glslc engine/platform/Vulkan/shaders/pbrNoMap.frag                          -I. -o bin/pbrNoMap.frag.spv

glslc engine/platform/Vulkan/shaders/pbrDiffuse.vert                        -I. -o bin/pbrDiffuse.vert.spv
glslc engine/platform/Vulkan/shaders/pbrDiffuse.frag                        -I. -o bin/pbrDiffuse.frag.spv

glslc engine/platform/Vulkan/shaders/pbrDiffuseNormal.vert                  -I. -o bin/pbrDiffuseNormal.vert.spv
glslc engine/platform/Vulkan/shaders/pbrDiffuseNormal.frag                  -I. -o bin/pbrDiffuseNormal.frag.spv

glslc engine/platform/Vulkan/shaders/pbrDiffuseNormalRoughnessMetallic.vert -I. -o bin/pbrDiffuseNormalRoughnessMetallic.vert.spv
glslc engine/platform/Vulkan/shaders/pbrDiffuseNormalRoughnessMetallic.frag -I. -o bin/pbrDiffuseNormalRoughnessMetallic.frag.spv

glslc engine/platform/Vulkan/shaders/deferredRendering.vert                 -I. -o bin/deferredRendering.vert.spv
glslc engine/platform/Vulkan/shaders/deferredRendering.frag                 -I. -o bin/deferredRendering.frag.spv

glslc engine/platform/Vulkan/shaders/spriteRenderer2D.vert                  -I. -o bin/spriteRenderer2D.vert.spv
glslc engine/platform/Vulkan/shaders/spriteRenderer2D.frag                  -I. -o bin/spriteRenderer2D.frag.spv

glslc engine/platform/Vulkan/shaders/guiShader.vert                         -I. -o bin/guiShader.vert.spv
glslc engine/platform/Vulkan/shaders/guiShader.frag                         -I. -o bin/guiShader.frag.spv

glslc engine/platform/Vulkan/shaders/guiShader2.vert                        -I. -o bin/guiShader2.vert.spv
glslc engine/platform/Vulkan/shaders/guiShader2.frag                        -I. -o bin/guiShader2.frag.spv

glslc engine/platform/Vulkan/shaders/skybox.vert                            -I. -o bin/skybox.vert.spv
glslc engine/platform/Vulkan/shaders/skybox.frag                            -I. -o bin/skybox.frag.spv

glslc engine/platform/Vulkan/shaders/shadowShader.vert                      -I. -o bin/shadowShader.vert.spv
glslc engine/platform/Vulkan/shaders/shadowShader.frag                      -I. -o bin/shadowShader.frag.spv

glslc engine/platform/Vulkan/shaders/debug.vert                             -I. -o bin/debug.vert.spv
glslc engine/platform/Vulkan/shaders/debug.frag                             -I. -o bin/debug.frag.spv
