#include "opengl_checker.h"

#include "QsLog.h"

OpenGLChecker::OpenGLChecker()
{

}

bool OpenGLChecker::hasCompatibleOpenGLVersion()
{
    QOpenGLContext * openGLContext = new QOpenGLContext();
    openGLContext->create();

    if(!openGLContext->isValid())
        return false;

    QSurfaceFormat format = openGLContext->format();

    int majorVersion = format.majorVersion();
    int minorVersion = format.minorVersion();

    delete openGLContext;

    QLOG_INFO() << QString("OpenGL version %1.%2").arg(majorVersion).arg(minorVersion);

    if(format.renderableType() != QSurfaceFormat::OpenGL) //Desktop OpenGL
        return false;

#ifdef Q_OS_WIN //TODO check Qt version, and set this values depending on the use of QOpenGLWidget or QGLWidget
    int majorTargetVersion = 1;
    int minorTargetVersion = 5;
#else
    int majorTargetVersion = 2;
    int minorTargetVersion = 1;
#endif

    if(majorVersion < majorTargetVersion)
        return false;
    if(majorVersion == majorTargetVersion && minorVersion < minorTargetVersion)
        return false;

    return true;
}
