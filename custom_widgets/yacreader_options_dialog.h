#ifndef YACREADER_OPTIONS_DIALOG_H
#define YACREADER_OPTIONS_DIALOG_H

#include <QDialog>

class YACReader3DFlowConfigWidget;
class QCheckBox;
class QPushButton;
class QSettings;
class QGroupBox;

class YACReaderOptionsDialog : public QDialog
{
    Q_OBJECT
protected:
    YACReader3DFlowConfigWidget *gl;

    QPushButton *accept;
    QPushButton *cancel;

    QGroupBox *shortcutsBox;

    QSettings *settings;
    QSettings *previousSettings;

public:
    YACReaderOptionsDialog(QWidget *parent);
public slots:
    virtual void restoreOptions(QSettings *settings);
    virtual void restoreOptions();
    virtual void saveOptions();
protected slots:
    virtual void savePerformance(int value);
    virtual void saveUseVSync(int b);
    virtual void saveXRotation(int value);
    virtual void saveYPosition(int value);
    virtual void saveCoverDistance(int value);
    virtual void saveCentralDistance(int value);
    virtual void saveZoomLevel(int value);
    virtual void saveYCoverOffset(int value);
    virtual void saveZCoverOffset(int value);
    virtual void saveCoverRotation(int value);
    virtual void saveFadeOutDist(int value);
    virtual void saveLightStrength(int value);
    virtual void saveMaxAngle(int value);
    virtual void loadConfig();
    virtual void setClassicConfig();
    virtual void setStripeConfig();
    virtual void setOverlappedStripeConfig();
    virtual void setModernConfig();
    virtual void setRouletteConfig();
    virtual void saveFlowParameters();

signals:
    void optionsChanged();
    void editShortcuts();
};

#endif // YACREADER_OPTIONS_DIALOG_H
