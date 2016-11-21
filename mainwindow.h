#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QDebug>
#include <QTimer>
#include <QList>
#include <QDesktopWidget>
#include <QMessageBox>

#include "displayitem.h"
#include "firstinputdialog.h"



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void keyPressEvent(QKeyEvent *keyEvent);

private:
    Ui::MainWindow *ui;
    QTimer inputTimer;

    bool isNeedClearInput;
    bool isAnyPortInWritting=false;//是否有正在写的端口
    QString inputStr;
    QString url;
    QList<DisplayItem *> displayItemList;
    QList<QString> writtedDevcieidlist;
    int width;
    int heigth;


    void displayItemCountChanged(int n);


    void writeDeviceId(QString);

    QList<QString> availableSerialPorts;

    QList<QString> getAvailableSerialPorts();

    QMap <QString,QString > *IdKeyList;

    DisplayItem * getNextDisplayItem();

    DisplayItem *currentWrittingDisplayItem;

    int startNumber;
    int endNumber;



    int nextValidDisplayItem();


    bool isInputValid(QString);

    bool isDeviceIdOverWritted(QString);

    bool isDeviceIdInValidRange(QString);

    QString getKeyFromId(QString deviceId);


    QString getDeviceIDfromInputString(QString);



private slots:
    void slotInputTimeout();
    void slotItemCountChangedto1(bool);
    void slotItemCountChangedto2(bool);
    void slotItemCountChangedto4(bool);
    void slotItemCountChangedto6(bool);
    void slotItemCountChangedto8(bool);

    void slotWritingResult(bool);
    void slotUpdateState();     //输入时，判断刷新状态

signals:
    void signalKeyScanOver();
};

#endif // MAINWINDOW_H
