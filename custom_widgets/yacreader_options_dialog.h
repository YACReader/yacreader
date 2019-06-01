#ifndef YACREADER_OPTIONS_DIALOG_H
#define YACREADER_OPTIONS_DIALOG_H

#include <QDialog>

class YACReaderFlowConfigWidget;
#ifndef NO_OPENGL
class YACReaderGLFlowConfigWidget;
#endif
class QCheckBox;
class QPushButton;
class QSettings;
class QGroupBox;

class YACReaderOptionsDialog : public QDialog
{
    Q_OBJECT
protected:
    YACReaderFlowConfigWidget *sw;
#ifndef NO_OPENGL
    YACReaderGLFlowConfigWidget *gl;
    QCheckBox *useGL;
#endif

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
#ifndef NO_OPENGL
    virtual void savePerformance(int value);
    virtual void saveUseVSync(int b);
    virtual void saveUseGL(int b);
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
#endif
    virtual void setClassicConfigSW();
    virtual void setStripeConfigSW();
    virtual void setOverlappedStripeConfigSW();

signals:
    void optionsChanged();
    void editShortcuts();
};

#endif // YACREADER_OPTIONS_DIALOG_H
