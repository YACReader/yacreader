#include "render.h"
#include <cmath>
#include <QList>
#include <algorithm>
#include <QByteArray>
#include <QPixmap>

#include <typeinfo>

#define NL 2
#define NR 2

#include "yacreader_global.h"

template<class T>
inline const T& kClamp( const T& x, const T& low, const T& high )
{
    if ( x < low )       return low;
    else if ( high < x ) return high;
    else                 return x;
}

inline
int changeBrightness( int value, int brightness )
    {
    return kClamp( value + brightness * 255 / 100, 0, 255 );
    }

inline
int changeContrast( int value, int contrast )
    {
    return kClamp((( value - 127 ) * contrast / 100 ) + 127, 0, 255 );
    }

inline
int changeGamma( int value, int gamma )
    {
    return kClamp( int( pow( value / 255.0, 100.0 / gamma ) * 255 ), 0, 255 );
    }

inline
int changeUsingTable( int value, const int table[] )
    {
    return table[ value ];
    }

template< int operation( int, int ) >
static
QImage changeImage( const QImage& image, int value )
    {
    QImage im = image;
    im.detach();
    if( im.numColors() == 0 ) /* truecolor */
        {
        if( im.format() != QImage::Format_RGB32 ) /* just in case */
            im = im.convertToFormat( QImage::Format_RGB32 );
        int table[ 256 ];
        for( int i = 0;
             i < 256;
             ++i )
            table[ i ] = operation( i, value );
        if( im.hasAlphaChannel() )
            {
            for( int y = 0;
                 y < im.height();
                 ++y )
                {
                QRgb* line = reinterpret_cast< QRgb* >( im.scanLine( y ));
                for( int x = 0;
                     x < im.width();
                     ++x )
                    line[ x ] = qRgba( changeUsingTable( qRed( line[ x ] ), table ),
                        changeUsingTable( qGreen( line[ x ] ), table ),
                        changeUsingTable( qBlue( line[ x ] ), table ),
                        changeUsingTable( qAlpha( line[ x ] ), table ));
                }
            }
        else
            {
            for( int y = 0;
                 y < im.height();
                 ++y )
                {
                QRgb* line = reinterpret_cast< QRgb* >( im.scanLine( y ));
                for( int x = 0;
                     x < im.width();
                     ++x )
                    line[ x ] = qRgb( changeUsingTable( qRed( line[ x ] ), table ),
                        changeUsingTable( qGreen( line[ x ] ), table ),
                        changeUsingTable( qBlue( line[ x ] ), table ));
                }
            }
        }
    else
        {
        QVector<QRgb> colors = im.colorTable();
        for( int i = 0;
             i < im.numColors();
             ++i )
            colors[ i ] = qRgb( operation( qRed( colors[ i ] ), value ),
                operation( qGreen( colors[ i ] ), value ),
                operation( qBlue( colors[ i ] ), value ));
        }
    return im;
    }


// brightness is multiplied by 100 in order to avoid floating point numbers
QImage changeBrightness( const QImage& image, int brightness )
    {
    if( brightness == 0 ) // no change
        return image;
    return changeImage< changeBrightness >( image, brightness );
    }


// contrast is multiplied by 100 in order to avoid floating point numbers
QImage changeContrast( const QImage& image, int contrast )
    {
    if( contrast == 100 ) // no change
        return image;
    return changeImage< changeContrast >( image, contrast );
    }

// gamma is multiplied by 100 in order to avoid floating point numbers
QImage changeGamma( const QImage& image, int gamma )
    {
    if( gamma == 100 ) // no change
        return image;
    return changeImage< changeGamma >( image, gamma );
    }

QMutex mutex;

//-----------------------------------------------------------------------------
// MeanNoiseReductionFilter
//-----------------------------------------------------------------------------

MeanNoiseReductionFilter::MeanNoiseReductionFilter(enum NeighborghoodSize ns)
:neighborghoodSize(ns)
{

}

