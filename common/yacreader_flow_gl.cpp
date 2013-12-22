#include "yacreader_flow_gl.h"

#include <QtGui>
#include <QtOpenGL>
//#include <math.h>

#ifdef Q_OS_MAC
	#include <OpenGL/glu.h>
#else
	#include <GL/glu.h>
#endif

#include <QGLContext>
#include <QGLPixelBuffer>
#include <cmath>

/*** Animation Settings ***/

/*** Position Configuration ***/

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
YACReaderFlowGL::YACReaderFlowGL(QWidget *parent,struct Preset p)
	:QGLWidget(QGLFormat(QGL::SampleBuffers), parent),numObjects(0),lazyPopulateObjects(-1),bUseVSync(false),hasBeenInitialized(false)
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

	QGLFormat f = format();
	f.setSwapInterval(0);
	setFormat(f);

	timerId = startTimer(16);
}

void YACReaderFlowGL::timerEvent(QTimerEvent * event)
{
	if(timerId == event->timerId())
		update();
	
	//if(!worker->isRunning())
		//worker->start();
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
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	defaultTexture = bindTexture(QImage(":/images/defaultCover.png"),GL_TEXTURE_2D,GL_RGBA,QGLContext::LinearFilteringBindOption | QGLContext::MipmapBindOption);
	markTexture = bindTexture(QImage(":/images/readRibbon.png"),GL_TEXTURE_2D,GL_RGBA,QGLContext::LinearFilteringBindOption | QGLContext::MipmapBindOption);
	readingTexture = bindTexture(QImage(":/images/readingRibbon.png"),GL_TEXTURE_2D,GL_RGBA,QGLContext::LinearFilteringBindOption | QGLContext::MipmapBindOption);
	if(lazyPopulateObjects!=-1)
		populate(lazyPopulateObjects);

	hasBeenInitialized = true;
}

void YACReaderFlowGL::paintGL()
{
	/*glClearDepth(1.0);
	glClearColor(1,1,1,1);*/
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*glLoadIdentity();
	glTranslatef(0.0, 0.0, -10.0);
	glPopMatrix();*/
	if(numObjects>0)
	{
		updatePositions();
		udpatePerspective(width(),height());
		draw();
	}
}

void YACReaderFlowGL::resizeGL(int width, int height)
{

	fontSize = (width + height) * 0.010;
	if(fontSize < 10)
		fontSize = 10;

	//int side = qMin(width, height);
	udpatePerspective(width,height);

	if(numObjects>0)
		updatePositions();
}

void YACReaderFlowGL::udpatePerspective(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(20.0, GLdouble(width) / (float)height, 1.0, 200.0);

	glMatrixMode(GL_MODELVIEW);
}

