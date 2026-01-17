# YACReader Flow RHI Shaders

This directory contains the GLSL 4.50 shaders for the QRhiWidget-based flow implementation.

## Files

- `flow.vert` - Vertex shader (GLSL 450)
- `flow.frag` - Fragment shader (GLSL 450)
- `flow.vert.qsb` - Compiled vertex shader (multi-platform)
- `flow.frag.qsb` - Compiled fragment shader (multi-platform)
- `compile_shaders.bat` - Windows compilation script
- `compile_shaders.sh` - Unix/macOS compilation script
- `shaders.qrc` - Qt resource file

## Compiling Shaders

The shaders must be compiled to `.qsb` format using Qt's `qsb` tool before building YACReader.

### Prerequisites

Ensure `qsb` is in your PATH. It's typically located in:
- Windows: `C:\Qt\6.x.x\msvc2019_64\bin\qsb.exe`
- macOS: `/opt/Qt/6.x.x/macos/bin/qsb`
- Linux: `/opt/Qt/6.x.x/gcc_64/bin/qsb`

### Compilation

**Windows:**
```cmd
cd common/rhi/shaders
compile_shaders.bat
```

**Unix/macOS:**
```bash
cd common/rhi/shaders
chmod +x compile_shaders.sh
./compile_shaders.sh
```

The compiled `.qsb` files contain shader variants for:
- OpenGL ES 2.0, 3.0
- OpenGL 2.1, 3.0+
- HLSL (Direct3D 11/12)
- Metal Shading Language (macOS/iOS)

## Note

The `.qsb` files are included in the repository for convenience. Recompile only if you modify the shader source.
