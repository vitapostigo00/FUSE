# Nombre del ejecutable
TARGET = mainfuse

# Archivos fuente
SRCS = FUSE.c fileSystemLib.c fileSystemUtils.c dataSystemLib.c

# Dependencias de FUSE
FUSE_CFLAGS = $(shell pkg-config fuse --cflags)
FUSE_LIBS = $(shell pkg-config fuse --libs)

# Bandera para definir _FILE_OFFSET_BITS=64
DEFINES = -D_FILE_OFFSET_BITS=64

# Flags adicionales
CFLAGS = -Wall -g

# Directorio de montaje
MOUNT_DIR = mount_point

# Nombre del archivo de datos
DATA_FILE = fileSystem.bin

# Compilar el proyecto
$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) $(FUSE_CFLAGS) $(FUSE_LIBS) $(DEFINES) $(CFLAGS)

# Montar el sistema de ficheros
mount: $(TARGET)
	mkdir -p $(MOUNT_DIR)
	./$(TARGET) $(DATA_FILE) $(MOUNT_DIR)

# Desmontar el sistema de ficheros
umount:
	fusermount -u $(MOUNT_DIR)

# Limpiar los archivos generados
clean:
	rm -f $(TARGET)
	rm -rf $(MOUNT_DIR)

# Iniciar en modo depuración
debug: $(TARGET)
	mkdir -p $(MOUNT_DIR)
	./$(TARGET) -d $(DATA_FILE) $(MOUNT_DIR)
