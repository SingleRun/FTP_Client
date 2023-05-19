//client.h �����õ��ĳ������������������Լ�ʵ������������ftpPre()
#ifndef CLIENT_H
#define CLIENT_H
#include <iostream>
#include <vector>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

const int MSGSIZE(1024);    // ��Ϣ��������С
const int BUFSIZE(4096);   // �ļ���������С
class Client
{
private:
    std::string serverIp;   // ������IP��ַ
    unsigned short serverPort;  // ������FTP�˿�
    std::string username,password;  // �û���������
    int ret,len;    // ret���շ���ֵ��len�� sockaddr ��С
    WSADATA WSAData;    // �汾��Ϣ�ǵ���
    SOCKET sClient,dataSock,sListen; // sClient�ǿͻ���,dataSock�Ǵ����ݵ�
    struct sockaddr_in addrServer,data_addr;  // addrServer�����Ӷ˿ڣ�data_addr�����ݶ˿�
    char sendBuf[MSGSIZE],recvBuf[MSGSIZE]; // ���ͽ�����Ϣ������
    char sendFileBuf[BUFSIZE],recvFileBuf[BUFSIZE]; // ���ͽ����ļ�������
    int connectMode;//����ģʽ��1Ϊ����ģʽ��2Ϊ����ģʽ
    void ftpPre()
    {
        std::cout<<"ftp>";
    }
    int sendMessage(const char * message);  // ������Ϣ
    int sendMessage(const std::string message);  // ������Ϣ
    int recvMessage();  // ������Ϣ������
    int login();    // ��¼
    int sendFile(const std::string filePath); // �����ļ�
    int recvFile(const std::string filePath,const std::string savePath); // �����ļ�
    int parseDataPort();    //�ú������ڽ������յ������ݶ˿�
    void cmd(); //�ú������ڵȴ��û�����ftp����
    void listDir(); //����������Ŀ¼
    void quit();    //����quit����
    void cd(const std::string listPath);  //cd����
    int chooseFtpModeAndReturnPort();//ѡ��ftpģʽ
public:
    Client();    // ���캯��
    void setServerPort(int port);   //���÷����ftp�˿�
    int start();    // ����FTP�����
};
#endif