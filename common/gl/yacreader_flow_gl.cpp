#include "yacreader_flow_gl.h"

#include <QtGui>
#include <QMatrix4x4>
#include <cmath>

// Structure for per-instance data
struct InstanceData {
    QMatrix4x4 modelMatrix;
    float leftUpShading;
    float leftDownShading;
    float rightUpShading;
    float rightDownShading;
    float opacity;
    float padding[3]; // Align to 16 bytes
};

/*** Preset Configurations ***/

int YACReaderFlowGL::updateInterval = 16;

struct Preset defaultYACReaderFlowConfig = {
    0.08f, // Animation_step sets the speed of the animation
    1.5f, // Animation_speedup sets the acceleration of the animation
    0.1f, // Animation_step_max sets the maximum speed of the animation
    3.f, // Animation_Fade_out_dis sets the distance of view

    1.5f, // pre_rotation sets the rotation increasion
    3.f, // View_rotate_light_strenght sets the light strenght on rotation
    0.01f, // View_rotate_add sets the speed of the rotation
    0.02f, // View_rotate_sub sets the speed of reversing the rotation
    20.f, // View_angle sets the maximum view angle

    0.f, // CF_X the X Position of the Coverflow
    0.f, // CF_Y the Y Position of the Coverflow
    -8.f, // CF_Z the Z Position of the Coverflow

    15.f, // CF_RX the X Rotation of the Coverflow
    0.f, // CF_RY the Y Rotation of the Coverflow
    0.f, // CF_RZ the Z Rotation of the Coverflow

    -50.f, // Rotation sets the rotation of each cover
    0.18f, // X_Distance sets the distance between the covers
    1.f, // Center_Distance sets the distance between the centered and the non centered covers
    0.1f, // Z_Distance sets the pushback amount
    0.0f, // Y_Distance sets the elevation amount

    30.f // zoom level

};

struct Preset presetYACReaderFlowClassicConfig = {
    0.08f, // Animation_step sets the speed of the animation
    1.5f, // Animation_speedup sets the acceleration of the animation
    0.1f, // Animation_step_max sets the maximum speed of the animation
    2.f, // Animation_Fade_out_dis sets the distance of view

    1.5f, // pre_rotation sets the rotation increasion
    3.f, // View_rotate_light_strenght sets the light strenght on rotation
    0.08f, // View_rotate_add sets the speed of the rotation
    0.08f, // View_rotate_sub sets the speed of reversing the rotation
    30.f, // View_angle sets the maximum view angle

    0.f, // CF_X the X Position of the Coverflow
    -0.2f, // CF_Y the Y Position of the Coverflow
    -7.f, // CF_Z the Z Position of the Coverflow

    0.f, // CF_RX the X Rotation of the Coverflow
    0.f, // CF_RY the Y Rotation of the Coverflow
    0.f, // CF_RZ the Z Rotation of the Coverflow

    -40.f, // Rotation sets the rotation of each cover
    0.18f, // X_Distance sets the distance between the covers
    1.f, // Center_Distance sets the distance between the centered and the non centered covers
    0.1f, // Z_Distance sets the pushback amount
    0.0f, // Y_Distance sets the elevation amount

    22.f // zoom level

};

struct Preset presetYACReaderFlowStripeConfig = {
    0.08f, // Animation_step sets the speed of the animation
    1.5f, // Animation_speedup sets the acceleration of the animation
    0.1f, // Animation_step_max sets the maximum speed of the animation
    6.f, // Animation_Fade_out_dis sets the distance of view

    1.5f, // pre_rotation sets the rotation increasion
    4.f, // View_rotate_light_strenght sets the light strenght on rotation
    0.08f, // View_rotate_add sets the speed of the rotation
    0.08f, // View_rotate_sub sets the speed of reversing the rotation
    30.f, // View_angle sets the maximum view angle

    0.f, // CF_X the X Position of the Coverflow
    -0.2f, // CF_Y the Y Position of the Coverflow
    -7.f, // CF_Z the Z Position of the Coverflow

    0.f, // CF_RX the X Rotation of the Coverflow
    0.f, // CF_RY the Y Rotation of the Coverflow
    0.f, // CF_RZ the Z Rotation of the Coverflow

    0.f, // Rotation sets the rotation of each cover
    1.1f, // X_Distance sets the distance between the covers
    0.2f, // Center_Distance sets the distance between the centered and the non centered covers
    0.01f, // Z_Distance sets the pushback amount
    0.0f, // Y_Distance sets the elevation amount

    22.f // zoom level

};

struct Preset presetYACReaderFlowOverlappedStripeConfig = {
    0.08f, // Animation_step sets the speed of the animation
    1.5f, // Animation_speedup sets the acceleration of the animation
    0.1f, // Animation_step_max sets the maximum speed of the animation
    2.f, // Animation_Fade_out_dis sets the distance of view

    1.5f, // pre_rotation sets the rotation increasion
    3.f, // View_rotate_light_strenght sets the light strenght on rotation
    0.08f, // View_rotate_add sets the speed of the rotation
    0.08f, // View_rotate_sub sets the speed of reversing the rotation
    30.f, // View_angle sets the maximum view angle

    0.f, // CF_X the X Position of the Coverflow
    -0.2f, // CF_Y the Y Position of the Coverflow
    -7.f, // CF_Z the Z Position of the Coverflow

    0.f, // CF_RX the X Rotation of the Coverflow
    0.f, // CF_RY the Y Rotation of the Coverflow
    0.f, // CF_RZ the Z Rotation of the Coverflow

    0.f, // Rotation sets the rotation of each cover
    0.18f, // X_Distance sets the distance between the covers
    1.f, // Center_Distance sets the distance between the centered and the non centered covers
    0.1f, // Z_Distance sets the pushback amount
    0.0f, // Y_Distance sets the elevation amount

    22.f // zoom level

};

struct Preset pressetYACReaderFlowUpConfig = {
    0.08f, // Animation_step sets the speed of the animation
    1.5f, // Animation_speedup sets the acceleration of the animation
    0.1f, // Animation_step_max sets the maximum speed of the animation
    2.5f, // Animation_Fade_out_dis sets the distance of view

    1.5f, // pre_rotation sets the rotation increasion
    3.f, // View_rotate_light_strenght sets the light strenght on rotation
    0.08f, // View_rotate_add sets the speed of the rotation
    0.08f, // View_rotate_sub sets the speed of reversing the rotation
    5.f, // View_angle sets the maximum view angle

    0.f, // CF_X the X Position of the Coverflow
    -0.2f, // CF_Y the Y Position of the Coverflow
    -7.f, // CF_Z the Z Position of the Coverflow

    0.f, // CF_RX the X Rotation of the Coverflow
    0.f, // CF_RY the Y Rotation of the Coverflow
    0.f, // CF_RZ the Z Rotation of the Coverflow

    -50.f, // Rotation sets the rotation of each cover
    0.18f, // X_Distance sets the distance between the covers
    1.f, // Center_Distance sets the distance between the centered and the non centered covers
    0.1f, // Z_Distance sets the pushback amount
    -0.1f, // Y_Distance sets the elevation amount

