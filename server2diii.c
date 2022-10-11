#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <sys/epoll.h>

long factorial(int n)
{
    if (n == 0){
        return 1;
    }
    return n * factorial(n - 1);
}

int main(){
    int databuffer = 1000;
    int serverfile_d, socket_n;
    struct sockaddr_in server_addr;

    struct sockaddr_in addr;
    int norm = 1;
    int addrlen = sizeof(addr);

    // Creating a socket
    serverfile_d = socket(AF_INET,SOCK_DGRAM, 0);
    if(serverfile_d<0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Atttaching a port for the socket
    if(setsockopt(serverfile_d, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &norm, sizeof(norm))){
        perror("setsocketopt");
        exit(EXIT_FAILURE);
    }

    // Setting the ip format as IPV4
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(4000);

    //Binding the socket to the port
    if(bind(serverfile_d, (struct sockaddr*)&server_addr,sizeof(server_addr))<0){
        perror("binding failed");
        exit(EXIT_FAILURE);
    }

    //listen
    // if(listen(serverfile_d, 3) < 0){
    //     perror("In Listen");
    //     exit(EXIT_FAILURE);
    // }

    printf("Server is listening\n");

    int nfds = 50, num_open_fds = nfds;
    struct epoll_event pollfd;
    struct epoll_event *pollfds;

    int efd = epoll_create1(0);
    pollfd.data.fd = serverfile_d; 
    pollfd.events = EPOLLIN;
    int s = epoll_ctl(efd, EPOLL_CTL_ADD, serverfile_d, &pollfd);
    if(s<0){
        perror("epoll_ctl error");
        exit(EXIT_FAILURE);
    }
    int max_connections = 10;
    int num =0;

    pollfds = (struct epoll_event*)calloc(max_connections, sizeof(pollfd));

    for (int i = 0; i < max_connections; i++)
    {
        pollfds[i].data.fd = 0;
        pollfds[i].events = EPOLLIN;
    }
    
    char buffer[databuffer];
    while(1){
        int val = epoll_wait(efd, pollfds, max_connections, -1);
        if(val<0){
            perror("epoll_wait error");
            exit(EXIT_FAILURE);
        }

        for (int  i = 0; i < val; i++)
        {
            for (int j = 0; j < 20; j++)
            {
                int fd = pollfds[i].data.fd;
                printf("returned fd is %d\n", fd);
                int recev = 0;
                // memset(buffer,0, sizeof(buffer));
                int buffer_size = read(pollfds[i].data.fd, &recev, sizeof(int));

                if(buffer_size<0){
                    pollfds[i].data.fd = 0;
                    pollfds[i].events = 0;
                    num--;
                    perror("read error");
                    exit(EXIT_FAILURE);
                }
                else if(buffer_size == 0){
                    pollfds[i].data.fd = 0;
                    pollfds[i].events = 0;
                    num--;
                    printf("Client disconnected\n");
                }
                else{
                    printf("Received %d\n", recev);
                    long a = factorial(recev);
                    long* temp = &a;
                    printf("Sending %ld\n", *temp);
                    send(pollfds[i].data.fd, temp, sizeof(temp), 0);
                }    
            }
        }   
    }
    for (int i = 0; i < max_connections; i++)
    {
        close(pollfds[i].data.fd);
    }
    free(pollfds);
    close(serverfile_d);
    
    return 0;
}