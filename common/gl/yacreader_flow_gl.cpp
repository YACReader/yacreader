#include "yacreader_flow_gl.h"

#include <QtGui>
#include <QtOpenGL>
#include <QMatrix4x4>
#include <cmath>

/*** Animation Settings ***/

/*** Position Configuration ***/

int YACReaderFlowGL::updateInterval = 16;

struct Preset defaultYACReaderFlowConfig = {
    0.08f, //Animation_step sets the speed of the animation
    1.5f, //Animation_speedup sets the acceleration of the animation
    0.1f, //Animation_step_max sets the maximum speed of the animation
    3.f, //Animation_Fade_out_dis sets the distance of view

    1.5f, //pre_rotation sets the rotation increasion
    3.f, //View_rotate_light_strenght sets the light strenght on rotation
    0.01f, //View_rotate_add sets the speed of the rotation
    0.02f, //View_rotate_sub sets the speed of reversing the rotation
    20.f, //View_angle sets the maximum view angle

    0.f, //CF_X the X Position of the Coverflow
    0.f, //CF_Y the Y Position of the Coverflow
    -8.f, //CF_Z the Z Position of the Coverflow

    15.f, //CF_RX the X Rotation of the Coverflow
    0.f, //CF_RY the Y Rotation of the Coverflow
    0.f, //CF_RZ the Z Rotation of the Coverflow

    -50.f, //Rotation sets the rotation of each cover
    0.18f, //X_Distance sets the distance between the covers
    1.f, //Center_Distance sets the distance between the centered and the non centered covers
    0.1f, //Z_Distance sets the pushback amount
    0.0f, //Y_Distance sets the elevation amount

    30.f //zoom level

};

struct Preset presetYACReaderFlowClassicConfig = {
    0.08f, //Animation_step sets the speed of the animation
    1.5f, //Animation_speedup sets the acceleration of the animation
    0.1f, //Animation_step_max sets the maximum speed of the animation
    2.f, //Animation_Fade_out_dis sets the distance of view

    1.5f, //pre_rotation sets the rotation increasion
    3.f, //View_rotate_light_strenght sets the light strenght on rotation
    0.08f, //View_rotate_add sets the speed of the rotation
    0.08f, //View_rotate_sub sets the speed of reversing the rotation
    30.f, //View_angle sets the maximum view angle

    0.f, //CF_X the X Position of the Coverflow
    -0.2f, //CF_Y the Y Position of the Coverflow
    -7.f, //CF_Z the Z Position of the Coverflow

    0.f, //CF_RX the X Rotation of the Coverflow
    0.f, //CF_RY the Y Rotation of the Coverflow
    0.f, //CF_RZ the Z Rotation of the Coverflow

    -40.f, //Rotation sets the rotation of each cover
    0.18f, //X_Distance sets the distance between the covers
    1.f, //Center_Distance sets the distance between the centered and the non centered covers
    0.1f, //Z_Distance sets the pushback amount
    0.0f, //Y_Distance sets the elevation amount

    22.f //zoom level

};

struct Preset presetYACReaderFlowStripeConfig = {
    0.08f, //Animation_step sets the speed of the animation
    1.5f, //Animation_speedup sets the acceleration of the animation
    0.1f, //Animation_step_max sets the maximum speed of the animation
    6.f, //Animation_Fade_out_dis sets the distance of view

    1.5f, //pre_rotation sets the rotation increasion
    4.f, //View_rotate_light_strenght sets the light strenght on rotation
    0.08f, //View_rotate_add sets the speed of the rotation
    0.08f, //View_rotate_sub sets the speed of reversing the rotation
    30.f, //View_angle sets the maximum view angle

    0.f, //CF_X the X Position of the Coverflow
    -0.2f, //CF_Y the Y Position of the Coverflow
    -7.f, //CF_Z the Z Position of the Coverflow

    0.f, //CF_RX the X Rotation of the Coverflow
    0.f, //CF_RY the Y Rotation of the Coverflow
    0.f, //CF_RZ the Z Rotation of the Coverflow

