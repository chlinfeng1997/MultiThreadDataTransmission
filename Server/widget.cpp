#include "widget.h"
#include "clientaddress.h"
#include "ui_widget.h"
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QEvent>
#include <stdio.h>
#include <string.h>
#include <sys/io.h>
#include <iostream>
#include <QNetworkInterface>
//#include "seekfile.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle(tr("局域网文件传输服务器"));
    /*清空列表文件*/
    if((fp = fopen("/home/leo/Desktop/NetworkFileTransfer-master/FileList/FILELIST.txt","w+"))==NULL)
    {
        qDebug()<<"Clear FILELIST.TXT error!";
    }
    fclose(fp);
    server = new Myserver(this);
    server->listen(QHostAddress::Any, 8888);
    closeCommand = 0;
    ui->textEdit->document()->setMaximumBlockCount(100);
//    ui->ClientPB1->setEnabled(false);
//    ui->ClientPB2->setEnabled(false);
//    ui->ClientPB3->setEnabled(false);
//    /*为三个按键安装事件过滤器*/
//    ui->ClientPB1->installEventFilter(this);
//    ui->ClientPB2->installEventFilter(this);
//    ui->ClientPB3->installEventFilter(this);

//    ui->LBstate1->setText(tr("断开"));
//    ui->LBstate2->setText(tr("断开"));
//    ui->LBstate3->setText(tr("断开"));

    ui->pushButton->setEnabled(false);

    connect(ui->pushButton,&QPushButton::clicked,this,&Widget::fileTranferButtonSlot);
    connect(ui->AddFilePB,&QPushButton::clicked,this,&Widget::addFilePBSlot);
//    connect(ui->DelFilePB,&QPushButton::clicked,this,&Widget::deleteFileListItem);
    connect(ui->DelFilePB,&QPushButton::clicked,this,&Widget::deleteFile);
    /*服务器端主动断开连接*/
    connect(this,&Widget::clientDiconnectSignal,server, &Myserver::clientDisconnectSlot);
    connect(ui->InfoPushButton,&QPushButton::clicked,this,&Widget::infoTranferButtonSlot);
    connect(ui->DelInfoPB,&QPushButton::clicked,this,&Widget::deleteInfo);
//    connect(ui->listWidget,&QAbstractItemView::doubleClicked,this,&Widget::doubleClickedItem);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::dealMessage(QNChatMessage *messageW, QListWidgetItem *item, QString text, QString time,  QNChatMessage::User_Type type)
{
    messageW->setFixedWidth(ui->listWidget->width()-10);
    QSize size = messageW->fontRect(text);
    item->setSizeHint(size);
    messageW->setText(text, time, size, type);
    ui->listWidget->setItemWidget(item, messageW);
}

void Widget::dealMessageTime(QString curMsgTime)
{
    bool isShowTime = false;
    if(ui->listWidget->count() > 0) {
        QListWidgetItem* lastItem = ui->listWidget->item(ui->listWidget->count() - 1);
        QNChatMessage* messageW = (QNChatMessage*)ui->listWidget->itemWidget(lastItem);
        int lastTime = messageW->time().toInt();
        int curTime = curMsgTime.toInt();
        qDebug() << "curTime lastTime:" << curTime - lastTime;
        isShowTime = ((curTime - lastTime) > 60); // 两个消息相差一分钟
//        isShowTime = true;
    } else {
        isShowTime = true;
    }
    if(isShowTime) {
        QNChatMessage* messageTime = new QNChatMessage(ui->listWidget->parentWidget());
        QListWidgetItem* itemTime = new QListWidgetItem(ui->listWidget);

        QSize size = QSize(ui->listWidget->width()-10, 40);
        messageTime->resize(size);
        itemTime->setSizeHint(size);
        messageTime->setText(curMsgTime, curMsgTime, size, QNChatMessage::User_Time);
        ui->listWidget->setItemWidget(itemTime, messageTime);
    }
}

void Widget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    for(int i = 0; i < ui->listWidget->count(); i++) {
        QNChatMessage* messageW = (QNChatMessage*)ui->listWidget->itemWidget(ui->listWidget->item(i));
        QListWidgetItem* item = ui->listWidget->item(i);

        dealMessage(messageW, item, messageW->text(), messageW->time(), messageW->userType());
    }
}

