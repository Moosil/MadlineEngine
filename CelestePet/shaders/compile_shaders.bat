@echo off
set vk_sdk=%VULKAN_SDK%
for %%i in (*.vert *.frag *.comp) do "%vk_sdk%\Bin\glslangValidator.exe" -V "%%~i" -o "spv/%%~i.spv"