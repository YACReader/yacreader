#ifndef CONSOLE_UI_LIBRARY_CREATOR_H
#define CONSOLE_UI_LIBRARY_CREATOR_H

#include <QtCore>

class ConsoleUILibraryCreator : public QObject
{
    Q_OBJECT
public:
    explicit ConsoleUILibraryCreator(QSettings *settings, QObject *parent = 0);
    bool createLibrary(const QString &name, const QString &path, bool recoverLock = false);
    bool updateLibrary(const QString &path, bool recoverLock = false);
    void addExistingLibrary(const QString &name, const QString &path);
    void removeLibrary(const QString &name);
    bool rescanXMLInfoLibrary(const QString &path, bool recoverLock = false);
    int repairLibrary(const QString &path, bool recoverLock = false);

private:
    uint numComicsProcessed;
    QSettings *settings;
    bool operationFailed { false };
signals:

public slots:

protected slots:
    void newComic(const QString &relativeComicPath, const QString &coverPath);
    void manageCreatingError(const QString &error);
    void manageUpdatingError(const QString &error);
    void done();
};

#endif // CONSOLE_UI_LIBRARY_CREATOR_H
