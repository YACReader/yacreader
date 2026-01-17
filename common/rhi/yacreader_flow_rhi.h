// Qt RHI-based Coverflow for YACReader
// Compatible with Qt 6.7+ using QRhiWidget
#ifndef __YACREADER_FLOW_RHI_H
#define __YACREADER_FLOW_RHI_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)

#include <QRhiWidget>
#include <QtWidgets>
#include <QtGui>
#include <rhi/qrhi.h>

#include "pictureflow.h"
#include "scroll_management.h"

// Reuse enums and structs from OpenGL version
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
struct YACReader3DImageRHI {
    QRhiTexture *texture;

    float width;
    float height;

    int index;

    YACReader3DVector current;
    YACReader3DVector animEnd;
};

struct Preset {
    /*** Animation Settings ***/
    float animationStep;
    float animationSpeedUp;
    float animationStepMax;
    float animationFadeOutDist;
    float preRotation;
    float viewRotateLightStrenght;
    float viewRotateAdd;
    float viewRotateSub;
    float viewAngle;

    /*** Position Configuration ***/
    float cfX;
    float cfY;
    float cfZ;
    float cfRX;
    float cfRY;
    float cfRZ;
    float rotation;
    float xDistance;
    float centerDistance;
    float zDistance;
    float yDistance;

    float zoom;
};

extern struct Preset defaultYACReaderFlowConfig;
extern struct Preset presetYACReaderFlowClassicConfig;
extern struct Preset presetYACReaderFlowStripeConfig;
extern struct Preset presetYACReaderFlowOverlappedStripeConfig;
extern struct Preset pressetYACReaderFlowUpConfig;
extern struct Preset pressetYACReaderFlowDownConfig;

class ImageLoader3D;
class ImageLoaderByteArray3D;

class YACReaderFlow3D : public QRhiWidget, public ScrollManagement
{
    Q_OBJECT

protected:
    int timerId;

    /*** System variables ***/
    YACReader3DImageRHI dummy;
    int viewRotateActive;
    float stepBackup;

    /*functions*/
    void calcPos(YACReader3DImageRHI &image, int pos);
    void calcVector(YACReader3DVector &vector, int pos);
    bool animate(YACReader3DVector &currentVector, YACReader3DVector &toVector);
    void prepareInstanceData(const YACReader3DImageRHI &image, bool isReflection, QVector<float> &data);

    int updateCount;
    int fontSize;

    // RHI resources
    QRhiTexture *defaultTexture = nullptr;
    QRhiTexture *markTexture = nullptr;
    QRhiTexture *readingTexture = nullptr;

    QRhiBuffer *vertexBuffer = nullptr;
    QRhiBuffer *instanceBuffer = nullptr;
    QRhiBuffer *uniformBuffer = nullptr;
    int alignedUniformSize = 0; // Cached aligned uniform buffer size
    int uniformBufferCapacity = 0; // Number of uniform slots allocated

    QRhiSampler *sampler = nullptr;
    QRhiGraphicsPipeline *pipeline = nullptr;
    QRhiShaderResourceBindings *shaderBindings = nullptr;

    // Cache of shader resource bindings per texture (to avoid recreating every frame)
    QMap<QRhiTexture *, QRhiShaderResourceBindings *> shaderBindingsCache;

    // Pending texture uploads (for async image loading)
    struct PendingTextureUpload {
        int index;
        QImage image;
        float x;
        float y;
    };
    QVector<PendingTextureUpload> pendingTextureUploads;

    // Uniform buffer data structure
    struct UniformData {
        QMatrix4x4 viewProjectionMatrix;
        QVector3D backgroundColor;
        float _pad0;
        QVector3D shadingColor;
        float _pad1;
        float reflectionUp;
        float reflectionDown;
        int isReflection;
        float _pad2;
    };

    void timerEvent(QTimerEvent *);

    int numObjects;
    int lazyPopulateObjects;
    bool showMarks;
    QVector<bool> loaded;
    QVector<YACReaderComicReadStatus> marks;

    QVector<YACReader3DImageRHI> images;

    bool hasBeenInitialized;

    Performance performance;

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

    // QRhiWidget overrides
    void initialize(QRhiCommandBuffer *cb) override;
    void render(QRhiCommandBuffer *cb) override;
    void releaseResources() override;
    void showEvent(QShowEvent *event) override;

