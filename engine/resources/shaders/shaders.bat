del /f vert.spv
del /f frag.spv
C:\VulkanSDK\1.3.261.1\Bin\glslc.exe shader.vert -o vert.spv
C:\VulkanSDK\1.3.261.1\Bin\glslc.exe shader.frag -o frag.spv
echo Successfully Recompiled Shaders
pause