#ifndef DISPLAYITEM_H
#define DISPLAYITEM_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QDateTime>

#include "UserUartLink.h"

#define STATE_OPEN_FAILED 0
#define STATE_WAITING_WRITE 1
#define STATE_WRITE_SUCCESS 2
#define STATE_WRITE_FAILED 3
#define STATE_WRITTING 4
#define STATE_CURRENT_WRITING 5

#define STYLE_OPEN_FAILED "background-color: gray"
#define STYLE_WAITTING_WRITE "background-color: green"
#define STYLE_WRITE_SUCCESS "background-color: blue"
#define STYLE_WRITE_FAILED "background-color: red"
#define STYLE_CURRENT_WRITTING "background-color: yellow"

namespace Ui {
class DisplayItem;
}

class DisplayItem : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayItem(QWidget *parent = 0);
    ~DisplayItem();

    bool openSerialPort(QString port);
    static bool CorrcetProtocol;   //修正原代码中的协议错误

    int getCurrentState() const;
    void setCurrentState(int value);

    void writeDeviceId(QString str,QString keys);

    void WriteMessage(QString str);

private:
    Ui::DisplayItem *ui;
    QSerialPort serialPort;

    int currentState=0; // 0:未打开串口 1：等待写入 2：写入成功 3：写入失败 4:正在写入

    unsigned char receivedFrame[60]={0};
    unsigned char sendingFrame[60]={0};

    QTimer timer;   //写入后间隔500毫秒后读取。
    QTimer timerWriteKey;
    QTimer receivedTimer;
    QTimer timerQuireKey;

    QTimer deviceTestingTimer;

    QString deviceId;
    QByteArray deviceIDArray;
    QString key;

    bool isDeviceIdWriteSuccess=false;
    bool isKeyWriteSuccess=false;



private slots:
    void slotReceivedData();
    void slotQueryId();
    void slotReceivedTimeOut();

    void slotDeviceTestingId();


    void slotWriteKey();

    void slotQuireKey();


signals:
    void signalWriteOver(bool);
};

#endif // DISPLAYITEM_H
