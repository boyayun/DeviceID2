#ifndef FIRSTINPUTDIALOG_H
#define FIRSTINPUTDIALOG_H

#include <QDialog>
#include <QValidator>
#include <QDesktopWidget>
#include <QUrl>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>

#define REGEXP_STRING "^(([0-9a-fA-F]{35})|((http|https)://www.[a-zA-Z1-9-~.]*.(com|cn|net|cc)/download[?]{1}(type=[0-9]{3})*&deviceid=[0-9a-fA-F]{35}))"
#define REG_DEVICEID "^[0-9a-fA-f]{35}"
#define REGEXP_STRING2 "[0-9a-fA-F]{56}"

namespace Ui {
class FirstInputDialog;
}

class FirstInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FirstInputDialog(QWidget *parent = 0);
    ~FirstInputDialog();
    void keyPressEvent(QKeyEvent *);

    int getIsParametterSetted() const;
    void setIsParametterSetted(int value);

    bool isInpurStringValid(QString s);

    QString getInputString() const;

    int getInputNumber() const;

    QString getKeyFilePath() const;
    void setKeyFilePath(const QString &value);

    QMap<QString, QString> *getIdKeyMap() const;

    QString getUrl() const;
    void setUrl(const QString &value);

private slots:
    void on_okButton_clicked();

    void on_inputStringLineEdit_textChanged(const QString &arg1);


    void on_numberlineEdit_textChanged(const QString &arg1);

    void slotTextEdited(QString);
    void slotEditFinished();

    void on_selectkeyfileButton_clicked();

private:
    Ui::FirstInputDialog *ui;
    int isParametterSetted;
    QString  inputString;
    QString keyFilePath;
    QString url;
    int inputNumber;
    bool isInputStringValid;
    bool isInputNumberValid;
    bool isNeedSelectAll;
    bool isKeyFileLoaded;
    QMap <QString,QString> *IdKeyMap;
};

#endif // FIRSTINPUTDIALOG_H
