// OpenGL Coverflow API by J.Roth - Modernized with Shaders
#ifndef __YACREADER_FLOW_GL_H
#define __YACREADER_FLOW_GL_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtOpenGL/QOpenGLTexture>
#else
#include <QOpenGLWidget>
#include <QOpenGLTexture>
#endif

#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QtWidgets>

#include "pictureflow.h"
#include "scroll_management.h"

class ImageLoaderGL;
class ImageLoaderByteArrayGL;

enum Performance {
    low = 0,
    medium,
    high,
    ultraHigh
};

// Cover Vector
struct YACReader3DVector {
    float x;
    float y;
    float z;
    float rot;
};

// the image/texture info struct
struct YACReader3DImage {
    QOpenGLTexture *texture;

    float width;
    float height;

    int index;

    YACReader3DVector current;
    YACReader3DVector animEnd;
};

struct Preset {
    /*** Animation Settings ***/
    // sets the speed of the animation
    float animationStep;
    // sets the acceleration of the animation
    float animationSpeedUp;
    // sets the maximum speed of the animation
    float animationStepMax;
    // sets the distance of view
    float animationFadeOutDist;
    // sets the rotation increasion
    float preRotation;
    // sets the light strenght on rotation
    float viewRotateLightStrenght;
    // sets the speed of the rotation
    float viewRotateAdd;
    // sets the speed of reversing the rotation
    float viewRotateSub;
    // sets the maximum view angle
    float viewAngle;

    /*** Position Configuration ***/
    // the X Position of the Coverflow
    float cfX;
    // the Y Position of the Coverflow
    float cfY;
    // the Z Position of the Coverflow
    float cfZ;
    // the X Rotation of the Coverflow
    float cfRX;
    // the Y Rotation of the Coverflow
    float cfRY;
    // the Z Rotation of the Coverflow
    float cfRZ;
    // sets the rotation of each cover
    float rotation;
    // sets the distance between the covers
    float xDistance;
    // sets the distance between the centered and the non centered covers
    float centerDistance;
    // sets the pushback amount
    float zDistance;
    // sets the elevation amount
    float yDistance;

    float zoom;
};

extern struct Preset defaultYACReaderFlowConfig;
extern struct Preset presetYACReaderFlowClassicConfig;
extern struct Preset presetYACReaderFlowStripeConfig;
extern struct Preset presetYACReaderFlowOverlappedStripeConfig;
extern struct Preset pressetYACReaderFlowUpConfig;
extern struct Preset pressetYACReaderFlowDownConfig;

class YACReaderFlowGL : public QOpenGLWidget, protected QOpenGLExtraFunctions, public ScrollManagement
{
    Q_OBJECT
protected:
    int timerId;
    /*** System variables ***/
    YACReader3DImage dummy;
    int viewRotateActive;
    float stepBackup;

    /*functions*/
    void calcPos(YACReader3DImage &image, int pos);
    void calcVector(YACReader3DVector &vector, int pos);
    bool animate(YACReader3DVector &currentVector, YACReader3DVector &toVector);
    void drawCover(const YACReader3DImage &image);
    void drawReflection(const YACReader3DImage &image);
    void prepareInstanceData(const YACReader3DImage &image, bool isReflection, QVector<GLfloat> &data);
    void drawMark(const YACReader3DImage &image, const QMatrix4x4 &viewProjectionMatrix);

    void updatePerspective(int width, int height);

    int updateCount;
    int fontSize;

    QOpenGLTexture *defaultTexture = nullptr;
    QOpenGLTexture *markTexture = nullptr;
    QOpenGLTexture *readingTexture = nullptr;

    // Shader program and buffers
    QOpenGLShaderProgram *shaderProgram = nullptr;
    QOpenGLBuffer *vbo = nullptr;
    QOpenGLBuffer *instanceVBO = nullptr;
    QOpenGLVertexArrayObject *vao = nullptr;

    void initializeGL();
    void paintGL();
    void timerEvent(QTimerEvent *);
    void setupShaders();
    void setupGeometry();

    int numObjects;
    int lazyPopulateObjects;
    bool showMarks;
    QVector<bool> loaded;
    QVector<YACReaderComicReadStatus> marks;

    QVector<YACReader3DImage> images;

    bool hasBeenInitialized;

    Performance performance;
    bool bUseVSync;

    /*** Animation Settings ***/
    Preset config;

    int currentSelected;

    YACReader3DVector centerPos;

    /*** Style ***/
    float shadingTop;
    float shadingBottom;

    float reflectionUp;
    float reflectionBottom;

    /*** Theme Colors ***/
    QColor backgroundColor;
    QColor textColor;
    QColor shadingColor;

    /*** System info ***/
    float viewRotate;

    static int updateInterval;

