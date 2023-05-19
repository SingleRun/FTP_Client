//client.cpp 用于实现client.h中的方法
//本版本存在的问题：在调用函数后没有对其返回值做相应处理，对于用户输入也没有进行合法性检查。
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
    cout<<"请输入ftp服务端ip地址，按回车结束\n";
    cin>>serverIp;
    ret=WSAStartup(MAKEWORD(2,2),&WSAData);
    if(SOCKET_ERROR==ret)
    {
        cout<<"WSAStartup 错误"<<endl;
        return -1;
    }

    sClient=socket(AF_INET,SOCK_STREAM,0);
    if(INVALID_SOCKET==sClient)
    {
        cout<<"socket 错误"<<endl;
        WSACleanup();
        return -2;
    }

    addrServer.sin_addr.S_un.S_addr=inet_addr(serverIp.c_str());
    addrServer.sin_family=AF_INET;
    addrServer.sin_port=htons(serverPort);
    ret=connect(sClient,(struct sockaddr*)&addrServer,len);
    if(INVALID_SOCKET==ret)
    {
        cout<<"connect 错误";
        closesocket(sClient);
        WSACleanup();
        return -3;
    }
    else
    {
        cout<<"连接到服务器："<<inet_ntoa(addrServer.sin_addr)<<endl;  
    }
    // 此处接收欢迎信息
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
    // 等待输入ftp命令
    cin.get();
    while(true)
    {
        ftpPre();
        string allstr;
        string firststr;
        string secondstr;
        getline(cin,allstr);
        // 找到要切割的位置
        int pos = allstr.find(" ");
        // 切割出第一部分
        firststr=allstr.substr(0, pos);
        // 切割出第二部分
        secondstr= allstr.substr(pos+1);
        if(firststr=="put")
        {
            sendFile(secondstr);
        }
        else if (firststr=="get")
        {
            string savePath;
            cout<<"请输入存储位置，按回车结束\n";
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
            cout<<"dir\t\t列出FTP服务器目录\n";
            cout<<"cd listpath\t进入服务器子目录\n";
            cout<<"put filepath\t将本地文件放到服务器上\n";
            cout<<"get filepath\t从服务器上下载文件\n";
            cout<<"quit\t\t退出ftp连接\n";
        }
        else
        {
            // cout<<"ftp命令值为："<<firststr;
            cout<<"你输的啥子命令哦\n";
        } 
    }
}
int Client::sendMessage(const char * message)
{
    memset(sendBuf,0,MSGSIZE);
    strcpy(sendBuf,message);
    cout<<"senBuf内容为："<<sendBuf<<endl;
    ret=send(sClient,sendBuf,strlen(sendBuf)+1,0);
    if(SOCKET_ERROR==ret)
    {
        cout<<"send 错误";
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
        cout<<"send 错误";
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
        cout<<"recv 失败";
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

    // 发送登录请求
	string login_request = "USER " + username + "\r\n";
	send(sClient, login_request.c_str(), login_request.length(), 0);

	// 接收服务器的响应
	recv(sClient, recvBuf, sizeof(recvBuf), 0);

	// 判断登录是否成功
	if (strstr(recvBuf, "331") != NULL)
	{
		// 发送密码
		string password_request = "PASS " + password + "\r\n";
		send(sClient, password_request.c_str(), password_request.length(), 0);
		// 接收服务器的响应
		recv(sClient, recvBuf, sizeof(recvBuf), 0);

		// 判断密码是否正确
		if (strstr(recvBuf, "230") != NULL)
		{
			cout<<"登录成功\n";
            return 0;
		}
		else
		{
            cout<<"登录失败\n";
			return -1;
		}
	}
	else
	{
        cout<<"登录失败\n";
		return -1;
	}
}
int Client::parseDataPort()
{
    // 查找数据端口的字符串
    char *pos = strstr(recvBuf, "(");
    if (pos == NULL)    return -1;
    char *port_str = strtok(pos + 1, ",");
    for (int i = 0; i < 4; i++)
        port_str = strtok(NULL, ",");

    // 计算端口号(倒数第二位*256+最后一位)
    int port = (atoi(port_str) << 8) + atoi(strtok(NULL, ","));

    return port;
}
int Client::recvFile(const string fileName,const string savePath)
{
    int port=chooseFtpModeAndReturnPort();
    // cout<<"端口为："<<port<<endl;
    //不同连接模式处理不同
    if(connectMode==1)
    {
        // 发送下载请求
        string download_request = "RETR " + fileName + "\r\n";
        sendMessage(download_request);

        // 接收服务器的响应
        recvMessage();
    }
    else if(connectMode==2)
    {
         // 发送下载请求
        string download_request = "RETR " + fileName + "\r\n";
        sendMessage(download_request);
        dataSock=accept(sListen,(sockaddr *)&data_addr,&len);
        // 接收服务器的响应
        recvMessage();
    }
    // 判断是否可以开始传输数据
    if (strstr(recvBuf, "150") != NULL)
    {
        // 创建文件并打开
        std::ofstream file(savePath, std::ios::binary);
        // 接收文件内容
        int length;
        while (length=recv(dataSock, recvFileBuf, BUFSIZE, 0))
        {
            file.write(recvFileBuf, length);
        }             
        // 关闭文件
        file.close();
        //关闭监听端口连接
        closesocket(sListen);
        // 关闭数据端口连接
        closesocket(dataSock);
    }
    else
    {
        cout<<"请检查是否获得下载该文件的权限\n";
        return -1;
    }
    
    // 接收服务器的响应
    memset(recvBuf,0,MSGSIZE);
    recv(sClient,recvBuf,MSGSIZE,0);
    // 判断下载是否成功
    cout<<recvBuf<<endl;
    if (strstr(recvBuf, "226") != NULL)
    {
        cout<<"下载成功\n";
        return 0;
    }
    else
    {
        cout<<"下载失败\n";
        return -1;
    }
}
int Client::sendFile(const string filePath)
{
    // 查找最后一个""的位置
    string fileName;
    size_t pos = filePath.find_last_of("\\");
    if (pos != string::npos)
    {
    // 取出文件名
     fileName= filePath.substr(pos + 1);
    }

    int port=chooseFtpModeAndReturnPort();
     // 设置上传模式为二进制
    string mode_request = "TYPE I\r\n";
    sendMessage(mode_request);
    if(connectMode==1)
    {
        // 发送上传请求
        string upload_request = "STOR " + fileName + "\r\n";
        sendMessage(upload_request);
        // 我也不知道为什么要发两次，但是第一次服务器返回了空值给我
        recvMessage();
        recvMessage();
    }
    else if(connectMode==2)
    {
        // 发送上传请求
        string upload_request = "STOR " + fileName + "\r\n";
        sendMessage(upload_request);
        dataSock=accept(sListen,(sockaddr *)&data_addr,&len);
        // 接收服务器的响应
        recvMessage();
        recvMessage();
    }

    // 读取文件内容
    std::ifstream file(filePath, std::ios::binary);
    while (file.read(sendFileBuf, BUFSIZE))
    {
        // 向服务器发送文件内容
        send(dataSock, sendFileBuf, BUFSIZE, 0);
    }

    // 关闭文件
    file.close();
    //关闭监听端口连接
    closesocket(sListen);
    // 关闭数据端口连接
    closesocket(dataSock);

    // 接收服务器的响应
    recvMessage();

    // 判断上传是否成功    
    if (strstr(recvBuf, "226") != NULL)
    {
        cout<<"上传成功\n";
        return 0;
    }
    else
    {
        cout<<"上传失败\n";
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
        // 接收服务器的响应
        recvMessage();
    }
    memset(recvFileBuf,0,MSGSIZE);
    recv(dataSock, recvFileBuf, BUFSIZE, 0);
    cout<<recvFileBuf<<endl;
    recvMessage();
    // 关闭数据端口连接
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
    cout<<"请输入序号，并按回车以结束输入\n";
    cout<<"1.被动模式   2.主动模式\n";
    int choose;
    cin>>choose;    
    //注意这里要清空缓冲区，不然回车键会在下一轮cmd()中被推出缓冲区
    cin.get();
    connectMode=choose;
    //注意choose==1时会进行connect操作，而choose==2时不会进行accept操作。
    //这么设计的原因是主动模式下，得发完将进行的操作命令再连接数据端口.例如要发完"LIST\r\n"才能进行accept
    if(choose==1)
    {
        int port;
        // 设置传输模式为被动模式
        string passive_request = "PASV\r\n";
        sendMessage(passive_request);
        // 接收服务器的响应，并解析出数据端口
        recvMessage();
        port=parseDataPort();
        // 连接数据端口
        data_addr.sin_family = AF_INET;
        data_addr.sin_addr.s_addr = inet_addr(serverIp.c_str());
        data_addr.sin_port = htons(port);
        dataSock = socket(AF_INET, SOCK_STREAM, 0);
        ret=connect(dataSock, (struct sockaddr*)&data_addr, sizeof(data_addr));
        return port;
    }
    else if(choose==2)
    {
        cout<<"请输入数据端口，按回车结束\n";
        int port;
        cin>>port;
        //注意这里要清空缓冲区，不然回车键会在下一轮cmd()中被推出缓冲区
        cin.get();
        //监听port端口
        sListen=socket(AF_INET,SOCK_STREAM,0);
        if(INVALID_SOCKET==sListen)
        {
            cout<<"socket 错误"<<endl;
            WSACleanup();
            return -2;
        }
        //获取客户端ip地址
        sockaddr_in clientAddr;
        getsockname(sClient,(struct sockaddr*)&clientAddr,&len);
        char *strClientAddr=inet_ntoa(clientAddr.sin_addr);
        //填充sockaddr_in
        data_addr.sin_family = AF_INET;
        data_addr.sin_port = htons(port);
        data_addr.sin_addr.s_addr = inet_addr(strClientAddr);
        ret=bind(sListen,(sockaddr *)&data_addr,len);
        if(INVALID_SOCKET==ret)
        {
            cout<<"bind 错误"<<endl;
            closesocket(sListen);
            WSACleanup();
            return -3;
        }
        int backlog=0;
        ret=listen(sListen,backlog);
        if(INVALID_SOCKET==ret)
        {
            cout<<"listen 错误"<<endl;
            closesocket(sListen);
            WSACleanup();
            return -4;
        }
        //拼接PORT命令
        string proactive="PORT ";

        //转换客户端ip地址格式：ipClient="172,24,27,100,"
        string ipClient="";
        char *p = strtok(strClientAddr, ".");
        while (p != NULL) 
        {
            ipClient+=(string(p)+",");
            p = strtok(NULL, ".");
        }
        //计算客户端端口
        proactive+=ipClient;
        char temp[100];
        itoa(port/256,temp,10);
        proactive+=temp;
        proactive+=",";
        itoa(port%256,temp,10);
        proactive+=temp;
        proactive+="\r\n";
        //发送PORT命令并接收响应回显
        sendMessage(proactive);
        recvMessage();
        return port;
    }
    else
    {
        return chooseFtpModeAndReturnPort();
    }
}