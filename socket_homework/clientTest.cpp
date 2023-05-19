//clientTest.cpp 用于启动ftp客户端服务
//直接引入client.cpp而不是引入client.h并不是一个好习惯，但在vs2010中我不知道该怎么做
#include "client.cpp"
int main()
{
    Client client=Client();
    // "172.24.27.1"
    client.start();
    return 0;
}