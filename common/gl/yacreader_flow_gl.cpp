#include "yacreader_flow_gl.h"

#include <QtGui>
#include <QMatrix4x4>
#include <QVector3D>

#include <cmath>
#include <iostream>
/*** Animation Settings ***/

/*** Position Configuration ***/

int YACReaderFlowGL::updateInterval = 16;

struct Preset defaultYACReaderFlowConfig = {
  0.08f, //Animation_step sets the speed of the animation
  1.5f,  //Animation_speedup sets the acceleration of the animation
  0.1f,  //Animation_step_max sets the maximum speed of the animation
  3.f,    //Animation_Fade_out_dis sets the distance of view

  1.5f,  //pre_rotation sets the rotation increasion
  3.f,    //View_rotate_light_strenght sets the light strenght on rotation
  0.01f, //View_rotate_add sets the speed of the rotation
  0.02f, //View_rotate_sub sets the speed of reversing the rotation
  20.f,   //View_angle sets the maximum view angle

  0.f,    //CF_X the X Position of the Coverflow
  0.f,    //CF_Y the Y Position of the Coverflow
  -8.f,  //CF_Z the Z Position of the Coverflow

  15.f,   //CF_RX the X Rotation of the Coverflow
  0.f,    //CF_RY the Y Rotation of the Coverflow
  0.f,    //CF_RZ the Z Rotation of the Coverflow

  -50.f,  //Rotation sets the rotation of each cover
  0.18f, //X_Distance sets the distance between the covers
  1.f,    //Center_Distance sets the distance between the centered and the non centered covers
  0.1f,  //Z_Distance sets the pushback amount
  0.0f,  //Y_Distance sets the elevation amount

  30.f    //zoom level

};

struct Preset presetYACReaderFlowClassicConfig = {
  0.08f, //Animation_step sets the speed of the animation
  1.5f,  //Animation_speedup sets the acceleration of the animation
  0.1f,  //Animation_step_max sets the maximum speed of the animation
  2.f,	  //Animation_Fade_out_dis sets the distance of view

  1.5f,  //pre_rotation sets the rotation increasion
  3.f,	  //View_rotate_light_strenght sets the light strenght on rotation
  0.08f, //View_rotate_add sets the speed of the rotation
  0.08f, //View_rotate_sub sets the speed of reversing the rotation
  30.f,	  //View_angle sets the maximum view angle

  0.f,	  //CF_X the X Position of the Coverflow
  -0.2f, //CF_Y the Y Position of the Coverflow
  -7.f,	  //CF_Z the Z Position of the Coverflow

  0.f,	  //CF_RX the X Rotation of the Coverflow
  0.f,	  //CF_RY the Y Rotation of the Coverflow
  0.f,	  //CF_RZ the Z Rotation of the Coverflow

  -40.f,  //Rotation sets the rotation of each cover
  0.18f, //X_Distance sets the distance between the covers
  1.f,	  //Center_Distance sets the distance between the centered and the non centered covers
  0.1f,  //Z_Distance sets the pushback amount
  0.0f,  //Y_Distance sets the elevation amount

  22.f    //zoom level

};

struct Preset presetYACReaderFlowStripeConfig = {
  0.08f, //Animation_step sets the speed of the animation
  1.5f,  //Animation_speedup sets the acceleration of the animation
  0.1f,  //Animation_step_max sets the maximum speed of the animation
  6.f,    //Animation_Fade_out_dis sets the distance of view

  1.5f,  //pre_rotation sets the rotation increasion
  4.f,	  //View_rotate_light_strenght sets the light strenght on rotation
  0.08f, //View_rotate_add sets the speed of the rotation
  0.08f, //View_rotate_sub sets the speed of reversing the rotation
  30.f,	  //View_angle sets the maximum view angle

  0.f,	  //CF_X the X Position of the Coverflow
  -0.2f, //CF_Y the Y Position of the Coverflow
  -7.f,	  //CF_Z the Z Position of the Coverflow

  0.f,	  //CF_RX the X Rotation of the Coverflow
  0.f,	  //CF_RY the Y Rotation of the Coverflow
  0.f,	  //CF_RZ the Z Rotation of the Coverflow

  0.f,	  //Rotation sets the rotation of each cover
  1.1f, //X_Distance sets the distance between the covers
  0.2f,	  //Center_Distance sets the distance between the centered and the non centered covers
  0.01f,  //Z_Distance sets the pushback amount
  0.0f,  //Y_Distance sets the elevation amount

  22.f    //zoom level

};

struct Preset presetYACReaderFlowOverlappedStripeConfig = {
  0.08f, //Animation_step sets the speed of the animation
  1.5f,  //Animation_speedup sets the acceleration of the animation
  0.1f,  //Animation_step_max sets the maximum speed of the animation
  2.f,	  //Animation_Fade_out_dis sets the distance of view

  1.5f,  //pre_rotation sets the rotation increasion
  3.f,	  //View_rotate_light_strenght sets the light strenght on rotation
  0.08f, //View_rotate_add sets the speed of the rotation
  0.08f, //View_rotate_sub sets the speed of reversing the rotation
  30.f,	  //View_angle sets the maximum view angle

  0.f,	  //CF_X the X Position of the Coverflow
  -0.2f, //CF_Y the Y Position of the Coverflow
  -7.f,	  //CF_Z the Z Position of the Coverflow

  0.f,	  //CF_RX the X Rotation of the Coverflow
  0.f,	  //CF_RY the Y Rotation of the Coverflow
  0.f,	  //CF_RZ the Z Rotation of the Coverflow

