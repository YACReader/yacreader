#include "yacreader_flow_rhi.h"
#include <QFile>
#include <cmath>

/*** Preset Configurations ***/
// Note: The preset configurations are already defined in yacreader_flow_gl.cpp
// We just reference them here as extern to avoid duplicate symbols

int YACReaderFlow3D::updateInterval = 16;

static QShader getShader(const QString &name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

/*Constructor*/
YACReaderFlow3D::YACReaderFlow3D(QWidget *parent, struct Preset p)
    : QRhiWidget(parent),
      numObjects(0),
      lazyPopulateObjects(-1),
      showMarks(true),
      hasBeenInitialized(false),
      backgroundColor(Qt::black),
      textColor(Qt::white),
      shadingColor(Qt::black),
      flowRightToLeft(false)
{
    updateCount = 0;
    config = p;
    currentSelected = 0;

    centerPos.x = 0.f;
    centerPos.y = 0.f;
    centerPos.z = 1.f;
    centerPos.rot = 0.f;

    shadingTop = 0.8f;
    shadingBottom = 0.02f;
    reflectionUp = 0.0f;
    reflectionBottom = 0.33f;

    setBackgroundColor(Qt::black);

    numObjects = 0;
    viewRotate = 0.f;
    viewRotateActive = 0;
    stepBackup = config.animationStep / config.animationSpeedUp;

    // Request 4x MSAA for the QRhiWidget's render target
    setSampleCount(4);

    timerId = -1;

    // Create and configure the index label
    indexLabel = new QLabel(this);
    indexLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    indexLabel->setAutoFillBackground(false);
    updateIndexLabelStyle();
}

YACReaderFlow3D::~YACReaderFlow3D()
{
    if (timerId != -1) {
        killTimer(timerId);
        timerId = -1;
    }

    // Clean up image textures (not owned by Scene)
    for (int i = 0; i < numObjects; i++) {
        if (images[i].texture && images[i].texture != scene.defaultTexture.get()) {
            delete images[i].texture;
        }
    }
    images.clear();
    numObjects = 0;

    // Release all RHI resources
    scene.reset();
}

void YACReaderFlow3D::timerEvent(QTimerEvent *event)
{
    if (timerId == event->timerId())
        update();
}

void YACReaderFlow3D::startAnimationTimer()
{
    if (timerId == -1)
        timerId = startTimer(updateInterval);
}

void YACReaderFlow3D::stopAnimationTimer()
{
    if (timerId != -1) {
        killTimer(timerId);
        timerId = -1;
    }
}

void YACReaderFlow3D::initialize(QRhiCommandBuffer *cb)
{
    auto newRhi = rhi();
    if (m_rhi != newRhi) {
        scene.reset();
        m_rhi = newRhi;
    }

    if (m_rhi == nullptr)
        return;

    // Helper to get or create resource update batch
    auto getResourceBatch = [this]() {
        if (scene.resourceUpdates == nullptr)
            scene.resourceUpdates = m_rhi->nextResourceUpdateBatch();
        return scene.resourceUpdates;
    };

    // Initialize default texture from image
    if (!scene.defaultTexture) {
        QImage defaultImage = QImage(":/images/defaultCover.png").convertToFormat(QImage::Format_RGBA8888);

        scene.defaultTexture.reset(m_rhi->newTexture(QRhiTexture::RGBA8, defaultImage.size(), 1, QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips));
        scene.defaultTexture->create();
        getResourceBatch()->uploadTexture(scene.defaultTexture.get(), defaultImage);
        getResourceBatch()->generateMips(scene.defaultTexture.get());
        qDebug() << "YACReaderFlow3D: Created defaultTexture" << defaultImage.size();
    }

#ifdef YACREADER_LIBRARY
    // Initialize mark textures
    if (!scene.markTexture) {
        QImage markImage = QImage(":/images/readRibbon.png").convertToFormat(QImage::Format_RGBA8888);
        if (!markImage.isNull()) {
            scene.markTexture.reset(m_rhi->newTexture(QRhiTexture::RGBA8, markImage.size(), 1, QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips));
            scene.markTexture->create();
            getResourceBatch()->uploadTexture(scene.markTexture.get(), markImage);
            getResourceBatch()->generateMips(scene.markTexture.get());
        }
    }

    if (!scene.readingTexture) {
        QImage readingImage = QImage(":/images/readingRibbon.png").convertToFormat(QImage::Format_RGBA8888);
        if (!readingImage.isNull()) {
            scene.readingTexture.reset(m_rhi->newTexture(QRhiTexture::RGBA8, readingImage.size(), 1, QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips));
            scene.readingTexture->create();
            getResourceBatch()->uploadTexture(scene.readingTexture.get(), readingImage);
            getResourceBatch()->generateMips(scene.readingTexture.get());
        }
    }
#endif

    // Create vertex buffer (quad geometry)
    if (!scene.vertexBuffer) {
        // Use a triangle list (two triangles = 6 vertices) because some RHI backends
        // don't support TriangleFan. Each vertex: x,y,z,u,v (5 floats).
        scene.vertexBuffer.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, 6 * 5 * sizeof(float)));
        scene.vertexBuffer->create();

        // Two triangles forming a quad (triangle list):
        // Tri 1: bottom-left, bottom-right, top-right
        // Tri 2: bottom-left, top-right, top-left
        // Texture coords flipped vertically to match OpenGL convention
        float vertices[] = {
            // Position (x, y, z), TexCoord (u, v)
            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom-left
            0.5f, -0.5f, 0.0f, 1.0f, 1.0f, // Bottom-right
            0.5f, 0.5f, 0.0f, 1.0f, 0.0f, // Top-right

            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom-left
            0.5f, 0.5f, 0.0f, 1.0f, 0.0f, // Top-right
            -0.5f, 0.5f, 0.0f, 0.0f, 0.0f // Top-left
        };

        getResourceBatch()->uploadStaticBuffer(scene.vertexBuffer.get(), vertices);
    }

    // Initialize alignment for uniform buffers
    if (scene.alignedUniformSize == 0) {
        scene.alignedUniformSize = m_rhi->ubufAligned(sizeof(UniformData));
    }

    // Create sampler with trilinear filtering (like the OpenGL version)
    if (!scene.sampler) {
        scene.sampler.reset(m_rhi->newSampler(
                QRhiSampler::Linear, // mag filter
                QRhiSampler::Linear, // min filter
                QRhiSampler::Linear, // mipmap filter (trilinear)
                QRhiSampler::ClampToEdge,
                QRhiSampler::ClampToEdge));
        scene.sampler->create();
    }

    // Create instance buffer for per-draw instance data
    if (!scene.instanceBuffer) {
        // Allocate buffer for per-instance data (model matrix + shading + opacity + flipFlag)
        // mat4 (16 floats) + vec4 (4 floats) + float (opacity) + float (flipFlag) = 22 floats
        scene.instanceBuffer.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, 22 * sizeof(float)));
        scene.instanceBuffer->create();
    }

    // Determine how many items we'll have (either already populated or pending lazy population)
    const int itemCount = (lazyPopulateObjects != -1) ? lazyPopulateObjects : numObjects;

    // Create uniform buffer sized for the actual content
    // Each item needs up to 3 draw slots: cover + reflection + mark
    // If no items yet, we'll create the buffer later in ensureUniformBufferCapacity()
    if (!scene.uniformBuffer && itemCount > 0) {
        const int requiredSlots = itemCount * 3;
        const int totalSize = requiredSlots * scene.alignedUniformSize;
        scene.uniformBuffer.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, totalSize));
        if (scene.uniformBuffer->create()) {
            scene.uniformBufferCapacity = requiredSlots;
        } else {
            qWarning() << "YACReaderFlow3D: Failed to create uniform buffer for" << itemCount << "items";
            scene.uniformBuffer.reset();
            scene.uniformBufferCapacity = 0;
        }
    }

    // Create shader bindings for pipeline (requires uniform buffer)
    if (!scene.shaderBindings && scene.uniformBuffer) {
        scene.shaderBindings.reset(m_rhi->newShaderResourceBindings());
        scene.shaderBindings->setBindings({ QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, scene.uniformBuffer.get(), sizeof(UniformData)),
                                            QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, scene.defaultTexture.get(), scene.sampler.get()) });
        scene.shaderBindings->create();
    }

    // Create pipeline if we have all prerequisites
    ensurePipeline();

    // Submit any pending resource updates
    if (scene.resourceUpdates) {
        cb->resourceUpdate(scene.resourceUpdates);
        scene.resourceUpdates = nullptr;
    }

    // Call populate only once per data loaded.
    if (!hasBeenInitialized && lazyPopulateObjects != -1) {
        populate(lazyPopulateObjects);
        lazyPopulateObjects = -1;
    }

    hasBeenInitialized = true;
}

