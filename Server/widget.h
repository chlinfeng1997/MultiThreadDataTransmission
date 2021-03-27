#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "myserver.h"
#include "debugonoroff.h"
#include "qnchatmessage.h"
class string;
class QEvent;
class Myserver;
class QListWidgetItem;
namespace Ui {
class Widget;
}


class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    /*重新实现事件过滤器函数*/
//    bool eventFilter(QObject *watched, QEvent *event);
    /*消息框*/
    void dealMessage(QNChatMessage *messageW, QListWidgetItem *item, QString text, QString time, QNChatMessage::User_Type type);
    void dealMessageTime(QString curMsgTime);
    /*获得发送文件列表*/
    void getSendFileList(QString path);
protected:
    void resizeEvent(QResizeEvent *event);
    /*深度优先递归遍历当前目录下文件夹和文件及子文件夹和文件*/
//    void DfsFolder(std::string path,int layer);
    /*层次遍历当前目录下的文件夹及子文件夹下的文件*/
//    bool TraverseFiles(std::string path,int &file_num);
signals:
    /*客户端主动断开连接信号*/
    void closeClientConnectSignal(int disconnectCommand);
    /*客户端被动断开连接信号*/
    void clientDiconnectSignal(int ID);
    /*获得文件列表信号*/
    void getFileListSignal(QString filelist);
    /*发送文件信号*/
    void sendFileWidgetSignal(QString filename);
    /*发送消息信号*/
    void sendInfoWidgetSignal(QString info);
    /*服务器发送断开信号*/
    void clientAddressWidgetSignal(int deleteIndex);
public slots:
    /*添加客户端连接信息到界面中*/
//    void addClientIPTGUISlot(QString CIP,int CPort,int ID,int state);
    /*客户端主动断开连接*/
    void closeClientConnectSlot(QString clientIp, int clientPort, QDateTime current_date_time);
    /*添加文件夹*/
    void addFilePBSlot();
    /*删除文件列表条目*/
    void deleteFile();
    /*显示日志信息*/
    void displayInfoTGUISlot(QString CIP,int CPort,int state,QDateTime current_time);
//    void deleteFileListItem();
//    /*List Widget双击事件槽函数*/
//    void doubleClickedItem();
    /*删除消息内容*/
    void deleteInfo();
    /*传输文件按钮槽函数*/
    void fileTranferButtonSlot();
    /*消息传输按钮槽函数*/
    void infoTranferButtonSlot();
    /*发送状态在GUI中显示*/
    void sendStateDisplaySlot(QString msg, QString info);
    /*发送消息在GUI中显示*/
    void sendInfoDisplaySlot(QString clientInfo, QString ip);
    /*服务器主动断开槽函数*/
    void clientAddressSlot(QString IP, int clientPort);
private:
    Ui::Widget *ui;
    /*断开客户端连接命令，1表示断开*/
    int closeCommand;
    /*存储添加的文件夹路径*/
    QString dirPath;
    /*文件列表指针*/
    FILE *fp;
    /*传输的消息内容*/
    QString info;
    /*本机IP*/
    QString ipAddress;
    QVector<QString> IPOrder; //记录listWidget_2里的客户端IP地址
    int index = 0; //记录listWidget_2当前索引
    /*断开连接 服务器 0 客户端 1*/
    int closeFlag_previous = 1;
    int closeFlag_current = 1;
    Myserver *server;
    /*目录列表存储结构体*/
    struct DirInformation{
        char fileName[260];           //文件名字
        char dirName[260];            //目录名字
        unsigned int isDirectory;  //是否是目录
        unsigned int isFile;       //是否是文件
        unsigned int topDir;       //是否是顶层目录
    };
    struct DirInformation dirInfo;
    /*是目录*/
    void ISDir()
    {
        dirInfo.topDir = 0;
        strcpy(dirInfo.fileName," ");
        dirInfo.isDirectory = 1;
        dirInfo.isFile = 0;
    }
    /*是文件*/
    void ISFile()
    {
        dirInfo.topDir = 0;
        strcpy(dirInfo.dirName," ");
        dirInfo.isDirectory = 0;
        dirInfo.isFile = 1;
    }

};

#endif // WIDGET_H