void Widget::closeClientConnectSlot(QString clientIp, int clientPort, QDateTime current_date_time)
{
    /*判断是哪个客户端断开，并做相应的清除工作*/
    qDebug() << "#";
    for(int i = 0; i < IPOrder.size(); i++)
    {
        qDebug() << IPOrder.at(i) << " ";
    }
    qDebug() << index << " ";
    qDebug() << clientIp << " ";
    closeFlag_current *= -1;
    int deleteIndex = IPOrder.indexOf(clientIp);
//                    qDebug() << QString::number(deleteIndex);
    QListWidgetItem *item = ui->clientListWidget->takeItem(deleteIndex);
    delete item;
    IPOrder.remove(deleteIndex);
    index--;
//    QString current_date = current_date_time.toString("yyyy.MM.dd hh:mm:ss");
//    /*判断连接的客户端数量，并根据连接的客户端信息进行界面更新*/
//    ui->textEdit->append(current_date + " —— IP: " + clientIp + " 断开连接");
    if(closeFlag_previous * closeFlag_current == -1)
    {
        QString current_date = current_date_time.toString("yyyy.MM.dd hh:mm:ss");
        /*判断连接的客户端数量，并根据连接的客户端信息进行界面更新*/
        ui->textEdit->append(current_date + " —— IP: " + clientIp + "  Port: " + QString::number(clientPort) + "  客户端断开连接");
    }
    server->socketNum--;
    closeFlag_previous = closeFlag_current;
}

void Widget::addFilePBSlot()
{
    /*打开目录获得文件目录*/
//    dirPath = QFileDialog::getExistingDirectory(this,0,"./");
    dirPath = QFileDialog::getOpenFileName(this, "选择文件", "./", "(*.*)");
    QDir dir(dirPath);
    if(dirPath.isEmpty())
        return;
      ui->lineEdit->setText(dirPath);
      ui->pushButton->setEnabled(true);
}
void Widget::getSendFileList(QString path)
{
//    FILE *fp;
//    /*已追加的方式打开文件*/
//    fp = fopen("./FileList/FILELIST.TXT","at+");
//    if(fp == NULL)
//    {
//        qDebug()<<"Open FILELIST.TXT failed!";
//        return;
//    }
    QFile filelist("./FileList/FILELIST.TXT");
    QTextStream out(&filelist);
    if(!filelist.open(QFile::WriteOnly|QFile::Text))
    {
        qDebug()<<"write FILELIST.TXT error!"<<endl;
        return;
    }
    QDir dir(path);
    QStringList str;
    str << "*";
    QFileInfoList files = dir.entryInfoList((QStringList)"*",
                                            QDir::Files|QDir::Dirs,QDir::DirsFirst);
    for(int i=0;i<files.count();i++)        //把目录中的文件名写入filelist.txt文件中发送至客户端
    {
        QFileInfo tmpFileInfo = files.at(i);
        QString fileName = tmpFileInfo.fileName();
        if(fileName=="."||fileName=="..")   //过滤掉“.”和“..”目录
            continue;
        if(tmpFileInfo.isFile())
            out << fileName << endl;
        else
            continue;
     }
    //fclose(fp);
    filelist.close();
}