  0.f,	  //Rotation sets the rotation of each cover
  0.18f, //X_Distance sets the distance between the covers
  1.f,	  //Center_Distance sets the distance between the centered and the non centered covers
  0.1f,  //Z_Distance sets the pushback amount
  0.0f,  //Y_Distance sets the elevation amount

  22.f    //zoom level

};

struct Preset pressetYACReaderFlowUpConfig = {
  0.08f, //Animation_step sets the speed of the animation
  1.5f,  //Animation_speedup sets the acceleration of the animation
  0.1f,  //Animation_step_max sets the maximum speed of the animation
  2.5f,	  //Animation_Fade_out_dis sets the distance of view

  1.5f,  //pre_rotation sets the rotation increasion
  3.f,	  //View_rotate_light_strenght sets the light strenght on rotation
  0.08f, //View_rotate_add sets the speed of the rotation
  0.08f, //View_rotate_sub sets the speed of reversing the rotation
  5.f,	  //View_angle sets the maximum view angle

  0.f,	  //CF_X the X Position of the Coverflow
  -0.2f, //CF_Y the Y Position of the Coverflow
  -7.f,	  //CF_Z the Z Position of the Coverflow

  0.f,	  //CF_RX the X Rotation of the Coverflow
  0.f,	  //CF_RY the Y Rotation of the Coverflow
  0.f,	  //CF_RZ the Z Rotation of the Coverflow

  -50.f,	  //Rotation sets the rotation of each cover
  0.18f, //X_Distance sets the distance between the covers
  1.f,	  //Center_Distance sets the distance between the centered and the non centered covers
  0.1f,  //Z_Distance sets the pushback amount
  -0.1f,  //Y_Distance sets the elevation amount

  22.f    //zoom level

};

struct Preset pressetYACReaderFlowDownConfig = {
  0.08f, //Animation_step sets the speed of the animation
  1.5f,  //Animation_speedup sets the acceleration of the animation
  0.1f,  //Animation_step_max sets the maximum speed of the animation
  2.5f,	  //Animation_Fade_out_dis sets the distance of view

  1.5f,  //pre_rotation sets the rotation increasion
  3.f,	  //View_rotate_light_strenght sets the light strenght on rotation
  0.08f, //View_rotate_add sets the speed of the rotation
  0.08f, //View_rotate_sub sets the speed of reversing the rotation
  5.f,	  //View_angle sets the maximum view angle

  0.f,	  //CF_X the X Position of the Coverflow
  -0.2f, //CF_Y the Y Position of the Coverflow
  -7.f,	  //CF_Z the Z Position of the Coverflow

  0.f,	  //CF_RX the X Rotation of the Coverflow
  0.f,	  //CF_RY the Y Rotation of the Coverflow
  0.f,	  //CF_RZ the Z Rotation of the Coverflow

  -50.f,	  //Rotation sets the rotation of each cover
  0.18f, //X_Distance sets the distance between the covers
  1.f,	  //Center_Distance sets the distance between the centered and the non centered covers
  0.1f,  //Z_Distance sets the pushback amount
  0.1f,  //Y_Distance sets the elevation amount

  22.f    //zoom level
};
/*Constructor*/
YACReaderFlowGL::YACReaderFlowGL(QWidget *parent, struct Preset p)
  :QOpenGLWidget(parent),
  numObjects(0),
  lazyPopulateObjects(-1),
  bUseVSync(false),
  hasBeenInitialized(false),
  flowRightToLeft(false)
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
  stepBackup = config.animationStep/config.animationSpeedUp;

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
    f.setSamples(4);
    f.setVersion(2, 1);
    f.setSwapInterval(0);
    setFormat(f);

    timerId = startTimer(updateInterval);
}

void YACReaderFlowGL::timerEvent(QTimerEvent * event)
{
  if (timerId == event->timerId())
  {
      update();
  }

  //if(!worker->isRunning())
    //worker->start();
}

void YACReaderFlowGL::startAnimationTimer()
{
  if (timerId == -1)
  {
    timerId = startTimer(updateInterval);
  }
}

void YACReaderFlowGL::stopAnimationTimer()
{
    if (timerId != -1)
    {
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

void YACReaderFlowGL::initializeGL()
{
  static QString vertex_shader =
  "//#version 110\n"
  "\n"
  "attribute vec3 position;\n"
  "attribute vec3 color;\n"
  "attribute vec2 texCoord;\n"

  "varying vec3 v_color;\n"
  "varying vec2 v_texCoord;\n"

  "uniform mat4 modelview;\n"
  "uniform mat4 projection;\n"

  "void main()\n"
  "{\n"
  "	v_color = color;\n"
  "	v_texCoord = texCoord;\n"
  "	gl_Position = (projection * modelview) * vec4(position, 1);\n"
  "}\n";

  static QString fragment_shader =
  "//#version 110\n"
  "#ifdef GL_ES\n"
  "	precision mediump float;\n"
  "#endif\n"
  "varying vec3 v_color;\n"
  "varying vec2 v_texCoord;\n"
  "uniform sampler2D texture;\n"

  "void main()\n"
  "{\n"
  "	gl_FragColor = texture2D(texture, v_texCoord) * vec4(v_color, 1.0);\n"
  "}\n";

  //use a vertex array object to safe all OpenGL settings
  vao = new QOpenGLVertexArrayObject();
  vao->create();
  if (vao->isCreated())
  {
    vao->bind();
  }

  pipeline = new QOpenGLShaderProgram(this);
  pipeline->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_shader);
  pipeline->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_shader);
  pipeline->link();

  v_buffer = new QOpenGLBuffer();
  v_buffer->create();
  v_buffer->bind();

  pipeline->setAttributeBuffer("position", GL_FLOAT, 0, 3, 8*sizeof(float));
  pipeline->setAttributeBuffer("texCoord", GL_FLOAT, 3*sizeof(float), 2, 8*sizeof(float));
  pipeline->setAttributeBuffer("color", GL_FLOAT, 5*sizeof(float), 3, 8*sizeof(float));
  pipeline->enableAttributeArray("position");
  pipeline->enableAttributeArray("texCoord");
  pipeline->enableAttributeArray("color");

  //cache uniform locations for later use
  m_projection_location = pipeline->uniformLocation("projection");
  m_modelview_location = pipeline->uniformLocation("modelview");

  v_buffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
  //v_buffer->allocate(3*6*8*sizeof(float));
  v_buffer->allocate(3*12*8*sizeof(float));
  v_buffer->release();

  defaultTexture = new QOpenGLTexture(QImage(":/images/defaultCover.png"));
  defaultTexture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::Linear);

