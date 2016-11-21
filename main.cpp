#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
//    qDebug()<<database.isValid();
//    database.setDatabaseName("database.db");
//    database.open();
//    QSqlQuery query;
//    query.exec("create table test(name varchar(30))");
//    query.clear();
//    query.exec("insert into test values ('asdf')");
//    query.clear();
//    query.exec("select * from test");
//    while(query.next())
//    {
//        qDebug()<<query.value("name").toString();
//    }
//    database.open();
//    FirstInputDialog f;
//    qDebug()<<f.exec();

//    if(f.getIsParametterSetted() == 1)
//    {
        MainWindow w;
        qDebug()<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__;
        w.setWindowFlags(Qt::WindowCloseButtonHint|Qt::WindowMaximizeButtonHint|Qt::WindowMinimizeButtonHint);
        w.showMaximized();
        w.show();
//    }
//    else
//    {
//        a.exit();
//    }

    return a.exec();
}
