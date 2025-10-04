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

// --- Bloque de compatibilidad para Windows y Linux ---
#ifdef _WIN32
    // Código y librerías para Windows
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib") // Para enlazar con la librería de sockets de Windows
    #define sleep(x) Sleep(1000 * (x))
#else
    // Código y librerías para Linux (POSIX)
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    // Se definen alias para que el código sea compatible
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
#endif
// --- Fin del bloque de compatibilidad ---


// Constantes de configuración del servidor
#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024
#define TOKEN_SIZE 32
#define MAX_USERNAME 50
#define MAX_PASSWORD 50


// Estructura que representa un cliente conectado
typedef struct {
    SOCKET socket;
    struct sockaddr_in address;
    char token[TOKEN_SIZE + 1];
    char username[MAX_USERNAME + 1];
    int user_type; // 1: ADMIN, 0: OBSERVER
    int active;
    time_t last_activity;
} client_t;

// Estructura que almacena los datos de telemetría del vehículo
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
void *client_handler(void *arg);
void *telemetry_broadcaster(void *arg);
void process_command(int client_idx, const char *command, const char *params);
void cleanup_and_exit(int sig);
void remove_client(int index);

/*
 * Función principal del servidor.
 * Inicializa el socket, configura el entorno y gestiona las conexiones de los clientes.
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <puerto> <archivo_log>\n", argv[0]);
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
        fprintf(stderr, "WSAStartup failed.\n");
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
    if (server_socket == INVALID_SOCKET) {
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

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Error en bind");
        closesocket(server_socket);
        fclose(log_file);
        #ifdef _WIN32
        WSACleanup();
        #endif
        return 1;
    }

    if (listen(server_socket, 10) == SOCKET_ERROR) {
        perror("Error en listen");
        closesocket(server_socket);
        fclose(log_file);
        #ifdef _WIN32
        WSACleanup();
        #endif
        return 1;
    }

    printf("Servidor iniciado en puerto %d\n", port);
    printf("Logs guardándose en: %s\n", log_filename);
    
    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);

    pthread_t telemetry_thread;
    pthread_create(&telemetry_thread, NULL, telemetry_broadcaster, NULL);

    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_socket == INVALID_SOCKET) {
            if (running) {
                perror("Error en accept");
            }
            continue;
        }
        
        pthread_mutex_lock(&clients_mutex);
        int client_index = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].active) {
                client_index = i;
                break;
            }
        }

        if (client_index == -1) {
            printf("Máximo de clientes alcanzado. Conexión rechazada.\n");
            closesocket(client_socket);
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
        
        pthread_t client_thread;
        int *client_idx_ptr = malloc(sizeof(int));
        *client_idx_ptr = client_index;
        pthread_create(&client_thread, NULL, client_handler, client_idx_ptr);
        pthread_detach(client_thread);

        char client_info[100];
        sprintf(client_info, "%s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        log_message(client_info, "CONNECT", "Nueva conexión establecida");

        pthread_mutex_unlock(&clients_mutex);
    }

    pthread_join(telemetry_thread, NULL);
    closesocket(server_socket);
    fclose(log_file);
    
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    return 0;
}

/*
 * Encargado de gestionar la comunicación con un cliente específico.
 * Procesa mensajes, responde y gestiona la desconexión.
 */
