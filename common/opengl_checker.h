#ifndef OPENGL_CHECKER_H
#define OPENGL_CHECKER_H

#include <QOpenGLContext>

class OpenGLChecker
{
public:
    OpenGLChecker();
    bool hasCompatibleOpenGLVersion();
    QString textVersionDescription();
private:
    QString description;
    bool compatibleOpenGLVersion;
};

#endif // OPENGL_CHECKER_H