#ifdef YACREADER_LIBRARY
  markTexture = new QOpenGLTexture(QImage(":/images/readRibbon.png"));
  markTexture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::Linear);
  readingTexture = new QOpenGLTexture(QImage(":/images/readingRibbon.png"));
  readingTexture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::Linear);
#endif
  if (lazyPopulateObjects!=-1)
  {
    populate(lazyPopulateObjects);
  }

  hasBeenInitialized = true;
  vao->release();
}

void YACReaderFlowGL::paintGL()
{
    QPainter painter;
    painter.begin(this);
    painter.beginNativePainting();

    vao->bind();
    pipeline->bind();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (numObjects>0)
    {
        updatePositions();
        udpatePerspective(width(),height());
        draw();
    }

    vao->release();
    pipeline->release();

    painter.endNativePainting();

    QFont font = painter.font() ;
    font.setFamily("Arial");
    font.setPixelSize(fontSize);
    painter.setFont(font);

    painter.setPen(QColor(76,76,76));
    painter.drawText(10,fontSize + 10, QString("%1/%2").arg(currentSelected+1).arg(numObjects));
    painter.end();
}

void YACReaderFlowGL::resizeGL(int width, int height)
{
  pipeline->bind();
  float pixelRatio = devicePixelRatio();
  fontSize = (width + height) * 0.010 * pixelRatio;
  if (fontSize < 10)
  {
    fontSize = 10;
  }
  udpatePerspective(width,height);

  if (numObjects>0)
  {
    updatePositions();
  }
  pipeline->release();
}

void YACReaderFlowGL::udpatePerspective(int width, int height)
{
  float pixelRatio = devicePixelRatio();
  glViewport(0, 0, width*pixelRatio, height*pixelRatio);
  m_projection.setToIdentity();
  m_projection.perspective(20.0, ((float)width/(float)height), 1.0, 200.0);
  pipeline->setUniformValue(m_projection_location, m_projection);
}

//-----------------------------------------------------------------------------
/*Private*/
void YACReaderFlowGL::calcPos(YACReader3DImage & image, int pos)
{
  if (flowRightToLeft)
  {
      pos = pos * -1;
  }

  if (pos == 0)
  {
      image.current = centerPos;
  }
  else
  {
    if (pos > 0)
    {
        image.current.x = (config.centerDistance)+(config.xDistance*pos);
        image.current.y = config.yDistance*pos*-1;
        image.current.z = config.zDistance*pos*-1;
        image.current.rot = config.rotation;
    }
    else
    {
        image.current.x = (config.centerDistance)*-1+(config.xDistance*pos);
        image.current.y =  config.yDistance*pos;
        image.current.z = config.zDistance*pos;
        image.current.rot = config.rotation*-1;
    }
  }
}

void YACReaderFlowGL::calcVector(YACReader3DVector & vector, int pos)
{
    calcPos(dummy,pos);

    vector.x = dummy.current.x;
    vector.y = dummy.current.y;
    vector.z = dummy.current.z;
    vector.rot = dummy.current.rot;
}

