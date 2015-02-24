#include "configuration.h"

#include <QFile>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QStringList>
#include <QMessageBox>

#include "yacreader_global.h"

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
	//if(!settings->contains(FIT))
	//	settings->setValue(FIT,false);
	if(!settings->contains(FLOW_TYPE))
		settings->setValue(FLOW_TYPE,0);
	if(!settings->contains(FULLSCREEN))
		settings->setValue(FULLSCREEN,false);
	
//TODO: Replace this with zoom mode
	if(!settings->contains(FIT_TO_WIDTH_RATIO))
		settings->setValue(FIT_TO_WIDTH_RATIO,1);
	
	if(!settings->contains(Y_WINDOW_SIZE))
		settings->setValue(Y_WINDOW_SIZE,QSize(0,0));
	if(!settings->contains(MAXIMIZED))
		settings->setValue(MAXIMIZED,false);
	if(!settings->contains(DOUBLE_PAGE))
		settings->setValue(DOUBLE_PAGE,false);
	//if(!settings->contains(ADJUST_TO_FULL_SIZE))
	//	settings->setValue(ADJUST_TO_FULL_SIZE,false);
	if(!settings->contains(BACKGROUND_COLOR))
		settings->setValue(BACKGROUND_COLOR,QColor(40,40,40));
	if(!settings->contains(ALWAYS_ON_TOP))
		settings->setValue(ALWAYS_ON_TOP,false);
	if(!settings->contains(SHOW_TOOLBARS))
		settings->setValue(SHOW_TOOLBARS, true);
	if(!settings->contains(FITMODE))
		settings->setValue(FITMODE, "full_page");
}