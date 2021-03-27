#include "mysocket.h"
#include <QDataStream>
#include "protocolcommand.h"
#include <QDir>
#include <QDirIterator>
Mysocket::Mysocket(int socket, int ID,QObject *parent)
    :QTcpSocket(parent), socketDescriptor(socket)
{
    socketID = ID;
    clearVariation();
    QObject::connect(this,&Mysocket::readyRead,this,&Mysocket::reveiveData);
}

Mysocket::~Mysocket()
{

}

void Mysocket::sendFile(QString path)
{
    /*初始化发送字节为0*/
    transferData.bytesWritten = 0;
    transferData.fileName = path;
    transferData.localFile = new QFile(path);
    if(!transferData.localFile->open(QFile::ReadOnly))
    {
        qDebug()<<tr("open file error!")<<endl;
        emit sendStateDisplaySignal("服务器发送文件失败", "");
        return;
    }
    transferData.totalBytes  = transferData.localFile->size();//获取文件大小

    QDataStream sendOut(&transferData.inOrOutBlock,QIODevice::WriteOnly);

    sendOut.setVersion(QDataStream::Qt_5_7); //设置版本
    /*获得文件名称*/
    QString currentFilename = transferData.fileName.right(transferData.fileName.size()
                                                -transferData.fileName.lastIndexOf('/')-1);

    /*保留总大小信息空间、命令、文件名大小信息空间、然后输入文件名*/
    sendOut << qint64(0) << qint64(0) << qint64(0)<< currentFilename;
    /*总的大小*/
    transferData.totalBytes += transferData.inOrOutBlock.size();

    sendOut.device()->seek(0);
    /*填充实际的存储空间*/
    sendOut << transferData.totalBytes<<_TRANSFER_FILE_
            <<qint64((transferData.inOrOutBlock.size()-(sizeof(qint64)*3)));

    qint64 sum = write(transferData.inOrOutBlock);

    transferData.bytesToWrite = transferData.totalBytes - sum;

    //transferData.inOrOutBlock.resize(0);
    /*表示数据没有发送完*/
    if(transferData.bytesToWrite>0)
    {
        /*发送文件内容部分*/
        transferData.inOrOutBlock = transferData.localFile->readAll();
        write(transferData.inOrOutBlock);
        transferData.inOrOutBlock.resize(0);
    }
    transferData.localFile->close();
    emit sendStateDisplaySignal("服务器发送文件成功", "");
}

void Mysocket::sendMSG(QString msg, QString IPAddress)
{
    emit sendStateDisplaySignal("服务器开始发送消息", msg);
    /*确保连接依然有效*/
    if(!isValid())
    {
        qDebug()<<"Already disconnected!";
        return;
    }
    qDebug() << msg;
    transferData.totalBytes = 0;
    /*使用数据流写入数据*/
    QDataStream outPut(&transferData.inOrOutBlock,QIODevice::WriteOnly);
    outPut.setVersion(QDataStream::Qt_5_7);
    transferData.totalBytes = msg.toUtf8().size();
//    qDebug() << transferData.totalBytes;

    /*保留总大小信息空间、命令、文件名大小信息空间、然后输入文件名*/
    outPut << qint64(0) << qint64(0) << qint64(0) << msg << IPAddress;
//    qDebug() << transferData.inOrOutBlock;
//    qDebug() << msg;
//    qDebug() << IPAddress << "6666";
    /*消息的总大小*/
    transferData.totalBytes += transferData.inOrOutBlock.size();
    /*定位到数据流的开始位置*/
    outPut.device()->seek(0);
    /*填充实际的存储空间*/
    outPut << transferData.totalBytes<<_TRANSFER_INFO_
            <<qint64((transferData.inOrOutBlock.size()-(sizeof(qint64)*3)));
//    qDebug() << transferData.inOrOutBlock;
    /*发送命令内容*/
    write(transferData.inOrOutBlock);
//    qDebug() << transferData.inOrOutBlock;
    transferData.inOrOutBlock.resize(0);
    emit sendStateDisplaySignal("服务器发送消息成功", msg);
//    transferData.bytesToWrite = transferData.totalBytes - sum;
//    if(transferData.bytesToWrite>0)
//    {
//        /*发送文件内容部分*/
//        transferData.inOrOutBlock = transferData.localFile->readAll();
//        write(transferData.inOrOutBlock);
//        transferData.inOrOutBlock.resize(0);
//    }
//    else
//    {
//        transferData.inOrOutBlock.resize(0);
//    }

}