void YACReaderFlow3D::ensureUniformBufferCapacity(int requiredSlots)
{
    if (!m_rhi || scene.alignedUniformSize == 0)
        return;

    // Check if we need to resize
    if (scene.uniformBufferCapacity >= requiredSlots && scene.uniformBuffer)
        return;

    // Reset uniform buffer
    scene.uniformBuffer.reset();

    // Create new larger buffer
    // Each draw needs its own uniform slot (cover + reflection + optional mark = 3 per object)
    const int totalSize = requiredSlots * scene.alignedUniformSize;
    scene.uniformBuffer.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, totalSize));
    if (scene.uniformBuffer->create()) {
        scene.uniformBufferCapacity = requiredSlots;

        // Invalidate shader bindings cache since the uniform buffer changed
        qDeleteAll(scene.shaderBindingsCache);
        scene.shaderBindingsCache.clear();

        // Recreate default shader bindings for pipeline (using dynamic offset)
        scene.shaderBindings.reset(m_rhi->newShaderResourceBindings());
        scene.shaderBindings->setBindings({ QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, scene.uniformBuffer.get(), sizeof(UniformData)),
                                            QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, scene.defaultTexture.get(), scene.sampler.get()) });
        scene.shaderBindings->create();

        // If pipeline doesn't exist yet (content added after initial empty initialization),
        // we need to create it now. This is handled by ensurePipeline().
    } else {
        qWarning() << "YACReaderFlow3D: Failed to create uniform buffer of size" << totalSize;
        scene.uniformBufferCapacity = 0;
    }
}