    22.f // zoom level

};

struct Preset pressetYACReaderFlowDownConfig = {
    0.08f, // Animation_step sets the speed of the animation
    1.5f, // Animation_speedup sets the acceleration of the animation
    0.1f, // Animation_step_max sets the maximum speed of the animation
    2.5f, // Animation_Fade_out_dis sets the distance of view

    1.5f, // pre_rotation sets the rotation increasion
    3.f, // View_rotate_light_strenght sets the light strenght on rotation
    0.08f, // View_rotate_add sets the speed of the rotation
    0.08f, // View_rotate_sub sets the speed of reversing the rotation
    5.f, // View_angle sets the maximum view angle

    0.f, // CF_X the X Position of the Coverflow
    -0.2f, // CF_Y the Y Position of the Coverflow
    -7.f, // CF_Z the Z Position of the Coverflow

    0.f, // CF_RX the X Rotation of the Coverflow
    0.f, // CF_RY the Y Rotation of the Coverflow
    0.f, // CF_RZ the Z Rotation of the Coverflow

    -50.f, // Rotation sets the rotation of each cover
    0.18f, // X_Distance sets the distance between the covers
    1.f, // Center_Distance sets the distance between the centered and the non centered covers
    0.1f, // Z_Distance sets the pushback amount
    0.1f, // Y_Distance sets the elevation amount

    22.f // zoom level
};
/*Constructor*/
YACReaderFlowGL::YACReaderFlowGL(QWidget *parent, struct Preset p)
    : QOpenGLWidget(parent), numObjects(0), lazyPopulateObjects(-1), hasBeenInitialized(false), bUseVSync(false), flowRightToLeft(false)
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
    reflectionUp = 0.f;
    reflectionBottom = 0.6f;

    setBackgroundColor(Qt::black);

    numObjects = 0;
    viewRotate = 0.f;
    viewRotateActive = 0;
    stepBackup = config.animationStep / config.animationSpeedUp;

    QSurfaceFormat f = format();
    f.setSamples(4);
    f.setSwapInterval(0);

    // Detect if we should use OpenGL ES
    // Check if app-level ES is forced (via Qt::AA_UseOpenGLES)
    bool forceES = QCoreApplication::testAttribute(Qt::AA_UseOpenGLES);

    if (forceES) {
        // Use OpenGL ES 3.0
        f.setRenderableType(QSurfaceFormat::OpenGLES);
        f.setVersion(3, 0);
        qDebug() << "YACReaderFlowGL: Requesting OpenGL ES 3.0 context";
    } else {
        // Use Desktop OpenGL 3.3 Core
        f.setRenderableType(QSurfaceFormat::OpenGL);
        f.setVersion(3, 3);
        f.setProfile(QSurfaceFormat::CoreProfile);
        qDebug() << "YACReaderFlowGL: Requesting Desktop OpenGL 3.3 Core context";
    }

    setFormat(f);

    timerId = startTimer(updateInterval);
}

void YACReaderFlowGL::timerEvent(QTimerEvent *event)
{
    if (timerId == event->timerId())
        update();
}

void YACReaderFlowGL::startAnimationTimer()
{
    if (timerId == -1)
        timerId = startTimer(updateInterval);
}

void YACReaderFlowGL::stopAnimationTimer()
{
    if (timerId != -1) {
        killTimer(timerId);
        timerId = -1;
    }
}

YACReaderFlowGL::~YACReaderFlowGL()
{
    makeCurrent();

    delete vao;
    delete vbo;
    delete instanceVBO;
    delete shaderProgram;

    if (defaultTexture) {
        if (defaultTexture->isCreated())
            defaultTexture->destroy();
        delete defaultTexture;
    }

#ifdef YACREADER_LIBRARY
    if (markTexture) {
        if (markTexture->isCreated())
            markTexture->destroy();
        delete markTexture;
    }

    if (readingTexture) {
        if (readingTexture->isCreated())
            readingTexture->destroy();
        delete readingTexture;
    }
#endif

    doneCurrent();
}

QSize YACReaderFlowGL::minimumSizeHint() const
{
    return QSize(320, 200);
}

void YACReaderFlowGL::setupShaders()
{
    bool isES = QOpenGLContext::currentContext()->isOpenGLES();

    // Vertex Shader - Desktop GL 3.3
    const char *vertexShaderSourceGL = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texCoord;
        layout(location = 2) in mat4 instanceModel;
        layout(location = 6) in vec4 instanceShading1;
        layout(location = 7) in float instanceOpacity;

        out vec2 vTexCoord;
        out vec4 vColor;

        uniform mat4 viewProjectionMatrix;

        void main()
        {
            gl_Position = viewProjectionMatrix * instanceModel * vec4(position, 1.0);
            vTexCoord = texCoord;

            float leftUpShading = instanceShading1.x;
            float leftDownShading = instanceShading1.y;
            float rightUpShading = instanceShading1.z;
            float rightDownShading = instanceShading1.w;

            float leftShading = mix(leftDownShading, leftUpShading, (position.y + 0.5));
            float rightShading = mix(rightDownShading, rightUpShading, (position.y + 0.5));
            float shading = mix(leftShading, rightShading, (position.x + 0.5));

            vColor = vec4(shading * instanceOpacity);
        }
    )";

    // Vertex Shader - OpenGL ES 3.0
    const char *vertexShaderSourceES = R"(
        #version 300 es
        precision highp float;

        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texCoord;
        layout(location = 2) in mat4 instanceModel;
        layout(location = 6) in vec4 instanceShading1;
        layout(location = 7) in float instanceOpacity;

        out vec2 vTexCoord;
        out vec4 vColor;

        uniform mat4 viewProjectionMatrix;

        void main()
        {
            gl_Position = viewProjectionMatrix * instanceModel * vec4(position, 1.0);
            vTexCoord = texCoord;

            float leftUpShading = instanceShading1.x;
            float leftDownShading = instanceShading1.y;
            float rightUpShading = instanceShading1.z;
            float rightDownShading = instanceShading1.w;

            float leftShading = mix(leftDownShading, leftUpShading, (position.y + 0.5));
            float rightShading = mix(rightDownShading, rightUpShading, (position.y + 0.5));
            float shading = mix(leftShading, rightShading, (position.x + 0.5));

            vColor = vec4(shading * instanceOpacity);
        }
    )";

    // Fragment Shader - Desktop GL 3.3
    const char *fragmentShaderSourceGL = R"(
        #version 330 core
        in vec2 vTexCoord;
        in vec4 vColor;

        out vec4 fragColor;

        uniform sampler2D texture;
        uniform bool isReflection;
        uniform float reflectionUp;
        uniform float reflectionDown;
        uniform vec3 backgroundColor;
        uniform vec3 shadingColor;

        void main()
        {
            vec2 texCoord = vTexCoord;

            if (isReflection) {
                texCoord.y = 1.0 - vTexCoord.y;
                vec4 texColor = texture2D(texture, texCoord);
                float gradientFade = mix(1.0/3.0, reflectionUp / 2.0, vTexCoord.y);
                float shadingAmount = vColor.r * gradientFade;
                vec3 shadedColor = mix(backgroundColor, texColor.rgb, shadingAmount);
                fragColor = vec4(shadedColor, texColor.a);
            } else {
                vec4 texColor = texture2D(texture, texCoord);
                float shadingAmount = vColor.r;
                vec3 shadedColor = mix(backgroundColor, texColor.rgb, shadingAmount);
                fragColor = vec4(shadedColor, texColor.a);
            }
        }
    )";

    // Fragment Shader - OpenGL ES 3.0
    const char *fragmentShaderSourceES = R"(
        #version 300 es
        precision highp float;

        in vec2 vTexCoord;
        in vec4 vColor;

        out vec4 fragColor;

        uniform sampler2D texture;
        uniform bool isReflection;
        uniform float reflectionUp;
        uniform float reflectionDown;
        uniform vec3 backgroundColor;
        uniform vec3 shadingColor;

        void main()
        {
            vec2 texCoord = vTexCoord;

            if (isReflection) {
                texCoord.y = 1.0 - vTexCoord.y;
                vec4 texColor = texture(texture, texCoord);
                float gradientFade = mix(1.0/3.0, reflectionUp / 2.0, vTexCoord.y);
                float shadingAmount = vColor.r * gradientFade;
                vec3 shadedColor = mix(backgroundColor, texColor.rgb, shadingAmount);
                fragColor = vec4(shadedColor, texColor.a);
            } else {
                vec4 texColor = texture(texture, texCoord);
                float shadingAmount = vColor.r;
                vec3 shadedColor = mix(backgroundColor, texColor.rgb, shadingAmount);
                fragColor = vec4(shadedColor, texColor.a);
            }
        }
    )";

    // Select shaders based on context type
    const char *vertexShader = isES ? vertexShaderSourceES : vertexShaderSourceGL;
    const char *fragmentShader = isES ? fragmentShaderSourceES : fragmentShaderSourceGL;

    qDebug() << "YACReaderFlowGL: Using" << (isES ? "OpenGL ES 3.0" : "Desktop OpenGL 3.3") << "shaders";

    shaderProgram = new QOpenGLShaderProgram(this);
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader);
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader);

    if (!shaderProgram->link()) {
        qWarning() << "YACReaderFlowGL: Shader linking failed:" << shaderProgram->log();
    }
}