void *client_handler(void *arg) {
    int client_index = *(int*)arg;
    free(arg);

    char client_info[100];
    pthread_mutex_lock(&clients_mutex);
    sprintf(client_info, "%s:%d", inet_ntoa(clients[client_index].address.sin_addr), ntohs(clients[client_index].address.sin_port));
    pthread_mutex_unlock(&clients_mutex);

    char recv_buffer[BUFFER_SIZE * 2] = {0};
    int recv_len = 0;

    while (running && clients[client_index].active) {
        int bytes_received = recv(clients[client_index].socket, recv_buffer + recv_len, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            break;
        }

        recv_len += bytes_received;
        recv_buffer[recv_len] = '\0';

        char *line_end;
        while ((line_end = strstr(recv_buffer, "\r\n")) != NULL) {
            char message[BUFFER_SIZE];
            int msg_len = line_end - recv_buffer;
            strncpy(message, recv_buffer, msg_len);
            message[msg_len] = '\0';
            
            clients[client_index].last_activity = time(NULL);
            log_message(client_info, "REQUEST", message);

            char *message_copy = strdup(message);
            char *message_type = strtok(message_copy, "|");
            // char *timestamp = strtok(NULL, "|");
            char *token = strtok(NULL, "|");
            char *data = strtok(NULL, "|");
            // char *checksum = strtok(NULL, "|");

            char response[BUFFER_SIZE];
            memset(response, 0, BUFFER_SIZE);

            if (message_type && strcmp(message_type, "AUTH_REQUEST") == 0) {
                 char *username = data ? strtok(data, ":") : NULL;
                 char *password = data ? strtok(NULL, ":") : NULL;
                 if (username && password) {
                    int auth_result = authenticate_user(username, password);
                    if (auth_result >= 0) {
                         generate_token(clients[client_index].token);
                         strcpy(clients[client_index].username, username);
                         clients[client_index].user_type = auth_result;
                         sprintf(response, "AUTH_RESPONSE|%ld|%s|%s:200|CHECKSUM",
                                 (long)time(NULL), clients[client_index].token,
                                 auth_result == 1 ? "ADMIN" : "OBSERVER");
                    } else {
                         sprintf(response, "AUTH_RESPONSE|%ld|NULL|ERROR:401|CHECKSUM", (long)time(NULL));
                    }
                 }
            } else if (message_type && strcmp(message_type, "COMMAND_REQUEST") == 0) {
                if (clients[client_index].user_type == 1 && token && strcmp(token, clients[client_index].token) == 0) {
                    char *command = data ? strtok(data, ":") : NULL;
                    char *params = data ? strtok(NULL, ":") : NULL;
                    process_command(client_index, command, params);
                    sprintf(response, "COMMAND_RESPONSE|%ld|NULL|200:Comando procesado|CHECKSUM", (long)time(NULL));
                } else {
                    sprintf(response, "COMMAND_RESPONSE|%ld|NULL|401:No autorizado|CHECKSUM", (long)time(NULL));
                }
            } else if (message_type && strcmp(message_type, "LIST_USERS_REQUEST") == 0) {
                 if (clients[client_index].user_type == 1 && token && strcmp(token, clients[client_index].token) == 0) {
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
                    sprintf(response, "LIST_USERS_RESPONSE|%ld|NULL|%d:%s|CHECKSUM", (long)time(NULL), user_count, user_list);
                 } else {
                    sprintf(response, "ERROR|%ld|NULL|401:No autorizado|CHECKSUM", (long)time(NULL));
                 }
            } else {
                sprintf(response, "ERROR|%ld|NULL|400:Mensaje inválido|CHECKSUM", (long)time(NULL));
            }

            char response_with_newline[BUFFER_SIZE + 4];
            snprintf(response_with_newline, sizeof(response_with_newline), "%s\r\n", response);
            send(clients[client_index].socket, response_with_newline, strlen(response_with_newline), 0);
            log_message(client_info, "RESPONSE", response);

            free(message_copy);
            memmove(recv_buffer, line_end + 2, recv_len - (msg_len + 2));
            recv_len -= (msg_len + 2);
            recv_buffer[recv_len] = '\0';
        }
    }

    log_message(client_info, "DISCONNECT", "Cliente desconectado");
    remove_client(client_index);
    return NULL;
}

/*
 * Encargado de enviar periódicamente los datos de telemetría a todos los clientes conectados.
 */