bool YACReaderFlowGL::animate(YACReader3DVector & currentVector,YACReader3DVector & toVector)
{
    float rotDiff = toVector.rot-currentVector.rot;
    float xDiff = toVector.x-currentVector.x;
    float yDiff = toVector.y-currentVector.y;
    float zDiff = toVector.z-currentVector.z;

    if (fabs(rotDiff) < 0.01
    && fabs(xDiff)  < 0.001
    && fabs(yDiff)  < 0.001
    && fabs(zDiff)  < 0.001)
    {
      return true;
    }

    //calculate and apply positions
    currentVector.x = currentVector.x+(xDiff)*config.animationStep;
    currentVector.y = currentVector.y+(yDiff)*config.animationStep;
    currentVector.z = currentVector.z+(zDiff)*config.animationStep;

    if (fabs(rotDiff) > 0.01)
    {
      currentVector.rot = currentVector.rot+(rotDiff)*(config.animationStep*config.preRotation);
    }
    else
    {
      viewRotateActive = 0;
    }

    return false;
}
void YACReaderFlowGL::drawCover(const YACReader3DImage & image)
{

  float w = image.width;
  float h = image.height;

  //fadeout
  float opacity = 1-1/(config.animationFadeOutDist+config.viewRotateLightStrenght*fabs(viewRotate))*fabs(0-image.current.x);
  if (opacity <= 0)
  {
    return;
  }

  //calculate shading
  float LShading = ((config.rotation != 0 )?((image.current.rot < 0)?1-1/config.rotation*image.current.rot:1):1);
  float RShading = ((config.rotation != 0 )?((image.current.rot > 0)?1-1/(config.rotation*-1)*image.current.rot:1):1);
  float LUP = shadingTop+(1-shadingTop)*LShading;
  float LDOWN = shadingBottom+(1-shadingBottom)*LShading;
  float RUP =  shadingTop+(1-shadingTop)*RShading;
  float RDOWN =  shadingBottom+(1-shadingBottom)*RShading;

  //3d coordinates
  auto quad3 = [] (std::array<float, 12> quad)
  {
    auto interpolate = [] (float first, float second) {
      return first + (second-first)/2;
    };
  return std::array<float, 36> {
    quad[0], quad[1], quad[2],
    quad[3], quad[4], quad[5],
    interpolate(quad[0], quad[6]),interpolate(quad[1], quad[7]),interpolate(quad[2], quad[8]),
    quad[3], quad[4], quad[5],
    quad[6], quad[7], quad[8],
    interpolate(quad[0], quad[6]),interpolate(quad[1], quad[7]),interpolate(quad[2], quad[8]),
    quad[6], quad[7], quad[8],
    quad[9], quad[10], quad[11],
    interpolate(quad[0], quad[6]),interpolate(quad[1], quad[7]),interpolate(quad[2], quad[8]),
    quad[9], quad[10], quad[11],
    quad[0], quad[1], quad[2],
    interpolate(quad[0], quad[6]),interpolate(quad[1], quad[7]),interpolate(quad[2], quad[8])};
  };

  // shadow/colors
  auto quadc = [] (std::array<float, 12> quad)
  {
    auto interpolate = [] (float first, float second,
    float third, float fourth) {
    return (((first + second)/2 + (third + fourth)/2)/2);
  };
  return std::array<float, 36> {
    quad[0], quad[1], quad[2],
    quad[3], quad[4], quad[5],

    interpolate(quad[0], quad[6], quad[3], quad[9]),
    interpolate(quad[1], quad[7], quad[4], quad[10]),
    interpolate(quad[2], quad[8], quad[5], quad[11]),

    quad[3], quad[4], quad[5],
    quad[6], quad[7], quad[8],

    interpolate(quad[0], quad[6], quad[3], quad[9]),
    interpolate(quad[1], quad[7], quad[4], quad[10]),
    interpolate(quad[2], quad[8], quad[5], quad[11]),

    quad[6], quad[7], quad[8],
    quad[9], quad[10], quad[11],

    interpolate(quad[0], quad[6], quad[3], quad[9]),
    interpolate(quad[1], quad[7], quad[4], quad[10]),
    interpolate(quad[2], quad[8], quad[5], quad[11]),

    quad[9], quad[10], quad[11],
    quad[0], quad[1], quad[2],

    interpolate(quad[0], quad[6], quad[3], quad[9]),
    interpolate(quad[1], quad[7], quad[4], quad[10]),
    interpolate(quad[2], quad[8], quad[5], quad[11])};
  };

  // texture coordinates
  auto quad2 = [] (std::array<float, 8> quad)	{
    return std::array<float, 24> {
      quad[0], quad[1],
      quad[2], quad[3],
      0.5,0.5,
      quad[2], quad[3],
      quad[4], quad[5],
      0.5,0.5,
      quad[4], quad[5],
      quad[6], quad[7],
      0.5,0.5,
      quad[6], quad[7],
      quad[0], quad[1],
      0.5,0.5};
  };

  // mix vertexes for upload
  auto interweave = [] (std::array<float, 36> coord,
  std::array<float, 24> cover,
  std::array<float, 36> color)
  {
    std::array<float, coord.size()
    + cover.size()
    + color.size()> interweaved;
    for (uint i=0; i < coord.size()/3; i++)
    {
      std::copy(&coord[i*3], &coord[i*3]+3, &interweaved[i*8]);
      std::copy(&cover[i*2], &cover[i*2]+2, &interweaved[i*8]+3);
      std::copy(&color[i*3], &color[i*3]+3, &interweaved[i*8]+5);
    }
    return interweaved;
  };

  // cover coordinates
  std::array<float, 12> cover = {w/2.f*-1.f, -0.5f, 0.f, //LU
  w/2.f, -0.5f, 0.f, //RU
  w/2.f, -0.5f+h, 0.f, //RO
  w/2.f*-1.f, -0.5f+h, 0.f}; //LO

  std::array<float, 12> rcover = {w/2.f*-1.f, -0.5f-h, 0.f,
  w/2.f, -0.5f-h, 0.f,
  w/2.f, -0.5f, 0.f,
  w/2.f*-1.f, -0.5f, 0.f};

  //cover textures
  std::array<float, 8> cover_t = {0, 1,
  1, 1,
  1, 0,
  0, 0};

  std::array<float, 8> rcover_t = {0, 0,
  1, 0,
  1, 1,
  0, 1};

  //cover shadows
  std::array<float, 12> cover_c = {LDOWN*opacity,LDOWN*opacity,LDOWN*opacity,
  RDOWN*opacity,RDOWN*opacity,RDOWN*opacity,
  RUP*opacity,RUP*opacity,RUP*opacity,
  LUP*opacity,LUP*opacity,LUP*opacity};

  std::array<float, 12> rcover_c = {LUP*opacity*reflectionUp/2,LUP*opacity*reflectionUp/2,LUP*opacity*reflectionUp/2,
  RUP*opacity*reflectionUp/2,RUP*opacity*reflectionUp/2,RUP*opacity*reflectionUp/2,
  RDOWN*opacity/3,RDOWN*opacity/3,RDOWN*opacity/3,
  LDOWN*opacity/3,LDOWN*opacity/3,LDOWN*opacity/3};

  v_buffer->bind();
  auto pointer = v_buffer->map(QOpenGLBuffer::WriteOnly);
  if (pointer)
  {
    auto vertex1 = interweave(quad3(cover), quad2(cover_t), quadc(cover_c));
    auto vertex2 = interweave(quad3(rcover), quad2(rcover_t), quadc(rcover_c));

    std::copy(vertex1.begin(), vertex1.end(), (float*)pointer);
    std::copy(vertex2.begin(), vertex2.end(), (float*)pointer + vertex1.size());
    if (showMarks && loaded[image.index] && marks[image.index] != Unread)
    {
      std::array <float, 12> mark = {w/2.f-0.2f, -0.688f+h, 0.001f,
        w/2.f-0.05f, -0.688f+h, 0.001f,
        w/2.f-0.05f, -0.488f+h, 0.001f,
        w/2.f-0.2f, -0.488f+h, 0.001f};

      std::array <float, 8> mark_t = {0, 1,
        1, 1,
        1, 0,
        0, 0};

      std::array <float, 12> mark_c = {RUP*opacity,RUP*opacity,RUP*opacity,
        RUP*opacity,RUP*opacity,RUP*opacity,
        RUP*opacity,RUP*opacity,RUP*opacity,
        RUP*opacity,RUP*opacity,RUP*opacity};

      auto vertex3 = interweave(quad3(mark), quad2(mark_t), quadc(mark_c));
      std::copy(vertex3.begin(), vertex3.end(), (float*)pointer
        + vertex1.size()
        + vertex2.size());
    }
    v_buffer->unmap();
  }

  //set transformation matrices
  m_modelview.setToIdentity();
  m_modelview.translate(config.cfX,config.cfY,config.cfZ);
  m_modelview.rotate(config.cfRX,1,0,0);
  m_modelview.rotate(viewRotate*config.viewAngle+config.cfRY,0,1,0);
  m_modelview.rotate(config.cfRZ,0,0,1);
  m_modelview.translate(image.current.x, image.current.y, image.current.z);
  m_modelview.rotate(image.current.rot,0,1,0);

  pipeline->setUniformValue(m_modelview_location, m_modelview);

  //bind cover texture
  image.texture->bind();

  //draw cover and reflection
  glDrawArrays(GL_TRIANGLES, 0, 24);
  image.texture->release();

  //reading marks
  if (showMarks && loaded[image.index] && marks[image.index] != Unread)
  {
    if (marks[image.index] == Read)
    {
      markTexture->bind();
    }
    else
    {
      readingTexture->bind();
    }
    glDrawArrays(GL_TRIANGLES, 24, 12);
    markTexture->release();
    readingTexture->release();
  }
    v_buffer->release();
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

  //Draw right Covers
  for (int count = numObjects-1;count > -1;count--)
  {
    if (count > CS)
    {
    	drawCover(images[count]);
    }
  }

  //Draw left Covers
  for (int count = 0;count < numObjects-1;count++)
  {
    if (count < CS)
    {
      drawCover(images[count]);
    }
  }

  //Draw Center Cover
  drawCover(images[CS]);
}

