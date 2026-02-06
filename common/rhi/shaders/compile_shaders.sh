#!/bin/bash
# Compile shaders to .qsb format for Qt RHI
# Requires qsb tool from Qt installation

echo "Compiling flow vertex shader..."
qsb --glsl "100 es,120,150" --hlsl 50 --msl 12 -o flow.vert.qsb flow.vert
if [ $? -ne 0 ]; then
    echo "Error compiling vertex shader"
    exit 1
fi

echo "Compiling flow fragment shader..."
qsb --glsl "100 es,120,150" --hlsl 50 --msl 12 -o flow.frag.qsb flow.frag
if [ $? -ne 0 ]; then
    echo "Error compiling fragment shader"
    exit 1
fi

echo "Shader compilation complete!"
