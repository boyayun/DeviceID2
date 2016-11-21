#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    FirstInputDialog f;
    f.setWindowFlags(Qt::WindowCloseButtonHint|Qt::WindowMaximizeButtonHint|Qt::WindowMinimizeButtonHint);
    f.showMaximized();
    f.exec();

    if(f.getIsParametterSetted() == 0)
    {
        this->close();
        exit(0);
    }
    this->url = f.getUrl();


    qDebug()<<f.getInputString().indexOf(QRegExp("[0-9]{35}"));

    int from = f.getInputString().indexOf(QRegExp("[0-9]{35}"));
    this->startNumber = f.getInputString().mid(from+23,8).toInt();//
    this->endNumber = f.getInputNumber()+this->startNumber;
    this->IdKeyList = f.getIdKeyMap();
    qDebug()<<IdKeyList->keys();
    qDebug()<<IdKeyList->values();

    qDebug()<<this->startNumber<<"~"<<this->endNumber;

    ui->setupUi(this);
    this->ui->lineEdit->setEnabled(false);
    isNeedClearInput = true;
    connect(&this->inputTimer,SIGNAL(timeout()),this,SLOT(slotInputTimeout()));

    connect(this,SIGNAL(signalKeyScanOver()),this,SLOT(slotUpdateState()));

//    QDesktopWidget *desktopWidget = QApplication::desktop();
//    this->width = desktopWidget->screenGeometry().width();
//    this->heigth = desktopWidget->screenGeometry().height();

    connect(this->ui->action1,SIGNAL(triggered(bool)),this,SLOT(slotItemCountChangedto1(bool)));
    connect(this->ui->action2,SIGNAL(triggered(bool)),this,SLOT(slotItemCountChangedto2(bool)));
    connect(this->ui->action4,SIGNAL(triggered(bool)),this,SLOT(slotItemCountChangedto4(bool)));
    connect(this->ui->action6,SIGNAL(triggered(bool)),this,SLOT(slotItemCountChangedto6(bool)));
    connect(this->ui->action8,SIGNAL(triggered(bool)),this,SLOT(slotItemCountChangedto8(bool)));
//    this->setFixedSize(width,heigth-80);
    this->availableSerialPorts = getAvailableSerialPorts();
    displayItemCountChanged(1);
    this->setWindowTitle("设备ID写入");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *keyValue)
{
    if(!isAnyPortInWritting)
    {
        if(isNeedClearInput)
        {
            this->inputStr = keyValue->text();
        }
        else
        {
            if(keyValue->key()!=16777220)
            {
                this->inputStr = inputStr + keyValue->text();
            }
        }
        if(keyValue->key() != 16777220)
        {
            isNeedClearInput = false;
            if(inputTimer.isActive())
            {
                inputTimer.stop();
                inputTimer.start(100);
            }
            else
            {
                inputTimer.start(100);
            }

        }
        else
        {
            inputTimer.stop();
            isNeedClearInput = true;
            if(inputStr.length()>5)
            {
                qDebug()<<"Input String is:"<<inputStr;
                this->ui->lineEdit->setText(inputStr);
                if(isInputValid(inputStr))
                {
                    //               DisplayItem * currentItem =  getNextDisplayItem();
                    slotUpdateState();
                    DisplayItem *currentItem = currentWrittingDisplayItem;
                    if(currentItem)
                    {
                        QString deviceidStr = getDeviceIDfromInputString(inputStr);
                        qDebug()<<__FILE__<<__LINE__<<deviceidStr;
                        if(isDeviceIdOverWritted(deviceidStr))  //是否已经写入
                        {
                            //QMessageBox::information(this,"错误",deviceidStr+"已经被写入");
                            this->currentWrittingDisplayItem->WriteMessage("重复！请重新扫码！");

                        }
                        else
                        {
                            if(isDeviceIdInValidRange(deviceidStr))
                            {

                                QString keyStr = getKeyFromId(deviceidStr);
                              if(keyStr!="")
                              {
                                this->isAnyPortInWritting = true;
                                currentItem->writeDeviceId(deviceidStr,keyStr); //写设备ID
                              }
                              else
                              {
                                  this->currentWrittingDisplayItem->WriteMessage("未找到秘钥");
                              }

                            }
                            else
                            {
//                                QMessageBox::information(this,"错误",deviceidStr+"不在可用范围");
                                this->currentWrittingDisplayItem->WriteMessage("二维码超出设定范围！");
                            }
                        }




                    }
//                    else
//                    {
//                        qDebug()<<"Circle Over";
//                        QMessageBox::information(this,"错误","未能找到可写串口端口");
//                    }
                }
                else
                {
                    QMessageBox::information(this,"错误","二维码格式错误");
                }
                qDebug()<<"输入完成";
                //            currentWrittingDisplayItem = getNextDisplayItem();
                //            if(currentWrittingDisplayItem)
                //            {
                //                currentWrittingDisplayItem->setCurrentState(STATE_CURRENT_WRITING);


                //            }
                //            else
                //            {
                //                qDebug()<<"Circle Over";
                //                QMessageBox::information(this,"一轮完成","请更换换主控，如未写成功需要重新写，请勿更换");
                //            }
            }
            //写入deviceid，是否成功

        }
    }
    else
    {
        qDebug()<<keyValue->text();
        QMessageBox::information(this,"警告","当前端口正在写入！");
    }
}