//-----------------------------------------------------------------------------
/*Private*/
void YACReaderFlowGL::calcPos(CFImage *CF,int pos)
{
	if(pos == 0){
		CF->current = centerPos;
	}else{
		if(pos > 0){
			CF->current.x = (config.centerDistance)+(config.xDistance*pos);
			CF->current.y = config.yDistance*pos*-1;
			CF->current.z = config.zDistance*pos*-1;
			CF->current.rot = config.rotation;
		}else{
			CF->current.x = (config.centerDistance)*-1+(config.xDistance*pos);
			CF->current.y =  config.yDistance*pos;
			CF->current.z = config.zDistance*pos;
			CF->current.rot = config.rotation*-1;
		}
	}

}
void YACReaderFlowGL::calcRV(RVect *RV,int pos)
{
	calcPos(&dummy,pos);

	RV->x = dummy.current.x;
	RV->y = dummy.current.y;
	RV->z = dummy.current.z;
	RV->rot = dummy.current.rot;

}
void YACReaderFlowGL::animate(RVect *Current,RVect to)
{
	//calculate and apply positions
	Current->x = Current->x+(to.x-Current->x)*config.animationStep;
	Current->y = Current->y+(to.y-Current->y)*config.animationStep;
	Current->z = Current->z+(to.z-Current->z)*config.animationStep;

	if(fabs(to.rot-Current->rot) > 0.01){
		Current->rot = Current->rot+(to.rot-Current->rot)*(config.animationStep*config.preRotation);
	}
	else
		viewRotateActive = 0;

}
void YACReaderFlowGL::drawCover(CFImage *CF)
{
	float w = CF->width;
	float h = CF->height;

	//fadeout 
	float opacity = 1-1/(config.animationFadeOutDist+config.viewRotateLightStrenght*fabs(viewRotate))*fabs(0-CF->current.x);

	glLoadIdentity();
	glTranslatef(config.cfX,config.cfY,config.cfZ);
	glRotatef(config.cfRX,1,0,0);
	glRotatef(viewRotate*config.viewAngle+config.cfRY,0,1,0);
	glRotatef(config.cfRZ,0,0,1);

	glTranslatef( CF->current.x, CF->current.y, CF->current.z );

	glPushMatrix();
	glRotatef(CF->current.rot,0,1,0);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, CF->img);

	//calculate shading
	float LShading = ((config.rotation != 0 )?((CF->current.rot < 0)?1-1/config.rotation*CF->current.rot:1):1);
	float RShading = ((config.rotation != 0 )?((CF->current.rot > 0)?1-1/(config.rotation*-1)*CF->current.rot:1):1);
	float LUP = shadingTop+(1-shadingTop)*LShading;
	float LDOWN = shadingBottom+(1-shadingBottom)*LShading;
	float RUP =  shadingTop+(1-shadingTop)*RShading;
	float RDOWN =  shadingBottom+(1-shadingBottom)*RShading;;


	//DrawCover
	glBegin(GL_QUADS);

	//esquina inferior izquierda
	glColor4f(LDOWN*opacity,LDOWN*opacity,LDOWN*opacity,1);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(w/2.f*-1.f, -0.5f, 0.f);

	//esquina inferior derecha
	glColor4f(RDOWN*opacity,RDOWN*opacity,RDOWN*opacity,1);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(w/2.f, -0.5f, 0.f);

	//esquina superior derecha
	glColor4f(RUP*opacity,RUP*opacity,RUP*opacity,1);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(w/2.f, -0.5f+h, 0.f);

	//esquina superior izquierda
	glColor4f(LUP*opacity,LUP*opacity,LUP*opacity,1);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(w/2.f*-1.f, -0.5f+h, 0.f);

	glEnd();



	//Draw reflection
	glBegin(GL_QUADS);

	//esquina inferior izquierda
	glColor4f(LUP*opacity*reflectionUp,LUP*opacity*reflectionUp,LUP*opacity*reflectionUp,opacity*reflectionUp);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(w/2.f*-1.f, -0.5f-h, 0.f);

	//esquina inferior derecha
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(w/2.f, -0.5f-h, 0.f);

	//esquina superior derecha
	glColor4f(opacity*reflectionBottom,opacity*reflectionBottom,opacity*reflectionBottom,opacity*reflectionBottom);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(w/2.f, -0.5f, 0.f);

	//esquina superior izquierda
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(w/2.f*-1.f, -0.5f, 0.f);

	glEnd();
	glDisable(GL_TEXTURE_2D);
	
	if(showMarks && loaded[CF->index] && marks[CF->index] != Unread)
	{
		glEnable(GL_TEXTURE_2D);
		if(marks[CF->index] == Read)
			glBindTexture(GL_TEXTURE_2D, markTexture);
		else
			glBindTexture(GL_TEXTURE_2D, readingTexture);
		glBegin(GL_QUADS);

		//esquina inferior izquierda
		glColor4f(RUP*opacity,RUP*opacity,RUP*opacity,1);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(w/2.f-0.2, -0.685f+h, 0.001f);

		//esquina inferior derecha
		glColor4f(RUP*opacity,RUP*opacity,RUP*opacity,1);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(w/2.f-0.05, -0.685f+h, 0.001f);

		//esquina superior derecha
		glColor4f(RUP*opacity,RUP*opacity,RUP*opacity,1);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(w/2.f-0.05, -0.485f+h, 0.001f);

		//esquina superior izquierda
		glColor4f(RUP*opacity,RUP*opacity,RUP*opacity,1);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(w/2.f-0.2, -0.485f+h, 0.001f);

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
	for(count = numObjects-1;count > -1;count--){
		if(count > CS){
			drawCover(&cfImages[count]);
		}
	}

	//Draw left Covers
	for(count = 0;count < numObjects-1;count++){
		if(count < CS){
			drawCover(&cfImages[count]);
		}
	}

	//Draw Center Cover
	drawCover(&cfImages[CS]);

	//glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-(float(width())/height())/2.0,(float(width())/height())/2.0, 0, 1, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );	

	renderText(10, fontSize + 10,QString("%1/%2").arg(currentSelected+1).arg(numObjects),QFont("Arial", fontSize));

	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void YACReaderFlowGL::showPrevious()
{
	if(currentSelected > 0){

		currentSelected--;
		emit centerIndexChanged(currentSelected);
		config.animationStep *= config.animationSpeedUp;

		if(config.animationStep > config.animationStepMax){
			config.animationStep = config.animationStepMax;
		}

		if(viewRotateActive && viewRotate > -1){
			viewRotate -= config.viewRotateAdd;
		}

		viewRotateActive = 1;

	}	
}

void YACReaderFlowGL::showNext()
{
	if(currentSelected < numObjects-1){

		currentSelected++;
		emit centerIndexChanged(currentSelected);
		config.animationStep *= config.animationSpeedUp;

		if(config.animationStep > config.animationStepMax){
			config.animationStep = config.animationStepMax;
		}

		if(viewRotateActive && viewRotate < 1){
			viewRotate += config.viewRotateAdd;
		}

		viewRotateActive = 1;
	}
}

void YACReaderFlowGL::setCurrentIndex(int pos)
{
	currentSelected = pos;

	config.animationStep *= config.animationSpeedUp;

	if(config.animationStep > config.animationStepMax){
		config.animationStep = config.animationStepMax;
	}

	if(viewRotateActive && viewRotate < 1){
		viewRotate += config.viewRotateAdd;
	}

	viewRotateActive = 1;
}

void YACReaderFlowGL::updatePositions()
{
	int count;

	for(count = numObjects-1;count > -1;count--){
		calcRV(&cfImages[count].animEnd,count-currentSelected);
		animate(&cfImages[count].current,cfImages[count].animEnd);
	}

	//slowly reset view angle
	if(!viewRotateActive){
		viewRotate += (0-viewRotate)*config.viewRotateSub;
	}

	if(fabs (cfImages[currentSelected].current.x - cfImages[currentSelected].animEnd.x) < 1)//viewRotate < 0.2)
	{
		cleanupAnimation();
		if(updateCount >= 0) //TODO parametrizar
		{

			updateCount = 0;
			updateImageData();
		}
		else
			updateCount++;
	}
	else
		updateCount++;

}

void YACReaderFlowGL::insert(char *name, GLuint Tex, float x, float y,int item)
{
	Q_UNUSED(name)
	//set a new entry 
	if(item == -1){

		if(numObjects == 0){
			cfImages = (CFImage*)malloc(sizeof(CFImage));
		}
		else
		{
			cfImages = (CFImage*)realloc(cfImages,(numObjects+1)*sizeof(CFImage));
		}

		item = numObjects;
		numObjects++;

		calcRV(&cfImages[item].current,item);
		cfImages[item].current.x += 1;
		cfImages[item].current.rot = 90;
	}

	cfImages[item].img = Tex;
	cfImages[item].width = x;
	cfImages[item].height = y;
	cfImages[item].index = item;
	//strcpy(cfImages[item].name,name);
}

void YACReaderFlowGL::remove(int item)
{
	loaded.removeAt(item);
	marks.removeAt(item);
	paths.removeAt(item);

	//reposition current selection
	if(item < currentSelected && currentSelected != 0){
		currentSelected--;
	}

	int count = item;
	while(count <= numObjects-2){
		cfImages[count] = cfImages[count+1];
		cfImages[count].index--;
		count++;
	}

	cfImages = (CFImage*)realloc(cfImages,numObjects*sizeof(CFImage));

	numObjects--;
}

/*Info*/
CFImage YACReaderFlowGL::getCurrentSelected()
{
	return cfImages[currentSelected];
}

void YACReaderFlowGL::replace(char *name, GLuint Tex, float x, float y,int item)
{
	Q_UNUSED(name)
	if(cfImages[item].index == item)
	{
		cfImages[item].img = Tex;
		cfImages[item].width = x;
		cfImages[item].height = y;
		loaded[item]=true;
	}
	else
		loaded[item]=false;
}

void YACReaderFlowGL::populate(int n)
{
	emit centerIndexChanged(0);
	float x = 1;
	float y = 1 * (700.f/480.0f);
	int i;
	
	for(i = 0;i<n;i++){
		insert("cover", defaultTexture, x, y);
	}

	/*
	for(int i = 0;i<n;i++){
		QPixmap img = QPixmap(QString("./cover%1.jpg").arg(i+1));
		GLuint cover = bindTexture(img, GL_TEXTURE_2D);
		float y = 0.5 * (float(img.height())/img.width());
		Insert("cover", cover, x, y);
	}*/

	loaded = QVector<bool>(n,false);
	//marks = QVector<bool>(n,false);

	

	//worker->start();
}

void YACReaderFlowGL::reset()
{
	currentSelected = 0;
	loaded.clear();

	for(int i = 0;i<numObjects;i++){
		if(cfImages[i].img != defaultTexture)
			deleteTexture(cfImages[i].img);
	}
	if(numObjects>0)
		delete[] cfImages;
	numObjects = 0;
	
	if(!hasBeenInitialized)
		lazyPopulateObjects = -1;
}

void YACReaderFlowGL::reload()
{
	int n = numObjects;
	reset();
	populate(n);
}

//slots
void YACReaderFlowGL::setCF_RX(int value)
{
	config.cfRX = value;
}
void YACReaderFlowGL::setCF_RY(int value)
{
	config.cfRY = value;
}
void YACReaderFlowGL::setCF_RZ(int value)
{
	config.cfRZ = value;
}

void YACReaderFlowGL::setZoom(int zoom)
{

	int width = this->width();
	int height = this->height();
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//float sideX = ((float(width)/height)/2)*1.5;
	//float sideY = 0.5*1.5;
	gluPerspective(zoom, (float)width / (float)height, 1.0, 200.0);
	//glOrtho(-sideX, sideX, -sideY+0.2, +sideY+0.2, 4, 11.0);

	glMatrixMode(GL_MODELVIEW);

}

void YACReaderFlowGL::setRotation(int angle)
{
	config.rotation = -angle;
}
//sets the distance between the covers
void YACReaderFlowGL::setX_Distance(int distance)
{
	config.xDistance = distance/100.0;
}
//sets the distance between the centered and the non centered covers
void YACReaderFlowGL::setCenter_Distance(int distance)
{
	config.centerDistance = distance/100.0;
}
//sets the pushback amount 
void YACReaderFlowGL::setZ_Distance(int distance)
{
	config.zDistance = distance/100.0;
}

void YACReaderFlowGL::setCF_Y(int value)
{
	config.cfY = value/100.0;
}

void YACReaderFlowGL::setCF_Z(int value)
{
	config.cfZ = value;
}

void YACReaderFlowGL::setY_Distance(int value)
{
	config.yDistance = value / 100.0;
}

void YACReaderFlowGL::setFadeOutDist(int value)
{
	config.animationFadeOutDist = value;
}

void YACReaderFlowGL::setLightStrenght(int value)
{
	config.viewRotateLightStrenght = value;
}

void YACReaderFlowGL::setMaxAngle(int value)
{
	config.viewAngle = value;
}

void YACReaderFlowGL::setPreset(const Preset & p)
{
	config = p;
}

void YACReaderFlowGL::setPerformance(Performance performance)
{
	if(this->performance != performance)
	{
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
			QGLFormat f = format();
			f.setSwapInterval(1);
			setFormat(f);
		}
		else
		{
			QGLFormat f = format();
			f.setSwapInterval(0);
			setFormat(f);
		}
		reset();
	}
}
void YACReaderFlowGL::setShowMarks(bool value)
{
	showMarks = value;
}
void YACReaderFlowGL::setMarks(QVector<YACReaderComicReadStatus> marks)
{
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
	marks[index] = status;
}
void YACReaderFlowGL::unmarkSlide(int index)
{
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

//EVENTOS
void YACReaderFlowGL::wheelEvent(QWheelEvent * event)
{
	if(event->delta()<0)
		showNext();
	else
		showPrevious();
	event->accept();
}

void YACReaderFlowGL::keyPressEvent(QKeyEvent *event)
{
	if(event->key() == Qt::Key_Left)
	{
		if(event->modifiers() == Qt::ControlModifier)
			setCurrentIndex((currentSelected-10<0)?0:currentSelected-10);
		else
			showPrevious();
		event->accept();
		return;
	}

	if(event->key() == Qt::Key_Right)
	{
		if(event->modifiers() == Qt::ControlModifier)
			setCurrentIndex((currentSelected+10>=numObjects)?numObjects-1:currentSelected+10);
		else
			showNext();
		event->accept();
		return;
	}

	if(event->key() == Qt::Key_Up)
	{
		emit selected(centerIndex());
		return;
	}

	event->ignore();
}

void YACReaderFlowGL::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
	{
		float x,y;
		x = event->x();
		y = event->y();
		GLint viewport[4];
		GLdouble modelview[16];
		GLdouble projection[16];
		GLfloat winX, winY, winZ;
		GLdouble posX, posY, posZ;

		glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
		glGetDoublev( GL_PROJECTION_MATRIX, projection );
		glGetIntegerv( GL_VIEWPORT, viewport );

		winX = (float)x;
		winY = (float)viewport[3] - (float)y;
		glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

		gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

		if(posX >= 0.5)
		{
			//int index = currentSelected+1;
			//while((cfImages[index].current.x-cfImages[index].width/(2.0*config.rotation)) < posX)
			//	index++;
			//setCurrentIndex(index-1);
			showNext();
		}
		else if(posX <=-0.5)
			showPrevious();
	} else
		QGLWidget::mousePressEvent(event);
}