void Mysocket::reveiveData()
{
    /*标志变量：下载标志变量  同步文件列表标志变量  传输文件标志变量*/
    int transferfileflag = 0;
    QString ip;
    qint32 temp;
    /*如果没有数据可读，就直接返回*/
    if(bytesAvailable()<=0)
    {
        return;
    }

    QDataStream in(this);
    in.setVersion(QDataStream::Qt_5_14);

    /*提取出总的大小、命令、文件名字的大小、文件名信息*/
    if(transferData.bytesReceived <= sizeof(qint64)*3)
    {

        if(bytesAvailable()>=sizeof(qint64)*3
                &&(transferData.fileNameSize==0))
        {
            /*从套接字中提取总大小信息、命令、文件名字大小*/
            in >> transferData.totalBytes >> transferData.command
               >> transferData.fileNameSize >> temp;
            transferData.bytesReceived += sizeof(qint64)*3;

        }
        if(bytesAvailable()>=transferData.fileNameSize
                &&transferData.fileNameSize!=0)
        {
            /*提取接收的文件名字*/
            in >> transferData.fileName >> ip;
            transferData.bytesReceived += transferData.fileNameSize;

        }
    }
    switch(transferData.command)
    {
        case _TRANSFER_FILE_ :
        {
            transferfileflag = 1;
            if(transferData.fileNameSize != 0)
            {
                QString tempfilename("/home/leo/Desktop/ReceiveFile/");
                tempfilename += transferData.fileName;
                /*创建本地文件*/
                transferData.localFile = new QFile(tempfilename);
                if(!transferData.localFile->open(QFile::WriteOnly))
                {
                    qDebug()<<"open local file error!";
                    return;
                }
                if(transferData.bytesReceived < transferData.totalBytes)
                {
                    transferData.bytesReceived += bytesAvailable();
                    transferData.inOrOutBlock = readAll();
                    transferData.localFile->write(transferData.inOrOutBlock);
                    transferData.inOrOutBlock.resize(0);
                }
                if(transferData.bytesReceived == transferData.totalBytes)
                {
                    clearVariation();
                    if(transferfileflag == 1)
                    {
                        transferfileflag = 0;
                        transferData.localFile->close();
                        qDebug()<<"Receive file success!";
                        emit sendStateDisplaySignal("服务器接收文件成功", "");
                    }
                }
            }
        }
        break;
        case _TRANSFER_INFO_ :
        {
            QString clientInfo = transferData.fileName;
    //        QString current_date = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
    //        /*判断连接的客户端数量，并根据连接的客户端信息进行界面更新*/
    //        ui->textEdit->append(current_date + " —— " + "客户端接收消息成功");
    //        QMessageBox::about(NULL, "服务器消息", serverInfo);
            emit sendInfoDisplaySignal(clientInfo, ip);
            transferData.bytesReceived = 0;
            transferData.fileNameSize = 0;
            transferData.inOrOutBlock.resize(0);
            qDebug()<<"Receive info success!";
        }
        break;
        default:
            qDebug()<<"Receive command nulity!";
        break;
    }
}

void Mysocket::clientDisconnectSlot()
{
    this->disconnectFromHost();
    this->close();
//    emit clientDisconnectSignal();
}

void Mysocket::clearVariation()
{
    transferData.totalBytes = 0;
    transferData.bytesReceived = 0;
    transferData.fileNameSize = 0;
    transferData.inOrOutBlock.resize(0);
    transferData.payloadSize = 1024 * 64; //每次发送64kb
    transferData.bytesWritten = 0;
    transferData.bytesToWrite = 0;
}

QString Mysocket::findDownloadFile(QString path, QString fileName)
{
    if(path.isEmpty()||fileName.isEmpty())
        return QString();
    QDir dir;
    QStringList filters;
    filters << fileName;
    dir.setPath(path);
    dir.setNameFilters(filters);
    QDirIterator iter(dir,QDirIterator::Subdirectories);
    while(iter.hasNext())
    {
        iter.next();
        QFileInfo info = iter.fileInfo();
        if(info.isFile())
        {
            return info.absoluteFilePath();
        }
    }
    return QString();
}