    // Helper methods
    QRhiTexture *createTextureFromImage(QRhiCommandBuffer *cb, const QImage &image);
    void updateUniformBuffer(QRhiCommandBuffer *cb, const UniformData &data);
    void prepareMarkInstanceData(const YACReader3DImageRHI &image, QVector<float> &data);
    void ensureUniformBufferCapacity(int requiredSlots);
    void prepareDrawData(const YACReader3DImageRHI &image, bool isReflection, bool isMark,
                         const QMatrix4x4 &viewProjectionMatrix, float *outInstanceData,
                         UniformData &outUniformData);
    void executeDrawWithOffset(QRhiCommandBuffer *cb, QRhiTexture *texture,
                               const float *instanceData, int uniformSlot);

protected:
    QRhi *m_rhi = nullptr;
    std::unique_ptr<QRhiBuffer> m_vbuf;
    std::unique_ptr<QRhiBuffer> m_ubuf;
    std::unique_ptr<QRhiShaderResourceBindings> m_srb;
    std::unique_ptr<QRhiGraphicsPipeline> m_pipeline;
    QMatrix4x4 m_viewProjection;
    float m_rotation = 0.0f;

public:
    YACReaderFlow3D(QWidget *parent = nullptr, struct Preset p = pressetYACReaderFlowDownConfig);
    virtual ~YACReaderFlow3D();

    QSize minimumSizeHint() const override;

    void showPrevious();
    void showNext();
    void setCurrentIndex(int pos);
    void cleanupAnimation();
    void draw();
    void updatePositions();
    void insert(char *name, QRhiTexture *texture, float x, float y, int item = -1);
    virtual void remove(int item);
    void add(int item);
    void replace(char *name, QRhiTexture *texture, float x, float y, int item);
    void populate(int n);
    YACReader3DImageRHI getCurrentSelected();

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
    void useVSync(bool b); // Compatibility method (no-op for RHI)
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
    void render(); // Compatibility method (triggers update())
    void resizeGL(int width, int height); // Compatibility method (no-op for RHI)

    QVector3D getPlaneIntersection(int x, int y, YACReader3DImageRHI plane);
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    friend class ImageLoader3D;
    friend class ImageLoaderByteArray3D;

signals:
    void centerIndexChanged(int);
    void selected(unsigned int);
};

class YACReaderComicFlow3D : public YACReaderFlow3D
{
public:
    YACReaderComicFlow3D(QWidget *parent = nullptr, struct Preset p = defaultYACReaderFlowConfig);
    void setImagePaths(QStringList paths);
    void updateImageData() override;
    void remove(int item) override;
    void add(const QString &path, int index);
    void resortCovers(QList<int> newOrder);
    friend class ImageLoader3D;

private:
    ImageLoader3D *worker;

protected:
    QList<QString> paths;
};

class YACReaderPageFlow3D : public YACReaderFlow3D
{
public:
    YACReaderPageFlow3D(QWidget *parent = nullptr, struct Preset p = defaultYACReaderFlowConfig);
    ~YACReaderPageFlow3D();
    void updateImageData() override;
    void populate(int n);
    QVector<bool> imagesReady;
    QVector<QByteArray> rawImages;
    QVector<bool> imagesSetted;
    friend class ImageLoaderByteArray3D;

private:
    ImageLoaderByteArray3D *worker;
};

class ImageLoader3D : public QThread
{
public:
    ImageLoader3D(YACReaderFlow3D *flow);
    ~ImageLoader3D();
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
    YACReaderFlow3D *flow;
    QImage loadImage(const QString &fileName);

protected:
    void run() override;

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

class ImageLoaderByteArray3D : public QThread
{
public:
    ImageLoaderByteArray3D(YACReaderFlow3D *flow);
    ~ImageLoaderByteArray3D();
    bool busy() const;
    void generate(int index, const QByteArray &raw);
    void reset()
    {
        idx = -1;
        rawData.clear();
    }
    int index() const { return idx; }
    QImage result();
    YACReaderFlow3D *flow;
    QImage loadImage(const QByteArray &rawData);

protected:
    void run() override;

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

#endif // QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)

#endif // __YACREADER_FLOW_RHI_H