void YACReaderFlowGL::setupGeometry()
{
    // VAO for regular covers
    vao = new QOpenGLVertexArrayObject(this);
    vao->create();
    vao->bind();

    vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vbo->create();
    vbo->bind();

    // Quad vertices with texture coordinates
    GLfloat vertices[] = {
        // Position (x, y, z), TexCoord (u, v)
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom-left
        0.5f, -0.5f, 0.0f, 1.0f, 1.0f, // Bottom-right
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f, // Top-right
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f // Top-left
    };

    vbo->allocate(vertices, sizeof(vertices));

    // Position attribute
    shaderProgram->enableAttributeArray(0);
    shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));

    // TexCoord attribute
    shaderProgram->enableAttributeArray(1);
    shaderProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

    // Create instance buffer (will be filled per-frame)
    instanceVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    instanceVBO->create();
    instanceVBO->bind();

    // Per-instance attributes (model matrix = 4 vec4s, shading = 1 vec4, opacity = 1 float)
    // Location 2-5: model matrix (mat4)
    for (int i = 0; i < 4; i++) {
        shaderProgram->enableAttributeArray(2 + i);
        shaderProgram->setAttributeBuffer(2 + i, GL_FLOAT, i * 4 * sizeof(GLfloat), 4, 21 * sizeof(GLfloat));
        glVertexAttribDivisor(2 + i, 1); // Advance once per instance
    }

    // Location 6: shading vec4 (leftUp, leftDown, rightUp, rightDown)
    shaderProgram->enableAttributeArray(6);
    shaderProgram->setAttributeBuffer(6, GL_FLOAT, 16 * sizeof(GLfloat), 4, 21 * sizeof(GLfloat));
    glVertexAttribDivisor(6, 1);

    // Location 7: opacity float
    shaderProgram->enableAttributeArray(7);
    shaderProgram->setAttributeBuffer(7, GL_FLOAT, 20 * sizeof(GLfloat), 1, 21 * sizeof(GLfloat));
    glVertexAttribDivisor(7, 1);

    vao->release();
    instanceVBO->release();
    vbo->release();
}

void YACReaderFlowGL::initializeGL()
{
    if (!context() || !context()->isValid()) {
        qWarning() << "YACReaderFlowGL: Invalid OpenGL context";
        return;
    }

    initializeOpenGLFunctions();

    // Verify instancing support (available in OpenGL 3.3+ and ES 3.0+)
    bool hasInstancing = context()->hasExtension(QByteArrayLiteral("GL_ARB_instanced_arrays")) ||
            context()->format().majorVersion() >= 3;

    if (!hasInstancing) {
        qWarning() << "YACReaderFlowGL: Instanced rendering not supported!";
        qWarning() << "YACReaderFlowGL: OpenGL version:" << context()->format().majorVersion() << "." << context()->format().minorVersion();
        return;
    }

    setupShaders();
    setupGeometry();

    defaultTexture = new QOpenGLTexture(QImage(":/images/defaultCover.png"));
    defaultTexture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::LinearMipMapLinear);

#ifdef YACREADER_LIBRARY
    markTexture = new QOpenGLTexture(QImage(":/images/readRibbon.png"));
    markTexture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::LinearMipMapLinear);

    readingTexture = new QOpenGLTexture(QImage(":/images/readingRibbon.png"));
    readingTexture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::LinearMipMapLinear);
#endif

    if (lazyPopulateObjects != -1)
        populate(lazyPopulateObjects);

    hasBeenInitialized = true;
}