void YACReaderFlow3D::ensurePipeline()
{
    if (scene.pipeline || !m_rhi || !scene.uniformBuffer || !scene.shaderBindings)
        return;

    // Load shaders
    QShader vertShader = getShader(QLatin1String(":/shaders/flow.vert.qsb"));
    QShader fragShader = getShader(QLatin1String(":/shaders/flow.frag.qsb"));

    if (!vertShader.isValid() || !fragShader.isValid()) {
        qWarning() << "YACReaderFlow3D: Failed to load shaders!";
        return;
    }

    // Create pipeline
    scene.pipeline.reset(m_rhi->newGraphicsPipeline());

    // Setup alpha blending
    QRhiGraphicsPipeline::TargetBlend blend;
    blend.enable = true;
    blend.srcColor = QRhiGraphicsPipeline::SrcAlpha;
    blend.dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    blend.srcAlpha = QRhiGraphicsPipeline::One;
    blend.dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    scene.pipeline->setTargetBlends({ blend });

    // Enable depth test
    scene.pipeline->setDepthTest(true);
    scene.pipeline->setDepthWrite(true);
    scene.pipeline->setDepthOp(QRhiGraphicsPipeline::Less);

    scene.pipeline->setCullMode(QRhiGraphicsPipeline::Back);

    // Determine the MSAA sample count to use
    int requestedSamples = sampleCount();
    if (requestedSamples > 1 && m_rhi) {
        QVector<int> supported = m_rhi->supportedSampleCounts();
        auto it = std::upper_bound(supported.begin(), supported.end(), requestedSamples);
        int samplesToUse = (it != supported.begin()) ? *std::prev(it) : 1;
        if (samplesToUse > 1)
            scene.pipeline->setSampleCount(samplesToUse);
    }

    // Set shaders
    scene.pipeline->setShaderStages({ { QRhiShaderStage::Vertex, vertShader },
                                      { QRhiShaderStage::Fragment, fragShader } });

    // Setup vertex input layout
    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({
            { 5 * sizeof(float) }, // Per-vertex data (position + texCoord)
            { 22 * sizeof(float), QRhiVertexInputBinding::PerInstance } // Per-instance data
    });
    inputLayout.setAttributes({
            // Per-vertex attributes
            { 0, 0, QRhiVertexInputAttribute::Float3, 0 }, // position
            { 0, 1, QRhiVertexInputAttribute::Float2, 3 * sizeof(float) }, // texCoord

            // Per-instance attributes (model matrix as 4 vec4s)
            { 1, 2, QRhiVertexInputAttribute::Float4, 0 * sizeof(float) }, // row 0
            { 1, 3, QRhiVertexInputAttribute::Float4, 4 * sizeof(float) }, // row 1
            { 1, 4, QRhiVertexInputAttribute::Float4, 8 * sizeof(float) }, // row 2
            { 1, 5, QRhiVertexInputAttribute::Float4, 12 * sizeof(float) }, // row 3
            { 1, 6, QRhiVertexInputAttribute::Float4, 16 * sizeof(float) }, // shading vec4
            { 1, 7, QRhiVertexInputAttribute::Float, 20 * sizeof(float) }, // opacity
            { 1, 8, QRhiVertexInputAttribute::Float, 21 * sizeof(float) }, // flipFlag (1.0 = reflection)
            // rotation removed
    });
    scene.pipeline->setVertexInputLayout(inputLayout);

    // Set shader resource bindings and render pass descriptor
    scene.pipeline->setShaderResourceBindings(scene.shaderBindings.get());
    scene.pipeline->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());

    if (!scene.pipeline->create()) {
        qWarning() << "YACReaderFlow3D: Failed to create graphics pipeline!";
        scene.pipeline.reset();
    }
}

