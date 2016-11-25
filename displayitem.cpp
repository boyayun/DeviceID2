#include "displayitem.h"
#include "ui_displayitem.h"

bool DisplayItem::CorrcetProtocol = true;

DisplayItem::DisplayItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DisplayItem)
{
    ui->setupUi(this);
    serialPort.setBaudRate(QSerialPort::Baud9600);
    serialPort.setParity(QSerialPort::NoParity);
    serialPort.setStopBits(QSerialPort::OneStop);
    serialPort.setFlowControl(QSerialPort::NoFlowControl);
    this->currentState = 0;
    setCurrentState(STATE_OPEN_FAILED);
    connect(&serialPort,SIGNAL(readyRead()),this,SLOT(slotReceivedData()));
    connect(&timer,SIGNAL(timeout()),this,SLOT(slotQueryId()));
    connect(&receivedTimer,SIGNAL(timeout()),this,SLOT(slotReceivedTimeOut()));
    connect(&timerQuireKey,SIGNAL(timeout()),this,SLOT(slotQuireKey()));

    connect(&timerWriteKey,SIGNAL(timeout()),this,SLOT(slotWriteKey()));
    connect(&deviceTestingTimer,SIGNAL(timeout()),this,SLOT(slotDeviceTestingId()));
    deviceTestingTimer.start(1000);
}

DisplayItem::~DisplayItem()
{
    this->serialPort.close();
    delete ui;
}


bool DisplayItem::openSerialPort(QString port)
{
    serialPort.setPortName(port);
    bool isOpenSuccess =  serialPort.open(QSerialPort::ReadWrite);
    if(isOpenSuccess)
    {
        this->ui->lineEdit->setText("串口" + port + "打开成功!");
        setCurrentState(STATE_WAITING_WRITE);
    }
    else
    {
        this->ui->lineEdit->setText("串口" + port + "打开失败!");
        setCurrentState(STATE_OPEN_FAILED);
    }
//    this->ui->lineEdit->setStyleSheet("color: blue");
    return isOpenSuccess;
}
int DisplayItem::getCurrentState() const
{
    return currentState;
}

void DisplayItem::setCurrentState(int value)
{
    this->ui->label->setStyleSheet("color:black");
    if(value == STATE_OPEN_FAILED)
    {
        this->setStyleSheet("background-color: gray");
        this->ui->label->setText("串口打开失败！");
    }
    else if(value == STATE_WAITING_WRITE)
    {
        this->setStyleSheet("background-color: green;");
        this->ui->label->setText("待输入！");
    }
    else if(value == STATE_WRITE_FAILED)
    {
        this->setStyleSheet("background-color: red;");
        this->ui->label->setText(deviceId.mid(deviceId.length()-6)+"写失败");
    }
    else if(value == STATE_WRITE_SUCCESS)
    {
        this->setStyleSheet("background-color: blue;");
        this->ui->label->setText(deviceId.mid(deviceId.length()-6)+"写成功");
    }
    else if(value == STATE_WRITTING)
    {
        this->setStyleSheet("background-color: green;");
        this->ui->label->setText(deviceId.mid(deviceId.length()-6)+"正在写");
    }
    else if(value == STATE_CURRENT_WRITING)
    {
        this->ui->label->setText("当前输入");
        ///////////////////////////////////
        this->serialPort.close();
        this->serialPort.open(QIODevice::ReadWrite);
        ////////////////////////////////////
        this->setStyleSheet(STYLE_CURRENT_WRITTING);
    }
    else
    {

    }
    currentState = value;
}

void DisplayItem::writeDeviceId(QString str,QString keys)
{

    this->deviceId = str;
    this->key = keys;

    setCurrentState(STATE_WRITTING);



    QByteArray writeData ;
    for( int i=0;i<str.length();i=i+2)
    {
        writeData.append(str.mid(i,2).toInt(0,16));
    }
    for(int i=0;i<8;i++)
    {
        writeData.append('\0');
    }
    this->deviceIDArray = writeData;
    qDebug()<<__FILE__<<__LINE__<<writeData.length();

    unsigned char dataFrame[60];
    dataFrame[0] = 0x85;


//    memcpy(dataFrame+1,writeData.data(),writeData.length());
    memcpy(dataFrame+1,writeData.data(),writeData.length());

    unsigned char len = UserUartLinkPack(this->sendingFrame,dataFrame,writeData.length()+1,0);
    qDebug()<<"写入设备ID帧："<<QByteArray((char *)this->sendingFrame,len).toHex();

    serialPort.write((char *)sendingFrame,len);

    //写秘钥


    this->timerWriteKey.setSingleShot(true);
    this->timerWriteKey.start(200);





}

void DisplayItem::WriteMessage(QString str)
{
    this->ui->label->setStyleSheet("color:red");
    this->ui->label->setText(str);

}

