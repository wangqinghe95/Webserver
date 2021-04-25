#include<iostream>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/select.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<cstring>

using namespace std;

#define MAXLINE 80
#define SERV_PORT 6666

int main(){
    // struct sockaddr_in server_addr, client_addr;
    // int listen_fd, connect_fd;
    char buf[MAXLINE] = {0};
    char str[INET_ADDRSTRLEN] = {0};

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);    //监听的文件描述符
    int connect_fd;
    struct sockaddr_in server_addr, client_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    if(bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        cout << "bind error" << endl;
        close(listen_fd);
        return -1;
    }

    listen(listen_fd, 20);

    cout << "Accepting connection ... \n" << endl;

    fd_set read_set;
    fd_set test_set;
    FD_ZERO(&read_set);
    FD_SET(listen_fd, &read_set);

    int nRead = 0;
    while (true)
    {
        test_set = read_set;
        int ret = select(FD_SETSIZE, &test_set, NULL,NULL,NULL);
        if (ret < 0){
            cout << "select error" << endl;
            close(listen_fd);
            return -1;
        }
        else if (ret > 0)
        {
            for(int fd = 0; fd < FD_SETSIZE; ++fd){
                
                if(FD_ISSET(fd, &test_set)){
                    if (fd == listen_fd){
                        int sock = accept(listen_fd, NULL, NULL);
                        FD_SET(sock,&read_set);
                    }`
                    else
                    {
                        cout << fd << endl;
                        nRead = read(fd, buf, MAXLINE);
                        if (nRead == 0){
                            cout << "222" << endl;
                            close(fd);
                            FD_CLR(fd, &read_set);
                            cout << "client has removed " << fd << endl;                           
                        }
                        else
                        {
                            char buf[128] = {0};
                            recv(fd, buf, 128, 0);
                            cout << "buf = " << buf << endl;
                            send(fd, buf, strlen(buf)+1, 0);
                        }
                        
                    }
                    
                }
            }
        }
        //     if (FD_ISSET(listen_fd, &test_set)){
        //         socklen_t client_addr_len = sizeof(client_addr);
        //         connect_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        //         printf("received from %s at PORT %d\n",
        //                 inet_ntop(AF_INET, &client_addr.sin_addr, str, sizeof(str)),
        //                 ntohs(client_addr.sin_port));
        //         FD_SET(connect_fd, &read_set); 
        //     }
        //     else
        //     {            
        //         for(int fd = 0; fd < FD_SETSIZE; ++fd){                    
        //             if (fd != listen_fd){
        //                 continue;
        //             }
        //             else
        //             {
                        
        //                 if (FD_ISSET(fd, &test_set)){
        //                     cout << fd << endl;
        //                     cout << "000" << endl;
        //                     nRead = read(connect_fd, buf, MAXLINE);
        //                     if ( nRead== 0){
        //                         cout << "222" << endl;
        //                         close(fd);
        //                         FD_CLR(fd, &read_set);
        //                         cout << "client has removed " << fd << endl;
        //                     }
        //                     else{
        //                         cout << "111" << endl;
        //                         printf("buf = %s\n", buf);
        //                         cout << buf << endl;
        //                         for(int j = 0; j < nRead; ++j){
        //                             buf[j] = toupper(buf[j]);
        //                         }
        //                         write(fd, buf, nRead);
        //                     }
        //                 }
        //             }
                    
        //         } 
 
  
        //     }
        // }
        
    }
    close(listen_fd);
    return 0;    
}