void YACReaderFlow3D::render(QRhiCommandBuffer *cb)
{
    if (!m_rhi || numObjects == 0)
        return;

    const QSize outputSize = renderTarget()->pixelSize();
    const QColor clearColor = backgroundColor;

    // Update positions and animations
    updatePositions();

    // Update index label if values changed
    updateIndexLabel();

    // Prepare view-projection matrix
    // Use fixed 20.0 degrees FOV - zoom is controlled via cfZ (camera distance)
    QMatrix4x4 projectionMatrix;
    projectionMatrix.perspective(20.0, float(outputSize.width()) / float(outputSize.height()), 1.0, 200.0);

    QMatrix4x4 viewMatrix;
    viewMatrix.translate(config.cfX, config.cfY, config.cfZ);
    viewMatrix.rotate(config.cfRX, 1, 0, 0);
    viewMatrix.rotate(viewRotate * config.viewAngle + config.cfRY, 0, 1, 0);
    viewMatrix.rotate(config.cfRZ, 0, 0, 1);

    QMatrix4x4 viewProjectionMatrix = projectionMatrix * viewMatrix;

    // Build draw order (back to front for proper alpha blending)
    QVector<int> drawOrder;
    for (int count = numObjects - 1; count > -1; count--) {
        if (count > currentSelected) {
            drawOrder.append(count);
        }
    }
    for (int count = 0; count < numObjects - 1; count++) {
        if (count < currentSelected) {
            drawOrder.append(count);
        }
    }
    drawOrder.append(currentSelected);

    // Structure to hold draw info
    struct DrawInfo {
        int imageIndex;
        bool isReflection;
        bool isMark;
        QRhiTexture *texture;
        float instanceData[22];
        UniformData uniformData;
    };

    // Collect all draws we need to make
    // Important: OpenGL draws reflections FIRST, then covers+marks (for correct depth sorting)
    QVector<DrawInfo> draws;

    // Phase 1: Add all reflections
    for (int idx : drawOrder) {
        if (idx < 0 || idx >= images.size() || !images[idx].texture)
            continue;

        DrawInfo reflDraw;
        reflDraw.imageIndex = idx;
        reflDraw.isReflection = true;
        reflDraw.isMark = false;
        reflDraw.texture = images[idx].texture;
        prepareDrawData(images[idx], true, false, viewProjectionMatrix, reflDraw.instanceData, reflDraw.uniformData);
        draws.append(reflDraw);
    }

    // Phase 2: Add all covers (and marks)
    for (int idx : drawOrder) {
        if (idx < 0 || idx >= images.size() || !images[idx].texture)
            continue;

        // Add cover draw
        DrawInfo coverDraw;
        coverDraw.imageIndex = idx;
        coverDraw.isReflection = false;
        coverDraw.isMark = false;
        coverDraw.texture = images[idx].texture;
        prepareDrawData(images[idx], false, false, viewProjectionMatrix, coverDraw.instanceData, coverDraw.uniformData);
        draws.append(coverDraw);

        if (idx < 0 || idx >= marks.size())
            continue;

        if (idx >= loaded.size())
            continue;

        // Add mark draw immediately after its cover
        if (showMarks && loaded[idx] && marks[idx] != Unread) {
            QRhiTexture *markTex = (marks[idx] == Read) ? scene.markTexture.get() : scene.readingTexture.get();
            if (markTex) {
                DrawInfo markDraw;
                markDraw.imageIndex = idx;
                markDraw.isReflection = false;
                markDraw.isMark = true;
                markDraw.texture = markTex;
                prepareDrawData(images[idx], false, true, viewProjectionMatrix, markDraw.instanceData, markDraw.uniformData);
                draws.append(markDraw);
            }
        }
    }

    // Ensure uniform buffer is large enough
    ensureUniformBufferCapacity(draws.size());

    if (!scene.uniformBuffer) {
        qWarning() << "YACReaderFlow3D: No uniform buffer available for rendering";
        return;
    }

    // Ensure pipeline exists (may not exist if content was added after empty initialization)
    ensurePipeline();

    if (!scene.pipeline) {
        qWarning() << "YACReaderFlow3D: No pipeline available for rendering";
        return;
    }

    // Ensure instance buffer is large enough for all draws
    auto requiredInstanceSize = static_cast<quint32>(draws.size() * 22 * sizeof(float));
    if (!scene.instanceBuffer || scene.instanceBuffer->size() < requiredInstanceSize) {
        scene.instanceBuffer.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, requiredInstanceSize));
        if (!scene.instanceBuffer->create()) {
            qWarning() << "YACReaderFlow3D: Failed to create instance buffer of size" << requiredInstanceSize;
            return;
        }
    }

    // === PHASE 1: PREPARE (BEFORE PASS) ===
    // Update ALL uniform and instance data for ALL draws in one batch
    QRhiResourceUpdateBatch *batch = m_rhi->nextResourceUpdateBatch();

    // Process pending texture uploads
    if (!pendingTextureUploads.isEmpty()) {
        for (const auto &upload : std::as_const(pendingTextureUploads)) {
            if (upload.index >= 0 && upload.index < images.size() && images[upload.index].texture) {
                batch->uploadTexture(images[upload.index].texture, upload.image);
                batch->generateMips(images[upload.index].texture);
            }
        }
        pendingTextureUploads.clear();
    }

    // Update uniform buffer with all draw data
    for (int i = 0; i < draws.size(); ++i) {
        int offset = i * scene.alignedUniformSize;
        batch->updateDynamicBuffer(scene.uniformBuffer.get(), offset, sizeof(UniformData), &draws[i].uniformData);
    }

    // Update instance buffer with all instance data
    for (int i = 0; i < draws.size(); ++i) {
        int offset = i * 22 * sizeof(float);
        batch->updateDynamicBuffer(scene.instanceBuffer.get(), offset, 22 * sizeof(float), draws[i].instanceData);
    }

    // === PHASE 2: RENDER (DURING PASS) ===
    cb->beginPass(renderTarget(), clearColor, { 1.0f, 0 }, batch);

    cb->setGraphicsPipeline(scene.pipeline.get());
    cb->setViewport(QRhiViewport(0, 0, outputSize.width(), outputSize.height()));

    // Execute all draws
    for (int i = 0; i < draws.size(); ++i) {
        const DrawInfo &draw = draws[i];
        executeDrawWithOffset(cb, draw.texture, draw.instanceData, i);
    }

    cb->endPass();
}

void YACReaderFlow3D::prepareDrawData(const YACReader3DImageRHI &image, bool isReflection, bool isMark,
                                      const QMatrix4x4 &viewProjectionMatrix,
                                      float *outInstanceData, UniformData &outUniformData)
{
    float w = image.width;
    float h = image.height;

    // Calculate opacity
    float opacity = 1 - 1 / (config.animationFadeOutDist + config.viewRotateLightStrenght * fabs(viewRotate)) * fabs(0 - image.current.x);

    // Calculate shading
    float LShading = ((config.rotation != 0) ? ((image.current.rot < 0) ? 1 - 1 / config.rotation * image.current.rot : 1) : 1);
    float RShading = ((config.rotation != 0) ? ((image.current.rot > 0) ? 1 - 1 / (config.rotation * -1) * image.current.rot : 1) : 1);
    float LUP = shadingTop + (1 - shadingTop) * LShading;
    float LDOWN = shadingBottom + (1 - shadingBottom) * LShading;
    float RUP = shadingTop + (1 - shadingTop) * RShading;
    float RDOWN = shadingBottom + (1 - shadingBottom) * RShading;

    QMatrix4x4 modelMatrix;
    modelMatrix.translate(image.current.x, image.current.y, image.current.z);
    modelMatrix.rotate(image.current.rot, 0, 1, 0);

    if (isMark) {
        // Mark-specific transform
        float markWidth = 0.15f;
        float markHeight = 0.2f;
        float markCenterX = w / 2.0f - 0.125f;
        float markCenterY = -0.588f + h;
        modelMatrix.translate(markCenterX, markCenterY, 0.001f);
        modelMatrix.scale(markWidth, markHeight, 1.0f);

        float shadingValue = RUP * opacity;
        outInstanceData[16] = shadingValue;
        outInstanceData[17] = shadingValue;
        outInstanceData[18] = shadingValue;
        outInstanceData[19] = shadingValue;
        outInstanceData[20] = 1.0f;
        outInstanceData[21] = isReflection ? 1.0f : 0.0f;
    } else {
        // Cover/reflection transform
        if (isReflection) {
            modelMatrix.translate(0.0f, -0.5f - h / 2.0f, 0.0f);
            // Swap vertical shading for reflection
            float temp = LUP;
            LUP = LDOWN;
            LDOWN = temp;
            temp = RUP;
            RUP = RDOWN;
            RDOWN = temp;
        } else {
            modelMatrix.translate(0.0f, -0.5f + h / 2.0f, 0.0f);
        }
        modelMatrix.scale(w, h, 1.0f);

        outInstanceData[16] = LUP;
        outInstanceData[17] = LDOWN;
        outInstanceData[18] = RUP;
        outInstanceData[19] = RDOWN;
        outInstanceData[20] = opacity;
        outInstanceData[21] = isReflection ? 1.0f : 0.0f;
    }

    // Pack model matrix into instance data
    const float *matData = modelMatrix.constData();
    for (int i = 0; i < 16; i++) {
        outInstanceData[i] = matData[i];
    }

    // Prepare uniform data (copy float data into POD arrays)
    const float *vp = viewProjectionMatrix.constData();
    for (int m = 0; m < 16; ++m)
        outUniformData.viewProjectionMatrix[m] = vp[m];

    outUniformData.backgroundColor[0] = backgroundColor.redF();
    outUniformData.backgroundColor[1] = backgroundColor.greenF();
    outUniformData.backgroundColor[2] = backgroundColor.blueF();
    outUniformData._pad0 = 0.0f;

    // shadingColor removed from uniform buffer; keep CPU-side shadingColor member intact

    outUniformData.reflectionUp = reflectionUp;
    outUniformData.reflectionDown = reflectionBottom;
    outUniformData.isReflection = isReflection ? 1.0f : 0.0f;
    outUniformData._pad1 = 0.0f;
}

