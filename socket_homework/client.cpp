//client.cpp ����ʵ��client.h�еķ���
//���汾���ڵ����⣺�ڵ��ú�����û�ж��䷵��ֵ����Ӧ���������û�����Ҳû�н��кϷ��Լ�顣
#include<iostream>
#include<vector>
#include<fstream>
#include<string>
#include<conio.h>
#include"client.h"
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
Client::Client()
{
    serverPort=21;
    len=sizeof(sockaddr);
}
void Client::setServerPort(int port)
{
    serverPort=port;
}
int Client::start()
{
    cout<<"������ftp�����ip��ַ�����س�����\n";
    cin>>serverIp;
    ret=WSAStartup(MAKEWORD(2,2),&WSAData);
    if(SOCKET_ERROR==ret)
    {
        cout<<"WSAStartup ����"<<endl;
        return -1;
    }

    sClient=socket(AF_INET,SOCK_STREAM,0);
    if(INVALID_SOCKET==sClient)
    {
        cout<<"socket ����"<<endl;
        WSACleanup();
        return -2;
    }

    addrServer.sin_addr.S_un.S_addr=inet_addr(serverIp.c_str());
    addrServer.sin_family=AF_INET;
    addrServer.sin_port=htons(serverPort);
    ret=connect(sClient,(struct sockaddr*)&addrServer,len);
    if(INVALID_SOCKET==ret)
    {
        cout<<"connect ����";
        closesocket(sClient);
        WSACleanup();
        return -3;
    }
    else
    {
        cout<<"���ӵ���������"<<inet_ntoa(addrServer.sin_addr)<<endl;  
    }
    // �˴����ջ�ӭ��Ϣ
    recvMessage();
    while(login());
    cmd();
    // recvFile("1.jpg","E:\\1.jpg");
    // sendFile("E:\\1.jpg");

    closesocket(sClient);
    WSACleanup();
    return 0;
}
void Client::cmd()
{
    // �ȴ�����ftp����
    cin.get();
    while(true)
    {
        ftpPre();
        string allstr;
        string firststr;
        string secondstr;
        getline(cin,allstr);
        // �ҵ�Ҫ�и��λ��
        int pos = allstr.find(" ");
        // �и����һ����
        firststr=allstr.substr(0, pos);
        // �и���ڶ�����
        secondstr= allstr.substr(pos+1);
        if(firststr=="put")
        {
            sendFile(secondstr);
        }
        else if (firststr=="get")
        {
            string savePath;
            cout<<"������洢λ�ã����س�����\n";
            cin>>savePath;
            cin.get();
            recvFile(secondstr,savePath);
        }
        else if (firststr=="dir"||firststr=="ls")
        {
            listDir();
        }
        else if(firststr=="quit"||firststr=="bye")
        {
            quit();
            break;
        }
        else if(firststr=="cd")
        {
            cd(secondstr);
        }
        else if (firststr=="help")
        {
            cout<<"dir\t\t�г�FTP������Ŀ¼\n";
            cout<<"cd listpath\t�����������Ŀ¼\n";
            cout<<"put filepath\t�������ļ��ŵ���������\n";
            cout<<"get filepath\t�ӷ������������ļ�\n";
            cout<<"quit\t\t�˳�ftp����\n";
        }
        else
        {
            // cout<<"ftp����ֵΪ��"<<firststr;
            cout<<"�����ɶ������Ŷ\n";
        } 
    }
}
int Client::sendMessage(const char * message)
{
    memset(sendBuf,0,MSGSIZE);
    strcpy(sendBuf,message);
    cout<<"senBuf����Ϊ��"<<sendBuf<<endl;
    ret=send(sClient,sendBuf,strlen(sendBuf)+1,0);
    if(SOCKET_ERROR==ret)
    {
        cout<<"send ����";
        closesocket(sClient);
        WSACleanup();
        return -4;
    } 
    return 0;
}
int Client::sendMessage(const string message) 
{
    ret=send(sClient,message.c_str(),message.length(),0);
    if(SOCKET_ERROR==ret)
    {
        cout<<"send ����";
        closesocket(sClient);
        WSACleanup();
        return -4;
    } 
    return 0;
}
int Client::recvMessage()
{
    memset(recvBuf,0,MSGSIZE);
    ret=recv(sClient,recvBuf,MSGSIZE,0);
    if(SOCKET_ERROR==ret)
    {
        cout<<"recv ʧ��";
        closesocket(sClient);
        WSACleanup();
        exit(1);
        return -5;
    }
    else
    {
        cout<<recvBuf<<endl; 
        return 0;
    }
}
int Client::login()
{
    cout<<"Username:";
	cin>>username;
	cout<<"\nPassword:";
	// cin>>password;
    password="";
    char ch;
    while((ch=_getch())!='\r')
    {
        password+=ch;
        cout<<'*';
    }
    cout<<'\n';

    // ���͵�¼����
	string login_request = "USER " + username + "\r\n";
	send(sClient, login_request.c_str(), login_request.length(), 0);

	// ���շ���������Ӧ
	recv(sClient, recvBuf, sizeof(recvBuf), 0);

	// �жϵ�¼�Ƿ�ɹ�
	if (strstr(recvBuf, "331") != NULL)
	{
		// ��������
		string password_request = "PASS " + password + "\r\n";
		send(sClient, password_request.c_str(), password_request.length(), 0);
		// ���շ���������Ӧ
		recv(sClient, recvBuf, sizeof(recvBuf), 0);

		// �ж������Ƿ���ȷ
		if (strstr(recvBuf, "230") != NULL)
		{
			cout<<"��¼�ɹ�\n";
            return 0;
		}
		else
		{
            cout<<"��¼ʧ��\n";
			return -1;
		}
	}
	else
	{
        cout<<"��¼ʧ��\n";
		return -1;
	}
}
int Client::parseDataPort()
{
    // �������ݶ˿ڵ��ַ���
    char *pos = strstr(recvBuf, "(");
    if (pos == NULL)    return -1;
    char *port_str = strtok(pos + 1, ",");
    for (int i = 0; i < 4; i++)
        port_str = strtok(NULL, ",");

    // ����˿ں�(�����ڶ�λ*256+���һλ)
    int port = (atoi(port_str) << 8) + atoi(strtok(NULL, ","));

    return port;
}
int Client::recvFile(const string fileName,const string savePath)
{
    int port=chooseFtpModeAndReturnPort();
    // cout<<"�˿�Ϊ��"<<port<<endl;
    //��ͬ����ģʽ����ͬ
    if(connectMode==1)
    {
        // ������������
        string download_request = "RETR " + fileName + "\r\n";
        sendMessage(download_request);

        // ���շ���������Ӧ
        recvMessage();
    }
    else if(connectMode==2)
    {
         // ������������
        string download_request = "RETR " + fileName + "\r\n";
        sendMessage(download_request);
        dataSock=accept(sListen,(sockaddr *)&data_addr,&len);
        // ���շ���������Ӧ
        recvMessage();
    }
    // �ж��Ƿ���Կ�ʼ��������
    if (strstr(recvBuf, "150") != NULL)
    {
        // �����ļ�����
        std::ofstream file(savePath, std::ios::binary);
        // �����ļ�����
        int length;
        while (length=recv(dataSock, recvFileBuf, BUFSIZE, 0))
        {
            file.write(recvFileBuf, length);
        }             
        // �ر��ļ�
        file.close();
        //�رռ����˿�����
        closesocket(sListen);
        // �ر����ݶ˿�����
        closesocket(dataSock);
    }
    else
    {
        cout<<"�����Ƿ������ظ��ļ���Ȩ��\n";
        return -1;
    }
    
    // ���շ���������Ӧ
    memset(recvBuf,0,MSGSIZE);
    recv(sClient,recvBuf,MSGSIZE,0);
    // �ж������Ƿ�ɹ�
    cout<<recvBuf<<endl;
    if (strstr(recvBuf, "226") != NULL)
    {
        cout<<"���سɹ�\n";
        return 0;
    }
    else
    {
        cout<<"����ʧ��\n";
        return -1;
    }
}
int Client::sendFile(const string filePath)
{
    // �������һ��""��λ��
    string fileName;
    size_t pos = filePath.find_last_of("\\");
    if (pos != string::npos)
    {
    // ȡ���ļ���
     fileName= filePath.substr(pos + 1);
    }

    int port=chooseFtpModeAndReturnPort();
     // �����ϴ�ģʽΪ������
    string mode_request = "TYPE I\r\n";
    sendMessage(mode_request);
    if(connectMode==1)
    {
        // �����ϴ�����
        string upload_request = "STOR " + fileName + "\r\n";
        sendMessage(upload_request);
        // ��Ҳ��֪��ΪʲôҪ�����Σ����ǵ�һ�η����������˿�ֵ����
        recvMessage();
        recvMessage();
    }
    else if(connectMode==2)
    {
        // �����ϴ�����
        string upload_request = "STOR " + fileName + "\r\n";
        sendMessage(upload_request);
        dataSock=accept(sListen,(sockaddr *)&data_addr,&len);
        // ���շ���������Ӧ
        recvMessage();
        recvMessage();
    }

    // ��ȡ�ļ�����
    std::ifstream file(filePath, std::ios::binary);
    while (file.read(sendFileBuf, BUFSIZE))
    {
        // ������������ļ�����
        send(dataSock, sendFileBuf, BUFSIZE, 0);
    }

    // �ر��ļ�
    file.close();
    //�رռ����˿�����
    closesocket(sListen);
    // �ر����ݶ˿�����
    closesocket(dataSock);

    // ���շ���������Ӧ
    recvMessage();

    // �ж��ϴ��Ƿ�ɹ�    
    if (strstr(recvBuf, "226") != NULL)
    {
        cout<<"�ϴ��ɹ�\n";
        return 0;
    }
    else
    {
        cout<<"�ϴ�ʧ��\n";
        return -1;
    }
}
void Client::listDir()
{
    int port=chooseFtpModeAndReturnPort();
    if(connectMode==1)
    {  
        sendMessage(string("LIST\r\n"));
        recvMessage();
    }
    else if(connectMode==2)
    {
        sendMessage(string("LIST\r\n"));
        dataSock=accept(sListen,(sockaddr *)&data_addr,&len);
        // ���շ���������Ӧ
        recvMessage();
    }
    memset(recvFileBuf,0,MSGSIZE);
    recv(dataSock, recvFileBuf, BUFSIZE, 0);
    cout<<recvFileBuf<<endl;
    recvMessage();
    // �ر����ݶ˿�����
    closesocket(dataSock);
}
void Client::quit()
{
    sendMessage("QUIT\r\n");
    recvMessage();
}
void Client::cd(const string listPath)
{
    sendMessage(string("CWD ")+listPath+string("\r\n"));
    recvMessage();
}
int Client::chooseFtpModeAndReturnPort()
{
    cout<<"��������ţ������س��Խ�������\n";
    cout<<"1.����ģʽ   2.����ģʽ\n";
    int choose;
    cin>>choose;    
    //ע������Ҫ��ջ���������Ȼ�س���������һ��cmd()�б��Ƴ�������
    cin.get();
    connectMode=choose;
    //ע��choose==1ʱ�����connect��������choose==2ʱ�������accept������
    //��ô��Ƶ�ԭ��������ģʽ�£��÷��꽫���еĲ����������������ݶ˿�.����Ҫ����"LIST\r\n"���ܽ���accept
    if(choose==1)
    {
        int port;
        // ���ô���ģʽΪ����ģʽ
        string passive_request = "PASV\r\n";
        sendMessage(passive_request);
        // ���շ���������Ӧ�������������ݶ˿�
        recvMessage();
        port=parseDataPort();
        // �������ݶ˿�
        data_addr.sin_family = AF_INET;
        data_addr.sin_addr.s_addr = inet_addr(serverIp.c_str());
        data_addr.sin_port = htons(port);
        dataSock = socket(AF_INET, SOCK_STREAM, 0);
        ret=connect(dataSock, (struct sockaddr*)&data_addr, sizeof(data_addr));
        return port;
    }
    else if(choose==2)
    {
        cout<<"���������ݶ˿ڣ����س�����\n";
        int port;
        cin>>port;
        //ע������Ҫ��ջ���������Ȼ�س���������һ��cmd()�б��Ƴ�������
        cin.get();
        //����port�˿�
        sListen=socket(AF_INET,SOCK_STREAM,0);
        if(INVALID_SOCKET==sListen)
        {
            cout<<"socket ����"<<endl;
            WSACleanup();
            return -2;
        }
        //��ȡ�ͻ���ip��ַ
        sockaddr_in clientAddr;
        getsockname(sClient,(struct sockaddr*)&clientAddr,&len);
        char *strClientAddr=inet_ntoa(clientAddr.sin_addr);
        //���sockaddr_in
        data_addr.sin_family = AF_INET;
        data_addr.sin_port = htons(port);
        data_addr.sin_addr.s_addr = inet_addr(strClientAddr);
        ret=bind(sListen,(sockaddr *)&data_addr,len);
        if(INVALID_SOCKET==ret)
        {
            cout<<"bind ����"<<endl;
            closesocket(sListen);
            WSACleanup();
            return -3;
        }
        int backlog=0;
        ret=listen(sListen,backlog);
        if(INVALID_SOCKET==ret)
        {
            cout<<"listen ����"<<endl;
            closesocket(sListen);
            WSACleanup();
            return -4;
        }
        //ƴ��PORT����
        string proactive="PORT ";

        //ת���ͻ���ip��ַ��ʽ��ipClient="172,24,27,100,"
        string ipClient="";
        char *p = strtok(strClientAddr, ".");
        while (p != NULL) 
        {
            ipClient+=(string(p)+",");
            p = strtok(NULL, ".");
        }
        //����ͻ��˶˿�
        proactive+=ipClient;
        char temp[100];
        itoa(port/256,temp,10);
        proactive+=temp;
        proactive+=",";
        itoa(port%256,temp,10);
        proactive+=temp;
        proactive+="\r\n";
        //����PORT���������Ӧ����
        sendMessage(proactive);
        recvMessage();
        return port;
    }
    else
    {
        return chooseFtpModeAndReturnPort();
    }
}