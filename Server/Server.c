/*
 * server.c - Servidor del Vehículo Autónomo
 * Descripción: Servidor TCP que gestiona la autenticación, comandos y telemetría de un vehículo autónomo.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define sleep(x) Sleep(1000 * (x))
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
#endif

#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024
#define TOKEN_SIZE 32
#define MAX_USERNAME 50
#define MAX_PASSWORD 50

typedef struct {
    SOCKET socket;
    struct sockaddr_in address;
    char token[TOKEN_SIZE + 1];
    char username[MAX_USERNAME + 1];
    int user_type; // 1: ADMIN, 0: OBSERVER
    int active;
    time_t last_activity;
} client_t;

typedef struct {
    float speed;
    float battery;
    float temperature;
    char direction[20];
    float latitude;
    float longitude;
    int running;
} vehicle_data_t;

client_t clients[MAX_CLIENTS];
vehicle_data_t vehicle;
SOCKET server_socket;
volatile int running = 1;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vehicle_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *log_file = NULL;

void log_message(const char *client_info, const char *type, const char *message);
void generate_token(char *token);
int authenticate_user(const char *username, const char *password);
void *cli*

