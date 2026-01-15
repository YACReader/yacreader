#include "opengl_checker.h"

#include "QsLog.h"

OpenGLChecker::OpenGLChecker()
    : compatibleOpenGLVersion(true)
{
    QOpenGLContext *openGLContext = new QOpenGLContext();
    openGLContext->create();

    if (!openGLContext->isValid()) {
        compatibleOpenGLVersion = false;
        description = "unable to create QOpenGLContext";
    }

    QSurfaceFormat format = openGLContext->format();

    int majorVersion = format.majorVersion();
    int minorVersion = format.minorVersion();
    QString type;

    switch (format.renderableType()) {
    case QSurfaceFormat::OpenGL:
        type = "desktop";
        break;

    case QSurfaceFormat::OpenGLES:
        type = "OpenGL ES";
        break;

    case QSurfaceFormat::OpenVG:
        type = "OpenVG";
        break;

    default:
    case QSurfaceFormat::DefaultRenderableType:
        type = "unknown";
        break;
    }

    delete openGLContext;

    description = QString("%1.%2 %3").arg(majorVersion).arg(minorVersion).arg(type);

    // Check for Desktop OpenGL OR OpenGL ES 3.0+
    bool isDesktopGL = (format.renderableType() == QSurfaceFormat::OpenGL);
    bool isOpenGLES = (format.renderableType() == QSurfaceFormat::OpenGLES);

    if (isDesktopGL) {
        // Desktop OpenGL requirements
#ifdef Q_OS_WIN
        if (majorVersion < 1 || (majorVersion == 1 && minorVersion < 4))
            compatibleOpenGLVersion = false;
#else
        if (majorVersion < 2)
            compatibleOpenGLVersion = false;
#endif
    } else if (isOpenGLES) {
        // OpenGL ES requirements: 3.0 or higher
        if (majorVersion < 3)
            compatibleOpenGLVersion = false;
    } else {
        // Unknown or unsupported renderable type
        compatibleOpenGLVersion = false;
    }
}

QString OpenGLChecker::textVersionDescription()
{
    return description;
}

bool OpenGLChecker::hasCompatibleOpenGLVersion()
{
    return compatibleOpenGLVersion;
}
