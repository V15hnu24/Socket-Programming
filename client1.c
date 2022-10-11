#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(){
    int soc = 0, client_filed;
    struct sockaddr_in server_addr;
    
    //Creating socket in client side
    soc = socket(AF_INET, SOCK_STREAM, 0);
    if(soc <0){
        printf("\n Client socket creation eroor \n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4000);

    if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <0){
        printf("\n Address format not correct");
        return -1;
    }

    //Connect
    client_filed = connect(soc, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (client_filed < 0)
    {
        printf("\n Connection falied\n");
        return -1;
    }

    printf("Connected to server\n");

    int valread;
    long recevd = 0;
    int i;
    for (i = 1; i < 21; i++)
    {
        int* temp = &i;
        send(soc, temp, sizeof(int), 0);
        printf("sent %d\n", i);
        valread = read(soc, &recevd, sizeof(recevd));
        printf("received %ld\n", recevd);
    }
    
    close(client_filed);

}