    0.f, //Rotation sets the rotation of each cover
    1.1f, //X_Distance sets the distance between the covers
    0.2f, //Center_Distance sets the distance between the centered and the non centered covers
    0.01f, //Z_Distance sets the pushback amount
    0.0f, //Y_Distance sets the elevation amount

    22.f //zoom level

};

struct Preset presetYACReaderFlowOverlappedStripeConfig = {
    0.08f, //Animation_step sets the speed of the animation
    1.5f, //Animation_speedup sets the acceleration of the animation
    0.1f, //Animation_step_max sets the maximum speed of the animation
    2.f, //Animation_Fade_out_dis sets the distance of view

    1.5f, //pre_rotation sets the rotation increasion
    3.f, //View_rotate_light_strenght sets the light strenght on rotation
    0.08f, //View_rotate_add sets the speed of the rotation
    0.08f, //View_rotate_sub sets the speed of reversing the rotation
    30.f, //View_angle sets the maximum view angle

    0.f, //CF_X the X Position of the Coverflow
    -0.2f, //CF_Y the Y Position of the Coverflow
    -7.f, //CF_Z the Z Position of the Coverflow

    0.f, //CF_RX the X Rotation of the Coverflow
    0.f, //CF_RY the Y Rotation of the Coverflow
    0.f, //CF_RZ the Z Rotation of the Coverflow

    0.f, //Rotation sets the rotation of each cover
    0.18f, //X_Distance sets the distance between the covers
    1.f, //Center_Distance sets the distance between the centered and the non centered covers
    0.1f, //Z_Distance sets the pushback amount
    0.0f, //Y_Distance sets the elevation amount

    22.f //zoom level

};

struct Preset pressetYACReaderFlowUpConfig = {
    0.08f, //Animation_step sets the speed of the animation
    1.5f, //Animation_speedup sets the acceleration of the animation
    0.1f, //Animation_step_max sets the maximum speed of the animation
    2.5f, //Animation_Fade_out_dis sets the distance of view

    1.5f, //pre_rotation sets the rotation increasion
    3.f, //View_rotate_light_strenght sets the light strenght on rotation
    0.08f, //View_rotate_add sets the speed of the rotation
    0.08f, //View_rotate_sub sets the speed of reversing the rotation
    5.f, //View_angle sets the maximum view angle

    0.f, //CF_X the X Position of the Coverflow
    -0.2f, //CF_Y the Y Position of the Coverflow
    -7.f, //CF_Z the Z Position of the Coverflow

    0.f, //CF_RX the X Rotation of the Coverflow
    0.f, //CF_RY the Y Rotation of the Coverflow
    0.f, //CF_RZ the Z Rotation of the Coverflow

    -50.f, //Rotation sets the rotation of each cover
    0.18f, //X_Distance sets the distance between the covers
    1.f, //Center_Distance sets the distance between the centered and the non centered covers
    0.1f, //Z_Distance sets the pushback amount
    -0.1f, //Y_Distance sets the elevation amount

    22.f //zoom level

};

struct Preset pressetYACReaderFlowDownConfig = {
    0.08f, //Animation_step sets the speed of the animation
    1.5f, //Animation_speedup sets the acceleration of the animation
    0.1f, //Animation_step_max sets the maximum speed of the animation
    2.5f, //Animation_Fade_out_dis sets the distance of view

    1.5f, //pre_rotation sets the rotation increasion
    3.f, //View_rotate_light_strenght sets the light strenght on rotation
    0.08f, //View_rotate_add sets the speed of the rotation
    0.08f, //View_rotate_sub sets the speed of reversing the rotation
    5.f, //View_angle sets the maximum view angle

    0.f, //CF_X the X Position of the Coverflow
    -0.2f, //CF_Y the Y Position of the Coverflow
    -7.f, //CF_Z the Z Position of the Coverflow

    0.f, //CF_RX the X Rotation of the Coverflow
    0.f, //CF_RY the Y Rotation of the Coverflow
    0.f, //CF_RZ the Z Rotation of the Coverflow

    -50.f, //Rotation sets the rotation of each cover
    0.18f, //X_Distance sets the distance between the covers
    1.f, //Center_Distance sets the distance between the centered and the non centered covers
    0.1f, //Z_Distance sets the pushback amount
    0.1f, //Y_Distance sets the elevation amount

