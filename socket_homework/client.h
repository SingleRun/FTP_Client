//client.h 声明用到的常量，变量，方法，以及实现了内联方法ftpPre()
#ifndef CLIENT_H
#define CLIENT_H
#include <iostream>
#include <vector>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

const int MSGSIZE(1024);    // 信息缓冲区大小
const int BUFSIZE(4096);   // 文件缓冲区大小
class Client
{
private:
    std::string serverIp;   // 服务器IP地址
    unsigned short serverPort;  // 服务器FTP端口
    std::string username,password;  // 用户名，密码
    int ret,len;    // ret接收返回值，len是 sockaddr 大小
    WSADATA WSAData;    // 版本信息记得是
    SOCKET sClient,dataSock,sListen; // sClient是客户端,dataSock是传数据滴
    struct sockaddr_in addrServer,data_addr;  // addrServer是连接端口，data_addr是数据端口
    char sendBuf[MSGSIZE],recvBuf[MSGSIZE]; // 发送接收信息缓冲区
    char sendFileBuf[BUFSIZE],recvFileBuf[BUFSIZE]; // 发送接收文件缓冲区
    int connectMode;//连接模式，1为被动模式，2为主动模式
    void ftpPre()
    {
        std::cout<<"ftp>";
    }
    int sendMessage(const char * message);  // 发送信息
    int sendMessage(const std::string message);  // 发送信息
    int recvMessage();  // 接收信息并回显
    int login();    // 登录
    int sendFile(const std::string filePath); // 发送文件
    int recvFile(const std::string filePath,const std::string savePath); // 接收文件
    int parseDataPort();    //该函数用于解析接收到的数据端口
    void cmd(); //该函数用于等待用户输入ftp命令
    void listDir(); //遍历并回显目录
    void quit();    //发出quit命令
    void cd(const std::string listPath);  //cd命令
    int chooseFtpModeAndReturnPort();//选择ftp模式
public:
    Client();    // 构造函数
    void setServerPort(int port);   //设置服务端ftp端口
    int start();    // 连接FTP服务端
};
#endif