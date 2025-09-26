// server.c - Servidor del Vehículo Autónomo
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <io.h>
    #include <process.h>
    #define sleep(x) Sleep(1000 * (x))
#else
    #include <unistd.h>
#endif
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024
#define TOKEN_SIZE 32
#define MAX_USERNAME 50
#define MAX_PASSWORD 50

typedef struct {
    int socket;
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
int server_socket;
int running = 1;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vehicle_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *log_file = NULL;

void log_message(const char *client_info, const char *type, const char *message);
void generate_token(char *token);
int authenticate_user(const char *username, const char *password);
void remove_client(int index);
void *client_handler(void *arg);
void *telemetry_broadcaster(void *arg);
void process_command(int client_idx, const char *command, const char *params);
void cleanup_and_exit(int sig);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <puerto> <archivo_log>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    char *log_filename = argv[2];

    log_file = fopen(log_filename, "a");
    if (!log_file) {
        perror("Error abriendo archivo de log");
        return 1;
    }

    #ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", WSAGetLastError());
        fclose(log_file);
        return 1;
    }
    #endif

    vehicle.speed = 0.0;
    vehicle.battery = 100.0;
    vehicle.temperature = 22.5;
    strcpy(vehicle.direction, "NORTH");
    vehicle.latitude = 6.2442;
    vehicle.longitude = -75.5812;
    vehicle.running = 1;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = 0;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creando socket");
        fclose(log_file);
        #ifdef _WIN32
        WSACleanup();
        #endif
        return 1;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        close(server_socket);
        fclose(log_file);
        #ifdef _WIN32
        WSACleanup();
        #endif
        return 1;
    }

    if (listen(server_socket, 10) < 0) {
        perror("Error en listen");
        close(server_socket);
        fclose(log_file);
        #ifdef _WIN32
        WSACleanup();
        #endif
        return 1;
    }

    printf("Servidor iniciado en puerto %d\n", port);
    printf("Logs guardándose en: %s\n", log_filename);
    printf("[DEBUG] Esperando conexiones en puerto %d...\n", port);

    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);

    pthread_t telemetry_thread;
    pthread_create(&telemetry_thread, NULL, telemetry_broadcaster, NULL);

    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        printf("[DEBUG] accept() retornó: %d\n", client_socket);
        if (client_socket < 0) {
            if (running) {
                perror("Error en accept");
            }
            continue;
        }
        printf("[DEBUG] Conexión aceptada de %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pthread_mutex_lock(&clients_mutex);
        int client_index = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].active) {
                client_index = i;
                break;
            }
        }
        if (client_index == -1) {
            close(client_socket);
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }
        clients[client_index].socket = client_socket;
        clients[client_index].address = client_addr;
        clients[client_index].active = 1;
        clients[client_index].user_type = -1;
        clients[client_index].last_activity = time(NULL);
        memset(clients[client_index].token, 0, TOKEN_SIZE + 1);
        memset(clients[client_index].username, 0, MAX_USERNAME + 1);
        pthread_mutex_unlock(&clients_mutex);

        pthread_t client_thread;
        int *client_idx = malloc(sizeof(int));
        *client_idx = client_index;
        pthread_create(&client_thread, NULL, client_handler, client_idx);
        pthread_detach(client_thread);

        char client_info[100];
        sprintf(client_info, "%s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        log_message(client_info, "CONNECT", "Nueva conexión establecida");
        printf("[DEBUG] Conexión aceptada de %s:%d (índice %d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_index);
    }

    pthread_join(telemetry_thread, NULL);
    close(server_socket);
    fclose(log_file);
    #ifdef _WIN32
    WSACleanup();
    #endif
    return 0;
}