    22.f //zoom level
};
/*Constructor*/
YACReaderFlowGL::YACReaderFlowGL(QWidget *parent, struct Preset p)
    : QOpenGLWidget(/*QOpenGLWidget migration QGLFormat(QGL::SampleBuffers),*/ parent), numObjects(0), lazyPopulateObjects(-1), defaultTexture(nullptr), hasBeenInitialized(false), bUseVSync(false), flowRightToLeft(false)
{
    updateCount = 0;
    config = p;

    currentSelected = 0;

    centerPos.x = 0.f;
    centerPos.y = 0.f;
    centerPos.z = 1.f;
    centerPos.rot = 0.f;

    /*** Style ***/
    shadingTop = 0.8f;
    shadingBottom = 0.02f;
    reflectionUp = 0.f;
    reflectionBottom = 0.6f;

    /*** System variables ***/
    numObjects = 0;
    //CFImage Dummy;
    viewRotate = 0.f;
    viewRotateActive = 0;
    stepBackup = config.animationStep / config.animationSpeedUp;

    /*QTimer * timer = new QTimer();
	connect(timer, SIGNAL(timeout()), this, SLOT(updateImageData()));
	timer->start(70);
	*/

    /*loader = new WidgetLoader(0,this);
	loader->flow = this;
	QThread * loaderThread = new QThread(parent);

	loader->moveToThread(loaderThread);

	loaderThread->start();*/

    QSurfaceFormat f = format();

    //TODO add antialiasing
    //f.setSamples(4);
    f.setVersion(2, 1);
    f.setSwapInterval(0);
    setFormat(f);

    timerId = startTimer(updateInterval);
}

void YACReaderFlowGL::timerEvent(QTimerEvent *event)
{
    if (timerId == event->timerId())
        update();

    //if(!worker->isRunning())
    //worker->start();
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
}

QSize YACReaderFlowGL::minimumSizeHint() const
{
    return QSize(320, 200);
}

/*QSize YACReaderFlowGL::sizeHint() const
{
	return QSize(320, 200);
}*/

void YACReaderFlowGL::initializeGL()
{
    glShadeModel(GL_SMOOTH);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
    QPainter painter;
    painter.begin(this);

    painter.beginNativePainting();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (numObjects > 0) {
        updatePositions();
        udpatePerspective(width(), height());
        draw();
    }

    glDisable(GL_MULTISAMPLE);
    glDisable(GL_BLEND);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    painter.endNativePainting();

    QFont font = painter.font();
    font.setFamily("Arial");
    font.setPixelSize(fontSize);
    painter.setFont(font);

    painter.setPen(QColor(76, 76, 76));
    painter.drawText(10, fontSize + 10, QString("%1/%2").arg(currentSelected + 1).arg(numObjects));

    painter.end();
}

void YACReaderFlowGL::resizeGL(int width, int height)
{
    float pixelRatio = devicePixelRatio();
    fontSize = (width + height) * 0.010 * pixelRatio;
    if (fontSize < 10)
        fontSize = 10;

    //int side = qMin(width, height);
    udpatePerspective(width, height);

    if (numObjects > 0)
        updatePositions();
}

void YACReaderFlowGL::udpatePerspective(int width, int height)
{
    float pixelRatio = devicePixelRatio();
    glViewport(0, 0, width * pixelRatio, height * pixelRatio);

    glMatrixMode(GL_PROJECTION);
    QMatrix4x4 perspectiveMatrix;
    perspectiveMatrix.setToIdentity();
    perspectiveMatrix.perspective(20.0, GLdouble(width) / (float)height, 1.0, 200.0);
    glLoadMatrixf(perspectiveMatrix.constData());
    glMatrixMode(GL_MODELVIEW);
}

