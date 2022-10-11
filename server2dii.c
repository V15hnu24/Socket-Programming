#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>

long factorial(int n)
{
    if (n == 0){
        return 1;
    }
    return n * factorial(n - 1);
}

void *connection_handler(void *socket_addr){

    int socket_n = *(int*)socket_addr;
    struct sockaddr_in addr = *(struct sockaddr_in*)socket_addr;

    FILE *dataDump_file = fopen("serverDump.txt", "a+");

    if(dataDump_file == NULL){
        perror("File open error");
    }

    // fprintf(dataDump_file, "Client connected with client id: %d IP address: %s and port number: %d\n",addr.sin_addr.s_addr , inet_ntoa(addr.sin_addr), addr.sin_port);
    // rewind(dataDump_file);
    
    // printf("Client connected with client id: %d IP address: %s and port number: %d\n",addr.sin_addr.s_addr , inet_ntoa(addr.sin_addr), addr.sin_port);

    int recevd = 0;
    int valread;

    for (int i = 0; i < 20; i++)
    {        
        valread = read(socket_n, &recevd , sizeof(int));
        // printf("received %d In server from client id: %d IP address: %s and port number: %d\n", recevd,addr.sin_addr.s_addr , inet_ntoa(addr.sin_addr), addr.sin_port);
        
        long a = factorial(recevd);
        long* temp = &a;
        send(socket_n, temp, sizeof(long), 0);

        // fprintf(dataDump_file,"received %d In server from client id: %d IP address: %s and port number: %d\n", recevd,addr.sin_addr.s_addr , inet_ntoa(addr.sin_addr), addr.sin_port);
        fprintf(dataDump_file,"sent %ld from server\n", *temp);
        printf("sent %ld from server\n", *temp);
    }
    rewind(dataDump_file);        
    fclose(dataDump_file);

    return 0;
}

int main(){
    int serverfile_d, socket_n;
    struct sockaddr_in server_addr;

    struct sockaddr_in addr;
    int norm = 1;
    int addrlen = sizeof(addr);

    // Creating a socket
    serverfile_d = socket(AF_INET, SOCK_STREAM, 0);
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
    if(listen(serverfile_d, 3) < 0){
        perror("In Listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening\n");

    //Initializing the current fd set
    nfds_t nfds = 0;
    struct pollfd *pollfds;
    int maxfds = 10, numfds = 0;

    if ((pollfds = malloc(maxfds * sizeof(struct pollfd))) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    maxfds = 10;

    pollfds[0].fd = serverfile_d;
    pollfds[0].events = POLLIN;
    pollfds->revents = 0;
    numfds = 1;

    int num =0;

    while (1)
    {
        nfds = numfds;
        if (poll(pollfds, nfds, -1) == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        for (int fd = 0; fd < (nfds+1); fd++)
        {
            if (pollfds[fd].revents == 0)
                continue;
            
            if (((pollfds + fd) -> revents & POLLIN) == POLLIN)
            {
                if (pollfds[fd].fd == serverfile_d)
                {
                    if ((socket_n = accept(serverfile_d, (struct sockaddr *)&addr, (socklen_t*)&addrlen)) < 0)
                    {
                        perror("In accept");
                        exit(EXIT_FAILURE);
                    }
                    printf("Client connected with client id: %d IP address: %s and port number: %d\n",addr.sin_addr.s_addr , inet_ntoa(addr.sin_addr), addr.sin_port);
                    if (numfds == maxfds) {
                        maxfds *= 2;
                        if ((pollfds = realloc(pollfds, maxfds * sizeof(struct pollfd))) == NULL) {
                            perror("realloc");
                            exit(EXIT_FAILURE);
                        }
                    }
                    pollfds[numfds].fd = socket_n;
                    pollfds[numfds].events = POLLIN;
                    pollfds[numfds].revents = 0;
                    numfds++;
                }
                else
                {
                    connection_handler(&addr);
                }
            }
            
        }
    }
    
    shutdown(serverfile_d, SHUT_RDWR);

    return 0;
}