void YACReaderFlowGL::showPrevious()
{
  startAnimationTimer();

  if (currentSelected > 0)
  {
    currentSelected--;
    emit centerIndexChanged(currentSelected);
    config.animationStep *= config.animationSpeedUp;

    if (config.animationStep > config.animationStepMax)
    {
      config.animationStep = config.animationStepMax;
    }

    if (viewRotateActive && viewRotate > -1)
    {
      viewRotate -= config.viewRotateAdd;
    }
    viewRotateActive = 1;
  }
}

void YACReaderFlowGL::showNext()
{
  startAnimationTimer();
  if (currentSelected < numObjects-1)
  {
    currentSelected++;
    emit centerIndexChanged(currentSelected);
    config.animationStep *= config.animationSpeedUp;

    if (config.animationStep > config.animationStepMax){
      config.animationStep = config.animationStepMax;
    }

    if (viewRotateActive && viewRotate < 1){
      viewRotate += config.viewRotateAdd;
    }
    viewRotateActive = 1;
  }
}

void YACReaderFlowGL::setCurrentIndex(int pos)
{
    if (!(pos>=0 && pos < images.length() && images.length()>0))
    {
      return;
    }
    if (pos >= images.length() && images.length() > 0)
    {
      pos = images.length()-1;
    }

    startAnimationTimer();

    currentSelected = pos;
    config.animationStep *= config.animationSpeedUp;

    if (config.animationStep > config.animationStepMax)
    {
      config.animationStep = config.animationStepMax;
    }

    if (viewRotateActive && viewRotate < 1)
    {
      viewRotate += config.viewRotateAdd;
    }
    viewRotateActive = 1;
}

void YACReaderFlowGL::updatePositions()
{
  int count;
  bool stopAnimation = true;
  for (count = numObjects-1; count > -1; count--)
  {
    calcVector(images[count].animEnd,count-currentSelected);
    if (!animate(images[count].current,images[count].animEnd))
    {
      stopAnimation = false;
    }
  }

  //slowly reset view angle
  if (!viewRotateActive)
  {
    viewRotate += (0-viewRotate)*config.viewRotateSub;
  }

  if (fabs (images[currentSelected].current.x - images[currentSelected].animEnd.x) < 1)//viewRotate < 0.2)
  {
    cleanupAnimation();
    if (updateCount >= 0) //TODO parametrizar
    {
      updateCount = 0;
      updateImageData();
    }
    else
    {
      updateCount++;
    }
  }
  else
  {
    updateCount++;
  }
    if (stopAnimation)
    {
      stopAnimationTimer();
    }
}

