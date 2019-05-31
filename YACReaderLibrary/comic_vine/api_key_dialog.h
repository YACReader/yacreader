#ifndef API_KEY_DIALOG_H
#define API_KEY_DIALOG_H

#include <QDialog>

class QPushButton;
class QLineEdit;
class QSettings;

class ApiKeyDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ApiKeyDialog(QWidget *parent = nullptr);
    ~ApiKeyDialog() override;
signals:

public slots:

protected slots:
    void enableAccept(const QString &text);
    void saveApiKey();

protected:
    QPushButton *acceptButton;
    QPushButton *cancelButton;
    QLineEdit *edit;
    QSettings *settings;
};

#endif // API_KEY_DIALOG_H
