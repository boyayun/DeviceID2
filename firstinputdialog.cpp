#include "firstinputdialog.h"
#include "ui_firstinputdialog.h"

FirstInputDialog::FirstInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FirstInputDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("信息输入V0.8");
//    QDesktopWidget *desktopWidget = QApplication::desktop();
//    this->setFixedSize(
//                        desktopWidget->screenGeometry().width(),
//                        desktopWidget->screenGeometry().height()-60);
//    setWindowFlags(windowFlags() &~ Qt::WindowCloseButtonHint);
//    this->setWindowFlags(Qt::WindowCloseButtonHint|Qt::WindowMaximizeButtonHint|Qt::WindowMinimizeButtonHint);
//    this->showMaximized();
    isParametterSetted = 0;
    this->ui->label_2->setVisible(false);
    this->ui->numberlineEdit->setVisible(false);

    this->ui->numberlineEdit->setValidator(new QIntValidator(1,65535));
    this->isInputNumberValid = true;
    this->isInputStringValid = false;
    this->isKeyFileLoaded = false;
    this->ui->okButton->setEnabled(false);
    this->isNeedSelectAll = true;
    this->IdKeyMap = NULL;
//    connect(this->ui->inputStringLineEdit,SIGNAL(textEdited(QString)),this,SLOT(slotTextEdited(QString)));
//    connect(this->ui->inputStringLineEdit,SIGNAL(editingFinished()),this,SLOT(slotEditFinished()));
}

FirstInputDialog::~FirstInputDialog()
{
    delete ui;
}

void FirstInputDialog::keyPressEvent(QKeyEvent *)
{

}

void FirstInputDialog::on_okButton_clicked()
{
    isParametterSetted = 1;
    this->inputString = this->ui->inputStringLineEdit->text();
    this->inputNumber = this->ui->numberlineEdit->text().toInt();
    this->close();
}
int FirstInputDialog::getIsParametterSetted() const
{
    return isParametterSetted;
}

void FirstInputDialog::setIsParametterSetted(int value)
{
    isParametterSetted = value;
}

bool FirstInputDialog::isInpurStringValid(QString s)
{
    QRegExp rx2(REGEXP_STRING2);

    return (s.indexOf(rx2) >=0);
}



void FirstInputDialog::on_inputStringLineEdit_textChanged(const QString &arg1)
{
    if(isInpurStringValid(arg1))
    {
        this->isInputStringValid = true;
        QString urlstr = arg1;
        this->url = urlstr.remove(QRegExp(REGEXP_STRING2));
        this->ui->Urllabe->setText(url);
        qDebug()<<this->url;
        if(this->url == "")
        {
            this->ui->Urllabe->setText("请扫描带URL的二维码");
            this->isInputStringValid = false;
        }
        this->ui->inputStringLineEdit->selectAll();
    }
    else
    {
        this->isInputStringValid  = false;
    }
    this->ui->okButton->setEnabled( isInputNumberValid && isInputStringValid && isKeyFileLoaded);
}


void FirstInputDialog::on_numberlineEdit_textChanged(const QString &arg1)
{
    if(arg1.toInt() > 0)
    {
        this->isInputNumberValid = true;
    }
    else
    {
       this->isInputNumberValid = false;
    }
    this->ui->okButton->setEnabled(isInputNumberValid && isInputStringValid && isKeyFileLoaded);
}


void FirstInputDialog::slotTextEdited(QString)
{
    if(isNeedSelectAll)
    {
        this->ui->inputStringLineEdit->selectAll();
    }
    this->isNeedSelectAll = false;

}

void FirstInputDialog::slotEditFinished()
{
   this->isNeedSelectAll = true;
}
int FirstInputDialog::getInputNumber() const
{
    return inputNumber;
}

QString FirstInputDialog::getInputString() const
{
    return inputString;
}


void FirstInputDialog::on_selectkeyfileButton_clicked()
{
   QString filepath = QFileDialog::getOpenFileName(this,"打开文件","","*.txt");
   qDebug()<<filepath;
   if(this->IdKeyMap)
   {
       this->IdKeyMap->clear();
   }
   else
   {
      this->IdKeyMap = new QMap<QString,QString>();
   }
   if(filepath !="")
   {
//       QRegExp regExp("[0-9A-Fa-f]{56}(,[.]*){5},((?=[\x21-\x7e]+)[^,]){16},.*");   //无用20161031
       this->keyFilePath = filepath;

       this->ui->pathLable->setText(filepath);
       QFile file(filepath);

       if(file.open(QFile::ReadOnly))
       {
           while(!file.atEnd())
           {
               QString s;
               s = QString(file.readLine());

               if(s.length()>1000)
               {

               }
               else
               {
                   QStringList list = s.split(",");
                   if(list.size() == 8)
                   {
                       this->IdKeyMap->insert(list.at(0),list.at(6));
                   }
               }
           }
           qDebug()<<this->IdKeyMap->keys();
           qDebug()<<this->IdKeyMap->values();

           if(this->IdKeyMap->size() == 0)
           {
               QMessageBox::warning(this,"提示","请选择正确的文件");
           }
           else
           {
               this->isKeyFileLoaded = true;
               this->ui->pathLable->setText(this->ui->pathLable->text() + "加载成功！");
           }
       }
       else
       {
          QMessageBox::warning(this,"错误","文件打开失败！");
       }
   }
   else
   {
       this->ui->pathLable->setText("请选择文件！");
       this->isKeyFileLoaded = false;
   }
    this->ui->okButton->setEnabled( isInputNumberValid && isInputStringValid && isKeyFileLoaded);
}
QString FirstInputDialog::getUrl() const
{
    return url;
}

void FirstInputDialog::setUrl(const QString &value)
{
    url = value;
}

QMap<QString, QString> *FirstInputDialog::getIdKeyMap() const
{
    return IdKeyMap;
}

QString FirstInputDialog::getKeyFilePath() const
{
    return keyFilePath;
}

void FirstInputDialog::setKeyFilePath(const QString &value)
{
    keyFilePath = value;
}