void YACReaderFlowGL::paintGL()
{
    if (!context() || !context()->isValid() || !shaderProgram)
        return;

    QPainter painter;
    painter.begin(this);
    painter.beginNativePainting();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF(), 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (numObjects > 0) {
        updatePositions();
        updatePerspective(width(), height());
        draw();
    }

    glDisable(GL_MULTISAMPLE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    painter.endNativePainting();

    QFont font = painter.font();
    font.setFamily("Arial");
    font.setPointSize(fontSize);
    painter.setFont(font);
    painter.setPen(textColor);
    painter.drawText(10, fontSize + 10, QString("%1/%2").arg(currentSelected + 1).arg(numObjects));

    painter.end();
}

void YACReaderFlowGL::resizeGL(int width, int height)
{
    if (!context() || !context()->isValid())
        return;

    fontSize = (width + height) * 0.010;
    if (fontSize < 10)
        fontSize = 10;

    updatePerspective(width, height);

    if (numObjects > 0)
        updatePositions();
}

void YACReaderFlowGL::updatePerspective(int width, int height)
{
    if (!context() || !context()->isValid())
        return;

    float pixelRatio = devicePixelRatioF();
    glViewport(0, 0, width * pixelRatio, height * pixelRatio);
}

void YACReaderFlowGL::calcPos(YACReader3DImage &image, int pos)
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

void YACReaderFlowGL::calcVector(YACReader3DVector &vector, int pos)
{
    calcPos(dummy, pos);
    vector.x = dummy.current.x;
    vector.y = dummy.current.y;
    vector.z = dummy.current.z;
    vector.rot = dummy.current.rot;
}

bool YACReaderFlowGL::animate(YACReader3DVector &currentVector, YACReader3DVector &toVector)
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

void YACReaderFlowGL::drawCover(const YACReader3DImage &image)
{
    if (!shaderProgram || !vao || !image.texture)
        return;

    float w = image.width;
    float h = image.height;

    // Calculate opacity - original formula exactly
    float opacity = 1 - 1 / (config.animationFadeOutDist + config.viewRotateLightStrenght * fabs(viewRotate)) * fabs(0 - image.current.x);

    // Setup matrices
    QMatrix4x4 projectionMatrix;
    projectionMatrix.perspective(config.zoom, GLdouble(width()) / (float)height(), 1.0, 200.0);

    QMatrix4x4 viewMatrix;
    viewMatrix.translate(config.cfX, config.cfY, config.cfZ);
    viewMatrix.rotate(config.cfRX, 1, 0, 0);
    viewMatrix.rotate(viewRotate * config.viewAngle + config.cfRY, 0, 1, 0);
    viewMatrix.rotate(config.cfRZ, 0, 0, 1);

    QMatrix4x4 modelMatrix;
    modelMatrix.translate(image.current.x, image.current.y, image.current.z);
    modelMatrix.rotate(image.current.rot, 0, 1, 0);
    modelMatrix.translate(0.0f, -0.5f + h / 2.0f, 0.0f);
    modelMatrix.scale(w, h, 1.0f);

    QMatrix4x4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

    // Calculate per-corner shading exactly as original
    float LShading = ((config.rotation != 0) ? ((image.current.rot < 0) ? 1 - 1 / config.rotation * image.current.rot : 1) : 1);
    float RShading = ((config.rotation != 0) ? ((image.current.rot > 0) ? 1 - 1 / (config.rotation * -1) * image.current.rot : 1) : 1);
    float LUP = shadingTop + (1 - shadingTop) * LShading;
    float LDOWN = shadingBottom + (1 - shadingBottom) * LShading;
    float RUP = shadingTop + (1 - shadingTop) * RShading;
    float RDOWN = shadingBottom + (1 - shadingBottom) * RShading;

    // Bind shader and set uniforms
    shaderProgram->bind();
    shaderProgram->setUniformValue("mvpMatrix", mvpMatrix);
    shaderProgram->setUniformValue("leftUpShading", LUP);
    shaderProgram->setUniformValue("leftDownShading", LDOWN);
    shaderProgram->setUniformValue("rightUpShading", RUP);
    shaderProgram->setUniformValue("rightDownShading", RDOWN);
    shaderProgram->setUniformValue("opacity", opacity);
    shaderProgram->setUniformValue("isReflection", false);

    // Bind texture and VAO
    image.texture->bind();
    vao->bind();

    // Draw cover
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Draw marks if needed
    if (showMarks && loaded[image.index] && marks[image.index] != Unread) {
        QOpenGLTexture *markTex = (marks[image.index] == Read) ? markTexture : readingTexture;

        QMatrix4x4 markModel;
        markModel.translate(image.current.x, image.current.y, image.current.z);
        markModel.rotate(image.current.rot, 0, 1, 0);

        float markWidth = 0.15f;
        float markHeight = 0.2f;
        float markCenterX = w / 2.0f - 0.125f;
        float markCenterY = -0.588f + h;

        markModel.translate(markCenterX, markCenterY, 0.001f);
        markModel.scale(markWidth, markHeight, 1.0f);

        QMatrix4x4 mvpMark = projectionMatrix * viewMatrix * markModel;

        shaderProgram->setUniformValue("mvpMatrix", mvpMark);
        shaderProgram->setUniformValue("leftUpShading", RUP * opacity);
        shaderProgram->setUniformValue("leftDownShading", RUP * opacity);
        shaderProgram->setUniformValue("rightUpShading", RUP * opacity);
        shaderProgram->setUniformValue("rightDownShading", RUP * opacity);
        shaderProgram->setUniformValue("opacity", 1.0f);
        shaderProgram->setUniformValue("isReflection", false);

        markTex->bind();
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    vao->release();
    shaderProgram->release();
}

void YACReaderFlowGL::drawReflection(const YACReader3DImage &image)
{
    if (!shaderProgram || !vao || !image.texture)
        return;

    float w = image.width;
    float h = image.height;

    // Calculate opacity - original formula exactly
    float opacity = 1 - 1 / (config.animationFadeOutDist + config.viewRotateLightStrenght * fabs(viewRotate)) * fabs(0 - image.current.x);

    // Setup matrices
    QMatrix4x4 projectionMatrix;
    projectionMatrix.perspective(config.zoom, GLdouble(width()) / (float)height(), 1.0, 200.0);

    QMatrix4x4 viewMatrix;
    viewMatrix.translate(config.cfX, config.cfY, config.cfZ);
    viewMatrix.rotate(config.cfRX, 1, 0, 0);
    viewMatrix.rotate(viewRotate * config.viewAngle + config.cfRY, 0, 1, 0);
    viewMatrix.rotate(config.cfRZ, 0, 0, 1);

    // Calculate per-corner shading exactly as original
    float LShading = ((config.rotation != 0) ? ((image.current.rot < 0) ? 1 - 1 / config.rotation * image.current.rot : 1) : 1);
    float RShading = ((config.rotation != 0) ? ((image.current.rot > 0) ? 1 - 1 / (config.rotation * -1) * image.current.rot : 1) : 1);
    float LUP = shadingTop + (1 - shadingTop) * LShading;
    float LDOWN = shadingBottom + (1 - shadingBottom) * LShading;
    float RUP = shadingTop + (1 - shadingTop) * RShading;
    float RDOWN = shadingBottom + (1 - shadingBottom) * RShading;

    // Draw reflection
    // In the old code, the reflection quad had vertices from y=-0.5-h (bottom) to y=-0.5 (top)
    // The OLD reflection shading was:
    // - Bottom corners (y = -0.5-h): LUP*opacity*reflectionUp/2, RUP*opacity*reflectionUp/2
    // - Top corners (y = -0.5): LDOWN*opacity/3, RDOWN*opacity/3
    // This means the reflection uses INVERTED vertical shading (LUP/RUP at bottom, LDOWN/RDOWN at top)
    // We need to pass swapped values to match this
    QMatrix4x4 reflectionMatrix;
    reflectionMatrix.translate(image.current.x, image.current.y, image.current.z);
    reflectionMatrix.rotate(image.current.rot, 0, 1, 0);
    reflectionMatrix.translate(0.0f, -0.5f - h / 2.0f, 0.0f);
    reflectionMatrix.scale(w, h, 1.0f);

    QMatrix4x4 mvpReflection = projectionMatrix * viewMatrix * reflectionMatrix;

    shaderProgram->bind();
    shaderProgram->setUniformValue("mvpMatrix", mvpReflection);
    // Swap UP and DOWN for reflection to match old behavior
    shaderProgram->setUniformValue("leftUpShading", LDOWN);
    shaderProgram->setUniformValue("leftDownShading", LUP);
    shaderProgram->setUniformValue("rightUpShading", RDOWN);
    shaderProgram->setUniformValue("rightDownShading", RUP);
    shaderProgram->setUniformValue("opacity", opacity);
    shaderProgram->setUniformValue("isReflection", true);
    shaderProgram->setUniformValue("reflectionUp", reflectionUp);
    shaderProgram->setUniformValue("reflectionDown", reflectionBottom);

    image.texture->bind();
    vao->bind();

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    vao->release();
    shaderProgram->release();
}

void YACReaderFlowGL::cleanupAnimation()
{
    config.animationStep = stepBackup;
    viewRotateActive = 0;
}

void YACReaderFlowGL::draw()
{
    if (!shaderProgram || !vao || numObjects == 0)
        return;

    // Calculate view-projection matrix once
    // Note: Old implementation used fixed 20.0 degrees FOV, not config.zoom
    QMatrix4x4 projectionMatrix;
    projectionMatrix.perspective(20.0, GLdouble(width()) / (float)height(), 1.0, 200.0);

    QMatrix4x4 viewMatrix;
    viewMatrix.translate(config.cfX, config.cfY, config.cfZ);
    viewMatrix.rotate(config.cfRX, 1, 0, 0);
    viewMatrix.rotate(viewRotate * config.viewAngle + config.cfRY, 0, 1, 0);
    viewMatrix.rotate(config.cfRZ, 0, 0, 1);

    QMatrix4x4 viewProjectionMatrix = projectionMatrix * viewMatrix;

    // Bind shader once
    shaderProgram->bind();
    shaderProgram->setUniformValue("viewProjectionMatrix", viewProjectionMatrix);
    shaderProgram->setUniformValue("backgroundColor",
                                   QVector3D(backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF()));
    shaderProgram->setUniformValue("shadingColor",
                                   QVector3D(shadingColor.redF(), shadingColor.greenF(), shadingColor.blueF()));

    int CS = currentSelected;

    // Prepare instance data for all covers
    QVector<GLfloat> instanceData;
    QVector<int> drawOrder;

    // Build draw order (back to front)
    for (int count = numObjects - 1; count > -1; count--) {
        if (count > CS) {
            drawOrder.append(count);
        }
    }
    for (int count = 0; count < numObjects - 1; count++) {
        if (count < CS) {
            drawOrder.append(count);
        }
    }
    drawOrder.append(CS);

    // Draw reflections first
    shaderProgram->setUniformValue("isReflection", true);
    shaderProgram->setUniformValue("reflectionUp", reflectionUp);
    shaderProgram->setUniformValue("reflectionDown", reflectionBottom);

    for (int idx : drawOrder) {
        if (images[idx].texture) {
            prepareInstanceData(images[idx], true, instanceData);

            instanceVBO->bind();
            instanceVBO->allocate(instanceData.data(), instanceData.size() * sizeof(GLfloat));

            images[idx].texture->bind();
            vao->bind();
            glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 1);

            instanceData.clear();
        }
    }

    // Draw covers
    shaderProgram->setUniformValue("isReflection", false);

    for (int idx : drawOrder) {
        if (images[idx].texture) {
            prepareInstanceData(images[idx], false, instanceData);

            instanceVBO->bind();
            instanceVBO->allocate(instanceData.data(), instanceData.size() * sizeof(GLfloat));

            images[idx].texture->bind();
            vao->bind();
            glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 1);

            // Draw marks if needed
            if (showMarks && loaded[images[idx].index] && marks[images[idx].index] != Unread) {
                drawMark(images[idx], viewProjectionMatrix);
            }

            instanceData.clear();
        }
    }

    vao->release();
    shaderProgram->release();
}

