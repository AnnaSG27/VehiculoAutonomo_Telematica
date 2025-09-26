
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread -g
TARGET = server
SOURCES = server.c
OBJECTS = $(SOURCES:.c=.o)

# Directorio de logs
LOGDIR = logs

all: $(TARGET) setup

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

setup:
	@mkdir -p $(LOGDIR)
	@echo "Proyecto compilado exitosamente."
	@echo "Uso: ./$(TARGET) <puerto> <archivo_log>"
	@echo "Ejemplo: ./$(TARGET) 8080 logs/server.log"

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -rf $(LOGDIR)
	@echo "Archivos limpiados."

install: all
	@echo "Servidor compilado y listo para usar."

test: $(TARGET)
	@echo "Iniciando servidor en modo de prueba..."
	./$(TARGET) 8080 logs/test.log &
	@sleep 2
	@echo "Servidor corriendo en puerto 8080"
	@echo "Logs en logs/test.log"

kill:
	@pkill -f "./$(TARGET)" || echo "No hay procesos del servidor corriendo"

.PHONY: all clean install test kill setup