QImage MeanNoiseReductionFilter::setFilter(const QImage & image)
{
	int width = image.width();
	int height = image.height();
	QImage result(width,height,image.format());
	int filterSize = sqrt((float)neighborghoodSize);
	int bound = filterSize/2;
	QRgb pix;
	int r,g,b;
	for(int j=bound;j<height-bound;j++){
		for(int i=bound;i<width-bound;i++){
			r=g=b=0;
			for(int y=j-bound;y<=j+bound;y++)
			{
				for(int x=i-bound;x<=i+bound;x++)
				{
					pix = image.pixel(x,y);
					r += qRed(pix);
					g += qGreen(pix);
					b += qBlue(pix);
				}
			}
			result.setPixel(i,j,QColor(r/neighborghoodSize,g/neighborghoodSize,b/neighborghoodSize).rgb());
			//qDebug((QString::number(redChannel.at(4))+" "+QString::number(greenChannel.at(4))+" "+QString::number(blueChannel.at(4))).toAscii());
			//qDebug((QString::number(redChannel.size())+" "+QString::number(greenChannel.size())+" "+QString::number(blueChannel.size())).toAscii());
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
// MedianNoiseReductionFilter
//-----------------------------------------------------------------------------

MedianNoiseReductionFilter::MedianNoiseReductionFilter(enum NeighborghoodSize ns)
:neighborghoodSize(ns)
{

}

QImage MedianNoiseReductionFilter::setFilter(const QImage & image)
{
	int width = image.width();
	int height = image.height();
	QImage result(width,height,image.format());
	int filterSize = sqrt((float)neighborghoodSize);
	int bound = filterSize/2;
	QRgb pix;
	QList<int> redChannel;
	QList<int> greenChannel;
	QList<int> blueChannel;
	for(int j=bound;j<height-bound;j++){
		for(int i=bound;i<width-bound;i++){
			redChannel.clear();
			greenChannel.clear();
			blueChannel.clear();
			for(int y=j-bound;y<=j+bound;y++)
			{
				for(int x=i-bound;x<=i+bound;x++)
				{
					pix = image.pixel(x,y);
					redChannel.push_back(qRed(pix));
					greenChannel.push_back(qGreen(pix));
					blueChannel.push_back(qBlue(pix));
				}
			}

			std::sort(redChannel.begin(),redChannel.end());
			std::sort(greenChannel.begin(),greenChannel.end());
			std::sort(blueChannel.begin(),blueChannel.end());
			result.setPixel(i,j,QColor(redChannel.at(4),greenChannel.at(4),blueChannel.at(4)).rgb());
		}
	}
	return result;
}


//-----------------------------------------------------------------------------
// BrightnessFilter
//-----------------------------------------------------------------------------
BrightnessFilter::BrightnessFilter(int l)
:level(l)
{

}

QImage BrightnessFilter::setFilter(const QImage & image)
{
	/*int width = image.width();
	int height = image.height();
	QImage result(width,height,image.format());

	for(int j=0;j<height;j++){
		for(int i=0;i<width;i++){
			result.setPixel(i,j,QColor(image.pixel(i,j)).light(level).rgb());
		}
	}
	return result;*/
	QSettings settings(QCoreApplication::applicationDirPath()+"/YACReader.ini",QSettings::IniFormat);
	return changeBrightness(image,settings.value(BRIGHTNESS,0).toInt());
}

//-----------------------------------------------------------------------------
// ContrastFilter
//-----------------------------------------------------------------------------
ContrastFilter::ContrastFilter(int l)
	:level(l)
{

}

QImage ContrastFilter::setFilter(const QImage & image)
{
	/*int width = image.width();
	int height = image.height();
	QImage result(width,height,image.format());

	int min,max,v;
	min = 0;
	max = 255;
	int sum = 0;

	QVector<int> hist(256,0);

	for(int j=0;j<height;j++){
		for(int i=0;i<width;i++){
			hist[QColor(image.pixel(i,j)).lightness()]++;
			sum++;
		}
	}

	long double count = sum;
	long double new_count = 0.0;
	long double percentage,next_percentage;

	for (int i = 0; i < 254; i++)
	{
		new_count += hist[i];
		percentage = new_count/count;
		next_percentage = (new_count+hist[i+1])/count;
		if(fabs (percentage - 0.006) < fabs (next_percentage - 0.006))
		{
			min = i+1;
			break;
		}
	}

	new_count=0.0;
	for (int i = 255; i > 1; i--)
	{
		new_count += hist[i];
		percentage = new_count/count;
		next_percentage = (new_count+hist[i-1])/count;
		if(fabs (percentage - 0.006) < fabs (next_percentage - 0.006))
		{
			max = i-1;
			break;
		}
	}
	QColor c;
	int range = max - min;
	for(int j=0;j<height;j++){
		for(int i=0;i<width;i++){
			c = QColor(image.pixel(i,j));
			result.setPixel(i,j,c.light(((c.lightness()-min)/range*1.0)*255).rgb());
		}
	}

	return result;*/
	QSettings settings(QCoreApplication::applicationDirPath()+"/YACReader.ini",QSettings::IniFormat);
	return changeContrast(image,settings.value(CONTRAST,100).toInt());
}
//-----------------------------------------------------------------------------
// ContrastFilter
//-----------------------------------------------------------------------------
GammaFilter::GammaFilter(int l)
	:level(l)
{
}

QImage GammaFilter::setFilter(const QImage & image)
{
	QSettings settings(QCoreApplication::applicationDirPath()+"/YACReader.ini",QSettings::IniFormat);
	return changeGamma(image,settings.value(GAMMA,100).toInt());
}

//-----------------------------------------------------------------------------
// PageRender
//-----------------------------------------------------------------------------
PageRender::PageRender()
:QThread()
{

}
PageRender::PageRender(int np, const QByteArray & rd, QImage * p,unsigned int d, QVector<ImageFilter *> f)
:QThread(),
numPage(np),
data(rd),
page(p),
degrees(d),
filters(f)
{
}

void PageRender::run()
{
	QMutexLocker locker(&mutex);
	
	QImage img;
	img.loadFromData(data);
	if(degrees > 0)
	{
		QMatrix m;
		m.rotate(degrees);
		img = img.transformed(m,Qt::SmoothTransformation);
	}
	for(int i=0;i<filters.size();i++)
	{
		img = filters[i]->setFilter(img);
	}

	
	*page = img;

	emit pageReady(numPage);
}

//-----------------------------------------------------------------------------
// DoublePageRender
//-----------------------------------------------------------------------------

DoublePageRender::DoublePageRender(int np, const QByteArray & rd, const QByteArray & rd2, QImage * p,unsigned int d, QVector<ImageFilter *> f)
:PageRender(),
numPage(np),
data(rd),
data2(rd2),
page(p),
degrees(d),
filters(f)
{

}

void DoublePageRender::run()
{
	//QImage result;
	QMutexLocker locker(&mutex);
	QImage img, img2;
	if(!data.isEmpty())
		img.loadFromData(data);
	if(!data2.isEmpty())
		img2.loadFromData(data2);
	/*if(img.isNull())
	img = QPixmap(img2.width(),img2.height());
	if(img2.isNull())
	img2 = QPixmap(img.width(),img.height());*/

	if(img.isNull() && !img2.isNull())
	{
		img = img2;
		img2 = QImage();
	}
		

	int totalWidth,totalHeight;
	//x = img.width()+img2.width();
	totalHeight = qMax(img.height(),img2.height());

	//widths fiting the normalized height
	int width1, width2;

	//altura normalizada
	if(!img2.isNull())
	{
		if(img.height()!=totalHeight)
			totalWidth = (width1 = ((img.width() * totalHeight) / img.height())) +  (width2 = img2.width());
		else
			totalWidth = (width1 = img.width()) + (width2 = ((img2.width() * totalHeight) / img2.height()));
	}
	else
		totalWidth = width1 = img.width();


	
	
	QImage auxImg(totalWidth,totalHeight,QImage::Format_RGB32);
	QPainter painter(&auxImg);
	painter.drawImage(QRect(0,0,width1,totalHeight),img);
	if(!img2.isNull())
		painter.drawImage(QRect(width1,0,width2,totalHeight),img2);
	painter.end();

	if(degrees > 0)
	{
		QMatrix m;
		m.rotate(degrees);
		auxImg = auxImg.transformed(m,Qt::SmoothTransformation);
	}
	for(int i=0;i<filters.size();i++)
	{
		auxImg = filters[i]->setFilter(auxImg);
	}
	
	*page = auxImg;

	emit pageReady(numPage);
}

//-----------------------------------------------------------------------------
// Render
//-----------------------------------------------------------------------------

Render::Render()
:currentIndex(0),doublePage(false),comic(0),loadedComic(false),imageRotation(0),numLeftPages(NL),numRightPages(NR)
{
	int size = numLeftPages+numRightPages+1;
	currentPageBufferedIndex = numLeftPages;
	for(int i = 0; i<size; i++)
	{
		buffer.push_back(new QImage());
		pageRenders.push_back(0);
	}

	filters.push_back(new BrightnessFilter());
	filters.push_back(new ContrastFilter());
	filters.push_back(new GammaFilter());
}

//Este método se encarga de forzar el renderizado de las páginas.
//Actualiza el buffer según es necesario.
//si la pagina actual no está renderizada, se lanza un hilo que la renderize (double or single page mode) y se emite una señal que indica que se está renderizando.
void Render::render()
{
	updateBuffer();
	if(buffer[currentPageBufferedIndex]->isNull())
	{
		if(pagesReady.size()>0)
		{
			if(doublePage)
			{
				if(pagesReady[currentIndex] && pagesReady[qMin(currentIndex+1,(int)comic->numPages()-1)])
					if(currentIndex+1 > comic->numPages()-1)
						pageRenders[currentPageBufferedIndex] = new DoublePageRender(currentIndex,comic->getRawData()->at(currentIndex),QByteArray(),buffer[currentPageBufferedIndex],imageRotation,filters);
					else
						pageRenders[currentPageBufferedIndex] = new DoublePageRender(currentIndex,comic->getRawData()->at(currentIndex),comic->getRawData()->at(currentIndex+1),buffer[currentPageBufferedIndex],imageRotation,filters);
				else
					//las páginas no están listas, y se están cargando en el cómic
					emit processingPage(); //para evitar confusiones esta señal debería llamarse de otra forma
			}
			else
				if(pagesReady[currentIndex])
					pageRenders[currentPageBufferedIndex] = new PageRender(currentIndex,comic->getRawData()->at(currentIndex),buffer[currentPageBufferedIndex],imageRotation,filters);
				else
					//las páginas no están listas, y se están cargando en el cómic
					emit processingPage(); //para evitar confusiones esta señal debería llamarse de otra forma

			//si se ha creado un hilo para renderizar la página actual, se arranca
			if(pageRenders[currentPageBufferedIndex]!=0)
			{
				//se conecta la señal pageReady del hilo, con el SLOT prepareAvailablePage
				connect(pageRenders[currentPageBufferedIndex],SIGNAL(pageReady(int)),this,SLOT(prepareAvailablePage(int)));
				//se emite la señal de procesando, debido a que los hilos se arrancan aquí
				if(doublePage || filters.size()>0)
					emit processingPage();
				pageRenders[currentPageBufferedIndex]->start();
				pageRenders[currentPageBufferedIndex]->setPriority(QThread::TimeCriticalPriority);
			}
			else
				//en qué caso sería necesario hacer esto??? //TODO: IMPORTANTE, puede que no sea necesario.
				emit processingPage();
		}
		else
			//no hay ninguna página lista para ser renderizada, es necesario esperar.
			emit processingPage();
	}
	else
		// la página actual está lista
		emit currentPageReady();

	//se renderizan las páginas restantes para llenar el buffer.
	if(doublePage)
		fillBufferDoublePage();
	else
		fillBuffer();
}

QPixmap * Render::getCurrentPage()
{
	QPixmap * page = new QPixmap();
	*page = page->fromImage(*buffer[currentPageBufferedIndex]);
	return page;
}

void Render::setRotation(int degrees)
{

}

void Render::setComic(Comic * c)
{
	if(comic !=0)
	{
		comic->moveToThread(QApplication::instance()->thread());
		comic->disconnect();
		comic->deleteLater();
	}
	comic = c;
}

void Render::prepareAvailablePage(int page)
{
	if(currentIndex == page)
		emit currentPageReady();
}

void Render::update()
{
	render();
}
//-----------------------------------------------------------------------------
// Comic interface
//-----------------------------------------------------------------------------
void Render::load(const QString & path, int atPage)
{
	if(comic!=0)
	{
		comic->moveToThread(QApplication::instance()->thread());
		comic->disconnect();
		comic->deleteLater();
	}
		//comic->moveToThread(QApplication::instance()->thread());
	comic = FactoryComic::newComic(path);

	
	if(comic == NULL)//archivo no encontrado o no válido
	{
		emit errorOpening();
		reset();
		return;
	}

	previousIndex = currentIndex = 0;

	connect(comic,SIGNAL(errorOpening()),this,SIGNAL(errorOpening()));
	connect(comic,SIGNAL(errorOpening()),this,SLOT(reset()));
	connect(comic,SIGNAL(imageLoaded(int)),this,SIGNAL(imageLoaded(int)));
	connect(comic,SIGNAL(imageLoaded(int)),this,SLOT(pageRawDataReady(int)));
	connect(comic,SIGNAL(openAt(int)),this,SLOT(renderAt(int)));
	connect(comic,SIGNAL(numPages(unsigned int)),this,SIGNAL(numPages(unsigned int)));
	connect(comic,SIGNAL(numPages(unsigned int)),this,SLOT(setNumPages(unsigned int)));
	connect(comic,SIGNAL(imageLoaded(int,QByteArray)),this,SIGNAL(imageLoaded(int,QByteArray)));
	connect(comic,SIGNAL(isBookmark(bool)),this,SIGNAL(currentPageIsBookmark(bool)));
	connect(comic,SIGNAL(isBookmark(bool)),this,SLOT(pageIsBookmark(bool)));

	connect(comic,SIGNAL(bookmarksUpdated()),this,SIGNAL(bookmarksUpdated()));

	connect(comic,SIGNAL(isLast()),this,SIGNAL(isLast()));
	connect(comic,SIGNAL(isCover()),this,SIGNAL(isCover()));

	QThread * thread = NULL;

	thread = new QThread();

	comic->moveToThread(thread);

	connect(thread, SIGNAL(started()), comic, SLOT(process()));
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

	pagesReady.clear();

	comic->load(path,atPage); //garantiza que se va a intentar abrir el cómic

	if(thread != NULL)
		thread->start();

	invalidate();
	loadedComic = true;
	update();

	
}

void Render::renderAt(int page)
{
	previousIndex = currentIndex = page;
	emit pageChanged(page);
}

void Render::reset()
{
	loadedComic = false;
	invalidate();
}
//si se solicita la siguiente página, se calcula cuál debe ser en función de si se lee en modo a doble página o no.
//la página sólo se renderiza, si realmente ha cambiado.
void Render::nextPage()
{
	int nextPage; //indica cuál será la próxima página
	if(doublePage)
	{
		nextPage = currentIndex;
		if(currentIndex+2<comic->numPages())
		{
			nextPage = currentIndex+2;
			if(currentIndex != nextPage)
				comic->setIndex(nextPage);
		}
	}
	else
	{
		nextPage = comic->nextPage();
	}

	//se fuerza renderizado si la página ha cambiado
	if(currentIndex != nextPage)
	{
		previousIndex = currentIndex;
		currentIndex = nextPage;
		update();
		emit pageChanged(currentIndex);
	}
	else
		emit isLast();
}
//si se solicita la página anterior, se calcula cuál debe ser en función de si se lee en modo a doble página o no.
//la página sólo se renderiza, si realmente ha cambiado.
void Render::previousPage()
{
	int previousPage; //indica cuál será la próxima página
	if(doublePage)
	{
		if(currentIndex == 1)
			invalidate();
		previousPage = qMax(currentIndex-2,0);
		if(currentIndex != previousPage)
		{
			comic->setIndex(previousPage);
		}
	}
	else
	{
		previousPage = comic->previousPage();
	}
	
	//se fuerza renderizado si la página ha cambiado
	if(currentIndex != previousPage)
	{
		previousIndex = currentIndex;
		currentIndex = previousPage;
		update();
		emit pageChanged(currentIndex);
	}
	else
		emit isCover();
}
unsigned int Render::getIndex()
{
	return comic->getIndex();
}
unsigned int Render::numPages()
{
	return comic->numPages();
}

bool Render::hasLoadedComic()
{
	if(comic!=0)
		return comic->loaded();
	return false;
}

void Render::setNumPages(unsigned int numPages)
{
	pagesReady.fill(false,numPages);
}

void Render::pageRawDataReady(int page)
{
	pagesEmited.push_back(page);
	if(pageRenders.size()>0)
	{
		for(int i=0;i<pagesEmited.size();i++)
		{
			pagesReady[pagesEmited.at(i)] = true;
			if(pagesEmited.at(i) == currentIndex)
				update();

			if(doublePage)
			{
				if(pagesEmited.at(i)==currentIndex+1)
					update();

				if ( ((pagesEmited.at(i) < currentIndex) && (pagesEmited.at(i) > currentIndex-2*numLeftPages)) ||
					((pagesEmited.at(i) > currentIndex+1) && (pagesEmited.at(i) < currentIndex+1+2*numRightPages)) )
				{
					fillBufferDoublePage();
				}
			}
			else
			{
				if ( ((pagesEmited.at(i) < currentIndex) && (pagesEmited.at(i) > currentIndex-numLeftPages)) ||
					((pagesEmited.at(i) > currentIndex) && (pagesEmited.at(i) < currentIndex+numRightPages)) )
				{
					fillBuffer();
				}
			}
		}
		pagesEmited.clear();
	}
}

//sólo se renderiza la página, si ha habido un cambio de página
void Render::goTo(int index)
{

	if(currentIndex != index)
	{
		comic->setIndex(index);
		previousIndex = currentIndex;
		currentIndex = index;

		//si cambia la paridad de las página en modo a doble página, se rellena el buffer.
		//esto solo debería orcurrir al llegar al principio o al final
		if(doublePage && ((previousIndex - index) % 2)!=0) 
			invalidate();

		update();
		emit pageChanged(currentIndex);
	}
}

void Render::rotateRight()
{
	imageRotation = (imageRotation+90) % 360;
    reload();
}
void Render::rotateLeft()
{
	if(imageRotation == 0)
		imageRotation = 270;
	else
		imageRotation = imageRotation - 90;
	reload();
}

//Actualiza el buffer, añadiendo las imágenes (vacías) necesarias para su posterior renderizado y
//eliminado aquellas que ya no sean necesarias. También libera los hilos (no estoy seguro de que sea responsabilidad suya)
//Calcula el número de nuevas páginas que hay que buferear y si debe hacerlo por la izquierda o la derecha (según sea el sentido de la lectura)
void Render::updateBuffer()
{
	QMutexLocker locker(&mutex);
	int windowSize = currentIndex - previousIndex;
	if(doublePage)
	{	
		windowSize = windowSize/2;
		if(currentIndex == 0 && windowSize == 0 && previousIndex == 1)
			windowSize = -1;

	}
	if(windowSize > 0)//add pages to right pages and remove on the left
	{
		windowSize = qMin(windowSize,buffer.size());
		for(int i = 0; i < windowSize; i++)
		{
			//renders
			PageRender * pr = pageRenders.front();
			pageRenders.pop_front();
			if(pr !=0)
			{
				if(pr->wait())
					delete pr;
			}
			pageRenders.push_back(0);

			//images
			
			if(buffer.front()!=0)
				delete buffer.front();
			buffer.pop_front();
			buffer.push_back(new QImage());
		}
	}
	else //add pages to left pages and remove on the right
		if(windowSize<0)
		{
			windowSize = -windowSize;
			windowSize = qMin(windowSize,buffer.size());
			for(int i = 0; i < windowSize; i++)
			{
				//renders
				PageRender * pr = pageRenders.back();
				pageRenders.pop_back();
				if(pr !=0)
				{
					if(pr->wait())
						delete pr;
				}
				pageRenders.push_front(0);

				//images
				buffer.push_front(new QImage());
				QImage * p = buffer.back();
				if(p!=0)
					delete p;
				buffer.pop_back();
			}
		}
		previousIndex = currentIndex;
}

void Render::fillBuffer()
{
	for(int i = 1; i <= qMax(numLeftPages,numRightPages); i++)
	{
		if ((currentIndex+i < comic->numPages()) && 
			buffer[currentPageBufferedIndex+i]->isNull() && 
			i <= numRightPages &&
			pageRenders[currentPageBufferedIndex+i]==0 &&
			pagesReady[currentIndex+1]) //preload next pages
		{
			pageRenders[currentPageBufferedIndex+i] = new PageRender(currentIndex+i,comic->getRawData()->at(currentIndex+i),buffer[currentPageBufferedIndex+i],imageRotation,filters);
			connect(pageRenders[currentPageBufferedIndex],SIGNAL(pageReady(int)),this,SLOT(prepareAvailablePage(int)));
			pageRenders[currentPageBufferedIndex+i]->start();
		}

		if ((currentIndex-i > 0) && 
			buffer[currentPageBufferedIndex-i]->isNull() && 
			i <= numLeftPages &&
			pageRenders[currentPageBufferedIndex-i]==0 &&
			pagesReady[currentIndex-1]) //preload previous pages
		{
			pageRenders[currentPageBufferedIndex-i] = new PageRender(currentIndex-i,comic->getRawData()->at(currentIndex-i),buffer[currentPageBufferedIndex-i],imageRotation,filters);
			connect(pageRenders[currentPageBufferedIndex],SIGNAL(pageReady(int)),this,SLOT(prepareAvailablePage(int)));
			pageRenders[currentPageBufferedIndex-i]->start();
		}
	}
}

void Render::fillBufferDoublePage()
{
	for(int i = 1; i <= qMax(numLeftPages,numRightPages); i++)
	{
		if ((currentIndex+2*i < comic->numPages()) && 
			buffer[currentPageBufferedIndex+i]->isNull() && 
			i <= numRightPages &&
			pageRenders[currentPageBufferedIndex+i]==0 &&
			(pagesReady[currentIndex+2*i] && pagesReady[qMin(currentIndex+(2*i)+1,(int)comic->numPages()-1)])) //preload next pages
		{
			if(currentIndex+(2*i)+1 > comic->numPages()-1)
				pageRenders[currentPageBufferedIndex+i] = new DoublePageRender(currentIndex+2*i,comic->getRawData()->at(currentIndex+(2*i)),QByteArray(),buffer[currentPageBufferedIndex+i],imageRotation,filters);
			else
				pageRenders[currentPageBufferedIndex+i] = new DoublePageRender(currentIndex+2*i,comic->getRawData()->at(currentIndex+(2*i)),comic->getRawData()->at(currentIndex+(2*i)+1),buffer[currentPageBufferedIndex+i],imageRotation,filters);
			connect(pageRenders[currentPageBufferedIndex],SIGNAL(pageReady(int)),this,SLOT(prepareAvailablePage(int)));
			pageRenders[currentPageBufferedIndex+i]->start();
		}

		if ((currentIndex-2*i >= -1) && 
			buffer[currentPageBufferedIndex-i]->isNull() && 
			i <= numLeftPages &&
			pageRenders[currentPageBufferedIndex-i]==0 &&
			(pagesReady[qMax(currentIndex-2*i,0)] && pagesReady[qMin(currentIndex-(2*i)+1,(int)comic->numPages()-1)])) //preload previous pages
		{
			if(currentIndex-2*i == -1)
				pageRenders[currentPageBufferedIndex-i] = new DoublePageRender(0,QByteArray(),comic->getRawData()->at(0),buffer[currentPageBufferedIndex-i],imageRotation,filters);
			else
				pageRenders[currentPageBufferedIndex-i] = new DoublePageRender(currentIndex-2*i,comic->getRawData()->at(currentIndex-(2*i)),comic->getRawData()->at(currentIndex-(2*i)+1),buffer[currentPageBufferedIndex-i],imageRotation,filters);
			connect(pageRenders[currentPageBufferedIndex],SIGNAL(pageReady(int)),this,SLOT(prepareAvailablePage(int)));
			pageRenders[currentPageBufferedIndex-i]->start();
		}
	}
}

//Método que debe ser llamado cada vez que la estructura del buffer se vuelve inconsistente con el modo de lectura actual.
//se terminan todos los hilos en ejecución y se libera la memoria (de hilos e imágenes)
void Render::invalidate()
{
	for(int i=0;i<pageRenders.size();i++)
	{
		if(pageRenders[i]!=0)
		{
			pageRenders[i]->wait();
			delete pageRenders[i];
			pageRenders[i] = 0;
		}
	}

	for(int i=0;i<buffer.size();i++)
	{
		delete buffer[i];
		buffer[i] = new QImage();
	}
}

void Render::doublePageSwitch()
{
	doublePage = !doublePage;
	if(comic)
	{
		invalidate();
		update();
	}
}

QString Render::getCurrentPagesInformation()
{
	QString s = QString::number(currentIndex+1);
	if (doublePage && (currentIndex+1 < comic->numPages()))
		s += "-"+QString::number(currentIndex+2);
	s += "/"+QString::number(comic->numPages());
	return s;
}

void Render::setBookmark()
{
	comic->setBookmark();
}

void Render::removeBookmark()
{
	comic->removeBookmark();
}

void Render::save()
{
	comic->saveBookmarks();
}

Bookmarks * Render::getBookmarks()
{
	return comic->bm;
}

void Render::reload()
{
	if(comic)
	{
		invalidate();
		update();
	}
}