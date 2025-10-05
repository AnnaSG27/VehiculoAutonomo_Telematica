
# Proyecto: Vehículo Autónomo 
## Documentación
### Tabla de Contenidos
1. [Información del Equipo y Sistemas](#Información-del-Equipo-y-Sistemas)
2. [Introducción](#introducción)
3. [Arquitectura del Sistema](#arquitectura-del-sistema)
4. [Instalación y Configuración](#instalación-y-configuración)
5. [Uso del Sistema](#uso-del-sistema)
6. [Protocolo de Comunicación](#protocolo-de-comunicación)
7. [Estructura del Código](#estructura-del-código)
8. [Pruebas y Validación](#pruebas-y-validación)
9. [Resolución de Problemas](#resolución-de-problemas)

## Información del Equipo y Sistemas
### Elaborado por: 
- Mariana García Zapata
- Isabella Márquez Cifuentes
- Anna Sofía Giraldo Carvajal

### Sistemas Operativos Utilizados:
- Windows 11Pro 24H2

### Versiones de Lenguajes Utilizados
-  Para C:
	- `Pacman v6.1.0 -lipbalpm v14.0.0`
	- `GNU Make 4.4.1`
	- `Gcc 15.2.0`
- Para Java
	- `Java 25`
	- `Javac 25`
	- Swing (Manejo de Interfaz de Usuario) `Incluido al instalar el JDK`
- Para Python
	- `Python 3.12.11`
	- `Pip 25.2`
	- Thinker (Manejo de Interfaz de Usuario) `Tlc/Tk 8.6.16`

## Introducción
Este proyecto implementa un sistema de telemetría para vehículos autónomos que permite la comunicación bidireccional entre un vehículo (servidor (programado en C)) y múltiples clientes (usuarios programados en Java y Python)). El sistema soporta dos tipos de usuarios: administradores que pueden enviar comandos de control, y observadores que solo reciben datos de telemetría.

### Características Principales
- Comunicación TCP/IP confiable
- Autenticación basada en tokens
- Telemetría en tiempo real cada 10 segundos
- Soporte para múltiples clientes simultáneos
- Control de comandos para administradores
- Logging completo de actividades
- Interfaces gráficas intuitivas




## Arquitectura del Sistema
La arquitectura está dada por un sistema cliente-servidor basado en comunicación TCP/IP. Donde se encuentra un **servidor**, implementado en lenguaje C y etiquetado como “Vehículo”. Este servidor es responsable de gestionar múltiples aspectos críticos del sistema: expone un puerto TCP para recibir conexiones entrantes, maneja hilos concurrentes para atender simultáneamente a varios clientes, registra eventos mediante un módulo de _logging_ y valida la identidad de los usuarios a través de un sistema de autenticación.

Conectados a este servidor, hay múltiples **clientes** que pueden ser de distintas tecnologías y roles. Entre ellos se encuentran un cliente desarrollado en Python con rol de “Observer” (observador) , un cliente en Java con rol de “Admin” (administrador), y además otros clientes adicionales que pueden asumir cualquiera de estos dos perfiles (El cliente de Python puede ser también Admin). Todos estos clientes se comunican con el servidor utilizando el protocolo TCP/IP, lo que asegura una conexión confiable y orientada a sesión.

Esta configuración permite que el sistema sea flexible y escalable, ya que puede soportar diferentes tipos de usuarios con distintos niveles de acceso —los observadores tienen permisos de lectura, mientras que los administradores tienen la capacidad de "Orientar" el vehículo—, y además admite la integración de clientes escritos en distintos lenguajes de programación, lo que facilita la interoperabilidad y el desarrollo multiplataforma.

La arquitectura refleja un modelo centralizado donde un único servidor actúa como punto de control y coordinación, mientras que los clientes, diversos en tecnología y función, interactúan con él de forma segura y estructurada, gracias a los mecanismos de autenticación, concurrencia y registro de actividad que el servidor proporciona.
  

### Componentes del Sistema

#### 1. Servidor (Vehículo Autónomo)

-  **Lenguaje**: C con Berkeley Sockets API
-  **Funcionalidad**:
	- Acepta múltiples conexiones simultáneas
	- Autentica usuarios y genera tokens
	- Envía telemetría cada 10 segundos
	- Procesa comandos de administradores
	- Mantiene logs detallados
#### 2. Cliente Python

-  **Framework**: Tkinter para GUI
-  **Funcionalidad**:
	- Conexión al servidor
	- Autenticación de usuario
	- Visualización de telemetría en tiempo real
	- Envío de comandos (si es admin)
#### 3. Cliente Java

-  **Framework**: Swing para GUI
-  **Funcionalidad**:
	- Interfaz más robusta
	- Mismas capacidades que cliente Python
	- Look & Feel nativo del sistema

  <img width="756" height="599" alt="image" src="https://github.com/user-attachments/assets/93dbe021-6734-4c47-b850-ce75e6102d8b" />


## Instalación y Configuración
### Requisitos del Sistema
#### Para el Servidor (C)

```bash
# Requiere Instalación previa de MSYS2 (Desde Windows)
```
#### Para Cliente Python
```bash
# Python 3.x con tkinter
sudo  apt-get  install  python3  python3-tk
# O usando pip
pip  install  tkinter 
```
#### Para Cliente Java

```bash
# Java JDK 8 o superior
sudo  apt-get  install  openjdk-11-jdk
# Verificar instalación
java  -version
javac  -version
```
### Compilación e Instalación
#### 1. Descargar el Proyecto

```bash
git  clone https://github.com/AnnaSG27/VehiculoAutonomo_Telematica#
cd  VehículoAutonomo
```

#### 2. Compilar el Servidor

```bash
Importante, ejecutar en este mismo orden
# Desde MSYS2, navegar en la carpeta del proyecto
# Ejemplo:
cd "C:\Users\Escritorio\U\TELEMÁTICA\VehículoAutonomo"
# Es importante el uso de Comillas para evitar errores por exceso de caracteres
cd  Server
make
mkdir -p logs
gcc -Wall -g server.c -o server -pthread -lws2_32 -lmingw32
./server 8080 logs/vehicle.log
# Desde esta última ejecución del comando server nos empezarán a aparecer los datos de Telemetría del vehículo con Velocidad, Batería y temperatura.
# Allí también aparecerán las conexiones de los clientes y las instrucciones que envíen al servidor.
```
#### 3. Compilar Cliente Java

```bash
cd  ClientJava
javac VehicleClientJava.java #Prepara el Compilador
java VehicleClienteJava.java #Ejecuta el Swing para manejo de usuario
```
#### 4. Verificar Cliente Python
```bash
cd  ClientPy
python3  Client.py
```
### Estructura de Directorios

```
VehiculoAutonomo/
	Server/
		Server.c # Código principal del servidor
		Makefile # Archivo de compilación
		logs/ # Directorio para logs
		server # Ejecutable compilado
	ClientPy/
		Client.py # Cliente Python con GUI
	ClientJava/
		VehicleClientJava.java # Cliente Java con Swing
		VehicleClientJava.class # Clase compilada
		VehicleClientJava$1.class #Clase compilada
	docs/
		Especificaciones.md # Especificación del protocolo
	tests/
		test_scripts/ # Scripts de prueba
```

## Uso del Sistema
### 1. Iniciar el Servidor

```bash
cd  Server/
./server <puerto> <archivo_log>
# Ejemplo:
./server  8080  logs/vehicle.log
```
**Salida esperada:**

```
Servidor iniciado en puerto 8080
Logs guardándose en: logs/vehicle.log
Telemetría enviada: Speed=0.0, Battery=100.0%, Temp=22.5°C
```
### 2. Conectar Cliente Python

```bash
cd  ClientPy/
python3  Client.py
```
**Pasos de uso:**
1. Ingresar servidor y puerto (localhost:8080)
2. Hacer clic en "Conectar"
3. Ingresar credenciales:
- Admin: `admin` / `admin123`
- Observer: `observer` / `observer123`

4. Hacer clic en "Autenticar"
5. Visualizar telemetría automática
  
### 3. Conectar Cliente Java
```bash
cd  ClientJava/
javac  VehicleClientJava.java
java  VehicleClientJava.java
```
### 4. Credenciales de Usuario
#### Administrador
-  **Usuario**: `admin`
-  **Contraseña**: `admin123`
-  **Permisos**:
- Recibir telemetría
- Enviar comandos de control
- Listar usuarios conectados
#### Observador
-  **Usuario**: `observer`
-  **Contraseña**: `observer123`
-  **Permisos**:
	- Solo recibir telemetría

## Protocolo de Comunicación

### Formato General de Mensajes

```
[TIPO_MENSAJE]|[TIMESTAMP]|[TOKEN]|[DATOS]|[CHECKSUM]
```

### Flujo de Comunicación
#### 1. Establecer Conexión
```
Cliente → Servidor: Conexión TCP
Cliente ← Servidor: Conexión aceptada
```
#### 2. Autenticación
```
Cliente → Servidor: AUTH_REQUEST|1634567890|NULL|admin:admin123|CHECKSUM
Cliente ← Servidor: AUTH_RESPONSE|1634567890|9F8E7D6C5B4A3918|ADMIN:200|CHECKSUM
```
#### 3. Recepción de Telemetría
```
Cliente ← Servidor: TELEMETRY|1634567900|NULL|45.5:78.2:25.1:NORTH:6.2442:-75.5812|CHECKSUM
Cliente ← Servidor: TELEMETRY|1634567910|NULL|47.1:77.8:25.3:NORTHEAST:6.2444:-75.5810|CHECKSUM
```

#### 4. Envío de Comandos (Solo Admin)
```
Cliente → Servidor: COMMAND_REQUEST|1634567920|9F8E7D6C5B4A3918|SPEED_UP:10|CHECKSUM
Cliente ← Servidor: COMMAND_RESPONSE|1634567920|NULL|200:Speed increased to 55.5 km/h|CHECKSUM
```
### Comandos Disponibles
-  `SPEED_UP:valor` - Incrementar velocidad
-  `SLOW_DOWN:valor` - Decrementar velocidad
-  `TURN_LEFT:grados` - Girar a la izquierda
-  `TURN_RIGHT:grados` - Girar a la derecha
-  `STOP` - Detener vehículo
### Códigos de Estado
-  `200`: Éxito
-  `400`: Solicitud incorrecta
-  `401`: No autorizado
-  `403`: Prohibido
-  `500`: Error interno del servidor

## Estructura del Código
### Servidor en C
#### Funciones Principales

```c
int  main(int  argc, char *argv[]) // Función principal
void *client_handler(void *arg) // Manejo de clientes individuales
void *telemetry_broadcaster(void *arg) // Broadcast de telemetría
void  process_command(int  client_idx, ...) // Procesamiento de comandos
int  authenticate_user(const  char *username, const  char *password) // Autenticación
void  log_message(const  char *client_info, const  char *type, const  char *message) // Logging
```
#### Estructuras de Datos

```c
typedef  struct {
int socket; // Socket del cliente
struct sockaddr_in address; // Dirección del cliente
char  token[TOKEN_SIZE + 1]; // Token de autenticación
char  username[MAX_USERNAME + 1]; // Nombre de usuario
int user_type; // 1: ADMIN, 0: OBSERVER
int active; // Estado activo
time_t last_activity; // Última actividad
} client_t;
typedef  struct {
float speed; // Velocidad actual
float battery; // Nivel de batería
float temperature; // Temperatura interna
char  direction[20]; // Dirección de movimiento
float latitude; // Latitud GPS
float longitude; // Longitud GPS
int running; // Estado del vehículo
} vehicle_data_t;
```
### Cliente Python

#### Clases Principales

```python
class  VehicleClient:
def  __init__(self) # Inicialización
def  setup_gui(self) # Configuración GUI
def  connect(self) # Conexión al servidor
def  authenticate(self) # Autenticació
def  send_command(self, command, params) # Envío de comandos
def  receive_messages(self) # Recepción de mensajes
def  process_telemetry(self, data) # Procesamiento de telemetría
def  update_telemetry_display(self) # Actualización de interfaz
```
### Cliente Java

#### Métodos Principales
```java
private  void  initializeGUI() // Inicialización de interfaz
private  void  connect() // Conexión al servidor
private  void  authenticate() // Autenticación
private  void  sendCommand(String command, String params) // Envío de comandos
private  void  receiveMessages() // Recepción de mensajes
private  void  handleTelemetry(String data) // Manejo de telemetría
private  void  updateTelemetryDisplay(...) // Actualización de interfaz
```

## Pruebas y Validación

### 1. Pruebas Unitarias del Servidor

```bash
# Probar compilación
make  clean && make
# Probar inicio del servidor
./server  8080  logs/test.log 
# Probar conexión básica
telnet  localhost  8080
```
### 2. Pruebas de Conectividad

```bash
# Terminal 1: Servidor
./server  8080  logs/test.log
# Terminal 2: Cliente Python
python3  Client.py
# Terminal 3: Cliente Java
java  VehicleClientJava.java
# Terminal 4: Monitoreo de logs
tail  -f  logs/test.log
```
### 3. Escenarios de Prueba

#### Escenario 1: Autenticación Exitosa
1. Conectar cliente
2. Usar credenciales correctas (admin/admin123)
3. Verificar recepción de token
4. Confirmar habilitación de botones de comando

#### Escenario 2: Autenticación Fallida

1. Conectar cliente
2. Usar credenciales incorrectas
3. Verificar mensaje de error
4. Confirmar botones deshabilitados

#### Escenario 3: Múltiples Clientes
1. Conectar cliente Python como admin
2. Conectar cliente Java como observer
3. Verificar que ambos reciben telemetría
4. Verificar que solo admin puede enviar comandos

#### Escenario 4: Comandos de Control

1. Autenticarse como admin
2. Enviar comando SPEED_UP:10
3. Verificar cambio en telemetría
4. Verificar log en servidor

#### Escenario 5: Desconexión Abrupta

1. Conectar cliente
2. Cerrar cliente bruscamente
3. Verificar limpieza en servidor
4. Verificar log de desconexión

### 4. Métricas de Rendimiento
#### Capacidad

-  **Clientes simultáneos**: Hasta 50 (configurable en MAX_CLIENTS)
-  **Frecuencia de telemetría**: Cada 10 segundos
-  **Latencia de comandos**: < 100ms típica
#### Recursos
-  **Memoria del servidor**: ~2MB + 50KB por cliente
-  **CPU del servidor**: < 5% en condiciones normales
-  **Ancho de banda**: ~1KB/cliente cada 10s para telemetría

## Resolución de Problemas

### Problemas Comunes
#### 1. Error "Address already in use"
**Síntoma**: El servidor no puede iniciar en el puerto especificado
**Solución**:

```bash
# Encontrar proceso usando el puerto
lsof  -i  :8080
netstat  -tulpn | grep  8080
# Terminar proceso
kill  -9 <PID>
# O usar puerto diferente
./server  8081  logs/vehicle.log
```
#### 2. Cliente no puede conectar
**Síntomas**: "Connection refused" o timeout
**Soluciones**:
1. Verificar que el servidor esté ejecutándose
2. Verificar firewall: `sudo ufw allow 8080`
3. Verificar IP/puerto correctos
4. Probar con telnet: `telnet localhost 8080`
#### 3. Autenticación falla constantemente
**Síntomas**: Credenciales correctas pero autenticación falla
**Soluciones**:
1. Verificar formato del mensaje (no hay caracteres especiales)
2. Revisar logs del servidor para ver mensaje recibido
3. Verificar que el cliente envía el formato correcto
#### 4. Telemetría no se recibe
**Síntomas**: Cliente conectado y autenticado pero no hay datos
**Soluciones**:
1. Verificar que el hilo de telemetría esté corriendo
2. Revisar logs del servidor
3. Verificar que el cliente esté procesando mensajes TELEMETRY
#### 5. Comandos no funcionan
**Síntomas**: Botones habilitados pero comandos no se ejecutan
**Soluciones**:
1. Verificar autenticación como ADMIN
2. Verificar token válido
3. Revisar formato del comando en logs
4. Verificar que el servidor procese COMMAND_REQUEST
#### 6. GUI no responde
**Síntomas**: Interfaz gráfica se congela
**Soluciones**:
1.  **Python**: Verificar que operaciones de red estén en hilos separados
2.  **Java**: Usar SwingUtilities.invokeLater() para actualizar GUI
3. Verificar que no hay bucles infinitos en receive_messages()
#### 7. Logs no se generan
**Síntomas**: Archivo de log vacío o no se crea
**Soluciones**:
```bash
# Verificar permisos
ls  -la  logs/
chmod  755  logs/
chmod  644  logs/vehicle.log
# Crear directorio si no existe
mkdir  -p  logs
# Verificar espacio en disco
df  -h
```
### Comandos de Diagnóstico
#### Monitoreo del Servidor
```bash
# Ver procesos activos
ps  aux | grep  server
# Ver conexiones de red
netstat  -an | grep  8080
# Monitorear logs en tiempo real
tail  -f  logs/vehicle.log
# Ver uso de memoria
top  -p $(pgrep  server)
```
#### Pruebas de Conectividad
```bash
# Probar conexión TCP básica
nc  -zv  localhost  8080
# Enviar mensaje manual
echo  "AUTH_REQUEST|$(date +%s)|NULL|admin:admin123|CHECKSUM" | nc  localhost  8080
# Verificar puerto abierto
lsof  -i  :8080
```
#### Depuración de Código
```bash
# Compilar con símbolos de depuración
gcc  -g  -pthread  -o  server  server.c
# Ejecutar con gdb
gdb  ./server
(gdb) run  8080  logs/debug.log
(gdb) bt  # Si hay crash
```
### Logs y Monitoreo
#### Formato de Logs del Servidor
```
[TIMESTAMP] [CLIENT_IP:PORT] [REQUEST/RESPONSE] [MESSAGE]
Ejemplos:
[2025-09-26 14:30:15] [192.168.1.100:54321] [CONNECT] Nueva conexión establecida
[2025-09-26 14:30:20] [192.168.1.100:54321] [REQUEST] AUTH_REQUEST|1727361020|NULL|admin:admin123|CHECKSUM
[2025-09-26 14:30:20] [192.168.1.100:54321] [RESPONSE] AUTH_RESPONSE|1727361020|9F8E7D6C5B4A3918|ADMIN:200|CHECKSUM
[2025-09-26 14:30:30] [192.168.1.100:54321] [REQUEST] COMMAND_REQUEST|1727361030|9F8E7D6C5B4A3918|SPEED_UP:10|CHECKSUM
```
#### Análisis de Logs

```bash
# Contar conexiones por día
grep  "CONNECT"  logs/vehicle.log | grep  "2025-09-26" | wc  -l
# Ver errores de autenticación
grep  "401\|403"  logs/vehicle.log
# Monitorear comandos más usados
grep  "COMMAND_REQUEST"  logs/vehicle.log | cut  -d'|'  -f4 | sort | uniq  -c
# Ver clientes más activos
grep  "REQUEST"  logs/vehicle.log | cut  -d' '  -f2 | sort | uniq  -c
```

## Extensiones y Mejoras Futuras

### 1. Mejoras de Seguridad
-  **Cifrado SSL/TLS**: Implementar conexiones seguras
-  **Autenticación mejorada**: Base de datos de usuarios, hashing de contraseñas
-  **Rate limiting**: Limitar número de comandos por minuto
-  **Logs de seguridad**: Detectar intentos de acceso no autorizado
### 2. Mejoras de Funcionalidad
-  **Persistencia de datos**: Guardar telemetría en base de datos
-  **API REST**: Interfaz web adicional
-  **Notificaciones**: Alertas por batería baja, errores, etc.
-  **Configuración dinámica**: Cambiar parámetros sin reiniciar
### 3. Mejoras de Rendimiento
-  **Pool de hilos**: Limitar recursos del servidor
-  **Compresión**: Reducir ancho de banda para telemetría
-  **Cache**: Optimizar respuestas frecuentes
-  **Balanceador de carga**: Múltiples instancias de servidor
### 4. Mejoras de Interfaz
-  **Dashboard web**: Interfaz moderna con JavaScript
-  **Gráficos en tiempo real**: Visualización de tendencias
-  **Mapas GPS**: Mostrar ubicación en mapa interactivo
-  **Aplicación móvil**: Cliente para Android/iOS
## Entrega del Proyecto
### Checklist de Entrega

- [x] Servidor compilado y funcionando

- [x] Cliente Python con GUI funcional

- [x] Cliente Java con GUI funcional

- [x] Makefile funcional

- [x] Documentación completa

- [ ] Video demostrativo (12 min)

- [x] Repositorio GitHub configurado

- [ ] Código comentado y limpio

- [ ] Pruebas realizadas

  

### Estructura del Repositorio GitHub

```
VehiculoAutonomo/
	Server/
		Server.c # Código principal del servidor
		Makefile # Archivo de compilación
		logs/ # Directorio para logs
		server # Ejecutable compilado
	ClientPy/
		Client.py # Cliente Python con GUI
	ClientJava/
		VehicleClientJava.java # Cliente Java con Swing
		VehicleClientJava.class # Clase compilada
		VehicleClientJava$1.class #Clase compilada
	docs/
		Especificaciones.md # Especificación del protocolo
	tests/
		test_scripts/ # Scripts de prueba
README.md # Instrucciones básicas
demo/
	screenshots/ # Capturas de pantalla
		Video # Enlace al video
		Archivos PNG # Evidencias de Compilacion

```
### Video Demostrativo (12 minutos)

1.  **Introducción (1 min)**: Explicación del proyecto y objetivos

2.  **Arquitectura (2 min)**: Diagrama y explicación componentes

3.  **Servidor (3 min)**: Compilación, ejecución y logs

4.  **Cliente Python (2 min)**: Demostración de conexión, autenticación, telemetría

5.  **Cliente Java (2 min)**: Misma demostración con diferente interfaz

6.  **Comandos de control (1 min)**: Mostrar funcionalidad admin

7.  **Conclusiones (1 min)**: Conclusiones
### Recursos Adicionales
#### Referencias 

- [Berkeley Sockets Guide](https://beej.us/guide/bgnet/)

- [TCP/IP Protocol Suite](https://datatracker.ietf.org/doc/rfc793/)

- [POSIX Threads Programming](https://computing.llnl.gov/tutorials/pthreads/)

- [Tkinter Documentation](https://docs.python.org/3/library/tkinter.html)

- [Java Swing Tutorial](https://docs.oracle.com/javase/tutorial/uiswing/)

## Conclusión
Este proyecto proporciona una implementación completa de un sistema de telemetría para vehículos autónomos, demostrando conceptos fundamentales de programación de red, protocolos de aplicación, y desarrollo de interfaces gráficas. La solución cumple con todos los requerimientos especificados y proporciona una base sólida para futuras correcciones. El sistema ha sido diseñado con énfasis en la escalabilidad y facilidad de uso.
