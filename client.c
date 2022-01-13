#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#define PORT 8080

int createClient()
{
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    return sock;
}

int lookupForServer(int *sock, struct sockaddr_in serv_addr)
{
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(*sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    return 0;
}

void writeToServer(char *buffer, int sock)
{
    int sizeOf = strlen(buffer);
    send(sock, &sizeOf, sizeof(int), 0);
    send(sock, buffer, sizeOf, 0);
}

void readFromServer(char *buffer, int sock)
{
    int sizeOf = 0;
    recv(sock, &sizeOf, sizeof(int), 0);
    recv(sock, buffer, sizeOf, 0);
}

int checkIfFirstLettersAreSame(char *word, char *firsts)
{
    if (strlen(firsts) == 1)
    {
        if (word[0] == firsts[0])
        {
            return 1;
        }
    }
    if (strlen(firsts) == 2)
    {
        if (word[0] == firsts[0] && word[1] == firsts[1])
        {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in serv_addr;
    int sock = createClient();
    int try = lookupForServer(&sock, serv_addr);

    while (try == -1)
    {
        printf("\nTry connecting again?[y/n]\t");
        char ans[34];
        scanf("%s", ans);
        if (ans[0] == 'y' || ans[0] == 'Y')
        {
            sock = createClient();
            try = lookupForServer(&sock, serv_addr);
        }
        else
        {
            printf("\nOK. Bye!\n");
            return 0;
        }
    }

    while (1)
    {
        int signal = 0;

        char buffer[1024] = {0};
        recv(sock, &signal, sizeof(int), 0);

        if (signal == 0)
        {
            printf("Enter room's name!(if room doesn't exist it will be created)\n");
            char roomName[1024];
            scanf("%s", roomName);
            send(sock, roomName, strlen(roomName), 0);
        }
        else if (signal == 1)
        {
            readFromServer(buffer, sock);
            printf("%s", buffer);
            memset(buffer, 0, sizeof(buffer));

            readFromServer(buffer, sock);
            printf("%s", buffer);
            memset(buffer, 0, sizeof(buffer));
        }
        else if (signal == 2)
        {
            int numPlayers = 0;
            recv(sock, &numPlayers, sizeof(int), 0);
            if (numPlayers == 1)
            {
                printf("There are not enough players!\n");
            }
            else
            {
                printf("There are currently %d players!Do you want to start?(y/n)\n", numPlayers);
                char ans[34];
                int willStart = 0;
                scanf("%s", ans);
                if (ans[0] == 'y' || ans[0] == 'Y')
                {
                    willStart = 1;
                }
                send(sock, &willStart, sizeof(int), 0);
            }
        }
        else if (signal == 3)
        {
            int phase = -1;
            recv(sock, &phase, sizeof(int), 0);
            if (phase == 0)
            {
                readFromServer(buffer, sock);
                printf("%s", buffer);
                close(sock);
                return 0;
            }
            else if (phase == 1)
            {
                char firsts[15];
                char messg[1024];
                readFromServer(buffer, sock);
                printf("%s", buffer);
                memset(firsts, 0, strlen(firsts));
                readFromServer(firsts, sock);
                memset(buffer, 0, strlen(buffer));
                readFromServer(buffer, sock);
                memset(messg, 0, 1024);
                strcpy(messg, buffer);
                memset(buffer, 0, strlen(buffer));
                printf("%s", messg);
                scanf("%s", buffer);
                while (strlen(buffer) <= 0 || checkIfFirstLettersAreSame(buffer, firsts) == 0)
                {
                    memset(buffer, 0, strlen(buffer));
                    printf("%s", messg);
                    scanf("%s", buffer);
                }
                writeToServer(buffer, sock);

                memset(buffer, 0, strlen(buffer));

                readFromServer(buffer, sock);
                printf("%s", buffer);

                if (strcmp(buffer, "\n**************\n* You lost :( *\n**************\n") == 0)
                {
                    close(sock);
                    return 0;
                }

                fflush(0);
            }
            else if (phase == 2)
            {
                readFromServer(buffer, sock);
                printf("%s", buffer);
                close(sock);
                return 0;
            }
            else if (phase == 3)
            {
                readFromServer(buffer, sock);
                printf("%s", buffer);
                memset(buffer, 0, strlen(buffer));
                readFromServer(buffer, sock);
                printf("%s", buffer);
                memset(buffer, 0, strlen(buffer));
            }
        }
        fflush(0);
    }

    return 0;
}