void YACReaderFlowGL::mouseDoubleClickEvent(QMouseEvent* event)
{
		float x,y;
		x = event->x();
		y = event->y();
		GLint viewport[4];
		GLdouble modelview[16];
		GLdouble projection[16];
		GLfloat winX, winY, winZ;
		GLdouble posX, posY, posZ;

		glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
		glGetDoublev( GL_PROJECTION_MATRIX, projection );
		glGetIntegerv( GL_VIEWPORT, viewport );

		winX = (float)x;
		winY = (float)viewport[3] - (float)y;
		glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

		gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

		if(posX <= 0.5 && posX >= -0.5)
		{
			emit selected(centerIndex());
			event->accept();
		}

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
	if(lazyPopulateObjects!=-1 || hasBeenInitialized)
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
	if(worker->busy())
		return;

	// set image of last one
	int idx = worker->index();
	if( idx >= 0 && !worker->result().isNull())
	{
		if(!loaded[idx])
		{
			float x = 1;
			QImage img = worker->result();
			GLuint cover;
			if(performance == high || performance == ultraHigh)
				cover = bindTexture(img, GL_TEXTURE_2D,GL_RGB,QGLContext::LinearFilteringBindOption | QGLContext::MipmapBindOption);
			else
				cover = bindTexture(img, GL_TEXTURE_2D,GL_RGB,QGLContext::LinearFilteringBindOption);
			float y = 1 * (float(img.height())/img.width());
			replace("cover", cover, x, y,idx);
			/*CFImages[idx].width = x;
			CFImages[idx].height = y;
			CFImages[idx].img = worker->resultTexture;
			strcpy(CFImages[idx].name,"cover");*/
			//loaded[idx] = true;
			//numImagesLoaded++;
		}
	}

	// try to load only few images on the left and right side 
	// i.e. all visible ones plus some extra
	int count=8;
	switch(performance)
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
	for(int j = 0; j < count; j++)
	{
		indexes[j*2+1] = center+j+1;
		indexes[j*2+2] = center-j-1;
	}  
	for(int c = 0; c < 2*count+1; c++)
	{
		int i = indexes[c];
		if((i >= 0) && (i < numObjects))
			if(!loaded[i])//slide(i).isNull())
			{
				//loader->loadTexture(i);
				//loaded[i]=true;
				// schedule thumbnail generation
				if(paths.size()>0)
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

void YACReaderComicFlowGL::remove(int item)
{
	worker->lock();
	worker->reset();
	YACReaderFlowGL::remove(item);
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
	free(cfImages);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void YACReaderPageFlowGL::updateImageData()
{
		// can't do anything, wait for the next possibility
	if(worker->busy())
		return;

	// set image of last one
	int idx = worker->index();
	if( idx >= 0 && !worker->result().isNull())
	{
		if(!loaded[idx])
		{
			float x = 1;
			QImage img = worker->result();
			GLuint cover;
			if(performance == high || performance == ultraHigh)
				cover = bindTexture(img, GL_TEXTURE_2D,GL_RGB,QGLContext::LinearFilteringBindOption | QGLContext::MipmapBindOption);
			else
				cover = bindTexture(img, GL_TEXTURE_2D,GL_RGB,QGLContext::LinearFilteringBindOption);
			float y = 1 * (float(img.height())/img.width());
			replace("cover", cover, x, y,idx);
			/*CFImages[idx].width = x;
			CFImages[idx].height = y;
			CFImages[idx].img = worker->resultTexture;
			strcpy(CFImages[idx].name,"cover");*/
			loaded[idx] = true;
			//numImagesLoaded++;
		}
	}

	// try to load only few images on the left and right side 
	// i.e. all visible ones plus some extra
	int count=8;
	switch(performance)
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
	for(int j = 0; j < count; j++)
	{
		indexes[j*2+1] = center+j+1;
		indexes[j*2+2] = center-j-1;
	}  
	for(int c = 0; c < 2*count+1; c++)
	{
		int i = indexes[c];
		if((i >= 0) && (i < numObjects))
			if(rawImages.size()>0)
			
			if(!loaded[i]&&imagesReady[i])//slide(i).isNull())
			{
				//loader->loadTexture(i);
				//loaded[i]=true;
				// schedule thumbnail generation

					//loaded[i]=true;

				worker->generate(i, rawImages.at(i));
				
				delete[] indexes;
				return;
			}
	}
}

void YACReaderPageFlowGL::populate(int n)
{
	worker->reset();
	if(lazyPopulateObjects!=-1 || hasBeenInitialized)
		YACReaderFlowGL::populate(n);
	lazyPopulateObjects = n;
	imagesReady = QVector<bool> (n,false);
	rawImages = QVector<QByteArray> (n);
	imagesSetted = QVector<bool> (n,false); //puede sobrar
}


//-----------------------------------------------------------------------------
//ImageLoader
//-----------------------------------------------------------------------------
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

	}

	if(!result)
		return QImage();

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
		start();
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
	for(;;)
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
QImage ImageLoaderByteArrayGL::loadImage(const QByteArray& raw)
{
	QImage image;
	bool result = image.loadFromData(raw);

	switch(flow->performance)
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

	if(!result)
		return QImage();

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
		start();
	else
	{
		// already running, wake up whenever ready
		restart = true;
		condition.wakeOne();
	}
}

void ImageLoaderByteArrayGL::run()
{
	for(;;)
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
			condition.wait(&mutex);
		restart = false;
		mutex.unlock();
	}
}

QImage ImageLoaderByteArrayGL::result() 
{ 
	return img; 
}

//WidgetLoader::WidgetLoader(QWidget *parent, QGLWidget * shared)
//	:QGLWidget(parent,shared)
//{
//}
//
//void WidgetLoader::loadTexture(int index)
//{
//	QImage image;
//	bool result = image.load(QString("./cover%1.jpg").arg(index+1));
//	//image = image.scaledToWidth(128,Qt::SmoothTransformation); //TODO parametrizar
//	flow->cfImages[index].width = 0.5;
//	flow->cfImages[index].height  = 0.5 * (float(image.height())/image.width());
//	flow->cfImages[index].img = bindTexture(image, GL_TEXTURE_2D,GL_RGBA,QGLContext::LinearFilteringBindOption | QGLContext::MipmapBindOption);
//}