/*遍历当前目录下的文件夹和文件,默认是按字母顺序遍历*/
//bool Widget::TraverseFiles(std::string path,int &file_num)
//{
//    _finddata_t file_info;
//    std::string current_path=path+"/*.*"; //可以定义后面的后缀为*.exe，*.txt等来查找特定后缀的文件，*.*是通配符，匹配所有类型,路径连接符最好是左斜杠/，可跨平台
//    /*打开文件查找句柄*/
//    int handle=_findfirst(current_path.c_str(),&file_info);
//    /*返回值为-1则查找失败*/
//    if(-1==handle)
//        return false;
//    do
//    {
//        /*判断是否子目录*/
//        std::string attribute;
//        if(file_info.attrib==_A_SUBDIR) //是目录
//        {
//            if(!(strcmp(file_info.name,"."))||!(strcmp(file_info.name,"..")))
//                continue;
//            ISDir();
//            strcpy(dirInfo.dirName,file_info.name);
//#ifdef __DEBUG__
//           qDebug()<<dirInfo.dirName<<endl;
//#endif
//            fwrite(&dirInfo,sizeof(struct DirInformation),1,fp);
//        }
//        else
//        {
//            ISFile();
//            strcpy(dirInfo.fileName,file_info.name);
//#ifdef __DEBUG__
//            std::cout<<dirInfo.fileName;
//#endif
//            fwrite(&dirInfo,sizeof(struct DirInformation),1,fp);
//        }
//        /*输出文件信息并计数,文件名(带后缀)、文件最后修改时间、文件字节数(文件夹显示0)、文件是否目录*/
//        std::cout<<file_info.name<<' '<<file_info.time_write<<' '<<file_info.size<<' '<<attribute<<endl; //获得的最后修改时间是time_t格式的长整型，需要用其他方法转成正常时间显示
//        file_num++;
//    }while(!_findnext(handle,&file_info));  //返回0则遍历完
//    /*关闭文件句柄*/
//    _findclose(handle);
//    return true;
// }

void Widget::deleteFile()
{
    dirPath.clear();
    ui->lineEdit->setText("");
    ui->pushButton->setEnabled(false);
}

void Widget::deleteInfo()
{
    info.clear();
    ui->textEdit_2->setText("");
}

//void Widget::deleteFileListItem()
//{
//    /*获得当前选中项,并删除*/
//    QListWidgetItem *item = ui->listWidget->currentItem();
//    delete item;
//}

//void Widget::doubleClickedItem()
//{
//    /*获得双击项文本*/
//    QString str = ui->listWidget->currentItem()->text();
//#if defined __DEBUG__
//    qDebug()<<"双击项文本内容str为"<<str;
//#endif
//    QDir dir;
//    /*设置路径为添加文件的路径*/
//    dir.setPath(dirPath);
//    /*切换路径为双击项的路径*/
//    dir.cd(str);
//    dirPath = dir.absolutePath();
//#if defined __DEBUG__
//    qDebug()<<"当前文件绝对路径为"<<dirPath;
//#endif
//    /*更新列表文件内容*/
//    this->getSendFileList(dirPath);
////   fp = fopen("./FileList/FILELIST.TXT","ab");
////    if(fp == NULL)
////    {
////        qDebug()<<"Open FILELIST.TXT failed!";
////        return;
////    }
////    this->DfsFolder(dirPath.toStdString(),0);
////    fclose(fp);
//    QStringList nameFilters;
//    nameFilters << "*";
//    QFileInfoList files = dir.entryInfoList(nameFilters,QDir::AllEntries,QDir::DirsFirst);
//    ui->listWidget->clear();
//    if(str=="..")                       //当返回上一级目录时，只显示目录不显示文件
//    {
//        for(int i =0 ;i<files.count();i++)
//        {
//            QFileInfo tmpFileInfo = files.at(i);
//            QString fileName = tmpFileInfo.fileName();
//            QListWidgetItem *tmp =new QListWidgetItem(fileName);
//            if(tmpFileInfo.isDir())         //显示目录
//            {
//                ui->listWidget->addItem(tmp);
//            }
//        }
//    }
//    else
//    {
//        for(int i =0 ;i<files.count();i++)
//        {
//            QFileInfo tmpFileInfo = files.at(i);
//            QString fileName = tmpFileInfo.fileName();
//            QListWidgetItem *tmp =new QListWidgetItem(fileName);
//            if(tmpFileInfo.isDir())         //显示目录
//            {
//                ui->listWidget->addItem(tmp);
//            }
//            if(tmpFileInfo.isFile())        //显示文件
//            {
//                ui->listWidget->addItem(tmp);
//            }
//        }
//    }
//}

void Widget::fileTranferButtonSlot()
{
//    QString fileName = QFileDialog::getOpenFileName(this);
//#if defined __DEBUG__
//    qDebug()<<"Open file's name is"<<fileName;
//#endif
//    if(!fileName.isEmpty())
//    {
//        qDebug()<<"Open file successed!";
//    }
    qDebug() << dirPath;
    if(server->socketNum > 0)
    {
       emit sendFileWidgetSignal(dirPath);
    }
    else
    {
        ui->textEdit->append("当前无客户端连接，传输文件无效！");
    }
}