void YACReaderFlow3D::executeDrawWithOffset(QRhiCommandBuffer *cb, QRhiTexture *texture,
                                            const float *instanceData, int uniformSlot)
{
    if (!texture || !scene.instanceBuffer || !scene.vertexBuffer)
        return;

    Q_UNUSED(instanceData)

    // Get or create shader resource bindings for this texture with dynamic offset support
    QRhiShaderResourceBindings *srb = scene.shaderBindingsCache.value(texture, nullptr);
    if (!srb) {
        srb = m_rhi->newShaderResourceBindings();
        srb->setBindings({ QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, scene.uniformBuffer.get(), sizeof(UniformData)),
                           QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, texture, scene.sampler.get()) });
        srb->create();
        scene.shaderBindingsCache.insert(texture, srb);
    }

    // Set shader resources with dynamic offset for uniform buffer
    QRhiCommandBuffer::DynamicOffset dynOfs[] = {
        { 0, quint32(uniformSlot * scene.alignedUniformSize) }
    };
    cb->setShaderResources(srb, 1, dynOfs);

    // Bind vertex buffers with offset into instance buffer
    const QRhiCommandBuffer::VertexInput vbufBindings[] = {
        { scene.vertexBuffer.get(), 0 },
        { scene.instanceBuffer.get(), quint32(uniformSlot * 22 * sizeof(float)) }
    };
    cb->setVertexInput(0, 2, vbufBindings);

    // Draw two triangles (6 vertices) forming a quad
    cb->draw(6);
}

// Note: The old drawCover() and drawMark() methods have been removed.
// Rendering now uses prepareDrawData() and executeDrawWithOffset() which properly
// batch all resource updates before the render pass begins, following Qt RHI best practices.

void YACReaderFlow3D::releaseResources()
{
    scene.reset();
    m_rhi = nullptr;
}

void YACReaderFlow3D::showEvent(QShowEvent *event)
{
    QRhiWidget::showEvent(event);
    startAnimationTimer();
}

void YACReaderFlow3D::resizeEvent(QResizeEvent *event)
{
    QRhiWidget::resizeEvent(event);
    updateIndexLabelStyle();
}

void YACReaderFlow3D::cleanupAnimation()
{
    config.animationStep = stepBackup;
    viewRotateActive = 0;
}

void YACReaderFlow3D::draw()
{
    update();
}

void YACReaderFlow3D::calcPos(YACReader3DImageRHI &image, int pos)
{
    if (flowRightToLeft) {
        pos = pos * -1;
    }

    if (pos == 0) {
        image.current = centerPos;
    } else {
        if (pos > 0) {
            image.current.x = (config.centerDistance) + (config.xDistance * pos);
            image.current.y = config.yDistance * pos * -1;
            image.current.z = config.zDistance * pos * -1;
            image.current.rot = config.rotation;
        } else {
            image.current.x = (config.centerDistance) * -1 + (config.xDistance * pos);
            image.current.y = config.yDistance * pos;
            image.current.z = config.zDistance * pos;
            image.current.rot = config.rotation * -1;
        }
    }
}

void YACReaderFlow3D::calcVector(YACReader3DVector &vector, int pos)
{
    calcPos(dummy, pos);
    vector.x = dummy.current.x;
    vector.y = dummy.current.y;
    vector.z = dummy.current.z;
    vector.rot = dummy.current.rot;
}