void YACReaderFlowGL::prepareInstanceData(const YACReader3DImage &image, bool isReflection, QVector<GLfloat> &data)
{
    float w = image.width;
    float h = image.height;

    // Calculate opacity
    float opacity = 1 - 1 / (config.animationFadeOutDist + config.viewRotateLightStrenght * fabs(viewRotate)) * fabs(0 - image.current.x);

    // Calculate model matrix
    QMatrix4x4 modelMatrix;
    modelMatrix.translate(image.current.x, image.current.y, image.current.z);
    modelMatrix.rotate(image.current.rot, 0, 1, 0);

    if (isReflection) {
        modelMatrix.translate(0.0f, -0.5f - h / 2.0f, 0.0f);
    } else {
        modelMatrix.translate(0.0f, -0.5f + h / 2.0f, 0.0f);
    }
    modelMatrix.scale(w, h, 1.0f);

    // Calculate shading
    float LShading = ((config.rotation != 0) ? ((image.current.rot < 0) ? 1 - 1 / config.rotation * image.current.rot : 1) : 1);
    float RShading = ((config.rotation != 0) ? ((image.current.rot > 0) ? 1 - 1 / (config.rotation * -1) * image.current.rot : 1) : 1);
    float LUP = shadingTop + (1 - shadingTop) * LShading;
    float LDOWN = shadingBottom + (1 - shadingBottom) * LShading;
    float RUP = shadingTop + (1 - shadingTop) * RShading;
    float RDOWN = shadingBottom + (1 - shadingBottom) * RShading;

    // For reflection, swap vertical shading
    if (isReflection) {
        float temp = LUP;
        LUP = LDOWN;
        LDOWN = temp;
        temp = RUP;
        RUP = RDOWN;
        RDOWN = temp;
    }

    // Pack instance data: mat4 (16 floats) + vec4 shading (4 floats) + float opacity (1 float) = 21 floats
    const float *matData = modelMatrix.constData();
    for (int i = 0; i < 16; i++) {
        data.append(matData[i]);
    }
    data.append(LUP);
    data.append(LDOWN);
    data.append(RUP);
    data.append(RDOWN);
    data.append(opacity);
}

void YACReaderFlowGL::drawMark(const YACReader3DImage &image, const QMatrix4x4 &viewProjectionMatrix)
{
    QOpenGLTexture *markTex = (marks[image.index] == Read) ? markTexture : readingTexture;

    float w = image.width;
    float h = image.height;
    float opacity = 1 - 1 / (config.animationFadeOutDist + config.viewRotateLightStrenght * fabs(viewRotate)) * fabs(0 - image.current.x);

    float LShading = ((config.rotation != 0) ? ((image.current.rot < 0) ? 1 - 1 / config.rotation * image.current.rot : 1) : 1);
    float RShading = ((config.rotation != 0) ? ((image.current.rot > 0) ? 1 - 1 / (config.rotation * -1) * image.current.rot : 1) : 1);
    float RUP = shadingTop + (1 - shadingTop) * RShading;

    QMatrix4x4 markModel;
    markModel.translate(image.current.x, image.current.y, image.current.z);
    markModel.rotate(image.current.rot, 0, 1, 0);

    float markWidth = 0.15f;
    float markHeight = 0.2f;
    float markCenterX = w / 2.0f - 0.125f;
    float markCenterY = -0.588f + h;

    markModel.translate(markCenterX, markCenterY, 0.001f);
    markModel.scale(markWidth, markHeight, 1.0f);

    // Prepare instance data for mark
    QVector<GLfloat> markData;
    const float *matData = markModel.constData();
    for (int i = 0; i < 16; i++) {
        markData.append(matData[i]);
    }
    float shadingValue = RUP * opacity;
    markData.append(shadingValue);
    markData.append(shadingValue);
    markData.append(shadingValue);
    markData.append(shadingValue);
    markData.append(1.0f);

    instanceVBO->bind();
    instanceVBO->allocate(markData.data(), markData.size() * sizeof(GLfloat));

    markTex->bind();
    vao->bind();
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 1);
}