void Widget::infoTranferButtonSlot()
{
    info = ui->textEdit_2->toPlainText();
    qDebug() << info;
    if(info.size() > 0)
    {
        QString time = QString::number(QDateTime::currentDateTime().toTime_t()); //时间戳
        qDebug()<<"addMessage" << info << time << ui->listWidget->count();
        dealMessageTime(time);
        QNChatMessage* messageW = new QNChatMessage(ui->listWidget->parentWidget(), ipAddress);
        QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
        dealMessage(messageW, item, info, time, QNChatMessage::User_Me);
//        else {
//            if(clientInfo != "") {
//                dealMessageTime(time);

//                QNChatMessage* messageW = new QNChatMessage(ui->listWidget->parentWidget());
//                QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
//                dealMessage(messageW, item, clientInfo, time, QNChatMessage::User_She);
//            }
//        }
        ui->listWidget->setCurrentRow(ui->listWidget->count()-1);
       emit sendInfoWidgetSignal(info);
    }
    else
    {
        ui->textEdit->append("无消息输入，传输消息无效！");
    }
}

//void Widget::addClientIPTGUISlot(QString CIP,int CPort, int ID,int state)
//{
//    /*判断连接的客户端数量，并根据连接的客户端信息进行界面更新*/
//    if((ID == 1)&&(state==QAbstractSocket::ConnectedState))
//    {
//        ui->LBDisplayIP1->setText(CIP);
//        ui->LBDisplayPort1->setText(QString::number(CPort));
//        ui->LBstate1->setText(tr("连接"));
//        ui->ClientPB1->setEnabled(true);
//    }
//    else if((ID == 2)&&(state==QAbstractSocket::ConnectedState))
//    {
//        ui->LBDisplayIP2->setText(CIP);
//        ui->LBDisplayPort2->setText(QString::number(CPort));
//        ui->LBstate2->setText(tr("连接"));
//        ui->ClientPB2->setEnabled(true);
//    }
//    else if((ID == 3)&&(state==QAbstractSocket::ConnectedState))
//    {
//        ui->LBDisplayIP3->setText(CIP);
//        ui->LBDisplayPort3->setText(QString::number(CPort));
//        ui->LBstate3->setText(tr("连接"));
//        ui->ClientPB3->setEnabled(true);
//    }
//}

void Widget::displayInfoTGUISlot(QString CIP,int CPort,int state,QDateTime current_time)
{
    QString current_date = current_time.toString("yyyy.MM.dd hh:mm:ss");
    /*判断连接的客户端数量，并根据连接的客户端信息进行界面更新*/
    if(state==QAbstractSocket::ConnectedState)
    {
        ui->textEdit->append(current_date + " —— IP: " + CIP + "  Port: " + QString::number(CPort) + "  客户端成功连接");
    }
    IPOrder.append(CIP);
    index++;
    QListWidgetItem * item = new QListWidgetItem(ui->clientListWidget);
    QSize size = item->sizeHint();
    item->setSizeHint(QSize(300, 50));
    ui->clientListWidget->addItem(item);
    clientAddress * client = new clientAddress(CIP, CPort);
    client->setParent(ui->clientListWidget);
    client->setSizeIncrement(size.width(), 56);
    ui->clientListWidget->setItemWidget(item, client);
    connect(client, &clientAddress::clientAddressSignal, this, &Widget::clientAddressSlot);
}

void Widget::clientAddressSlot(QString IP, int clientPort)
{
    int deleteIndex = IPOrder.indexOf(IP);
    closeFlag_current *= -1;
    QString current_date = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
    /*判断连接的客户端数量，并根据连接的客户端信息进行界面更新*/
    ui->textEdit->append(current_date + " —— IP: " + IP + "  Port: " + QString::number(clientPort) + "  服务器断开连接");
    emit clientAddressWidgetSignal(deleteIndex);
}

