# YACReader Flow RHI Implementation

This directory contains the QRhiWidget-based implementation of the YACReader 3D cover flow, providing cross-platform 3D rendering support for Qt 6.7+.

## Overview

The RHI (Rendering Hardware Interface) implementation is a modern replacement for the OpenGL-based flow (`yacreader_flow_gl`) that:

- ✅ Supports **multiple graphics APIs**: Vulkan, Metal, Direct3D 11/12, OpenGL
- ✅ Provides **native performance** on modern platforms (Metal on macOS, D3D on Windows)
- ✅ Maintains **100% API compatibility** with the OpenGL version
- ✅ Works seamlessly with **Qt 6.7+** while Qt5 continues using OpenGL
- ✅ Enables **future-proof** rendering infrastructure

## Architecture

### Class Hierarchy

```
QRhiWidget (Qt base class)
    └── YACReaderFlow3D (Base implementation)
            ├── YACReaderComicFlow3D (File path-based loading for library)
            └── YACReaderPageFlow3D (Byte array-based loading for viewer)
```

### Files

- **yacreader_flow_rhi.h** - Header with class definitions
- **yacreader_flow_rhi.cpp** - Implementation
- **shaders/** - GLSL 450 shader source files (compiled to .qsb at build time by `qt_add_shaders`)
- **README.md** - This file

## Key Features

### Graphics Pipeline

The implementation uses:
- **Instanced rendering** for efficient batch drawing
- **Dual-pass rendering** (reflections + covers)
- **MSAA** (4x by default) for anti-aliasing
- **Depth testing** and back-face culling
- **Alpha blending** for transparency

### Resource Management

QRhi resources managed:
- `QRhiBuffer` for vertices, instance data, and uniforms
- `QRhiTexture` for cover images, marks, and default texture
- `QRhiSampler` for texture filtering
- `QRhiGraphicsPipeline` for render state
- `QRhiShaderResourceBindings` for uniform/texture bindings

### Shader System

Shaders are written in **GLSL 4.50** and compiled to `.qsb` format at build time
via `qt_add_shaders()` in CMake, supporting:
- OpenGL ES 2.0, 3.0
- OpenGL 2.1, 3.0+
- HLSL (Direct3D 11/12)
- Metal Shading Language (macOS/iOS)

## Integration

### Qt5 vs Qt6 Selection

The build system automatically selects the appropriate implementation:

**Qt 5.x**: Uses `YACReaderFlowGL` (OpenGL-based)
**Qt 6.0-6.6**: Uses `YACReaderFlowGL` (OpenGL-based)
**Qt 6.7+**: Uses `YACReaderFlow3D` (RHI-based) if `YACREADER_USE_RHI` is defined

### Type Aliases

Applications use type aliases for seamless switching:

```cpp
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0) && defined(YACREADER_USE_RHI)
using YACReaderPageFlowImpl = YACReaderPageFlow3D;
using YACReaderComicFlowImpl = YACReaderComicFlow3D;
#else
using YACReaderPageFlowImpl = YACReaderPageFlowGL;
using YACReaderComicFlowImpl = YACReaderComicFlowGL;
#endif
```

### Example Usage (YACReader Viewer)

```cpp
// goto_flow_gl.cpp
flow = new YACReaderPageFlowImpl(this);
flow->setShowMarks(false);
flow->populate(numPages);
connect(flow, &YACReaderPageFlowImpl::selected, this, &GoToFlowGL::goToPage);
```

### Example Usage (YACReaderLibrary)

```cpp
// comic_flow_widget.cpp
flow = new YACReaderComicFlowImpl(parent);
flow->setImagePaths(pathsList);
connect(flow, &YACReaderComicFlowImpl::centerIndexChanged,
        this, &ComicFlowWidget::centerIndexChanged);
```

## API Compatibility

All public methods from `YACReaderFlowGL` are preserved:

### Navigation
- `showPrevious()`, `showNext()`
- `setCurrentIndex(int)`, `setCenterIndex(unsigned int)`
- `showSlide(int)`, `centerIndex()`

### Configuration
- `setPreset(const Preset &)`
- `setPerformance(Performance)`
- `setFlowRightToLeft(bool)`
- `setZoom(int)`, `setRotation(int)`
- `setCF_RX/RY/RZ(int)`, `setCF_Y/Z(int)`
- `setX_Distance(int)`, `setCenter_Distance(int)`, etc.

### Appearance
- `setBackgroundColor(const QColor &)`
- `setTextColor(const QColor &)`
- `setShadingColor(const QColor &)`
- `setShowMarks(bool)`, `setMarks(QVector<...>)`

### Content Management
- `populate(int)`, `clear()`, `reset()`, `reload()`
- `insert()`, `remove(int)`, `add(int)`, `replace()`
- Subclass-specific: `setImagePaths()`, `resortCovers()`, etc.

## Building

### Prerequisites

1. **Qt 6.7 or later** (with ShaderTools module)
2. **CMake 3.25+**
3. **C++20 compiler**

### Build YACReader

Shaders are compiled automatically at build time via `qt_add_shaders()`:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

No manual shader compilation step is needed.

## Graphics API Selection

QRhiWidget auto-selects the best API per platform:

- **macOS/iOS**: Metal (native)
- **Windows**: Direct3D 11 (default) or Direct3D 12
- **Linux**: Vulkan or OpenGL
- **Android**: OpenGL ES 3.0 or Vulkan

You can force a specific API via environment variables (for testing):

```bash
# Force OpenGL
export QSG_RHI_BACKEND=opengl

# Force Vulkan
export QSG_RHI_BACKEND=vulkan

# Force Direct3D 11 (Windows)
set QSG_RHI_BACKEND=d3d11
```

## Performance

Performance tiers match the OpenGL version:

- **Low**: 8 covers loaded, 128px textures (page flow) / 200px (comic flow)
- **Medium**: 10 covers, 196px / 256px textures
- **High**: 12 covers, 256px / 320px textures
- **Ultra High**: 14-16 covers, full resolution textures

Texture loading happens asynchronously via worker threads (`ImageLoader3D`, `ImageLoaderByteArray3D`).

## Debugging

### Enable Validation Layers

For debugging graphics issues, enable validation:

```cpp
flow->setDebugLayerEnabled(true);  // Call before widget is shown
```

This activates:
- **Vulkan**: VK_LAYER_KHRONOS_validation
- **Direct3D**: D3D11 debug layer
- **Metal**: Metal API validation

### Check Active Graphics API

```cpp
QRhi *rhi = flow->rhi();
qDebug() << "Backend:" << rhi->backend();
qDebug() << "Driver:" << rhi->driverInfo();
```

### Common Issues

**Problem**: Shaders fail to load
**Solution**: Ensure Qt ShaderTools module is installed and `qt_add_shaders()` ran during build

**Problem**: Black screen on Qt 6.7
**Solution**: Check if `YACREADER_USE_RHI` is defined in build

**Problem**: Crashes on resize/reparent
**Solution**: `releaseResources()` properly cleans up all QRhi objects

## Migration from OpenGL

The RHI version is a **drop-in replacement** requiring no application code changes beyond the build system.

### What's Different (Internal)

| OpenGL API | QRhi Equivalent |
|------------|-----------------|
| `glDrawArraysInstanced()` | `cb->drawIndexed(instanceCount)` |
| `glUniform*()` | Update `QRhiBuffer` with uniform data |
| `glBindTexture()` | `QRhiShaderResourceBindings` |
| `glBlendFunc()` | `QRhiGraphicsPipeline::setTargetBlends()` |
| `glEnable(GL_DEPTH_TEST)` | `pipeline->setDepthTest(true)` |

### What's the Same

- All public API methods and signatures
- Animation system and timing
- Preset configurations
- Event handling (mouse, keyboard, wheel)
- Worker thread texture loading
- Performance tiers

## Known Limitations

1. **Qt Version**: Requires Qt 6.7+ (released April 2024)
2. **QRhi Stability**: QRhi APIs may change in minor Qt releases
3. **Mixed Renderers**: Only one graphics API per window
4. **Shader Compilation**: Shaders are recompiled automatically when source files change

## Future Improvements

Potential enhancements:
- [ ] GPU-side frustum culling
- [ ] Compute shader for texture generation
- [ ] HDR/wide color gamut support
- [ ] Ray-traced reflections (via RHI compute)
- [ ] Dynamic LOD based on distance

## References

- [QRhiWidget Class Documentation](https://doc.qt.io/qt-6/qrhiwidget.html)
- [QRhi Overview](https://doc.qt.io/qt-6/qrhi.html)
- [Qt RHI Examples](https://doc.qt.io/qt-6/qtwidgets-rhi-cuberhiwidget-example.html)
- [Qt Shader Tools (qsb)](https://doc.qt.io/qt-6/qtshadertools-index.html)

## License

Same as YACReader project license.

## Author

Generated as part of the YACReader OpenGL modernization initiative.