    bool flowRightToLeft;

    void startAnimationTimer();
    void stopAnimationTimer();

public:
    YACReaderFlowGL(QWidget *parent = 0, struct Preset p = pressetYACReaderFlowDownConfig);
    virtual ~YACReaderFlowGL();

    QSize minimumSizeHint() const;

    void showPrevious();
    void showNext();
    void setCurrentIndex(int pos);
    void cleanupAnimation();
    void draw();
    void updatePositions();
    void insert(char *name, QOpenGLTexture *texture, float x, float y, int item = -1);
    virtual void remove(int item);
    void add(int item);
    void replace(char *name, QOpenGLTexture *texture, float x, float y, int item);
    void populate(int n);
    YACReader3DImage getCurrentSelected();

public slots:
    void setCF_RX(int value);
    void setCF_RY(int value);
    void setCF_RZ(int value);
    void setZoom(int zoom);
    void setRotation(int angle);
    void setX_Distance(int distance);
    void setCenter_Distance(int distance);
    void setZ_Distance(int distance);
    void setCF_Y(int value);
    void setCF_Z(int value);
    void setY_Distance(int value);
    void setFadeOutDist(int value);
    void setLightStrenght(int value);
    void setMaxAngle(int value);
    void setPreset(const Preset &p);
    void setPerformance(Performance performance);
    void useVSync(bool b);
    void setFlowRightToLeft(bool b);

    // Theme color setters
    void setBackgroundColor(const QColor &color);
    void setTextColor(const QColor &color);
    void setShadingColor(const QColor &color);

    virtual void updateImageData() = 0;

    void reset();
    void reload();

    void setShowMarks(bool value);
    void setMarks(QVector<YACReader::YACReaderComicReadStatus> marks);
    void setMarkImage(QImage &image);
    void markSlide(int index, YACReader::YACReaderComicReadStatus status);
    void unmarkSlide(int index);
    void setSlideSize(QSize size);
    void clear();
    void setCenterIndex(unsigned int index);
    void showSlide(int index);
    int centerIndex();
    void updateMarks();
    void render();

    QVector3D getPlaneIntersection(int x, int y, YACReader3DImage plane);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void resizeGL(int width, int height);

    friend class ImageLoaderGL;
    friend class ImageLoaderByteArrayGL;

signals:
    void centerIndexChanged(int);
    void selected(unsigned int);
};

class YACReaderComicFlowGL : public YACReaderFlowGL
{
public:
    YACReaderComicFlowGL(QWidget *parent = 0, struct Preset p = defaultYACReaderFlowConfig);
    void setImagePaths(QStringList paths);
    void updateImageData();
    void remove(int item);
    void add(const QString &path, int index);
    void resortCovers(QList<int> newOrder);
    friend class ImageLoaderGL;

private:
    ImageLoaderGL *worker;

protected:
    QList<QString> paths;
};

class YACReaderPageFlowGL : public YACReaderFlowGL
{
public:
    YACReaderPageFlowGL(QWidget *parent = 0, struct Preset p = defaultYACReaderFlowConfig);
    ~YACReaderPageFlowGL();
    void updateImageData();
    void populate(int n);
    QVector<bool> imagesReady;
    QVector<QByteArray> rawImages;
    QVector<bool> imagesSetted;
    friend class ImageLoaderByteArrayGL;

private:
    ImageLoaderByteArrayGL *worker;
};

class ImageLoaderGL : public QThread
{
public:
    ImageLoaderGL(YACReaderFlowGL *flow);
    ~ImageLoaderGL();
    bool busy() const;
    void generate(int index, const QString &fileName);
    void reset()
    {
        idx = -1;
        fileName = "";
    }
    int index() const { return idx; }
    void lock();
    void unlock();
    QImage result();
    YACReaderFlowGL *flow;
    GLuint resultTexture;
    QImage loadImage(const QString &fileName);

protected:
    void run();

private:
    QMutex mutex;
    QWaitCondition condition;

    bool restart;
    bool working;
    int idx;
    QString fileName;
    QSize size;
    QImage img;
};

class ImageLoaderByteArrayGL : public QThread
{
public:
    ImageLoaderByteArrayGL(YACReaderFlowGL *flow);
    ~ImageLoaderByteArrayGL();
    bool busy() const;
    void generate(int index, const QByteArray &raw);
    void reset()
    {
        idx = -1;
        rawData.clear();
    }
    int index() const { return idx; }
    QImage result();
    YACReaderFlowGL *flow;
    GLuint resultTexture;
    QImage loadImage(const QByteArray &rawData);

protected:
    void run();

private:
    QMutex mutex;
    QWaitCondition condition;

    bool restart;
    bool working;
    int idx;
    QByteArray rawData;
    QSize size;
    QImage img;
};

#endif