void YACReaderFlowGL::insert(char *name, QOpenGLTexture * texture, float x, float y,int item)
{
  startAnimationTimer();

  Q_UNUSED(name)
  //set a new entry
  if (item == -1)
  {
    images.push_back(YACReader3DImage());

    item = numObjects;
    numObjects++;

    calcVector(images[item].current,item);
    images[item].current.z = images[item].current.z-1;
  }

  images[item].texture = texture;
  images[item].width = x;
  images[item].height = y;
  images[item].index = item;
}

void YACReaderFlowGL::remove(int item)
{
  if (item < 0 || item >= images.size())
  {
    return;
  }
  startAnimationTimer();

  loaded.remove(item);
  marks.remove(item);

  //reposition current selection
  if (item <= currentSelected && currentSelected != 0)
  {
    currentSelected--;
  }

  QOpenGLTexture * texture = images[item].texture;

  int count = item;
  while (count <= numObjects-2)
  {
    images[count].index--;
    count++;
  }
  images.removeAt(item);

  if (texture != defaultTexture)
  {
    delete(texture);
  }
  numObjects--;
}

/*Info*/
YACReader3DImage YACReaderFlowGL::getCurrentSelected()
{
    return images[currentSelected];
}

void YACReaderFlowGL::replace(char *name, QOpenGLTexture * texture, float x, float y,int item)
{
  startAnimationTimer();

  Q_UNUSED(name)
  if (images[item].index == item)
  {
    images[item].texture = texture;
    images[item].width = x;
    images[item].height = y;
    loaded[item]=true;
  }
  else
  {
    loaded[item]=false;
  }
}

void YACReaderFlowGL::populate(int n)
{
  emit centerIndexChanged(0);
  float x = 1;
  float y = 1 * (700.f/480.0f);
  int i;

  for (i = 0;i<n;i++)
  {
    QString s = "cover";
    insert(s.toLocal8Bit().data(), defaultTexture, x, y);
  }
  loaded = QVector<bool>(n,false);
}

void YACReaderFlowGL::reset()
{
  makeCurrent();
  startAnimationTimer();

  currentSelected = 0;
  loaded.clear();

  for (int i = 0; i < numObjects; i++)
  {
    if (images[i].texture != defaultTexture)
    {
      delete(images[i].texture);
    }
  }

  numObjects = 0;
  images.clear();

  if(!hasBeenInitialized)
  {
    lazyPopulateObjects = -1;
  }
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
  m_projection.setToIdentity();
  m_projection.perspective(zoom, (float) width/ (float)height, 1.0, 200.0);
  pipeline->setUniformValue(m_projection_location, m_projection);
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
  config.xDistance = distance/100.0;
}

//sets the distance between the centered and the non centered covers
void YACReaderFlowGL::setCenter_Distance(int distance)
{
  startAnimationTimer();
  config.centerDistance = distance/100.0;
}

//sets the pushback amount
void YACReaderFlowGL::setZ_Distance(int distance)
{
  startAnimationTimer();
  config.zDistance = distance/100.0;
}

