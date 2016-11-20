#ifndef __SERVER_CONFIG_DIALOG_H
#define __SERVER_CONFIG_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPixmap>
#include <QProcess>
#include <QPixmap>
#include <QComboBox>
#include <QCheckBox>

class ServerConfigDialog : public QDialog
{
Q_OBJECT
	public:
		ServerConfigDialog(QWidget * parent = 0);
	private:
		QComboBox * ip;
		QLineEdit * port;
		
		QCheckBox * check;

		QPushButton * close;
		QPushButton * accept;
		QLabel * qrCode;
		QPixmap * qrCodeImage;

		QProcess * qrGenerator;

	public slots:
		void generateQR();
		void generateQR(const QString & serverAddress);
		void regenerateQR(const QString & ip);
		void updateImage();
		void enableServer(int status);
		void updatePort();
signals:
		void portChanged(QString port);

};


#endif
