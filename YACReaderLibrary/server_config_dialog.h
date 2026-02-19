#ifndef __SERVER_CONFIG_DIALOG_H
#define __SERVER_CONFIG_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>

#include "themable.h"

class ServerConfigDialog : public QDialog, protected Themable
{
    Q_OBJECT
public:
    ServerConfigDialog(QWidget *parent = 0);
    void showEvent(QShowEvent *event) override;

protected:
    void applyTheme(const Theme &theme) override;

private:
    QComboBox *ip;
    QLineEdit *port;

    QCheckBox *check;

    QPushButton *close;
    QPushButton *accept;
    QLabel *qrCode;
    
    // Labels for themable styling
    QLabel *titleLabel;
    QLabel *qrMessageLabel;
    QLabel *propagandaLabel;
    QLabel *ipLabel;
    QLabel *portLabel;
    QLabel *backgroundDecoration;

public slots:
    void generateQR();
    void generateQR(const QString &serverAddress);
    void regenerateQR(const QString &ip);
    void enableServer(int status);
    void updatePort();
signals:
    void portChanged(QString port);
};
#endif
