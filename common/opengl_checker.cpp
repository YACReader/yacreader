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

    if (format.renderableType() != QSurfaceFormat::OpenGL) //Desktop OpenGL
        compatibleOpenGLVersion = false;

#ifdef Q_OS_WIN //TODO check Qt version, and set this values depending on the use of QOpenGLWidget or QGLWidget
    static const int majorTargetVersion = 1;
    static const int minorTargetVersion = 4;
#else
    static const int majorTargetVersion = 2;
    static const int minorTargetVersion = 0;
#endif

    if (majorVersion < majorTargetVersion)
        compatibleOpenGLVersion = false;
    if (majorVersion == majorTargetVersion && minorVersion < minorTargetVersion)
        compatibleOpenGLVersion = false;
}

QString OpenGLChecker::textVersionDescription()
{
    return description;
}

bool OpenGLChecker::hasCompatibleOpenGLVersion()
{
    return compatibleOpenGLVersion;
}
