# Makefile para compilar y gestionar el servidor del Vehículo Autónomo
# Uso principal: Compilación, limpieza, instalación y pruebas del servidor

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread -g
TARGET = Server
SOURCES = Server.c
OBJECTS = $(SOURCES:.c=.o)

# Directorio de logs
LOGDIR = logs

all: $(TARGET) setup  # Compila el servidor y prepara el entorno

$(TARGET): $(OBJECTS)  # Enlaza los objetos y genera el ejecutable
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c  # Compila archivos fuente a objetos
	$(CC) $(CFLAGS) -c $< -o $@

setup:  # Crea el directorio de logs y muestra instrucciones
	@mkdir -p $(LOGDIR)
	@echo "Proyecto compilado exitosamente."
	@echo "Uso: ./$(TARGET) <puerto> <archivo_log>"
	@echo "Ejemplo: ./$(TARGET) 8080 logs/server.log"

clean:  # Elimina archivos generados y directorio de logs
	rm -f $(OBJECTS) $(TARGET)
	rm -rf $(LOGDIR)
	@echo "Archivos limpiados."

install: all  # Compila y muestra mensaje de instalación
	@echo "Servidor compilado y listo para usar."

test: $(TARGET)  # Ejecuta el servidor en modo de prueba
	@echo "Iniciando servidor en modo de prueba..."
	./$(TARGET) 8080 logs/test.log &
	@sleep 2
	@echo "Servidor corriendo en puerto 8080"
	@echo "Logs en logs/test.log"

kill:  # Finaliza procesos del servidor
	@pkill -f "./$(TARGET)" || echo "No hay procesos del servidor corriendo"

.PHONY: all clean install test kill setup  # Evita conflictos con nombres de archivos