void YACReaderFlowGL::showPrevious()
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

void YACReaderFlowGL::showNext()
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

void YACReaderFlowGL::setCurrentIndex(int pos)
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

void YACReaderFlowGL::updatePositions()
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

void YACReaderFlowGL::insert(char *name, QOpenGLTexture *texture, float x, float y, int item)
{
    startAnimationTimer();

    Q_UNUSED(name)
    if (item == -1) {
        images.push_back(YACReader3DImage());
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

void YACReaderFlowGL::remove(int item)
{
    if (item < 0 || item >= images.size())
        return;

    startAnimationTimer();

    loaded.remove(item);
    marks.remove(item);

    if (item <= currentSelected && currentSelected != 0) {
        currentSelected--;
    }

    QOpenGLTexture *texture = images[item].texture;

    int count = item;
    while (count <= numObjects - 1) {
        images[count].index--;
        count++;
    }
    images.removeAt(item);

    if (texture != defaultTexture)
        delete (texture);

    numObjects--;
}

void YACReaderFlowGL::add(int item)
{
    float x = 1;
    float y = 1 * (700.f / 480.0f);
    QString s = "cover";

    images.insert(item, YACReader3DImage());
    loaded.insert(item, false);
    marks.insert(item, Unread);
    numObjects++;

    for (int i = item + 1; i < numObjects; i++) {
        images[i].index++;
    }

    insert(s.toLocal8Bit().data(), defaultTexture, x, y, item);
}

YACReader3DImage YACReaderFlowGL::getCurrentSelected()
{
    return images[currentSelected];
}

void YACReaderFlowGL::replace(char *name, QOpenGLTexture *texture, float x, float y, int item)
{
    startAnimationTimer();

    Q_UNUSED(name)
    if (images[item].index == item) {
        images[item].texture = texture;
        images[item].width = x;
        images[item].height = y;
        loaded[item] = true;
    } else
        loaded[item] = false;
}

void YACReaderFlowGL::populate(int n)
{
    emit centerIndexChanged(0);

    float x = 1;
    float y = 1 * (700.f / 480.0f);
    int i;

    for (i = 0; i < n; i++) {
        QString s = "cover";
        insert(s.toLocal8Bit().data(), defaultTexture, x, y);
    }

    loaded = QVector<bool>(n, false);
}

void YACReaderFlowGL::reset()
{
    makeCurrent();
    startAnimationTimer();

    currentSelected = 0;
    loaded.clear();

    for (int i = 0; i < numObjects; i++) {
        if (images[i].texture != defaultTexture)
            delete (images[i].texture);
    }

    numObjects = 0;
    images.clear();

    if (!hasBeenInitialized)
        lazyPopulateObjects = -1;

    doneCurrent();
}

void YACReaderFlowGL::reload()
{
    startAnimationTimer();
    int n = numObjects;
    reset();
    populate(n);
}

// Slot implementations
void YACReaderFlowGL::setCF_RX(int value)
{
    startAnimationTimer();
    config.cfRX = value;
}
void YACReaderFlowGL::setCF_RY(int value)
{
    startAnimationTimer();
    config.cfRY = value;
}
void YACReaderFlowGL::setCF_RZ(int value)
{
    startAnimationTimer();
    config.cfRZ = value;
}
void YACReaderFlowGL::setRotation(int angle)
{
    startAnimationTimer();
    config.rotation = -angle;
}
void YACReaderFlowGL::setX_Distance(int distance)
{
    startAnimationTimer();
    config.xDistance = distance / 100.0;
}
void YACReaderFlowGL::setCenter_Distance(int distance)
{
    startAnimationTimer();
    config.centerDistance = distance / 100.0;
}
void YACReaderFlowGL::setZ_Distance(int distance)
{
    startAnimationTimer();
    config.zDistance = distance / 100.0;
}
void YACReaderFlowGL::setCF_Y(int value)
{
    startAnimationTimer();
    config.cfY = value / 100.0;
}
void YACReaderFlowGL::setCF_Z(int value)
{
    startAnimationTimer();
    config.cfZ = value;
}
void YACReaderFlowGL::setY_Distance(int value)
{
    startAnimationTimer();
    config.yDistance = value / 100.0;
}
void YACReaderFlowGL::setFadeOutDist(int value)
{
    startAnimationTimer();
    config.animationFadeOutDist = value;
}
void YACReaderFlowGL::setLightStrenght(int value)
{
    startAnimationTimer();
    config.viewRotateLightStrenght = value;
}
void YACReaderFlowGL::setMaxAngle(int value)
{
    startAnimationTimer();
    config.viewAngle = value;
}
void YACReaderFlowGL::setPreset(const Preset &p)
{
    startAnimationTimer();
    config = p;
}

void YACReaderFlowGL::setZoom(int zoom)
{
    startAnimationTimer();
    config.zoom = zoom;
}

void YACReaderFlowGL::setPerformance(Performance performance)
{
    if (this->performance != performance) {
        startAnimationTimer();
        this->performance = performance;
        reload();
    }
}

void YACReaderFlowGL::useVSync(bool b)
{
    if (bUseVSync != b) {
        bUseVSync = b;
        QSurfaceFormat f = format();
        f.setVersion(3, 3);
        f.setProfile(QSurfaceFormat::CoreProfile);
        f.setSwapInterval(b ? 1 : 0);
        setFormat(f);
        reset();
    }
}

void YACReaderFlowGL::setShowMarks(bool value)
{
    startAnimationTimer();
    showMarks = value;
}
void YACReaderFlowGL::setMarks(QVector<YACReader::YACReaderComicReadStatus> marks)
{
    startAnimationTimer();
    this->marks = marks;
}
void YACReaderFlowGL::setMarkImage(QImage &image)
{
    Q_UNUSED(image);
}
void YACReaderFlowGL::markSlide(int index, YACReader::YACReaderComicReadStatus status)
{
    startAnimationTimer();
    marks[index] = status;
}
void YACReaderFlowGL::unmarkSlide(int index)
{
    startAnimationTimer();
    marks[index] = YACReader::Unread;
}
void YACReaderFlowGL::setSlideSize(QSize size)
{
    Q_UNUSED(size);
}
void YACReaderFlowGL::clear()
{
    reset();
}
void YACReaderFlowGL::setCenterIndex(unsigned int index)
{
    setCurrentIndex(index);
}
void YACReaderFlowGL::showSlide(int index)
{
    setCurrentIndex(index);
}
int YACReaderFlowGL::centerIndex()
{
    return currentSelected;
}
void YACReaderFlowGL::updateMarks() { }
void YACReaderFlowGL::render() { }
void YACReaderFlowGL::setFlowRightToLeft(bool b)
{
    flowRightToLeft = b;
}

void YACReaderFlowGL::setBackgroundColor(const QColor &color)
{
    backgroundColor = color;

    // Auto-calculate shadingColor based on background brightness
    qreal luminance = (backgroundColor.redF() * 0.299 +
                       backgroundColor.greenF() * 0.587 +
                       backgroundColor.blueF() * 0.114);

    if (luminance < 0.5) {
        // Dark background - shade towards white
        shadingColor = QColor(255, 255, 255);
        // Use original shading values for dark backgrounds
        shadingTop = 0.8f;
        shadingBottom = 0.02f;
    } else {
        // Light background - shade towards black
        shadingColor = QColor(0, 0, 0);
        // Adjust shading range for better contrast on light backgrounds
        shadingTop = 0.95f;
        shadingBottom = 0.3f;
    }

    update();
}

void YACReaderFlowGL::setTextColor(const QColor &color)
{
    textColor = color;
    update();
}

void YACReaderFlowGL::setShadingColor(const QColor &color)
{
    shadingColor = color;
    update();
}

// Event handlers
void YACReaderFlowGL::wheelEvent(QWheelEvent *event)
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

void YACReaderFlowGL::keyPressEvent(QKeyEvent *event)
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

void YACReaderFlowGL::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QVector3D intersection = getPlaneIntersection(event->x(), event->y(), images[currentSelected]);
        if ((intersection.x() > 0.5 && !flowRightToLeft) || (intersection.x() < -0.5 && flowRightToLeft)) {
            showNext();
        } else if ((intersection.x() < -0.5 && !flowRightToLeft) || (intersection.x() > 0.5 && flowRightToLeft)) {
            showPrevious();
        }
    } else {
        QOpenGLWidget::mousePressEvent(event);
    }
    doneCurrent();
}

