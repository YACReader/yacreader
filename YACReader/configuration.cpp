#include "configuration.h"

#include <QFile>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QStringList>
#include <QMessageBox>

Configuration::Configuration()
{
	//read configuration
	//load("/YACReader.conf");
}

/*Configuration::Configuration(const Configuration & conf)
{
	//nothing
}*/

void Configuration::load(QSettings * settings)
{
	this->settings = settings;

	//TODO set defaults
	if(!settings->contains(PATH))
		settings->setValue(PATH,".");
	if(!settings->contains(GO_TO_FLOW_SIZE))
		settings->setValue(GO_TO_FLOW_SIZE,QSize(126,200));
	if(!settings->contains(MAG_GLASS_SIZE))
		settings->setValue(MAG_GLASS_SIZE,QSize(350,175));
	if(!settings->contains(ZOOM_LEVEL))
		settings->setValue(MAG_GLASS_SIZE,QSize(350,175));
	if(!settings->contains(FIT))
		settings->setValue(FIT,false);
	if(!settings->contains(FLOW_TYPE))
		settings->setValue(FLOW_TYPE,0);
	if(!settings->contains(FULLSCREEN))
		settings->setValue(FULLSCREEN,false);
	if(!settings->contains(FIT_TO_WIDTH_RATIO))
		settings->setValue(FIT_TO_WIDTH_RATIO,1);
	if(!settings->contains(Y_WINDOW_SIZE))
		settings->setValue(Y_WINDOW_SIZE,QSize(0,0));
	if(!settings->contains(MAXIMIZED))
		settings->setValue(MAXIMIZED,false);
	if(!settings->contains(DOUBLE_PAGE))
		settings->setValue(DOUBLE_PAGE,false);
	if(!settings->contains(ADJUST_TO_FULL_SIZE))
		settings->setValue(ADJUST_TO_FULL_SIZE,false);
	if(!settings->contains(BACKGROUND_COLOR))
		settings->setValue(BACKGROUND_COLOR,QColor(40,40,40));
	if(!settings->contains(ALWAYS_ON_TOP))
		settings->setValue(ALWAYS_ON_TOP,false);
	if(!settings->contains(SHOW_TOOLBARS))
		settings->setValue(SHOW_TOOLBARS, true);
}

void Configuration::load(const QString & path)
{
	//load default configuration
	defaultPath = ".";
	magnifyingGlassSize = QSize(350,175);
	gotoSlideSize = QSize(126,200); //normal
	//gotoSlideSize = QSize(79,125); //small
	//gotoSlideSize = QSize(173,275); //big
	//gotoSlideSize = QSize(220,350); //huge
	zoomLevel = 0.5;
	adjustToWidth = true;
	flowType = Strip;
	fullScreen = false;
	fitToWidthRatio = 1;
	windowSize = QSize(0,0);
	maximized = false;
	doublePage = false;
	adjustToFullSize = false;
	backgroundColor = QColor(40,40,40);
	alwaysOnTop = false;

	//load from file
	QFile f(QCoreApplication::applicationDirPath()+path);
	f.open(QIODevice::ReadOnly);
	QTextStream txtS(&f);
	QString content = txtS.readAll();
	QStringList lines = content.split('\n');
	QString line,name;
	int i=0;
	foreach(line,lines)
	{
		if((i%2)==0) 
		{
			name = line.trimmed();
		}
		else
		{
			if(name==PATH)
				defaultPath = line.trimmed();
			else
				if(name==MAG_GLASS_SIZE)
				{
					QStringList values = line.split(',');
					magnifyingGlassSize = QSize(values[0].toInt(),values[1].toInt());
				}
				else
					if(name==ZOOM_LEVEL)
						zoomLevel = line.toFloat();
					else
						if(name==SLIDE_SIZE)
						{
							int height = line.toInt();
							gotoSlideSize = QSize(static_cast<int>(height/SLIDE_ASPECT_RATIO),height);
						}
						else
							if(name==FIT)
								adjustToWidth = line.toInt();
							else
								if(name==FLOW_TYPE)
									flowType = (FlowType)line.toInt();
								else
									if(name==FULLSCREEN)
										fullScreen = line.toInt();
									else
										if(name==FIT_TO_WIDTH_RATIO)
											fitToWidthRatio = line.toFloat();
										else
											if(name==Y_WINDOW_POS)
											{
												QStringList l = line.split(',');
												windowPos = QPoint(l[0].toInt(),l[1].toInt());
											}
											else
												if(name==Y_WINDOW_SIZE)
												{
													QStringList l = line.split(',');
													windowSize = QSize(l[0].toInt(),l[1].toInt());
												}
												else
													if(name==MAXIMIZED)
														maximized = line.toInt();
													else
														if(name==DOUBLE_PAGE)
															doublePage = line.toInt();
														else
															if(name==ADJUST_TO_FULL_SIZE)
																adjustToFullSize = line.toInt();
															else
																if(name==BACKGROUND_COLOR)
																{
																	QStringList l = line.split(',');
																	backgroundColor = QColor(l[0].toInt(),l[1].toInt(),l[2].toInt());
																}
																else
																	if(name==ALWAYS_ON_TOP)
																		alwaysOnTop = line.toInt();
													


		}
		i++;
	}
}

void Configuration::save()
{
	QFile f(QCoreApplication::applicationDirPath()+"/YACReader.conf");
	if(!f.open(QIODevice::WriteOnly))
	{
		QMessageBox::critical(NULL,tr("Saving config file...."),tr("There was a problem saving YACReader configuration. Please, check if you have enough permissions in the YACReader root folder."));
	}
	else
	{
	QTextStream txtS(&f);

	txtS << PATH << "\n";
	txtS << defaultPath << "\n";

	txtS << MAG_GLASS_SIZE << "\n";
	txtS << magnifyingGlassSize.width() <<","<< magnifyingGlassSize.height() << "\n";

	txtS << ZOOM_LEVEL << "\n";
	txtS << zoomLevel << "\n";

	txtS << SLIDE_SIZE << "\n";
	txtS << gotoSlideSize.height() << "\n";

	txtS << FIT << "\n";
	txtS << (int)adjustToWidth << "\n";

	txtS << FLOW_TYPE << "\n";
	txtS << (int)flowType << "\n";

	txtS << FULLSCREEN << "\n";
	txtS << (int)fullScreen << "\n";

	txtS << FIT_TO_WIDTH_RATIO << "\n";
	txtS << fitToWidthRatio << "\n";

	txtS << Y_WINDOW_POS << "\n";
	txtS << windowPos.x() << "," << windowPos.y() << "\n";

	txtS << Y_WINDOW_SIZE << "\n";
	txtS << windowSize.width() << "," << windowSize.height() << "\n";

	txtS << MAXIMIZED << "\n";
	txtS << (int)maximized << "\n";

	txtS << DOUBLE_PAGE << "\n";
	txtS << (int)doublePage << "\n";

	txtS << ADJUST_TO_FULL_SIZE << "\n";
	txtS << (int) adjustToFullSize << "\n";

	txtS << BACKGROUND_COLOR << "\n";
	txtS << backgroundColor.red() << "," << backgroundColor.green() << ","  << backgroundColor.blue() << "\n";

	txtS << ALWAYS_ON_TOP << "\n";
	txtS << (int)alwaysOnTop << "\n";
	}
}
