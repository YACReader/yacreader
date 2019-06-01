#ifndef NO_LIBRARIES_WIDGET_H
#define NO_LIBRARIES_WIDGET_H

#include <QWidget>

class NoLibrariesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NoLibrariesWidget(QWidget *parent = 0);

signals:
    void createNewLibrary();
    void addExistingLibrary();
public slots:
};

#endif // NO_LIBRARIES_WIDGET_H