void *telemetry_broadcaster(void *arg) {
    char telemetry_msg[BUFFER_SIZE];
    while (running) {
        sleep(10);

        pthread_mutex_lock(&vehicle_mutex);
        vehicle.speed += (rand() % 20 - 10) * 0.1;
        if (vehicle.speed < 0) vehicle.speed = 0;
        if (vehicle.speed > 120) vehicle.speed = 120;
        vehicle.battery -= 0.01;
        if (vehicle.battery < 0) vehicle.battery = 100.0;
        
        sprintf(telemetry_msg, "TELEMETRY|%ld|NULL|%.1f:%.1f:%.1f:%s:%.6f:%.6f|CHECKSUM\r\n",
                (long)time(NULL), vehicle.speed, vehicle.battery, vehicle.temperature,
                vehicle.direction, vehicle.latitude, vehicle.longitude);
        pthread_mutex_unlock(&vehicle_mutex);

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active && clients[i].user_type >= 0) {
                send(clients[i].socket, telemetry_msg, strlen(telemetry_msg), 0);
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

/*
 * Procesa los comandos recibidos de los clientes administradores y actualiza el estado del vehículo.
 */
void process_command(int client_idx, const char *command, const char *params) {
    pthread_mutex_lock(&vehicle_mutex);
    if (strcmp(command, "SPEED_UP") == 0) {
        float increment = params ? atof(params) : 5.0;
        vehicle.speed += increment;
        if (vehicle.speed > 120) vehicle.speed = 120;
    } else if (strcmp(command, "SLOW_DOWN") == 0) {
        float decrement = params ? atof(params) : 5.0;
        vehicle.speed -= decrement;
        if (vehicle.speed < 0) vehicle.speed = 0;
    } else if (strcmp(command, "TURN_LEFT") == 0) {
        strcpy(vehicle.direction, "WEST");
    } else if (strcmp(command, "TURN_RIGHT") == 0) {
        strcpy(vehicle.direction, "EAST");
    } else if (strcmp(command, "STOP") == 0) {
        vehicle.speed = 0;
    }
    pthread_mutex_unlock(&vehicle_mutex);
}

/*
 * Verifica las credenciales de usuario y retorna el tipo de usuario.
 * Retorna 1 para ADMIN, 0 para OBSERVER, -1 para no autorizado.
 */
int authenticate_user(const char *username, const char *password) {
    if (username && password) {
        if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) return 1;
        if (strcmp(username, "observer") == 0 && strcmp(password, "observer123") == 0) return 0;
    }
    return -1;
}

/*
 * Genera un token aleatorio para la sesión del cliente.
 */
void generate_token(char *token) {
    const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    srand(time(NULL)); // Semilla para el generador de números aleatorios
    for (int i = 0; i < TOKEN_SIZE; i++) {
        token[i] = chars[rand() % (sizeof(chars) - 1)];
    }
    token[TOKEN_SIZE] = '\0';
}

/*
 * Elimina y limpia la información de un cliente desconectado.
 */
void remove_client(int index) {
    pthread_mutex_lock(&clients_mutex);
    if(clients[index].active) {
        closesocket(clients[index].socket);
        clients[index].active = 0;
        // Limpiar la estructura del cliente para reutilización
        memset(clients[index].username, 0, MAX_USERNAME + 1);
        memset(clients[index].token, 0, TOKEN_SIZE + 1);
        clients[index].user_type = -1;
    }
    pthread_mutex_unlock(&clients_mutex);
}

/*
 * Registra mensajes en el log y en consola, con timestamp y tipo de evento.
 */
void log_message(const char *client_info, const char *type, const char *message) {
    time_t now = time(NULL);
    char time_buf[26];
    char* time_str = ctime_r(&now, time_buf); // Usar ctime_r para ser thread-safe
    time_str[strlen(time_str) - 1] = '\0';
    
    printf("[%s] [%s] [%s] %s\n", time_str, client_info, type, message);
    if (log_file) {
        fprintf(log_file, "[%s] [%s] [%s] %s\n", time_str, client_info, type, message);
        fflush(log_file);
    }
}

/*
 * Maneja la limpieza y cierre seguro del servidor ante señales de terminación.
 */
void cleanup_and_exit(int sig) {
    printf("\nCerrando servidor (señal %d)...\n", sig);
    running = 0;
    
    // Despertar al accept() para que el bucle termine
    // Creando una conexión dummy a nosotros mismos
    SOCKET dummy_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in self_addr;
    memset(&self_addr, 0, sizeof(self_addr));
    self_addr.sin_family = AF_INET;
    self_addr.sin_port = htons(12345); // Asume que el servidor corre en un puerto diferente
    self_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connect(dummy_socket, (struct sockaddr*)&self_addr, sizeof(self_addr));
    closesocket(dummy_socket);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            closesocket(clients[i].socket);
        }
    }
    
    closesocket(server_socket);
    
    if (log_file) {
        fclose(log_file);
    }
    
    printf("Servidor cerrado limpiamente.\n");
    exit(0);
}