void *client_handler(void *arg) {
    int client_index = *(int*)arg;
    free(arg);

    char client_info[100];
    pthread_mutex_lock(&clients_mutex);
    sprintf(client_info, "%s:%d",
            inet_ntoa(clients[client_index].address.sin_addr),
            ntohs(clients[client_index].address.sin_port));
    pthread_mutex_unlock(&clients_mutex);

    char recv_buffer[BUFFER_SIZE * 2] = {0};
    int recv_len = 0;
    while (running && clients[client_index].active) {
        int bytes_received = recv(clients[client_index].socket, recv_buffer + recv_len, BUFFER_SIZE - recv_len - 1, 0);
        printf("[DEBUG] recv() bytes: %d\n", bytes_received);
        if (bytes_received <= 0) {
            break;
        }
        recv_len += bytes_received;
        recv_buffer[recv_len] = '\0';
        printf("[DEBUG] Buffer recibido: '%s'\n", recv_buffer);

        char *line_end;
        while ((line_end = strstr(recv_buffer, "\r\n")) != NULL) {
            int msg_len = line_end - recv_buffer;
            char message[BUFFER_SIZE];
            strncpy(message, recv_buffer, msg_len);
            message[msg_len] = '\0';

            clients[client_index].last_activity = time(NULL);
            log_message(client_info, "REQUEST", message);

            char *message_type = strtok(message, "|");
            char *timestamp = strtok(NULL, "|");
            char *token = strtok(NULL, "|");
            char *data = strtok(NULL, "|");
            char *checksum = strtok(NULL, "|");

            char response[BUFFER_SIZE];
            memset(response, 0, BUFFER_SIZE);

            if (message_type && strcmp(message_type, "AUTH_REQUEST") == 0) {
                char *username = data ? strtok(data, ":") : NULL;
                char *password = data ? strtok(NULL, ":") : NULL;
                int auth_result = authenticate_user(username, password);
                if (auth_result >= 0) {
                    generate_token(clients[client_index].token);
                    strcpy(clients[client_index].username, username);
                    clients[client_index].user_type = auth_result;
                    sprintf(response, "AUTH_RESPONSE|%lld|%s|%s:200|CHECKSUM", (long long)time(NULL), clients[client_index].token,
                            auth_result == 1 ? "ADMIN" : "OBSERVER");
                } else {
                    sprintf(response, "AUTH_RESPONSE|%lld|NULL|ERROR:401|CHECKSUM", (long long)time(NULL));
                }
            }
            else if (message_type && strcmp(message_type, "COMMAND_REQUEST") == 0) {
                if (clients[client_index].user_type == 1 &&
                    token && strcmp(token, clients[client_index].token) == 0) {
                    char *command = data ? strtok(data, ":") : NULL;
                    char *params = data ? strtok(NULL, ":") : NULL;
                    process_command(client_index, command, params);
                    sprintf(response, "COMMAND_RESPONSE|%ld|NULL|200:Comando procesado|CHECKSUM",
                            time(NULL));
                } else {
                    sprintf(response, "COMMAND_RESPONSE|%ld|NULL|401:No autorizado|CHECKSUM",
                            time(NULL));
                }
            }
            else if (message_type && strcmp(message_type, "LIST_USERS_REQUEST") == 0) {
                if (clients[client_index].user_type == 1 &&
                    token && strcmp(token, clients[client_index].token) == 0) {
                    pthread_mutex_lock(&clients_mutex);
                    int user_count = 0;
                    char user_list[500] = "";
                    for (int i = 0; i < MAX_CLIENTS; i++) {
                        if (clients[i].active && clients[i].user_type >= 0) {
                            user_count++;
                            strcat(user_list, clients[i].username);
                            strcat(user_list, ",");
                        }
                    }
                    pthread_mutex_unlock(&clients_mutex);
                    sprintf(response, "LIST_USERS_RESPONSE|%ld|NULL|%d:%s|CHECKSUM",
                            time(NULL), user_count, user_list);
                } else {
                    sprintf(response, "ERROR|%ld|NULL|401:No autorizado|CHECKSUM", time(NULL));
                }
            }
            else {
                sprintf(response, "ERROR|%ld|NULL|400:Mensaje inválido|CHECKSUM", time(NULL));
            }

            char response_with_newline[BUFFER_SIZE + 4];
            snprintf(response_with_newline, sizeof(response_with_newline), "%s\r\n", response);
            #ifdef _WIN32
            send(clients[client_index].socket, response_with_newline, strlen(response_with_newline), 0);
            #else
            ssize_t sent_bytes = write(clients[client_index].socket, response_with_newline, strlen(response_with_newline));
            #endif
            log_message(client_info, "RESPONSE", response_with_newline);

            int total_consumed = msg_len + 2;
            memmove(recv_buffer, line_end + 2, recv_len - total_consumed);
            recv_len -= total_consumed;
            recv_buffer[recv_len] = '\0';
        }
    }

    log_message(client_info, "DISCONNECT", "Cliente desconectado");
    remove_client(client_index);
    return NULL;
}

