#ifndef NO_LIBRARIES_WIDGET_H
#define NO_LIBRARIES_WIDGET_H

#include "themable.h"

#include <QWidget>

class QLabel;

class NoLibrariesWidget : public QWidget, protected Themable
{
    Q_OBJECT
public:
    explicit NoLibrariesWidget(QWidget *parent = nullptr);

signals:
    void createNewLibrary();
    void addExistingLibrary();
public slots:

protected:
    void applyTheme(const Theme &theme) override;

private:
    QLabel *iconLabel;
    QLabel *text;
    QLabel *textDescription;
};

#endif // NO_LIBRARIES_WIDGET_H