bool YACReaderFlow3D::animate(YACReader3DVector &currentVector, YACReader3DVector &toVector)
{
    float rotDiff = toVector.rot - currentVector.rot;
    float xDiff = toVector.x - currentVector.x;
    float yDiff = toVector.y - currentVector.y;
    float zDiff = toVector.z - currentVector.z;

    if (fabs(rotDiff) < 0.01 && fabs(xDiff) < 0.001 && fabs(yDiff) < 0.001 && fabs(zDiff) < 0.001)
        return true;

    currentVector.x = currentVector.x + (xDiff)*config.animationStep;
    currentVector.y = currentVector.y + (yDiff)*config.animationStep;
    currentVector.z = currentVector.z + (zDiff)*config.animationStep;

    if (fabs(rotDiff) > 0.01) {
        currentVector.rot = currentVector.rot + (rotDiff) * (config.animationStep * config.preRotation);
    } else {
        viewRotateActive = 0;
    }

    return false;
}

void YACReaderFlow3D::showPrevious()
{
    startAnimationTimer();

    if (currentSelected > 0) {
        currentSelected--;
        emit centerIndexChanged(currentSelected);
        config.animationStep *= config.animationSpeedUp;

        if (config.animationStep > config.animationStepMax) {
            config.animationStep = config.animationStepMax;
        }

        if (viewRotateActive && viewRotate > -1) {
            viewRotate -= config.viewRotateAdd;
        }

        viewRotateActive = 1;
    }
}

void YACReaderFlow3D::showNext()
{
    startAnimationTimer();

    if (currentSelected < numObjects - 1) {
        currentSelected++;
        emit centerIndexChanged(currentSelected);
        config.animationStep *= config.animationSpeedUp;

        if (config.animationStep > config.animationStepMax) {
            config.animationStep = config.animationStepMax;
        }

        if (viewRotateActive && viewRotate < 1) {
            viewRotate += config.viewRotateAdd;
        }

        viewRotateActive = 1;
    }
}

void YACReaderFlow3D::setCurrentIndex(int pos)
{
    if (!(pos >= 0 && pos < images.length() && images.length() > 0))
        return;
    if (pos >= images.length() && images.length() > 0)
        pos = images.length() - 1;

    startAnimationTimer();

    currentSelected = pos;
    config.animationStep *= config.animationSpeedUp;

    if (config.animationStep > config.animationStepMax) {
        config.animationStep = config.animationStepMax;
    }

    if (viewRotateActive && viewRotate < 1) {
        viewRotate += config.viewRotateAdd;
    }

    viewRotateActive = 1;
}

void YACReaderFlow3D::updatePositions()
{
    int count;
    bool stopAnimation = true;

    for (count = numObjects - 1; count > -1; count--) {
        calcVector(images[count].animEnd, count - currentSelected);
        if (!animate(images[count].current, images[count].animEnd))
            stopAnimation = false;
    }

    if (!viewRotateActive) {
        viewRotate += (0 - viewRotate) * config.viewRotateSub;
    }

    if (fabs(images[currentSelected].current.x - images[currentSelected].animEnd.x) < 1) {
        cleanupAnimation();
        if (updateCount >= 0) {
            updateCount = 0;
            updateImageData();
        } else
            updateCount++;
    } else
        updateCount++;

    if (stopAnimation)
        stopAnimationTimer();
}

void YACReaderFlow3D::insert(QRhiTexture *texture, float x, float y, int item)
{
    startAnimationTimer();

    if (item == -1) {
        images.push_back(YACReader3DImageRHI());
        item = numObjects;
        numObjects++;
        calcVector(images[item].current, item);
        images[item].current.z = images[item].current.z - 1;
    }

    images[item].texture = texture;
    images[item].width = x;
    images[item].height = y;
    images[item].index = item;
}

void YACReaderFlow3D::remove(int item)
{
    if (item < 0 || item >= images.size())
        return;

    startAnimationTimer();

    loaded.remove(item);
    marks.remove(item);

    if (item <= currentSelected && currentSelected != 0) {
        currentSelected--;
    }

    QRhiTexture *texture = images[item].texture;

    int count = item;
    while (count <= numObjects - 1) {
        images[count].index--;
        count++;
    }
    images.removeAt(item);

    if (texture != scene.defaultTexture.get()) {
        // Remove shader bindings for this texture before deleting it
        auto it = scene.shaderBindingsCache.find(texture);
        if (it != scene.shaderBindingsCache.end()) {
            delete it.value();
            scene.shaderBindingsCache.erase(it);
        }
        delete texture;
    }

    numObjects--;
}

void YACReaderFlow3D::add(int item)
{
    float x = 1;
    float y = 1 * (700.f / 480.0f);

    images.insert(item, YACReader3DImageRHI());
    loaded.insert(item, false);
    marks.insert(item, Unread);
    numObjects++;

    for (int i = item + 1; i < numObjects; i++) {
        images[i].index++;
    }

    insert(scene.defaultTexture.get(), x, y, item);
}

YACReader3DImageRHI YACReaderFlow3D::getCurrentSelected()
{
    return images[currentSelected];
}

void YACReaderFlow3D::replace(QRhiTexture *texture, float x, float y, int item)
{
    startAnimationTimer();

    if (images[item].index == item) {
        images[item].texture = texture;
        images[item].width = x;
        images[item].height = y;
        loaded[item] = true;
    } else
        loaded[item] = false;
}

void YACReaderFlow3D::populate(int n)
{
    if (hasBeenInitialized) {
        clear();
    }
    emit centerIndexChanged(0);

    float x = 1;
    float y = 1 * (700.f / 480.0f);
    int i;

    for (i = 0; i < n; i++) {
        insert(scene.defaultTexture.get(), x, y);
    }

    loaded = QVector<bool>(n, false);
}