void YACReaderFlowGL::mouseDoubleClickEvent(QMouseEvent *event)
{
    QVector3D intersection = getPlaneIntersection(event->x(), event->y(), images[currentSelected]);

    if (intersection.x() < 0.5 && intersection.x() > -0.5) {
        emit selected(centerIndex());
        event->accept();
    }
}

QVector3D YACReaderFlowGL::getPlaneIntersection(int x, int y, YACReader3DImage plane)
{
    if (!context() || !context()->isValid())
        return QVector3D(0, 0, 0);

    GLint viewport[4];
    QMatrix4x4 m_modelview;
    QMatrix4x4 m_projection;

    makeCurrent();
    glGetIntegerv(GL_VIEWPORT, viewport);

    m_projection.perspective(config.zoom, GLdouble(width()) / (float)height(), 1.0, 200.0);

    m_modelview.translate(config.cfX, config.cfY, config.cfZ);
    m_modelview.rotate(config.cfRX, 1, 0, 0);
    m_modelview.rotate(viewRotate * config.viewAngle + config.cfRY, 0, 1, 0);
    m_modelview.rotate(config.cfRZ, 0, 0, 1);
    m_modelview.translate(plane.current.x, plane.current.y, plane.current.z);
    m_modelview.rotate(plane.current.rot, 0, 1, 0);
    m_modelview.scale(plane.width, plane.height, 1.0f);

    doneCurrent();

    QVector3D ray_origin(x * devicePixelRatioF(), y * devicePixelRatioF(), 0);
    QVector3D ray_end(x * devicePixelRatioF(), y * devicePixelRatioF(), 1.0);

    ray_origin = ray_origin.unproject(m_modelview, m_projection, QRect(viewport[0], viewport[1], viewport[2], viewport[3]));
    ray_end = ray_end.unproject(m_modelview, m_projection, QRect(viewport[0], viewport[1], viewport[2], viewport[3]));

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

// YACReaderComicFlowGL implementation
YACReaderComicFlowGL::YACReaderComicFlowGL(QWidget *parent, struct Preset p)
    : YACReaderFlowGL(parent, p)
{
    worker = new ImageLoaderGL(this);
    worker->flow = this;
}

void YACReaderComicFlowGL::setImagePaths(QStringList paths)
{
    worker->reset();
    reset();
    numObjects = 0;
    if (lazyPopulateObjects != -1 || hasBeenInitialized)
        YACReaderFlowGL::populate(paths.size());
    lazyPopulateObjects = paths.size();
    this->paths = paths;
}

void YACReaderComicFlowGL::updateImageData()
{
    if (worker->busy())
        return;

    int idx = worker->index();
    if (idx >= 0 && !worker->result().isNull()) {
        if (!loaded[idx]) {
            float x = 1;
            QImage img = worker->result();
            QOpenGLTexture *texture = new QOpenGLTexture(img);

            if (performance == high || performance == ultraHigh) {
                texture->setAutoMipMapGenerationEnabled(true);
                texture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::LinearMipMapLinear);
            } else {
                texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
            }

            float y = 1 * (float(img.height()) / img.width());
            QString s = "cover";
            replace(s.toLocal8Bit().data(), texture, x, y, idx);
        }
    }

    int count = 8;
    switch (performance) {
    case low:
        count = 8;
        break;
    case medium:
        count = 10;
        break;
    case high:
        count = 12;
        break;
    case ultraHigh:
        count = 16;
        break;
    }

    int *indexes = new int[2 * count + 1];
    int center = currentSelected;
    indexes[0] = center;
    for (int j = 0; j < count; j++) {
        indexes[j * 2 + 1] = center + j + 1;
        indexes[j * 2 + 2] = center - j - 1;
    }

    for (int c = 0; c < 2 * count + 1; c++) {
        int i = indexes[c];
        if ((i >= 0) && (i < numObjects))
            if (!loaded[i]) {
                if (paths.size() > 0) {
                    QString fname = paths.at(i);
                    worker->generate(i, fname);
                }
                delete[] indexes;
                return;
            }
    }

    delete[] indexes;
}

void YACReaderComicFlowGL::remove(int item)
{
    worker->lock();
    worker->reset();
    YACReaderFlowGL::remove(item);
    if (item >= 0 && item < paths.size()) {
        paths.removeAt(item);
    }
    worker->unlock();
}

void YACReaderComicFlowGL::add(const QString &path, int index)
{
    worker->lock();
    worker->reset();
    paths.insert(index, path);
    YACReaderFlowGL::add(index);
    worker->unlock();
}

