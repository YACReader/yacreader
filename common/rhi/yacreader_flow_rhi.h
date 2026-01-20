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

    // Precomputed bounding sphere radius (world-space) for fast culling
    float boundingRadius;

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

class QLabel;
class ImageLoader3D;
class ImageLoaderByteArray3D;
class YACReaderComicFlow3D;
class YACReaderPageFlow3D;

class YACReaderFlow3D : public QRhiWidget, public ScrollManagement
{
    Q_OBJECT

protected:
    int timerId;

    YACReader3DImageRHI dummy;
    int viewRotateActive;
    float stepBackup;

    void calcPos(YACReader3DImageRHI &image, int pos);
    void calcVector(YACReader3DVector &vector, int pos);
    bool animate(YACReader3DVector &currentVector, YACReader3DVector &toVector);
    void prepareInstanceData(const YACReader3DImageRHI &image, bool isReflection, QVector<float> &data);

    int updateCount;
    int fontSize;

    // Cached state for the index label to avoid unnecessary updates
    struct IndexLabelState {
        int current = -1;
        int total = -1;
    };

    // Uniform buffer data structure (must match shader layout)
    struct UniformData {
        float viewProjectionMatrix[16]; // column-major 4x4
        float backgroundColor[3];
        float _pad0; // pad to vec4
        float reflectionUp;
        float reflectionDown;
        float isReflection;
        float _pad1;
    };

    // Pending texture uploads (for async image loading)
    struct PendingTextureUpload {
        int index;
        QImage image;
        float x;
        float y;
    };

    // Scene struct encapsulating all RHI resources
    struct Scene {
        // Textures
        std::unique_ptr<QRhiTexture> defaultTexture;
        std::unique_ptr<QRhiTexture> markTexture;
        std::unique_ptr<QRhiTexture> readingTexture;

        // Buffers
        std::unique_ptr<QRhiBuffer> vertexBuffer;
        std::unique_ptr<QRhiBuffer> instanceBuffer;
        std::unique_ptr<QRhiBuffer> uniformBuffer;

        // Pipeline and bindings
        std::unique_ptr<QRhiSampler> sampler;
        std::unique_ptr<QRhiGraphicsPipeline> pipeline;
        std::unique_ptr<QRhiShaderResourceBindings> shaderBindings;

        // Cache of shader resource bindings per texture (raw pointers, owned by this struct)
        QMap<QRhiTexture *, QRhiShaderResourceBindings *> shaderBindingsCache;

        // Uniform buffer sizing
        int alignedUniformSize = 0;
        int uniformBufferCapacity = 0;

        // Pending resource updates (accumulated between frames)
        QRhiResourceUpdateBatch *resourceUpdates = nullptr;

        // Reset all resources (cleans up cache manually)
        void reset()
        {
            qDeleteAll(shaderBindingsCache);
            shaderBindingsCache.clear();
            defaultTexture.reset();
            markTexture.reset();
            readingTexture.reset();
            vertexBuffer.reset();
            instanceBuffer.reset();
            uniformBuffer.reset();
            sampler.reset();
            pipeline.reset();
            shaderBindings.reset();
            alignedUniformSize = 0;
            uniformBufferCapacity = 0;
            resourceUpdates = nullptr;
        }
    };

    Scene scene;
    QVector<PendingTextureUpload> pendingTextureUploads;

    // Index label (shows "current/total" in top-left corner)
    QLabel *indexLabel = nullptr;
    IndexLabelState indexLabelState;

    void timerEvent(QTimerEvent *) override;

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
    void resizeEvent(QResizeEvent *event) override;

    // Index label helpers
    void updateIndexLabel();
    void updateIndexLabelStyle();

    // Helper methods
    QRhiTexture *createTextureFromImage(QRhiCommandBuffer *cb, const QImage &image);
    void updateUniformBuffer(QRhiCommandBuffer *cb, const UniformData &data);
    void prepareMarkInstanceData(const YACReader3DImageRHI &image, QVector<float> &data);
    void ensureUniformBufferCapacity(int requiredSlots);
    void ensurePipeline();
    void prepareDrawData(const YACReader3DImageRHI &image, bool isReflection, bool isMark,
                         const QMatrix4x4 &viewProjectionMatrix, float *outInstanceData,
                         UniformData &outUniformData);
    QMatrix4x4 buildModelMatrix(const YACReader3DImageRHI &image, bool isReflection, bool isMark);
    QMatrix4x4 buildModelMatrixNoScale(const YACReader3DImageRHI &image, bool isReflection, bool isMark);
    void executeDrawWithOffset(QRhiCommandBuffer *cb, QRhiTexture *texture,
                               const float *instanceData, int uniformSlot);

protected:
    QRhi *m_rhi = nullptr;

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
    void insert(QRhiTexture *texture, float x, float y, int item = -1);
    virtual void remove(int item);
    void add(int item);
    void replace(QRhiTexture *texture, float x, float y, int item);
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

/* Derived flow & loader classes moved to dedicated files:
   - common/rhi/yacreader_comic_flow_rhi.h/.cpp
   - common/rhi/yacreader_page_flow_rhi.h/.cpp
*/

#endif // QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)

#endif // __YACREADER_FLOW_RHI_H