void YACReaderFlow3D::reset()
{
    startAnimationTimer();

    currentSelected = 0;
    loaded.clear();
    marks.clear();

    // Clean up image textures and remove their entries from shader bindings cache
    for (int i = 0; i < numObjects; i++) {
        if (images[i].texture != scene.defaultTexture.get()) {
            // Remove shader bindings for this texture before deleting it
            auto it = scene.shaderBindingsCache.find(images[i].texture);
            if (it != scene.shaderBindingsCache.end()) {
                delete it.value();
                scene.shaderBindingsCache.erase(it);
            }
            delete images[i].texture;
        }
    }

    numObjects = 0;
    images.clear();

    if (!hasBeenInitialized)
        lazyPopulateObjects = -1;
}

void YACReaderFlow3D::reload()
{
    startAnimationTimer();
    int n = numObjects;
    reset();
    populate(n);
}

// Slot implementations
void YACReaderFlow3D::setCF_RX(int value)
{
    startAnimationTimer();
    config.cfRX = value;
}

void YACReaderFlow3D::setCF_RY(int value)
{
    startAnimationTimer();
    config.cfRY = value;
}

void YACReaderFlow3D::setCF_RZ(int value)
{
    startAnimationTimer();
    config.cfRZ = value;
}

void YACReaderFlow3D::setRotation(int angle)
{
    startAnimationTimer();
    config.rotation = -angle;
}

void YACReaderFlow3D::setX_Distance(int distance)
{
    startAnimationTimer();
    config.xDistance = distance / 100.0;
}

void YACReaderFlow3D::setCenter_Distance(int distance)
{
    startAnimationTimer();
    config.centerDistance = distance / 100.0;
}

void YACReaderFlow3D::setZ_Distance(int distance)
{
    startAnimationTimer();
    config.zDistance = distance / 100.0;
}

void YACReaderFlow3D::setCF_Y(int value)
{
    startAnimationTimer();
    config.cfY = value / 100.0;
}

void YACReaderFlow3D::setCF_Z(int value)
{
    startAnimationTimer();
    config.cfZ = value;
}

void YACReaderFlow3D::setY_Distance(int value)
{
    startAnimationTimer();
    config.yDistance = value / 100.0;
}

void YACReaderFlow3D::setFadeOutDist(int value)
{
    startAnimationTimer();
    config.animationFadeOutDist = value;
}

void YACReaderFlow3D::setLightStrenght(int value)
{
    startAnimationTimer();
    config.viewRotateLightStrenght = value;
}

void YACReaderFlow3D::setMaxAngle(int value)
{
    startAnimationTimer();
    config.viewAngle = value;
}

void YACReaderFlow3D::setPreset(const Preset &p)
{
    startAnimationTimer();
    config = p;
}

void YACReaderFlow3D::setZoom(int zoom)
{
    startAnimationTimer();
    config.zoom = zoom;
}

void YACReaderFlow3D::setPerformance(Performance performance)
{
    if (this->performance != performance) {
        startAnimationTimer();
        this->performance = performance;
        reload();
    }
}

void YACReaderFlow3D::useVSync(bool b)
{
    // No-op for RHI - VSync is handled by the platform
    Q_UNUSED(b);
}

void YACReaderFlow3D::setShowMarks(bool value)
{
    startAnimationTimer();
    showMarks = value;
}

void YACReaderFlow3D::setMarks(QVector<YACReader::YACReaderComicReadStatus> marks)
{
    startAnimationTimer();
    this->marks = marks;
}

void YACReaderFlow3D::setMarkImage(QImage &image)
{
    Q_UNUSED(image);
}

void YACReaderFlow3D::markSlide(int index, YACReader::YACReaderComicReadStatus status)
{
    startAnimationTimer();
    marks[index] = status;
}

void YACReaderFlow3D::unmarkSlide(int index)
{
    startAnimationTimer();
    marks[index] = YACReader::Unread;
}

void YACReaderFlow3D::setSlideSize(QSize size)
{
    Q_UNUSED(size);
}

void YACReaderFlow3D::clear()
{
    reset();
}

void YACReaderFlow3D::setCenterIndex(unsigned int index)
{
    setCurrentIndex(index);
}

void YACReaderFlow3D::showSlide(int index)
{
    setCurrentIndex(index);
}

int YACReaderFlow3D::centerIndex()
{
    return currentSelected;
}

void YACReaderFlow3D::updateMarks() { }

void YACReaderFlow3D::render()
{
    update();
}

void YACReaderFlow3D::resizeGL(int width, int height)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
    // No-op for RHI - handled automatically
}

void YACReaderFlow3D::setFlowRightToLeft(bool b)
{
    flowRightToLeft = b;
}

void YACReaderFlow3D::setBackgroundColor(const QColor &color)
{
    backgroundColor = color;
    update();
}

void YACReaderFlow3D::setTextColor(const QColor &color)
{
    textColor = color;

    QPalette palette = indexLabel->palette();
    palette.setColor(QPalette::WindowText, textColor);
    indexLabel->setPalette(palette);

    update();
}

void YACReaderFlow3D::setShadingColor(const QColor &color)
{
    shadingColor = color;
    update();
}

// Event handlers
void YACReaderFlow3D::wheelEvent(QWheelEvent *event)
{
    Movement m = getMovement(event);
    switch (m) {
    case None:
        return;
    case Forward:
        showNext();
        break;
    case Backward:
        showPrevious();
        break;
    default:
        break;
    }
}

void YACReaderFlow3D::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Left && !flowRightToLeft) || (event->key() == Qt::Key_Right && flowRightToLeft)) {
        if (event->modifiers() == Qt::ControlModifier)
            setCurrentIndex((currentSelected - 10 < 0) ? 0 : currentSelected - 10);
        else
            showPrevious();
        event->accept();
        return;
    }

    if ((event->key() == Qt::Key_Right && !flowRightToLeft) || (event->key() == Qt::Key_Left && flowRightToLeft)) {
        if (event->modifiers() == Qt::ControlModifier)
            setCurrentIndex((currentSelected + 10 >= numObjects) ? numObjects - 1 : currentSelected + 10);
        else
            showNext();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Up) {
        return;
    }

    event->ignore();
}