void MainWindow::displayItemCountChanged(int n)
{
    this->isAnyPortInWritting = false;
    int count = this->displayItemList.size();
    int availablePortCount;
    for(int i = 0; i < count; i++)
    {
        delete displayItemList.last();
        displayItemList.removeLast();
    }

    this->availableSerialPorts = getAvailableSerialPorts();
    availablePortCount = this->availableSerialPorts.size();
    for(int j = 0; j < n; j++)
    {
        DisplayItem *displayItem = new DisplayItem();
        connect(displayItem,SIGNAL(signalWriteOver(bool)),this,SLOT(slotWritingResult(bool)));
        if(j < availablePortCount )
        {
            displayItem->openSerialPort(availableSerialPorts.at(j));
        }


        this->ui->gridLayout->addWidget(displayItem,j/2,j%2);
        displayItemList.append(displayItem);
    }
//    this->setFixedSize(width,heigth-80);
    currentWrittingDisplayItem = getNextDisplayItem();
    if(currentWrittingDisplayItem)
    {
        currentWrittingDisplayItem->setCurrentState(STATE_CURRENT_WRITING);
    }
}

void MainWindow::writeDeviceId(QString id)
{
    qDebug()<<__LINE__<<id;

}

QList<QString> MainWindow::getAvailableSerialPorts()
{
    QList<QSerialPortInfo> listInfo = QSerialPortInfo::availablePorts();
    QList<QString>portList;
    for(int i=0;i<listInfo.size();i++)
    {
        if(!listInfo.at(i).isBusy())
        {
            portList.append(listInfo.at(i).portName());
        }
    }
    qDebug()<<portList;
    return portList;
}

DisplayItem *MainWindow::getNextDisplayItem()
{
    //    QList<DisplayItem *>::iterator it = displayItemList.begin();
    //    while((it != displayItemList.end())&&((*it)->getCurrentState() != STATE_WAITING_WRITE))
    //    {
    //        qDebug()<<(*it)->getCurrentState();
    //        it++;

    //    }

    //    return *it;
    for(int i = 0; i < displayItemList.size(); i++)
    {
        if( displayItemList.at(i)->getCurrentState() == STATE_WAITING_WRITE)
        {
            return displayItemList.at(i);
        }
    }
    return NULL;

}

int MainWindow::nextValidDisplayItem()
{
    return 0;
}

void MainWindow::slotUpdateState()
{
    if(!currentWrittingDisplayItem)
    {
        for(int i=0;i<displayItemList.size();i++)
        {
            if((displayItemList.at(i)->getCurrentState() == STATE_WRITE_SUCCESS) || (displayItemList.at(i)->getCurrentState() == STATE_WRITE_FAILED))
            {
                displayItemList.at(i)->setCurrentState(STATE_WAITING_WRITE);
            }
        }
        this->currentWrittingDisplayItem = getNextDisplayItem();
        if(currentWrittingDisplayItem)
        {
            this->currentWrittingDisplayItem->setCurrentState(STATE_CURRENT_WRITING);
        }
    }
}

bool MainWindow::isInputValid(QString s)
{

      QRegExp rx(REGEXP_STRING);
      QRegExp rx2(REGEXP_STRING2);
      QString urlstr = s;
      urlstr.remove(QRegExp(REGEXP_STRING2));


      if(!(urlstr == this->url || urlstr == "" ))
      {
         return false;
      }

      if(s.indexOf(rx2) >=0)
      {
          return true;
      }
      else
      {
          return false;
      }
}

bool MainWindow::isDeviceIdOverWritted(QString id)
{
    return writtedDevcieidlist.contains(id);
}


bool MainWindow::isDeviceIdInValidRange(QString id)
{
   int num = id.mid(23,8).toInt();

   return true;

   if(num >= this->startNumber && num <= this->endNumber)
   {
       return true;
   }
   else
   {
       return false;
   }

}

QString MainWindow::getKeyFromId(QString deviceId)
{

    return this->IdKeyList->value(deviceId);

}

QString MainWindow::getDeviceIDfromInputString(QString s)
{
//   return s.mid(s.indexOf((QRegExp("[0-9]{35}"))),35);
   return s.mid(s.indexOf((QRegExp(REGEXP_STRING2))),56);
}

void MainWindow::slotInputTimeout()
{
    this->inputTimer.stop();
    inputStr.clear();

    qDebug()<<"Input TimeOut";


}

void MainWindow::slotItemCountChangedto1(bool)
{
    displayItemCountChanged(1);
}

void MainWindow::slotItemCountChangedto2(bool)
{
    displayItemCountChanged(2);
}

void MainWindow::slotItemCountChangedto4(bool)
{

    displayItemCountChanged(4);
}

void MainWindow::slotItemCountChangedto6(bool)
{
    displayItemCountChanged(6);
}

void MainWindow::slotItemCountChangedto8(bool)
{
    displayItemCountChanged(8);
}

void MainWindow::slotWritingResult(bool b)
{
    this->isAnyPortInWritting = false;
    qDebug()<<__LINE__<<b;
    if(b) //写入完成，将已经写入的deviceid加入。
    {
        this->writtedDevcieidlist.append(getDeviceIDfromInputString(inputStr));
    }

    currentWrittingDisplayItem = getNextDisplayItem();
    if(currentWrittingDisplayItem)
    {
        currentWrittingDisplayItem->setCurrentState(STATE_CURRENT_WRITING);
    }
//    else
//    {
//        qDebug()<<"Circle Over";
//        /*20161103生产要求修改显示界面
//        QMessageBox message(QMessageBox::NoIcon,"一轮完成","请更换主控板，若未写成功请勿跟换！");
//        message.setIconPixmap(QPixmap(":/enter.png"));
//        message.exec();
//        20161103生产要求修改显示界面*/
//        updateState();
//    }
}