void YACReaderComicFlowGL::resortCovers(QList<int> newOrder)
{
    worker->lock();
    worker->reset();
    startAnimationTimer();

    QList<QString> pathsNew;
    QVector<bool> loadedNew;
    QVector<YACReaderComicReadStatus> marksNew;
    QVector<YACReader3DImage> imagesNew;

    int index = 0;
    foreach (int i, newOrder) {
        if (i < 0 || i >= images.size()) {
            continue;
        }

        pathsNew << paths.at(i);
        loadedNew << loaded.at(i);
        marksNew << marks.at(i);
        imagesNew << images.at(i);
        imagesNew.last().index = index++;
    }

    paths = pathsNew;
    loaded = loadedNew;
    marks = marksNew;
    images = imagesNew;

    worker->unlock();
}

// YACReaderPageFlowGL implementation
YACReaderPageFlowGL::YACReaderPageFlowGL(QWidget *parent, struct Preset p)
    : YACReaderFlowGL(parent, p)
{
    worker = new ImageLoaderByteArrayGL(this);
    worker->flow = this;
}

YACReaderPageFlowGL::~YACReaderPageFlowGL()
{
    this->killTimer(timerId);
    rawImages.clear();

    makeCurrent();

    for (auto &image : images) {
        if (image.texture != defaultTexture) {
            if (image.texture->isCreated()) {
                image.texture->destroy();
            }
            delete image.texture;
        }
    }

    if (defaultTexture != nullptr) {
        if (defaultTexture->isCreated()) {
            defaultTexture->destroy();
        }
        delete defaultTexture;
    }

    doneCurrent();
}

void YACReaderPageFlowGL::updateImageData()
{
    if (worker->busy())
        return;

    int idx = worker->index();
    if (idx >= 0 && !worker->result().isNull()) {
        if (!loaded[idx]) {
            float x = 1;
            QImage img = worker->result();
            QOpenGLTexture *texture = new QOpenGLTexture(img);

            if (performance == high || performance == ultraHigh) {
                texture->setAutoMipMapGenerationEnabled(true);
                texture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::LinearMipMapLinear);
            } else {
                texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
            }

            float y = 1 * (float(img.height()) / img.width());
            QString s = "cover";
            replace(s.toLocal8Bit().data(), texture, x, y, idx);
            loaded[idx] = true;
        }
    }

    int count = 8;
    switch (performance) {
    case low:
        count = 8;
        break;
    case medium:
        count = 10;
        break;
    case high:
        count = 12;
        break;
    case ultraHigh:
        count = 14;
        break;
    }

    int *indexes = new int[2 * count + 1];
    int center = currentSelected;
    indexes[0] = center;
    for (int j = 0; j < count; j++) {
        indexes[j * 2 + 1] = center + j + 1;
        indexes[j * 2 + 2] = center - j - 1;
    }

    for (int c = 0; c < 2 * count + 1; c++) {
        int i = indexes[c];
        if ((i >= 0) && (i < numObjects))
            if (rawImages.size() > 0)
                if (!loaded[i] && imagesReady[i]) {
                    worker->generate(i, rawImages.at(i));
                    delete[] indexes;
                    return;
                }
    }

    delete[] indexes;
}

void YACReaderPageFlowGL::populate(int n)
{
    worker->reset();
    if (lazyPopulateObjects != -1 || hasBeenInitialized)
        YACReaderFlowGL::populate(n);
    lazyPopulateObjects = n;
    imagesReady = QVector<bool>(n, false);
    rawImages = QVector<QByteArray>(n);
    imagesSetted = QVector<bool>(n, false);
}

// ImageLoaderGL implementation
QImage ImageLoaderGL::loadImage(const QString &fileName)
{
    QImage image;

    if (!image.load(fileName)) {
        return QImage();
    }

    switch (flow->performance) {
    case low:
        image = image.scaledToWidth(200, Qt::SmoothTransformation);
        break;
    case medium:
        image = image.scaledToWidth(256, Qt::SmoothTransformation);
        break;
    case high:
        image = image.scaledToWidth(320, Qt::SmoothTransformation);
        break;
    case ultraHigh:
        break;
    }

    return image;
}

ImageLoaderGL::ImageLoaderGL(YACReaderFlowGL *flow)
    : QThread(), flow(flow), restart(false), working(false), idx(-1)
{
}

ImageLoaderGL::~ImageLoaderGL()
{
    mutex.lock();
    condition.wakeOne();
    mutex.unlock();
    wait();
}

bool ImageLoaderGL::busy() const
{
    return isRunning() ? working : false;
}

void ImageLoaderGL::generate(int index, const QString &fileName)
{
    mutex.lock();
    this->idx = index;
    this->fileName = fileName;
    this->size = size;
    this->img = QImage();
    mutex.unlock();

    if (!isRunning())
        start();
    else {
        restart = true;
        condition.wakeOne();
    }
}

void ImageLoaderGL::lock()
{
    mutex.lock();
}

void ImageLoaderGL::unlock()
{
    mutex.unlock();
}

void ImageLoaderGL::run()
{
    for (;;) {
        mutex.lock();
        this->working = true;
        QString fileName = this->fileName;
        mutex.unlock();

        QImage image = loadImage(fileName);

        mutex.lock();
        this->working = false;
        this->img = image;
        mutex.unlock();

        mutex.lock();
        if (!this->restart)
            condition.wait(&mutex);
        restart = false;
        mutex.unlock();
    }
}

QImage ImageLoaderGL::result()
{
    return img;
}

// ImageLoaderByteArrayGL implementation
QImage ImageLoaderByteArrayGL::loadImage(const QByteArray &raw)
{
    QImage image;

    if (!image.loadFromData(raw)) {
        return QImage();
    }

    switch (flow->performance) {
    case low:
        image = image.scaledToWidth(128, Qt::SmoothTransformation);
        break;
    case medium:
        image = image.scaledToWidth(196, Qt::SmoothTransformation);
        break;
    case high:
        image = image.scaledToWidth(256, Qt::SmoothTransformation);
        break;
    case ultraHigh:
        image = image.scaledToWidth(320, Qt::SmoothTransformation);
        break;
    }

    return image;
}

ImageLoaderByteArrayGL::ImageLoaderByteArrayGL(YACReaderFlowGL *flow)
    : QThread(), flow(flow), restart(false), working(false), idx(-1)
{
}

ImageLoaderByteArrayGL::~ImageLoaderByteArrayGL()
{
    mutex.lock();
    condition.wakeOne();
    mutex.unlock();
    wait();
}

bool ImageLoaderByteArrayGL::busy() const
{
    return isRunning() ? working : false;
}

void ImageLoaderByteArrayGL::generate(int index, const QByteArray &raw)
{
    mutex.lock();
    this->idx = index;
    this->rawData = raw;
    this->size = size;
    this->img = QImage();
    mutex.unlock();

    if (!isRunning())
        start();
    else {
        restart = true;
        condition.wakeOne();
    }
}

void ImageLoaderByteArrayGL::run()
{
    for (;;) {
        mutex.lock();
        this->working = true;
        QByteArray raw = this->rawData;
        mutex.unlock();

        QImage image = loadImage(raw);

        mutex.lock();
        this->working = false;
        this->img = image;
        mutex.unlock();

        mutex.lock();
        if (!this->restart)
            condition.wait(&mutex);
        restart = false;
        mutex.unlock();
    }
}

QImage ImageLoaderByteArrayGL::result()
{
    return img;
}
