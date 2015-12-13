#ifndef CONSOLE_UI_LIBRARY_CREATOR_H
#define CONSOLE_UI_LIBRARY_CREATOR_H

#include <QtCore>

class ConsoleUILibraryCreator : public QObject
{
    Q_OBJECT
public:
    explicit ConsoleUILibraryCreator(QObject *parent = 0);
    void createLibrary(const QString & name, const QString & path);
    void updateLibrary(const QString & path);

private:
    uint numComicsProcessed;
signals:

public slots:

protected slots:
    void newComic(const QString & relativeComicPath, const QString & coverPath);
    void manageCreatingError(const QString & error);
    void done();
};

#endif // CONSOLE_UI_LIBRARY_CREATOR_H