//-----------------------------------------------------------------------------
/*Private*/
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

    //calculate and apply positions
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
    float w = image.width;
    float h = image.height;

    //fadeout
    float opacity = 1 - 1 / (config.animationFadeOutDist + config.viewRotateLightStrenght * fabs(viewRotate)) * fabs(0 - image.current.x);

    glLoadIdentity();
    glTranslatef(config.cfX, config.cfY, config.cfZ);
    glRotatef(config.cfRX, 1, 0, 0);
    glRotatef(viewRotate * config.viewAngle + config.cfRY, 0, 1, 0);
    glRotatef(config.cfRZ, 0, 0, 1);

    glTranslatef(image.current.x, image.current.y, image.current.z);

    glPushMatrix();
    glRotatef(image.current.rot, 0, 1, 0);

    glEnable(GL_TEXTURE_2D);
    image.texture->bind();

    //calculate shading
    float LShading = ((config.rotation != 0) ? ((image.current.rot < 0) ? 1 - 1 / config.rotation * image.current.rot : 1) : 1);
    float RShading = ((config.rotation != 0) ? ((image.current.rot > 0) ? 1 - 1 / (config.rotation * -1) * image.current.rot : 1) : 1);
    float LUP = shadingTop + (1 - shadingTop) * LShading;
    float LDOWN = shadingBottom + (1 - shadingBottom) * LShading;
    float RUP = shadingTop + (1 - shadingTop) * RShading;
    float RDOWN = shadingBottom + (1 - shadingBottom) * RShading;
    ;

    //DrawCover
    glBegin(GL_QUADS);

    //esquina inferior izquierda
    glColor4f(LDOWN * opacity, LDOWN * opacity, LDOWN * opacity, 1);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(w / 2.f * -1.f, -0.5f, 0.f);

    //esquina inferior derecha
    glColor4f(RDOWN * opacity, RDOWN * opacity, RDOWN * opacity, 1);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(w / 2.f, -0.5f, 0.f);

    //esquina superior derecha
    glColor4f(RUP * opacity, RUP * opacity, RUP * opacity, 1);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(w / 2.f, -0.5f + h, 0.f);

    //esquina superior izquierda
    glColor4f(LUP * opacity, LUP * opacity, LUP * opacity, 1);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(w / 2.f * -1.f, -0.5f + h, 0.f);

    glEnd();

    //Draw reflection
    glBegin(GL_QUADS);

    //esquina inferior izquierda
    glColor4f(LUP * opacity * reflectionUp / 2, LUP * opacity * reflectionUp / 2, LUP * opacity * reflectionUp / 2, 1);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(w / 2.f * -1.f, -0.5f - h, 0.f);

    //esquina inferior derecha
    glColor4f(RUP * opacity * reflectionUp / 2, RUP * opacity * reflectionUp / 2, RUP * opacity * reflectionUp / 2, 1);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(w / 2.f, -0.5f - h, 0.f);

    //esquina superior derecha
    glColor4f(RDOWN * opacity / 3, RDOWN * opacity / 3, RDOWN * opacity / 3, 1);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(w / 2.f, -0.5f, 0.f);

    //esquina superior izquierda
    glColor4f(LDOWN * opacity / 3, LDOWN * opacity / 3, LDOWN * opacity / 3, 1);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(w / 2.f * -1.f, -0.5f, 0.f);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    if (showMarks && loaded[image.index] && marks[image.index] != Unread) {
        glEnable(GL_TEXTURE_2D);
        if (marks[image.index] == Read)
            markTexture->bind();
        else
            readingTexture->bind();
        glBegin(GL_QUADS);

        //esquina inferior izquierda
        glColor4f(RUP * opacity, RUP * opacity, RUP * opacity, 1);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(w / 2.f - 0.2, -0.688f + h, 0.001f);

        //esquina inferior derecha
        glColor4f(RUP * opacity, RUP * opacity, RUP * opacity, 1);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(w / 2.f - 0.05, -0.688f + h, 0.001f);

        //esquina superior derecha
        glColor4f(RUP * opacity, RUP * opacity, RUP * opacity, 1);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(w / 2.f - 0.05, -0.488f + h, 0.001f);

        //esquina superior izquierda
        glColor4f(RUP * opacity, RUP * opacity, RUP * opacity, 1);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(w / 2.f - 0.2, -0.488f + h, 0.001f);

        glEnd();
        glDisable(GL_TEXTURE_2D);
    }

    glPopMatrix();
}