void YACReaderFlowGL::setCF_Y(int value)
{
  startAnimationTimer();
  config.cfY = value/100.0;
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

void YACReaderFlowGL::setPreset(const Preset & p)
{
  startAnimationTimer();
  config = p;
}

void YACReaderFlowGL::setPerformance(Performance performance)
{
  if(this->performance != performance)
  {
    startAnimationTimer();
    this->performance = performance;
    reload();
  }
}

void YACReaderFlowGL::useVSync(bool b)
{
  if(bUseVSync != b)
  {
    bUseVSync = b;
    if(b)
    {
      QSurfaceFormat f = format();
      f.setVersion(2, 1);
      f.setSwapInterval(1);
      setFormat(f);
    }
    else
    {
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

void YACReaderFlowGL::setMarkImage(QImage & image)
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

void YACReaderFlowGL::render()
{
  //do nothing
}

void YACReaderFlowGL::setFlowRightToLeft(bool b)
{
    flowRightToLeft = b;
}

void YACReaderFlowGL::wheelEvent(QWheelEvent * event)
{
    Movement m = getMovement(event);
    switch (m)
    {
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
  if ((event->key() == Qt::Key_Left && !flowRightToLeft) || (event->key() == Qt::Key_Right && flowRightToLeft))
  {
    if (event->modifiers() == Qt::ControlModifier)
    {
      setCurrentIndex((currentSelected-10<0)?0:currentSelected-10);
    }
    else
    {
      showPrevious();
    }
    event->accept();
    return;
  }

  if ((event->key() == Qt::Key_Right && !flowRightToLeft) || (event->key() == Qt::Key_Left && flowRightToLeft))
  {
    if (event->modifiers() == Qt::ControlModifier)
    {
      setCurrentIndex((currentSelected+10>=numObjects)?numObjects-1:currentSelected+10);
    }
    else
    {
      showNext();
    }
      event->accept();
      return;
  }

  if (event->key() == Qt::Key_Up)
  {
      //emit selected(centerIndex());
      return;
  }
  event->ignore();
}

void YACReaderFlowGL::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    QVector3D intersection = getPlaneIntersection(event->x(), event->y(), images[currentSelected]);
    if ((intersection.x() > 0.5 && !flowRightToLeft) || (intersection.x() < -0.5 && flowRightToLeft))
    {
      showNext();
    }
    else if ((intersection.x() < -0.5 && !flowRightToLeft) || (intersection.x() > 0.5 && flowRightToLeft))
    {
      showPrevious();
    }
  }
  else
  {
    QOpenGLWidget::mousePressEvent(event);
  }
}

void YACReaderFlowGL::mouseDoubleClickEvent(QMouseEvent* event)
{
 	QVector3D intersection = getPlaneIntersection(event->x(), event->y(), images[currentSelected]);
    if (intersection.x() < 0.5 && intersection.x() > -0.5)
    {
        emit selected(centerIndex());
        event->accept();
    }
}

QVector3D YACReaderFlowGL::getPlaneIntersection(int x, int y, YACReader3DImage plane)
{
  //get viewport (TODO: cache viewport!)
  GLint viewport[4];
  makeCurrent();
  glGetIntegerv( GL_VIEWPORT, viewport );
  doneCurrent();

  //create the picking ray
  QVector3D ray_origin(x*devicePixelRatio(), y*devicePixelRatio(), 0);
  QVector3D ray_end(x*devicePixelRatio(), y*devicePixelRatio(), 1.0);

  ray_origin = ray_origin.unproject(m_modelview, m_projection, QRect(viewport[0], viewport[1], viewport[2], viewport[3]));
  ray_end = ray_end.unproject(m_modelview, m_projection, QRect(viewport[0], viewport[1], viewport[2], viewport[3]));

  QVector3D ray_vector = ray_end - ray_origin;

  //calculate the plane vectors
  QVector3D plane_origin((plane.width/2)*-1, -0.5, 0);
  QVector3D plane_vektor_1 = QVector3D(plane.width/2, -0.5, 0) - plane_origin;
  QVector3D plane_vektor_2 = QVector3D((plane.width/2)*-1, -0.5*plane.height, 0) - plane_origin;


  //get the intersection using Cramer's rule. We only x for the line, not the plane
  double intersection_LES_determinant = ((plane_vektor_1.x()*plane_vektor_2.y()*(-1)*ray_vector.z())
    + (plane_vektor_2.x()*(-1)*ray_vector.y()*plane_vektor_1.z())
    + ((-1)*ray_vector.x()*plane_vektor_1.y()*plane_vektor_2.z())
    - ((-1)*ray_vector.x()*plane_vektor_2.y()*plane_vektor_1.z())
    - (plane_vektor_1.x()*(-1)*ray_vector.y()*plane_vektor_2.z())
    - (plane_vektor_2.x()*plane_vektor_1.y()*(-1)*ray_vector.z()));

  QVector3D det = ray_origin - plane_origin;

  double intersection_ray_determinant = ((plane_vektor_1.x()*plane_vektor_2.y()*det.z())
    + (plane_vektor_2.x()*det.y()*plane_vektor_1.z())
    + (det.x()*plane_vektor_1.y()*plane_vektor_2.z())
    - (det.x()*plane_vektor_2.y()*plane_vektor_1.z())
    - (plane_vektor_1.x()*det.y()*plane_vektor_2.z())
    - (plane_vektor_2.x()*plane_vektor_1.y()*det.z()));

//return the intersection point
  return ray_origin + ray_vector * (intersection_ray_determinant / intersection_LES_determinant);

}

YACReaderComicFlowGL::YACReaderComicFlowGL(QWidget *parent,struct Preset p )
  :YACReaderFlowGL(parent,p)
{
  worker = new ImageLoaderGL(this);
  worker->flow = this;
}

void YACReaderComicFlowGL::setImagePaths(QStringList paths)
{
  worker->reset();
  reset();
  numObjects = 0;
  if (lazyPopulateObjects!=-1 || hasBeenInitialized)
  {
    YACReaderFlowGL::populate(paths.size());
  }
  lazyPopulateObjects = paths.size();
  this->paths = paths;
}

void YACReaderComicFlowGL::updateImageData()
{
  // can't do anything, wait for the next possibility
  if (worker->busy())
  {
    return;
  }

  // set image of last one
  int idx = worker->index();
  if ( idx >= 0 && !worker->result().isNull())
  {
    if (!loaded[idx])
    {
      float x = 1;
      QImage img = worker->result();
      QOpenGLTexture * texture = new QOpenGLTexture(img);

      if (performance == high || performance == ultraHigh)
      {
        texture->setAutoMipMapGenerationEnabled(true);
        texture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::Linear);
      }
      else
      {
        texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
      }
      float y = 1 * (float(img.height())/img.width());
      QString s = "cover";
      replace(s.toLocal8Bit().data(), texture, x, y,idx);
    }
  }

  // try to load only few images on the left and right side
  // i.e. all visible ones plus some extra
  int count = 8;
  switch (performance)
  {
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
  int * indexes = new int[2*count+1];
  int center = currentSelected;
  indexes[0] = center;
  for (int j = 0; j < count; j++)
  {
    indexes[j*2+1] = center+j+1;
    indexes[j*2+2] = center-j-1;
  }
  for (int c = 0; c < 2*count+1; c++)
  {
    int i = indexes[c];
    if ((i >= 0) && (i < numObjects))
    {
      if (!loaded[i])//slide(i).isNull())
      {
        //loader->loadTexture(i);
        //loaded[i]=true;
        // schedule thumbnail generation
        if (paths.size()>0)
        {
          QString fname = paths.at(i);
          //loaded[i]=true;
          worker->generate(i, fname);
        }
          delete[] indexes;
          return;
      }
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
    {
        paths.removeAt(item);
    }
    worker->unlock();
}

void YACReaderComicFlowGL::resortCovers(QList<int> newOrder)
{
    worker->lock();
    worker->reset();//is this necesary?
    startAnimationTimer();
    QList<QString> pathsNew;
    QVector<bool> loadedNew;
    QVector<YACReaderComicReadStatus> marksNew;
    QVector<YACReader3DImage> imagesNew;

    int index = 0;
    foreach (int i, newOrder)
    {
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


YACReaderPageFlowGL::YACReaderPageFlowGL(QWidget *parent,struct Preset p )
  :YACReaderFlowGL(parent,p)
{
  worker = new ImageLoaderByteArrayGL(this);
  worker->flow = this;
}

YACReaderPageFlowGL::~YACReaderPageFlowGL()
{
  this->killTimer(timerId);
  //worker->deleteLater();
  rawImages.clear();

    //TODO: remove checking for a valid context
    //checking is needed because of this bug this bug: https://bugreports.qt.io/browse/QTBUG-60148
    if (this->context() != nullptr && this->context()->isValid())
    {
        for(int i = 0; i<numObjects; i++) {
            if (images[i].texture != defaultTexture) {
                delete(images[i].texture);
            }
        }

        if (defaultTexture != nullptr) {
            delete defaultTexture;
        }
    }

#ifdef YACREADER_LIBRARY
    delete markTexture;
    delete readingTexture;
#endif

    delete v_buffer;
    delete vao;
    delete pipeline;
}

void YACReaderPageFlowGL::updateImageData()
{
  // can't do anything, wait for the next possibility
  if (worker->busy())
  {
    return;
  }

  // set image of last one
  int idx = worker->index();
  if ( idx >= 0 && !worker->result().isNull())
  {
    if (!loaded[idx])
    {
      float x = 1;
      QImage img = worker->result();
      QOpenGLTexture * texture = new QOpenGLTexture(img);

      if (performance == high || performance == ultraHigh)
      {
          texture->setAutoMipMapGenerationEnabled(true);
          texture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::Linear);
      }
      else
      {
          texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
      }
      float y = 1 * (float(img.height())/img.width());
      QString s = "cover";
      replace(s.toLocal8Bit().data(), texture, x, y,idx);
      loaded[idx] = true;
    }
  }

  // try to load only few images on the left and right side
  // i.e. all visible ones plus some extra
  int count=8;
  switch (performance)
  {
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
  int * indexes = new int[2*count+1];
  int center = currentSelected;
  indexes[0] = center;
  for (int j = 0; j < count; j++)
  {
    indexes[j*2+1] = center+j+1;
    indexes[j*2+2] = center-j-1;
  }
  for (int c = 0; c < 2*count+1; c++)
  {
    int i = indexes[c];
    if ((i >= 0) && (i < numObjects))
    {
      if ((rawImages.size() > 0) && !loaded[i]&&imagesReady[i])//slide(i).isNull())
      {
        worker->generate(i, rawImages.at(i));
        delete[] indexes;
        return;
      }
    }
  }
  delete[] indexes;
}

void YACReaderPageFlowGL::populate(int n)
{
  worker->reset();
  if (lazyPopulateObjects!=-1 || hasBeenInitialized)
  {
    YACReaderFlowGL::populate(n);
  }
  lazyPopulateObjects = n;
  imagesReady = QVector<bool> (n,false);
  rawImages = QVector<QByteArray> (n);
  imagesSetted = QVector<bool> (n,false); //puede sobrar
}

// ImageLoader
QImage ImageLoaderGL::loadImage(const QString& fileName)
{
  QImage image;
  bool result = image.load(fileName);

  switch(flow->performance)
  {
    case low:
      image = image.scaledToWidth(200,Qt::SmoothTransformation);
      break;
    case medium:
      image = image.scaledToWidth(256,Qt::SmoothTransformation);
      break;
    case high:
      image = image.scaledToWidth(320,Qt::SmoothTransformation);
      break;
    case ultraHigh:
      break; //no scaling in ultraHigh
  }

  if(!result)
  {
    return QImage();
  }
    return image;
}

ImageLoaderGL::ImageLoaderGL(YACReaderFlowGL * flow):
QThread(),flow(flow),restart(false), working(false), idx(-1)
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

void ImageLoaderGL::generate(int index, const QString& fileName)
{
  mutex.lock();
  this->idx = index;
  this->fileName = fileName;
  this->size = size;
  this->img = QImage();
  mutex.unlock();

  if (!isRunning())
  {
    start();
  }
  else
  {
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
  for (;;)
  {
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
    {
      condition.wait(&mutex);
    }
    restart = false;
    mutex.unlock();
  }
}

QImage ImageLoaderGL::result()
{
  return img;
}

// ImageLoader
QImage ImageLoaderByteArrayGL::loadImage(const QByteArray& raw)
{
  QImage image;
  bool result = image.loadFromData(raw);

  switch (flow->performance)
  {
    case low:
      image = image.scaledToWidth(128,Qt::SmoothTransformation);
      break;
    case medium:
      image = image.scaledToWidth(196,Qt::SmoothTransformation);
      break;
    case high:
      image = image.scaledToWidth(256,Qt::SmoothTransformation);
      break;
    case ultraHigh:
      image = image.scaledToWidth(320,Qt::SmoothTransformation);
      break;
  }

  if (!result)
  {
    return QImage();
  }
    return image;
}

ImageLoaderByteArrayGL::ImageLoaderByteArrayGL(YACReaderFlowGL * flow):
QThread(),flow(flow),restart(false), working(false), idx(-1)
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

void ImageLoaderByteArrayGL::generate(int index, const QByteArray& raw)
{
  mutex.lock();
  this->idx = index;
  this->rawData = raw;
  this->size = size;
  this->img = QImage();
  mutex.unlock();

  if (!isRunning())
  {
    start();
  }
  else
  {
    // already running, wake up whenever ready
    restart = true;
    condition.wakeOne();
  }
}

void ImageLoaderByteArrayGL::run()
{
  for (;;)
  {
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
  {
    condition.wait(&mutex);
  }
    restart = false;
    mutex.unlock();
  }
}

QImage ImageLoaderByteArrayGL::result()
{
  return img;
}