void DisplayItem::slotReceivedData()
{
    QByteArray receivedData = serialPort.readAll();
    //   UserUartLinkUnpack((unsigned char *)receivedData.data(),receivedData.length());
    //   UserUartLinkClass userUartLinkClass;
    bool uartsuccess = UserUartLinkUnpack((unsigned char *)receivedData.data(),receivedData.length());
    //   int len = getUserUartLinkMsg(this->receivedFrame);
    if(uartsuccess)
    {
        int len = getUserUartLinkMsg(this->receivedFrame);
        qDebug()<<"received frame is:"<<QByteArray((char*)receivedFrame,len);
        if(this->receivedFrame[0] == 0x11)
        {
            qDebug()<<"deviceid is:"<<deviceId;
            qDebug()<<"received deviceid is:"<<QByteArray((char*)receivedFrame+1,len);
            if(this->currentState == STATE_WRITTING)
            {
//                if(this->deviceId == QString(QByteArray((char *)receivedFrame+1,35)))
                if(this->deviceIDArray == QByteArray((char *)receivedFrame+1,36))
                {
                    this->isDeviceIdWriteSuccess = true;
                }
                else
                {
                    setCurrentState(STATE_WRITE_FAILED);
                    this->receivedTimer.stop();
                    emit signalWriteOver(false);
                }
            }
        }
        if(this->receivedFrame[0] == 0x15) // key report frame
        {
            qDebug()<<QTime::currentTime()<<":key is:"<<this->key;
            qDebug()<<"received key is:"<<QByteArray((char *)receivedFrame+1,16);
            if(this->currentState == STATE_WRITTING)
            {
               if(this->key == QString(QByteArray((char *)receivedFrame+1,16)))
               {
                   this->isKeyWriteSuccess = true;
               }
               else
               {
                   this->isKeyWriteSuccess = false;
                   setCurrentState(STATE_WRITE_FAILED);
                   this->receivedTimer.stop();
                   emit signalWriteOver(false);
               }
            }
        }
        if(this->isDeviceIdWriteSuccess && this->isKeyWriteSuccess)
        {
            this->isDeviceIdWriteSuccess = false;
            this->isKeyWriteSuccess = false;
            this->receivedTimer.stop();
            setCurrentState(STATE_WRITE_SUCCESS);
            emit signalWriteOver(true);
        }

        //       qDebug()<<QByteArray((char *)this->receivedFrame,len);
        //       this->receivedTimer.stop();
        //       setCurrentState(STATE_WRITE_SUCCESS);
        //       else
        //       {
        //           setCurrentState(STATE_WRITE_FAILED);
        //       }
    }
}

void DisplayItem::slotQueryId()
{
    //发送查询id指令
    unsigned char queryFrame[]={0xf5,0xf5,0x00,0x03,0x10,0x55,0x10};
    serialPort.write((char *)queryFrame,7);
    qDebug()<<QTime::currentTime().toString()<<"发送查询帧 id:"<<QByteArray((char *)queryFrame,7).toHex();



    this->timerQuireKey.setSingleShot(true);
    this->timerQuireKey.start(300);
    this->receivedTimer.setSingleShot(true);
    this->receivedTimer.start(1200);
}

void DisplayItem::slotReceivedTimeOut()// 未收到返回数据
{
    qDebug()<<__LINE__<<"未收到数据";
    setCurrentState(STATE_WRITE_FAILED);
    emit signalWriteOver(false);
}

void DisplayItem::slotDeviceTestingId()
{

//    unsigned char queryFrame[]={0xf5,0xf5,0x00,0x03,0x10,0x55,0x10};
    //    serialPort.write((char *)queryFrame,7);
}

void DisplayItem::slotWriteKey()
{
    qDebug()<<"写秘钥";
    QByteArray writeData;
    unsigned char dataFrame[60];
    writeData = this->key.toLocal8Bit();

    dataFrame[0] = 0x14;
    if(CorrcetProtocol)
    {
        dataFrame[0] = 0x87;
    }
    memcpy(dataFrame+1,writeData.data(),writeData.length());
    int len = UserUartLinkPack(this->sendingFrame,dataFrame,writeData.length()+1,0);
    qDebug()<<QByteArray((char *)this->sendingFrame,len).toHex();
    serialPort.write((char *)sendingFrame,len);
    this->timer.setSingleShot(true);
    this->timer.start(500);

}

void DisplayItem::slotQuireKey()
{
    unsigned char queryKeyFrame[] ={0xf5,0xf5,0x00,0x03,0x87,0xb5,0x26};
    if(CorrcetProtocol)
    {
        queryKeyFrame[4] = 0x14;
        queryKeyFrame[5] = 0x13;
        queryKeyFrame[6] = 0x34;
    }
    serialPort.write((char *) queryKeyFrame,7);
    qDebug()<<"send query key:"<<QByteArray((char *)queryKeyFrame,7).toHex();
}

