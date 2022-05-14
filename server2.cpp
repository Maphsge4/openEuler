#include<netinet/in.h>   // sockaddr_in
#include<sys/types.h>    // socket
#include<sys/socket.h>   // socket
#include<arpa/inet.h>
#include<unistd.h>
#include<poll.h>     // poll 
#include<sys/ioctl.h>
#include<sys/time.h>
#include<iostream>
#include<vector>
#include<string>
#include<cstdlib>
#include<cstdio>
#include<cstring>
using namespace std;
#define BUFFER_SIZE 1024
#define MAX_FD 1000

struct PACKET_HEAD
{
    int length;
};

class Server
{
private:
    struct sockaddr_in server_addr;
    socklen_t server_addr_len;
    int listen_fd;    // 监听的fd
    struct pollfd fds[MAX_FD];    // fd数组，大小为1000
    int nfds;
public:
    Server(int port);
    ~Server();
    void Bind();
    void Listen(int queue_len = 20);
    void Accept();
    void Run();
    void Recv();
};

Server::Server(int port)
{
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(port);
    // create socket to listen
    listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0)
    {
        cout << "Create Socket Failed!";
        exit(1);
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

Server::~Server()
{
    for(int i=0; i<MAX_FD; ++i)
    {
        if(fds[i].fd >=0)
        {
            close(fds[i].fd);
        }
    }
}

void Server::Bind()
{
    if(-1 == (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))))
    {
        cout << "Server Bind Failed!";
        exit(1);
    }
    cout << "Bind Successfully.\n";
}

void Server::Listen(int queue_len)
{
    if(-1 == listen(listen_fd, queue_len))
    {
        cout << "Server Listen Failed!";
        exit(1);
    }
    cout << "Listen Successfully.\n";
}

void Server::Accept()
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int new_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if(new_fd < 0)
    {
        cout << "Server Accept Failed!";
        exit(1);
    }

    cout << "new connection was accepted.\n";
    // 将新建立的连接的fd加入fds[]
    int i;
    for(i=1; i<MAX_FD; ++i)
    {
        if(fds[i].fd < 0)
        {
            fds[i].fd = new_fd;
            break;
        }
    }
    // 超过最大连接数
    if(i == MAX_FD)
    {
        cout << "Too many clients.\n";
        exit(1);
    }

    fds[i].events = POLLIN;      // 设置新描述符的读事件
    nfds = i > nfds ? i : nfds;  // 更新连接数
}

void Server::Run()
{
    fds[0].fd = listen_fd;        // 添加监听描述符
    fds[0].events = POLLIN;
    nfds = 0;

    for(int i=1; i<MAX_FD; ++i)
        fds[i].fd = -1;

    while(1)
    {
        int nums = poll(fds, nfds+1, -1);
        if(nums < 0)
        {
            cout << "poll() error!";
            exit(1);
        }

        if(nums == 0)
        {
            continue;
        }

        if(fds[0].revents & POLLIN)
            Accept();   // 有新的客户端请求
        else
            Recv();
    }
}

void Server::Recv()
{
    for(int i=1; i<MAX_FD; ++i)
    {
        if(fds[i].fd < 0)
            continue;
        if(fds[i].revents & POLLIN)       // 读就绪
        {
            int fd = fds[i].fd;
            bool close_conn = false;  // 标记当前连接是否断开了

            PACKET_HEAD head;
            recv(fd, &head, sizeof(head), 0);   // 先接受包头，即数据总长度

            char* buffer = new char[head.length];
            bzero(buffer, head.length);
            int total = 0;
            while(total < head.length)
            {
                int len = recv(fd, buffer + total, head.length - total, 0);
                if(len < 0)
                {
                    cout << "recv() error!";
                    close_conn = true;
                    break;
                }
                total = total + len;
            }

            if(total == head.length)  // 将收到的消息原样发回给客户端
            {
                int ret1 = send(fd, &head, sizeof(head), 0);
                int ret2 = send(fd, buffer, head.length, 0);
                if(ret1 < 0 || ret2 < 0)
                {
                    cout << "send() error!";
                    close_conn = true;
                }
            }

            delete buffer;

            if(close_conn)  // 当前这个连接有问题，关闭它
            {
                close(fd);
                fds[i].fd = -1;
            }
        }
    }
}

int main()
{
    Server server(15000);
    server.Bind();
    server.Listen();
    server.Run();
    return 0;
}