/*Public*/
void YACReaderFlowGL::cleanupAnimation()
{
    config.animationStep = stepBackup;
    viewRotateActive = 0;
}

void YACReaderFlowGL::draw()
{
    int CS = currentSelected;
    int count;

    //Draw right Covers
    for (count = numObjects - 1; count > -1; count--) {
        if (count > CS) {
            drawCover(images[count]);
        }
    }

    //Draw left Covers
    for (count = 0; count < numObjects - 1; count++) {
        if (count < CS) {
            drawCover(images[count]);
        }
    }

    //Draw Center Cover
    drawCover(images[CS]);
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

    //slowly reset view angle
    if (!viewRotateActive) {
        viewRotate += (0 - viewRotate) * config.viewRotateSub;
    }

    if (fabs(images[currentSelected].current.x - images[currentSelected].animEnd.x) < 1) //viewRotate < 0.2)
    {
        cleanupAnimation();
        if (updateCount >= 0) //TODO parametrizar
        {

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
    //set a new entry
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
    //strcpy(cfImages[item].name,name);
}

void YACReaderFlowGL::remove(int item)
{
    if (item < 0 || item >= images.size())
        return;

    startAnimationTimer();

    loaded.remove(item);
    marks.remove(item);

    //reposition current selection
    if (item <= currentSelected && currentSelected != 0) {
        currentSelected--;
    }

    QOpenGLTexture *texture = images[item].texture;

    int count = item;
    while (count <= numObjects - 2) {
        images[count].index--;
        count++;
    }
    images.removeAt(item);

    if (texture != defaultTexture)
        delete (texture);

    numObjects--;
}

/*Info*/
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

    /*
	for(int i = 0;i<n;i++){
		QPixmap img = QPixmap(QString("./cover%1.jpg").arg(i+1));
		GLuint cover = bindTexture(img, GL_TEXTURE_2D);
		float y = 0.5 * (float(img.height())/img.width());
		Insert("cover", cover, x, y);
	}*/

    loaded = QVector<bool>(n, false);
    //marks = QVector<bool>(n,false);

    //worker->start();
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

//slots
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

void YACReaderFlowGL::setZoom(int zoom)
{
    startAnimationTimer();

    int width = this->width();
    int height = this->height();
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    QMatrix4x4 zoomMatrix;
    zoomMatrix.setToIdentity();
    zoomMatrix.perspective(zoom, (float)width / (float)height, 1.0, 200.0);
    glLoadMatrixf(zoomMatrix.constData());
    glMatrixMode(GL_MODELVIEW);
}

void YACReaderFlowGL::setRotation(int angle)
{
    startAnimationTimer();

    config.rotation = -angle;
}
//sets the distance between the covers
void YACReaderFlowGL::setX_Distance(int distance)
{
    startAnimationTimer();

    config.xDistance = distance / 100.0;
}
//sets the distance between the centered and the non centered covers
void YACReaderFlowGL::setCenter_Distance(int distance)
{
    startAnimationTimer();

    config.centerDistance = distance / 100.0;
}
//sets the pushback amount
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
        if (b) {
            QSurfaceFormat f = format();
            f.setVersion(2, 1);
            f.setSwapInterval(1);
            setFormat(f);
        } else {
            QSurfaceFormat f = format();
            f.setVersion(2, 1);
            f.setSwapInterval(0);
            setFormat(f);
        }
        reset();
    }
}
void YACReaderFlowGL::setShowMarks(bool value)
{
    startAnimationTimer();

    showMarks = value;
}
void YACReaderFlowGL::setMarks(QVector<YACReaderComicReadStatus> marks)
{
    startAnimationTimer();

    this->marks = marks;
}
void YACReaderFlowGL::setMarkImage(QImage &image)
{
    Q_UNUSED(image);
    //qué pasa la primera vez??
    //deleteTexture(markTexture);
    //markTexture = bindTexture(image,GL_TEXTURE_2D,GL_RGBA,QGLContext::LinearFilteringBindOption | QGLContext::MipmapBindOption);
}
void YACReaderFlowGL::markSlide(int index, YACReaderComicReadStatus status)
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
    //TODO calcular el tamaño del widget
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
void YACReaderFlowGL::updateMarks()
{
    //do nothing
}
/*void YACReaderFlowGL::setFlowType(FlowType flowType)
{
	//TODO esperar a que se reimplemente flowtype
}*/
void YACReaderFlowGL::render()
{
    //do nothing
}

