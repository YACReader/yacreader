@echo off
REM Compile shaders to .qsb format for Qt RHI
REM Requires qsb tool from Qt installation

echo Compiling flow vertex shader...
qsb --glsl "100 es,120,150" --hlsl 50 --msl 12 -o flow.vert.qsb flow.vert
if %ERRORLEVEL% NEQ 0 (
    echo Error compiling vertex shader
    exit /b 1
)

echo Compiling flow fragment shader...
qsb --glsl "100 es,120,150" --hlsl 50 --msl 12 -o flow.frag.qsb flow.frag
if %ERRORLEVEL% NEQ 0 (
    echo Error compiling fragment shader
    exit /b 1
)

echo Shader compilation complete!
