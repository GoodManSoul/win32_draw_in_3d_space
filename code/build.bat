@echo off

mkdir ..\build
pushd ..\build


cl -Zi ..\code\win32_draw_in_3D_space.cpp User32.lib Gdi32.lib Kernel32.lib

popd