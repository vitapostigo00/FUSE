# 🧱 Sistema de Archivos FUSE con Persistencia Binaria

Este proyecto implementa un **sistema de archivos virtual** en espacio de usuario mediante **[FUSE (Filesystem in Userspace)](https://github.com/libfuse/libfuse)**.
El sistema almacena tanto los **metadatos** (estructura de directorios, permisos, tiempos, enlaces, etc.) como los **bloques de datos** de forma **persistente** en ficheros binarios.

---

## 📚 Descripción General

El proyecto se compone de tres módulos principales y una interfaz FUSE que conecta todo el sistema:

| Módulo                                   | Archivo             | Descripción                                                                                                                  |
| ---------------------------------------- | ------------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| 🧩 **Interfaz FUSE**                     | `FUSE.c`            | Define las funciones requeridas por la API de FUSE (getattr, readdir, read, write, etc.) y las enlaza con la lógica interna. |
| 🧮 **Gestión de Metadatos (FileSystem)** | `FileSystemLib.c`   | Gestiona directorios, permisos, rutas, tiempos y enlaces mediante estructuras persistentes almacenadas en `fileSystem.bin`.  |
| 💾 **Gestión de Datos (DataSystem)**     | `DataSystemLib.c`   | Administra los bloques binarios de datos en `dataSystem.bin`, simulando el almacenamiento físico.                            |
| 🧰 **Utilidades**                        | `fileSystemUtils.c` | Incluye funciones auxiliares de manejo de rutas, concatenación y comprobaciones.                                             |

---

## ⚙️ Requisitos

* Linux o WSL con soporte FUSE
* `libfuse-dev` instalado
* Compilador GCC

Instalación de dependencias en Debian/Ubuntu:

```bash
sudo apt install libfuse-dev
```

---

## 🏗️ Compilación

Ejecuta el siguiente comando en la raíz del proyecto:

```bash
gcc FUSE.c FileSystemLib.c DataSystemLib.c fileSystemUtils.c -o myfs -lfuse
```

---

## 🚀 Ejecución

El programa necesita **dos argumentos obligatorios**:

1. El archivo binario donde se guardará el estado (`fileSystem.bin`)
2. El **punto de montaje** FUSE.

Ejemplo de uso:

```bash
mkdir /tmp/mountpoint
./myfs fileSystem.bin /tmp/mountpoint
```

El sistema quedará montado en `/tmp/mountpoint`.
Para desmontarlo:

```bash
fusermount -u /tmp/mountpoint
```

---

## 🧩 Arquitectura del Sistema

### 1️⃣ Estructuras Principales

#### `FileSystemInfo`

Contiene los metadatos de cada archivo o directorio:

```c
typedef struct {
    char path[MAX_PATH_SIZE];
    mode_t mode;
    uid_t uid;
    gid_t gid;
    time_t creation_time, last_access, last_modification;
    int nlink;
    int hasData;
    int siguiente;
} FileSystemInfo;
```

#### `DataSystemInfo`

Define la estructura de cada bloque de datos:

```c
typedef struct {
    int firstDataBlock;
    int currentBlockSize;
    int totalSize;
    int siguiente;
    char dat[BLOCKSIZE];
} DataSystemInfo;
```

---

### 2️⃣ Funcionamiento Interno

1. **Inicialización:**

   * Se mapean `fileSystem.bin` y `dataSystem.bin` en memoria mediante `mmap()`.
   * Si el sistema está vacío, se inicializa la raíz `/`.

2. **Gestión de Archivos:**

   * Cada archivo tiene un nodo con metadatos (`FileSystemLib`) y uno o más bloques de datos (`DataSystemLib`).
   * Los bloques están encadenados mediante punteros `siguiente`.

3. **Persistencia:**

   * Los cambios se reflejan directamente sobre los ficheros binarios, manteniendo el estado tras desmontar el sistema.

4. **Interfaz FUSE:**

   * FUSE intercepta llamadas POSIX (`open`, `mkdir`, `read`, etc.) y las redirige a las funciones internas implementadas en el código.

---

## 🧠 Funciones Soportadas

La estructura `fuse_operations` enlaza las siguientes funciones:

| Operación              | Función                  | Descripción                                                   |
| ---------------------- | ------------------------ | ------------------------------------------------------------- |
| 🔹 `init`              | `fs_init`                | Inicializa los sistemas de datos y metadatos.                 |
| 🔹 `getattr`           | `fs_getattr`             | Obtiene información sobre un archivo/directorio.              |
| 🔹 `readdir`           | `fs_readdir`             | Lista el contenido de un directorio.                          |
| 🔹 `mkdir` / `rmdir`   | `fs_mkdir`, `fs_rmdir`   | Crea o elimina directorios.                                   |
| 🔹 `create` / `unlink` | `fs_create`, `fs_unlink` | Crea o borra archivos.                                        |
| 🔹 `read` / `write`    | `fs_read`, `fs_write`    | Lee o escribe datos en bloques binarios.                      |
| 🔹 `rename`            | `fs_rename`              | Renombra archivos o directorios actualizando las rutas hijas. |
| 🔹 `truncate`          | `fs_truncate`            | Trunca el contenido de un archivo.                            |
| 🔹 `destroy`           | `fs_destroy`             | Guarda y libera los recursos al desmontar.                    |

---

## 🧪 Ejemplo de Uso

```bash
mkdir /mnt/fusefs
./myfs fileSystem.bin /mnt/fusefs

# Dentro del punto de montaje:
cd /mnt/fusefs
mkdir docs
echo "Hola FUSE" > docs/hola.txt
cat docs/hola.txt

# Persistencia tras desmontar:
fusermount -u /mnt/fusefs
./myfs fileSystem.bin /mnt/fusefs
cat docs/hola.txt  # -> "Hola FUSE"
```

---

## 🗂️ Archivos Generados

| Archivo             | Función                                                 |
| ------------------- | ------------------------------------------------------- |
| `fileSystem.bin`    | Contiene la estructura de directorios y metadatos.      |
| `dataSystem.bin`    | Contiene los bloques binarios de datos de los archivos. |
| `salida` (opcional) | Archivo temporal de depuración generado al desmontar.   |

---

## ⚠️ Limitaciones

* Tamaño máximo determinado por `DATASYSTEM_SIZE * BLOCKSIZE`.
* No se soportan enlaces simbólicos ni atributos extendidos (`xattr`).
* Los datos se gestionan exclusivamente en espacio de usuario.
* Los bloqueos de archivos (`flock`, `fcntl`) no están implementados.

---

## 🧰 Herramientas y Tecnologías

* **Lenguaje:** C
* **Bibliotecas:** FUSE, `mmap`, POSIX I/O
* **Entorno:** Linux / WSL
* **Compilador:** GCC

---

## 👨‍💻 Autores

Proyecto desarrollado con fines académicos para explorar el funcionamiento interno de un **sistema de archivos virtual** con soporte para **persistencia binaria**.

---

## 📎 Referencias

* [FUSE Documentation](https://github.com/libfuse/libfuse)
* [Linux man page: fuse(4)](https://man7.org/linux/man-pages/man4/fuse.4.html)
* [Memory Mapping (mmap)](https://man7.org/linux/man-pages/man2/mmap.2.html)

---

> 💡 **Sugerencia:**
> Puedes añadir un script `Makefile` con comandos `make build`, `make run` y `make clean` para automatizar la compilación y montaje.
