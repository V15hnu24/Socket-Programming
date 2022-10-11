#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

long factorial(int n)
{
    if (n == 0){
        return 1;
    }
    return n * factorial(n - 1);
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
    

    FILE *dataDump_file = fopen("serverDump.txt", "r+");

    if(dataDump_file == NULL){
        perror("File open error");
    }
    printf("Txt File opened\n");

    //Initializing the current fd set
    fd_set current_fd_set, ready_socket;
    FD_ZERO(&current_fd_set);
    FD_SET(serverfile_d, &current_fd_set);
    int max_fd = FD_SETSIZE;

    while (1)
    {
        ready_socket = current_fd_set;
        if (select(max_fd, &ready_socket, NULL, NULL, NULL) < 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
        
        for (int i = 0; i < max_fd; i++)
        {
            if (FD_ISSET(i, &ready_socket))
            {   
                if (i == serverfile_d)
                {
                    //Accept
                    socket_n = accept(serverfile_d, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
                    if (socket_n<0)
                    {
                        perror("In Accept");
                        exit(EXIT_FAILURE);
                    }
                    printf("Connection accepted\n");
                    
                    fprintf(dataDump_file, "Client connected with client id: %d IP address: %s and port number: %d\n",addr.sin_addr.s_addr , inet_ntoa(addr.sin_addr), addr.sin_port);
                    rewind(dataDump_file);

                    FD_SET(socket_n, &current_fd_set);
                }else{
                    int recevd = 0;
                    int valread;

                    for (int i = 0; i < 20; i++)
                    {        
                        valread = read(socket_n, &recevd , sizeof(int));
                        printf("received %d In server from client id: %d IP address: %s and port number: %d\n", recevd,addr.sin_addr.s_addr , inet_ntoa(addr.sin_addr), addr.sin_port);
                        
                        long a = factorial(recevd);
                        long* temp = &a;
                        send(socket_n, temp, sizeof(long), 0);


                        fprintf(dataDump_file,"received %d In server from client id: %d IP address: %s and port number: %d\n", recevd,addr.sin_addr.s_addr , inet_ntoa(addr.sin_addr), addr.sin_port);
                        rewind(dataDump_file);

                        printf("sent %ld from server\n", *temp);
                    }
                    
                    // close(socket_n);
                    FD_CLR(i, &current_fd_set);
                }
                // fclose(dataDump_file);
            }
        }
    }

    
    shutdown(serverfile_d, SHUT_RDWR);

    return 0;

}