void YACReaderFlowGL::setFlowRightToLeft(bool b)
{
    flowRightToLeft = b;
}

//EVENTOS

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
        //emit selected(centerIndex());
        return;
    }

    event->ignore();
}

void YACReaderFlowGL::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
	{
        QVector3D intersection = getPlaneIntersection(event->x(), event->y(), images[currentSelected]);
        if((intersection.x() > 0.5 && !flowRightToLeft) || (intersection.x() < -0.5 && flowRightToLeft))
        {
            showNext();
        }
        else if((intersection.x() < -0.5 && !flowRightToLeft) || (intersection.x() > 0.5 && flowRightToLeft) )
        {
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

    if(intersection.x() < 0.5 && intersection.x() > -0.5)
    {
        emit selected(centerIndex());
        event->accept();
    }
    doneCurrent();
}

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
    //numObjects = paths.size();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void YACReaderComicFlowGL::updateImageData()
{
    // can't do anything, wait for the next possibility
    if (worker->busy())
        return;

    // set image of last one
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

    // try to load only few images on the left and right side
    // i.e. all visible ones plus some extra
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
            if (!loaded[i]) //slide(i).isNull())
            {
                //loader->loadTexture(i);
                //loaded[i]=true;
                // schedule thumbnail generation
                if (paths.size() > 0) {
                    QString fname = paths.at(i);
                    //loaded[i]=true;

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
    if (item >= 0 && item < paths.size())
        paths.removeAt(item);
    worker->unlock();
}

void YACReaderComicFlowGL::resortCovers(QList<int> newOrder)
{
    worker->lock();
    worker->reset(); //is this necesary?
    startAnimationTimer();
    QList<QString> pathsNew;
    QVector<bool> loadedNew;
    QVector<YACReaderComicReadStatus> marksNew;
    QVector<YACReader3DImage> imagesNew;

    int index = 0;
    foreach (int i, newOrder) {
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

    for (auto image : images) {
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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void YACReaderPageFlowGL::updateImageData()
{
    // can't do anything, wait for the next possibility
    if (worker->busy())
        return;

    // set image of last one
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

    // try to load only few images on the left and right side
    // i.e. all visible ones plus some extra
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

                if (!loaded[i] && imagesReady[i]) //slide(i).isNull())
                {
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
    imagesSetted = QVector<bool>(n, false); //puede sobrar
}

//-----------------------------------------------------------------------------
//ImageLoader
//-----------------------------------------------------------------------------
QImage ImageLoaderGL::loadImage(const QString &fileName)
{
    QImage image;
    bool result = image.load(fileName);

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
        break; //no scaling in ultraHigh
    }

    if (!result)
        return QImage();

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
        // already running, wake up whenever ready
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
        // copy necessary data
        mutex.lock();
        this->working = true;
        QString fileName = this->fileName;
        mutex.unlock();

        QImage image = loadImage(fileName);

        // let everyone knows it is ready
        mutex.lock();
        this->working = false;
        this->img = image;
        mutex.unlock();

        // put to sleep
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

//-----------------------------------------------------------------------------
//ImageLoader
//-----------------------------------------------------------------------------
QImage ImageLoaderByteArrayGL::loadImage(const QByteArray &raw)
{
    QImage image;
    bool result = image.loadFromData(raw);

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

    if (!result)
        return QImage();

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
        // already running, wake up whenever ready
        restart = true;
        condition.wakeOne();
    }
}

void ImageLoaderByteArrayGL::run()
{
    for (;;) {
        // copy necessary data
        mutex.lock();
        this->working = true;
        QByteArray raw = this->rawData;
        mutex.unlock();

        QImage image = loadImage(raw);

        // let everyone knows it is ready
        mutex.lock();
        this->working = false;
        this->img = image;
        mutex.unlock();

        // put to sleep
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
