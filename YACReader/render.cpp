#include "render.h"
#include <cmath>
#include <QList>
#include <algorithm>
#include <QByteArray>
#include <QPixmap>

#define NL 2
#define NR 2

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
	int width = image.width();
	int height = image.height();
	QImage result(width,height,image.format());

	for(int j=0;j<height;j++){
		for(int i=0;i<width;i++){
			result.setPixel(i,j,QColor(image.pixel(i,j)).light(level).rgb());
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
// ContrastFilter
//-----------------------------------------------------------------------------
ContrastFilter::ContrastFilter()
{

}

QImage ContrastFilter::setFilter(const QImage & image)
{
	int width = image.width();
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

	return result;
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
	//QMutexLocker locker(&mutex);
	
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
	//QMutexLocker locker(&mutex);
	QImage img, img2;
	if(!data.isEmpty())
		img.loadFromData(data);
	if(!data2.isEmpty())
		img2.loadFromData(data2);
	/*if(img.isNull())
	img = QPixmap(img2.width(),img2.height());
	if(img2.isNull())
	img2 = QPixmap(img.width(),img.height());*/

	int x,y;
	x = img.width()+img2.width();
	y = qMax(img.height(),img2.height());
	
	
	QImage auxImg(x,y,QImage::Format_RGB32);
	QPainter painter(&auxImg);
	painter.drawImage(0,0,img);
	painter.drawImage(img.width(),0,img2);
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

	//filters.push_back(new ContrastFilter());
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
		delete comic;
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
void Render::load(const QString & path)
{
	if(comic!=0)
		delete comic;
	comic = new Comic();
	previousIndex = currentIndex = 0;

	connect(comic,SIGNAL(errorOpening()),this,SIGNAL(errorOpening()));
	connect(comic,SIGNAL(errorOpening()),this,SLOT(reset()));
	connect(comic,SIGNAL(imageLoaded(int)),this,SIGNAL(imageLoaded(int)));
	connect(comic,SIGNAL(imageLoaded(int)),this,SLOT(pageRawDataReady(int)));
	//connect(comic,SIGNAL(pageChanged(int)),this,SIGNAL(pageChanged(int)));
	connect(comic,SIGNAL(numPages(unsigned int)),this,SIGNAL(numPages(unsigned int)));
	connect(comic,SIGNAL(numPages(unsigned int)),this,SLOT(setNumPages(unsigned int)));
	connect(comic,SIGNAL(imageLoaded(int,QByteArray)),this,SIGNAL(imageLoaded(int,QByteArray)));
	connect(comic,SIGNAL(isBookmark(bool)),this,SIGNAL(currentPageIsBookmark(bool)));
	connect(comic,SIGNAL(bookmarksLoaded(const Bookmarks &)),this,SIGNAL(bookmarksLoaded(const Bookmarks &)));
	pagesReady.clear();
	if(comic->load(path)) //garantiza que se va a intentar abrir el cómic
	{
		invalidate();
		loadedComic = true;
		update();
	}
	
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
	}
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
	}
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
	}
}

void Render::rotateRight()
{
	imageRotation = (imageRotation+90) % 360;
	invalidate();
	update();
}
void Render::rotateLeft()
{
	if(imageRotation == 0)
		imageRotation = 270;
	else
		imageRotation = imageRotation - 90;
	invalidate();
	update();
}

//Actualiza el buffer, añadiendo las imágenes (vacías) necesarias para su posterior renderizado y
//eliminado aquellas que ya no sean necesarias. También libera los hilos (no estoy seguro de que sea responsabilidad suya)
//Calcula el número de nuevas páginas que hay que buferear y si debe hacerlo por la izquierda o la derecha (según sea el sentido de la lectura)
void Render::updateBuffer()
{
	//QMutexLocker locker(&mutex);
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
			pageRenders[currentPageBufferedIndex+i] = new PageRender(currentIndex+i,comic->getRawData()->at(currentIndex+i),buffer[currentPageBufferedIndex+i],imageRotation);
			connect(pageRenders[currentPageBufferedIndex],SIGNAL(pageReady(int)),this,SLOT(prepareAvailablePage(int)));
			pageRenders[currentPageBufferedIndex+i]->start();
		}

		if ((currentIndex-i > 0) && 
			buffer[currentPageBufferedIndex-i]->isNull() && 
			i <= numLeftPages &&
			pageRenders[currentPageBufferedIndex-i]==0 &&
			pagesReady[currentIndex-1]) //preload previous pages
		{
			pageRenders[currentPageBufferedIndex-i] = new PageRender(currentIndex-i,comic->getRawData()->at(currentIndex-i),buffer[currentPageBufferedIndex-i],imageRotation);
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
				pageRenders[currentPageBufferedIndex+i] = new DoublePageRender(currentIndex+2*i,comic->getRawData()->at(currentIndex+(2*i)),QByteArray(),buffer[currentPageBufferedIndex+i],imageRotation);
			else
				pageRenders[currentPageBufferedIndex+i] = new DoublePageRender(currentIndex+2*i,comic->getRawData()->at(currentIndex+(2*i)),comic->getRawData()->at(currentIndex+(2*i)+1),buffer[currentPageBufferedIndex+i],imageRotation);
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
				pageRenders[currentPageBufferedIndex-i] = new DoublePageRender(0,QByteArray(),comic->getRawData()->at(0),buffer[currentPageBufferedIndex-i],imageRotation);
			else
				pageRenders[currentPageBufferedIndex-i] = new DoublePageRender(currentIndex-2*i,comic->getRawData()->at(currentIndex-(2*i)),comic->getRawData()->at(currentIndex-(2*i)+1),buffer[currentPageBufferedIndex-i],imageRotation);
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
			pageRenders[i]->terminate();
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
