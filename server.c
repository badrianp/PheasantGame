#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#define PORT 8080

// pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int ACTIVE_SESSIONS = 0;
struct GameSession
{
    char name[1024];
    int players[7];
    int connectedPlayers;
    int scores[7];
    int gameState;
    sem_t *semaphore;
    int turn;
    int owner;
    char lastWord[1024];
    int lastPlayer;
} sessions[100];

int createServer(int server_fd, struct sockaddr_in address)
{
    int opt = 1;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

int lookupForClient(int server_fd, struct sockaddr_in address, int addrlen)
{
    int new_socket;
    if (listen(server_fd, 50) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    return new_socket;
}

void writeToclient(char *buffer, int sock)
{
    int sizeOf = strlen(buffer);
    send(sock, &sizeOf, sizeof(int), 0);
    send(sock, buffer, sizeOf, 0);
}

void readFromClient(char *buffer, int sock)
{
    int sizeOf = 0;
    recv(sock, &sizeOf, sizeof(int), 0);
    recv(sock, buffer, sizeOf, 0);
}

int num_of_sessions = 0;
pthread_mutex_t mutex;

int checkIfRoomExists(char *room)
{
    for (int i = 0; i < num_of_sessions; i++)
    {
        if (strcmp(room, sessions[i].name) == 0)
        {
            return i;
        }
    }
    return -1;
}

char pickARandomLetter()
{
    char *alphabet = "abcdefghijklmnopqrstuvxz";
    int index = rand() % strlen(alphabet);
    return alphabet[index];
}

char *extractFinalLetters(char *word)
{
    char *finalLetters = malloc(sizeof(char) * 5);
    strcpy(finalLetters, "");
    for (int i = strlen(word) - 2; i < strlen(word); i++)
    {
        strncat(finalLetters, &word[i], 1);
    }
    printf("Wut %s\n", finalLetters);
    return finalLetters;
}

int checkIfTerminationIsTerminal(char *word)
{
    char combs[25][4] = {"nt", "rt", "ct", "ng", "rs", "ns", "lb", "tm", "mt"};
    int wordLen = strlen(word);
    for (int i = 0; i < sizeof(combs); i++)
    {
        if (combs[i][0] == word[wordLen - 2] && combs[i][1] == word[wordLen - 1])
        {
            return 1;
        }
    }
    return 0;
}

void controlBroadcast(int sessionIndex, int playerIndex)
{
    for (int i = 0; i < sessions[sessionIndex].connectedPlayers; i++)
    {
        if (i != playerIndex)
        {
            if (sessions[sessionIndex].scores[i] > 0)
            {
                sem_post(sessions[sessionIndex].semaphore);
                printf("sem_broadcast_post");
            }
        }
    }
    return;
}

int getNextPlayerTurn(int sessionIndex, int playerIndex)
{
    for (int i = playerIndex + 1; i < sessions[sessionIndex].connectedPlayers; i++)
    {
        if (sessions[sessionIndex].scores[i] > 0)
        {
            return i;
        }
    }
    for (int i = 0; i < playerIndex; i++)
    {
        if (sessions[sessionIndex].scores[i] > 0)
        {
            return i;
        }
    }
    return -1;
}

int isWinner(int sessionIndex, int playerIndex)
{
    for (int i = 0; i < sessions[sessionIndex].connectedPlayers; i++)
    {
        if (i != playerIndex)
        {
            if (sessions[sessionIndex].scores[i] > 0)
            {
                return 0;
            }
        }
    }
    return 1;
}

void fazanGameLogic(int client, int playerIndex, int sessionIndex)
{
    char buffer[2048] = {0};
    int phase = -1;

    if (playerIndex == sessions[sessionIndex].turn)
    {
        if (isWinner(sessionIndex, playerIndex) == 1)
        {
            phase = 0;
            send(client, &phase, sizeof(int), 0);
            sprintf(buffer, "You won! The game ended!\n");
            writeToclient(buffer, client);
            sessions[sessionIndex].gameState = 2;
            close(client);
            return;
        }

        // if (sessions[sessionIndex].lastWord && strlen(sessions[sessionIndex].lastWord) > 0)
        if (strlen(sessions[sessionIndex].lastWord) > 0)
        {
            printf("pre\n");
            int isTerminal = checkIfTerminationIsTerminal(sessions[sessionIndex].lastWord);
            printf("%d\n", isTerminal);
            if (isTerminal == 1)
            {
                strcpy(sessions[sessionIndex].lastWord, "");
                sessions[sessionIndex].scores[playerIndex] -= 1;
            }
            printf("post\n");
        }
        if (sessions[sessionIndex].scores[playerIndex] > 0)
        {
            phase = 1;
            send(client, &phase, sizeof(int), 0);
            sprintf(buffer, "It's your turn! You currently have %d points!\n", sessions[sessionIndex].scores[playerIndex]);
            writeToclient(buffer, client);
            memset(buffer, 0, strlen(buffer));
            // if (!sessions[sessionIndex].lastWord || strlen(sessions[sessionIndex].lastWord) <= 0)
            if (strlen(sessions[sessionIndex].lastWord) <= 0)
            {
                char randC = pickARandomLetter();
                sprintf(buffer, "Your word should start with %c:\n", randC);
                char firsts[15];
                memset(firsts, 0, 15);
                sprintf(firsts, "%c", randC);
                printf("you were sanctionaed!!!\n");
                printf("%s\n", firsts);
                writeToclient(firsts, client);
                writeToclient(buffer, client);
            }
            else
            {
                writeToclient(extractFinalLetters(sessions[sessionIndex].lastWord), client);
                sprintf(buffer, "Last word was %s !\nYour word should start with %s:\n", sessions[sessionIndex].lastWord, extractFinalLetters(sessions[sessionIndex].lastWord));
                writeToclient(buffer, client);
            }
            memset(buffer, 0, strlen(buffer));
            readFromClient(buffer, client);
            // here comes the check for dictionary
            strcpy(sessions[sessionIndex].lastWord, buffer);
            memset(buffer, 0, strlen(buffer));
            sessions[sessionIndex].lastPlayer = playerIndex;
            sessions[sessionIndex].turn = getNextPlayerTurn(sessionIndex, playerIndex);
            controlBroadcast(sessionIndex, playerIndex);
        }
        else
        {
            phase = 2;
            send(client, &phase, sizeof(int), 0);
            sprintf(buffer, "You lost!");
            writeToclient(buffer, client);
            sessions[sessionIndex].lastPlayer = playerIndex;
            memset(buffer, 0, strlen(buffer));
            close(client);
            sessions[sessionIndex].turn = getNextPlayerTurn(sessionIndex, playerIndex);
            controlBroadcast(sessionIndex, playerIndex);
            return;
        }
    }
    else
    {
        phase = 3;
        send(client, &phase, sizeof(int), 0);
        sem_wait(sessions[sessionIndex].semaphore);
        printf("sem_268_wait");
        sprintf(buffer, "It's player %d's turn!\n", sessions[sessionIndex].turn);
        writeToclient(buffer, client);
        memset(buffer, 0, strlen(buffer));
        if (sessions[sessionIndex].scores[sessions[sessionIndex].turn] <= 0)
        {
            sprintf(buffer, "Player %d is out of game!\n", sessions[sessionIndex].turn);
            writeToclient(buffer, client);
        }
        else
        {
            sprintf(buffer, "Player %d has to continue: %s\n", sessions[sessionIndex].turn, sessions[sessionIndex].lastWord);
            writeToclient(buffer, client);
        }
        memset(buffer, 0, strlen(buffer));
    }
}

void *handleClient(void *arg)
{
    char buffer[1024] = {0};
    int client = *((int *)arg);
    int signal = 0;
    int sessionIndex = -1;
    int playerIndex = -1;

    while (1)
    {
        if (sessionIndex != -1 && sessions[sessionIndex].gameState == 1)
        {
            signal = 3;
        }

        send(client, &signal, sizeof(int), 0);
        memset(buffer, 0, strlen(buffer));

        if (signal == 0)
        { // client
            recv(client, buffer, 1024, 0);
            printf("Room name choice: %s\n", buffer);
            sessionIndex = checkIfRoomExists(buffer);
            pthread_mutex_lock(&mutex);
            printf("pre_befif_pre");
            if (sessionIndex == -1)
            {
                num_of_sessions += 1;
                sessionIndex = num_of_sessions - 1;
                strcpy(sessions[sessionIndex].name, buffer);
                sessions[sessionIndex].owner = client;
                sessions[sessionIndex].connectedPlayers = 1;
                playerIndex = 0;
                sessions[sessionIndex].players[0] = client;
                sessions[sessionIndex].gameState = 0;
                sessions[sessionIndex].scores[0] = 5;
                sessions[sessionIndex].turn = 0;
                char semname[10] = {0};
                int val = 0;
                sprintf(semname, "/sem%d", sessionIndex);
                // printf("\nsemaphore name:  ---  %s  ---\n", semname);
                sessions[sessionIndex].semaphore = sem_open(semname, O_CREAT, 0644, val);
                signal = 2;
            }
            else
            {
                printf("pre_else_pre");
                signal = 1;
                playerIndex = sessions[sessionIndex].connectedPlayers;
                sessions[sessionIndex].connectedPlayers += 1;
                sessions[sessionIndex].players[playerIndex] = client;
                sessions[sessionIndex].scores[playerIndex] = 5;
            }
            pthread_mutex_unlock(&mutex);
        }
        else if (signal == 1)
        {
            char msg1[1024];
            sprintf(msg1, "Currently there are %d sessions!\n", num_of_sessions);
            writeToclient(msg1, client);

            char msg2[1024];
            sprintf(msg2, "Your room number is %d\n", sessionIndex);
            writeToclient(msg2, client);
            printf("sem_347_wait");
            sem_wait(sessions[sessionIndex].semaphore);
        }
        else if (signal == 2)
        {
            int start = -1;
            if (sessions[sessionIndex].connectedPlayers <= 1)
            {
                send(client, &sessions[sessionIndex].connectedPlayers, sizeof(int), 0);
                sleep(2);
            }
            else
            {
                send(client, &sessions[sessionIndex].connectedPlayers, sizeof(int), 0);
                recv(client, &start, sizeof(int), 0);
                if (start == 1)
                {
                    pthread_mutex_lock(&mutex);
                    sessions[sessionIndex].gameState = 1;
                    pthread_mutex_unlock(&mutex);
                    for (int i = 0; i < sessions[sessionIndex].connectedPlayers; i++)
                    {
                        printf("sem_369_post");
                        sem_post(sessions[sessionIndex].semaphore);
                    }
                }
            }
        }
        else if (signal == 3)
        {
            fazanGameLogic(client, playerIndex, sessionIndex);
            if (sessions[sessionIndex].scores[playerIndex] <= 0)
            {
                return NULL;
            }
            if (sessions[sessionIndex].gameState == 2)
            {
                return NULL;
            }
        }
        fflush(0);
    }
}

int main(int argc, char const *argv[])
{
    pthread_t tid[100];
    int server_fd, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = createServer(server_fd, address);
    int i = 0;
    while (1)
    {
        int client = lookupForClient(server_fd, address, addrlen);

        if (pthread_create(&tid[i++], NULL, handleClient, &client) != 0)
        {
            printf("failed to create thread\n");
        }

        if (i >= 50)
        {
            i = 0;
            while (i < 50)
            {
                pthread_join(tid[i++], NULL);
            }
            i = 0;
        }
    }
    return 0;
}