void Widget::sendStateDisplaySlot(QString msg, QString info)
{
    if(msg == "服务器开始发送消息")
    {
        /*消息框*/
//        QString time = QString::number(QDateTime::currentDateTime().toTime_t()); //时间戳
//        qDebug()<<"addMessage" << info << time << ui->listWidget->count();
//        dealMessageTime(time);
//        QNChatMessage* messageW = new QNChatMessage(ui->listWidget->parentWidget(), IPAddress);
//        QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
//        dealMessage(messageW, item, info, time, QNChatMessage::User_Me);
////        else {
////            if(clientInfo != "") {
////                dealMessageTime(time);

////                QNChatMessage* messageW = new QNChatMessage(ui->listWidget->parentWidget());
////                QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
////                dealMessage(messageW, item, clientInfo, time, QNChatMessage::User_She);
////            }
////        }
//        ui->listWidget->setCurrentRow(ui->listWidget->count()-1);
    }
    else if(msg == "服务器发送消息成功")
    {
        qDebug() << "6666666";
        /*暂停发送消息状态*/
        qDebug() << ui->listWidget->count();
        qDebug() << info;
        for(int i = ui->listWidget->count() - 1; i > 0; i--) {
            QNChatMessage* messageW = (QNChatMessage*)ui->listWidget->itemWidget(ui->listWidget->item(i));
            if(messageW->text() == info) {
                qDebug() << "7777777";
                messageW->setTextSuccess();
            }
        }

        QString current_date = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
        /*判断连接的客户端数量，并根据连接的客户端信息进行界面更新*/
        ui->textEdit->append(current_date + " —— " + msg);
    }
    else
    {
        QString current_date = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
        /*判断连接的客户端数量，并根据连接的客户端信息进行界面更新*/
        ui->textEdit->append(current_date + " —— " + msg);
    }
}

void Widget::sendInfoDisplaySlot(QString clientInfo, QString ip)
{

    /*消息框*/
    QString time = QString::number(QDateTime::currentDateTime().toTime_t()); //时间戳
    qDebug()<<"addMessage" << clientInfo << time << ui->listWidget->count();
    if(clientInfo != "") {
        dealMessageTime(time);

        QNChatMessage* messageW = new QNChatMessage(ui->listWidget->parentWidget(), ip);
        QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
        dealMessage(messageW, item, clientInfo, time, QNChatMessage::User_She);
    }

    ui->listWidget->setCurrentRow(ui->listWidget->count()-1);
    /*判断连接的客户端数量，并根据连接的客户端信息进行界面更新*/
    QString current_date = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
    ui->textEdit->append(current_date + " —— " + "服务器接收消息成功");
//    QMessageBox::about(NULL, "客户端消息", clientInfo);
}

/*深度优先递归遍历当前目录下文件夹和文件及子文件夹和文件*/
//void Widget::DfsFolder(std::string path,int layer)
//{
//    _finddata_t file_info;
//    std::string current_path=path+"/*.*"; //也可以用/*来匹配所有
//    int handle=_findfirst(current_path.c_str(),&file_info);
//    /*返回值为-1则查找失败*/
//    if(-1==handle)
//    {
//        std::cout<<"cannot match the path"<<endl;
//        return;
//    }
//    do
//    {
//        /*判断是否子目录*/
//        if(file_info.attrib==_A_SUBDIR)
//        {
//            if(!(strcmp(file_info.name,"."))||!(strcmp(file_info.name,"..")))
//                continue;
//            ISDir();
//            /*递归遍历子目录*/
//            strcpy(dirInfo.dirName,file_info.name);
//#ifdef __DEBUG__
//           qDebug()<<dirInfo.dirName<<endl;
//#endif
//            fwrite(&dirInfo,sizeof(struct DirInformation),1,fp);
//            int layer_tmp=layer;
//            if(strcmp(file_info.name,"..")!=0&&strcmp(file_info.name,".")!=0)  //.是当前目录，..是上层目录，必须排除掉这两种情况
//                DfsFolder(path+'/'+file_info.name,layer_tmp+1); //再windows下可以用\\转义分隔符，不推荐
//        }
//        else
//        {
//            ISFile();
//            strcpy(dirInfo.fileName,file_info.name);
//#ifdef __DEBUG__
//            std::cout<<dirInfo.fileName;
//#endif
//            fwrite(&dirInfo,sizeof(struct DirInformation),1,fp);
//        }
//    }while(!_findnext(handle,&file_info));  /*返回0则遍历完*/
//    /*关闭文件句柄*/
//    _findclose(handle);
//}