void *telemetry_broadcaster(void *arg) {
    char telemetry_msg[BUFFER_SIZE];
    while (running) {
        pthread_mutex_lock(&vehicle_mutex);
        vehicle.speed += (rand() % 20 - 10) * 0.1;
        if (vehicle.speed < 0) vehicle.speed = 0;
        if (vehicle.speed > 120) vehicle.speed = 120;
        vehicle.battery -= 0.1;
        if (vehicle.battery < 0) vehicle.battery = 0;
        vehicle.temperature += (rand() % 6 - 3) * 0.1;
        sprintf(telemetry_msg, "TELEMETRY|%ld|NULL|%.1f:%.1f:%.1f:%s:%.6f:%.6f|CHECKSUM",
                time(NULL), vehicle.speed, vehicle.battery, vehicle.temperature,
                vehicle.direction, vehicle.latitude, vehicle.longitude);
        pthread_mutex_unlock(&vehicle_mutex);

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active && clients[i].user_type >= 0) {
                send(clients[i].socket, telemetry_msg, strlen(telemetry_msg), 0);
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        printf("Telemetría enviada: Speed=%.1f, Battery=%.1f%%, Temp=%.1f°C\n",
               vehicle.speed, vehicle.battery, vehicle.temperature);
        sleep(10);
    }
    return NULL;
}

void process_command(int client_idx, const char *command, const char *params) {
    pthread_mutex_lock(&vehicle_mutex);
    if (strcmp(command, "SPEED_UP") == 0) {
        float increment = params ? atof(params) : 5.0;
        vehicle.speed += increment;
        if (vehicle.speed > 120) vehicle.speed = 120;
        printf("Comando: Acelerar a %.1f km/h\n", vehicle.speed);
    }
    else if (strcmp(command, "SLOW_DOWN") == 0) {
        float decrement = params ? atof(params) : 5.0;
        vehicle.speed -= decrement;
        if (vehicle.speed < 0) vehicle.speed = 0;
        printf("Comando: Desacelerar a %.1f km/h\n", vehicle.speed);
    }
    else if (strcmp(command, "TURN_LEFT") == 0) {
        strcpy(vehicle.direction, "WEST");
        printf("Comando: Girar a la izquierda\n");
    }
    else if (strcmp(command, "TURN_RIGHT") == 0) {
        strcpy(vehicle.direction, "EAST");
        printf("Comando: Girar a la derecha\n");
    }
    else if (strcmp(command, "STOP") == 0) {
        vehicle.speed = 0;
        printf("Comando: Detener vehículo\n");
    }
    pthread_mutex_unlock(&vehicle_mutex);
}

int authenticate_user(const char *username, const char *password) {
    if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
        return 1;
    }
    if (strcmp(username, "observer") == 0 && strcmp(password, "observer123") == 0) {
        return 0;
    }
    return -1;
}

void generate_token(char *token) {
    const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < TOKEN_SIZE; i++) {
        token[i] = chars[rand() % (sizeof(chars) - 1)];
    }
    token[TOKEN_SIZE] = '\0';
}

void remove_client(int index) {
    pthread_mutex_lock(&clients_mutex);
    close(clients[index].socket);
    clients[index].active = 0;
    memset(&clients[index], 0, sizeof(client_t));
    pthread_mutex_unlock(&clients_mutex);
}

void log_message(const char *client_info, const char *type, const char *message) {
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    printf("[%s] [%s] [%s] %s\n", time_str, client_info, type, message);
    if (log_file) {
        fprintf(log_file, "[%s] [%s] [%s] %s\n", time_str, client_info, type, message);
        fflush(log_file);
    }
}

void cleanup_and_exit(int sig) {
    printf("\nCerrando servidor...\n");
    running = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            close(clients[i].socket);
        }
    }
    close(server_socket);
    if (log_file) {
        fclose(log_file);
    }
    exit(0);
}