#ifndef __SERVER_CONFIG_DIALOG_H
#define __SERVER_CONFIG_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPixmap>
#include <QComboBox>
#include <QCheckBox>
#include <QLibrary>

class ServerConfigDialog : public QDialog
{
Q_OBJECT
	public:
		ServerConfigDialog(QWidget * parent = 0);
	private:
		QComboBox * ip;
		QLineEdit * port;

		QCheckBox * check;
    QCheckBox * performanceWorkaroundCheck;

		QPushButton * close;
		QPushButton * accept;
		QLabel * qrCode;

	public slots:
		void generateQR();
		void generateQR(const QString & serverAddress);
		void regenerateQR(const QString & ip);
		void enableServer(int status);
    void enableperformanceWorkaround(int status);
		void updatePort();
signals:
		void portChanged(QString port);

};

class QrEncoder
{
	public:
		QrEncoder();
		QBitmap encode(const QString & string);
	private:
		/*libqrencode data structures*/
		typedef struct {
			 int version;         ///< version of the symbol
			 int width;           ///< width of the symbol
			 unsigned char *data; ///< symbol data
		} QRcode;
		typedef QRcode* (*_QRcode_encodeString8bit)(char [], int, int);
		typedef void (*_QRcode_free)(QRcode*);
		_QRcode_free QRcode_free;
		_QRcode_encodeString8bit QRcode_encodeString8bit;
};

#endif
