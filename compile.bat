

REM 设置Vulkan SDK路径
set VULKAN_SDK=C:\VulkanSDK\1.3.283.0

REM 编译所有.vert文件
for %%f in (shaders\*.vert) do (
    echo Compiling %%f
    %VULKAN_SDK%\Bin\glslc.exe %%f -o %%f.spv
)

REM 编译所有.frag文件
for %%f in (shaders\*.frag) do (
    echo Compiling %%f
    %VULKAN_SDK%\Bin\glslc.exe %%f -o %%f.spv
)

pause