void YACReaderFlow3D::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && currentSelected >= 0 && currentSelected < images.size()) {
        auto position = event->position();
        QVector3D intersection = getPlaneIntersection(position.x(), position.y(), images[currentSelected]);
        if ((intersection.x() > 0.5 && !flowRightToLeft) || (intersection.x() < -0.5 && flowRightToLeft)) {
            showNext();
        } else if ((intersection.x() < -0.5 && !flowRightToLeft) || (intersection.x() > 0.5 && flowRightToLeft)) {
            showPrevious();
        }
    } else {
        QRhiWidget::mousePressEvent(event);
    }
}

void YACReaderFlow3D::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (currentSelected >= 0 && currentSelected < images.size()) {
        auto position = event->position();
        QVector3D intersection = getPlaneIntersection(position.x(), position.y(), images[currentSelected]);

        if (intersection.x() < 0.5 && intersection.x() > -0.5) {
            emit selected(centerIndex());
            event->accept();
        }
    }
}

QVector3D YACReaderFlow3D::getPlaneIntersection(int x, int y, YACReader3DImageRHI plane)
{
    // Simplified for now - proper ray-plane intersection calculation needed
    // This requires access to the viewport and matrices
    const QSize outputSize = renderTarget()->pixelSize();

    QMatrix4x4 m_projection;
    m_projection.perspective(config.zoom, float(outputSize.width()) / float(outputSize.height()), 1.0, 200.0);

    QMatrix4x4 m_modelview;
    m_modelview.translate(config.cfX, config.cfY, config.cfZ);
    m_modelview.rotate(config.cfRX, 1, 0, 0);
    m_modelview.rotate(viewRotate * config.viewAngle + config.cfRY, 0, 1, 0);
    m_modelview.rotate(config.cfRZ, 0, 0, 1);
    m_modelview.translate(plane.current.x, plane.current.y, plane.current.z);
    m_modelview.rotate(plane.current.rot, 0, 1, 0);
    m_modelview.scale(plane.width, plane.height, 1.0f);

    QVector3D ray_origin(x * devicePixelRatioF(), y * devicePixelRatioF(), 0);
    QVector3D ray_end(x * devicePixelRatioF(), y * devicePixelRatioF(), 1.0);

    ray_origin = ray_origin.unproject(m_modelview, m_projection, QRect(0, 0, outputSize.width(), outputSize.height()));
    ray_end = ray_end.unproject(m_modelview, m_projection, QRect(0, 0, outputSize.width(), outputSize.height()));

    QVector3D ray_vector = ray_end - ray_origin;

    QVector3D plane_origin(-0.5f, -0.5f, 0);
    QVector3D plane_vektor_1 = QVector3D(0.5f, -0.5f, 0) - plane_origin;
    QVector3D plane_vektor_2 = QVector3D(-0.5f, 0.5f, 0) - plane_origin;

    double intersection_LES_determinant = ((plane_vektor_1.x() * plane_vektor_2.y() * (-1) * ray_vector.z()) +
                                           (plane_vektor_2.x() * (-1) * ray_vector.y() * plane_vektor_1.z()) +
                                           ((-1) * ray_vector.x() * plane_vektor_1.y() * plane_vektor_2.z()) -
                                           ((-1) * ray_vector.x() * plane_vektor_2.y() * plane_vektor_1.z()) -
                                           (plane_vektor_1.x() * (-1) * ray_vector.y() * plane_vektor_2.z()) -
                                           (plane_vektor_2.x() * plane_vektor_1.y() * (-1) * ray_vector.z()));

    QVector3D det = ray_origin - plane_origin;

    double intersection_ray_determinant = ((plane_vektor_1.x() * plane_vektor_2.y() * det.z()) +
                                           (plane_vektor_2.x() * det.y() * plane_vektor_1.z()) +
                                           (det.x() * plane_vektor_1.y() * plane_vektor_2.z()) -
                                           (det.x() * plane_vektor_2.y() * plane_vektor_1.z()) -
                                           (plane_vektor_1.x() * det.y() * plane_vektor_2.z()) -
                                           (plane_vektor_2.x() * plane_vektor_1.y() * det.z()));

    return ray_origin + ray_vector * (intersection_ray_determinant / intersection_LES_determinant);
}

void YACReaderFlow3D::updateIndexLabel()
{
    int currentDisplay = currentSelected + 1;
    int totalDisplay = numObjects;

    if (indexLabelState.current != currentDisplay || indexLabelState.total != totalDisplay) {
        indexLabelState.current = currentDisplay;
        indexLabelState.total = totalDisplay;
        indexLabel->setText(QString("%1/%2").arg(currentDisplay).arg(totalDisplay));
        indexLabel->adjustSize();
    }
}

void YACReaderFlow3D::updateIndexLabelStyle()
{
    int w = width();
    int h = height();

    int newFontSize = static_cast<int>((w + h) * 0.010);
    if (newFontSize < 10)
        newFontSize = 10;

    QFont font("Arial", newFontSize);
    indexLabel->setFont(font);

    QPalette palette = indexLabel->palette();
    palette.setColor(QPalette::WindowText, textColor);
    indexLabel->setPalette(palette);

    indexLabel->move(10, 10);
    indexLabel->adjustSize();
}

QSize YACReaderFlow3D::minimumSizeHint() const
{
    return QSize(320, 200);
}
