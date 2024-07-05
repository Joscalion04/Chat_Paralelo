// Tarea Programada #2
// Grupo#2
// Estudiantes:
// David Serrano Medrano
// Joseph León Cabezas
// Raquel Hernandez Campos

// Servidor
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <semaphore.h>

#define PUERTO_SERVIDOR 7000
#define CONEXIONES 10
#define BUFFER_SIZE 4096

typedef struct {
    char mensaje_texto[BUFFER_SIZE];
    char usuario[50];
    char conversacion_nombre[50];
} mensaje_completo;

typedef struct {
    int cliente_socket;
    char nombre_usuario[50];
    char conversacion_nombre[50];
} usuario_conectado;

usuario_conectado usuarios[CONEXIONES];
char conversaciones[CONEXIONES][50];
int num_conversaciones = 0;

sem_t semaforo_conexiones;
pthread_mutex_t mutex_usuarios;
pthread_mutex_t mutex_conversaciones;

int abrir_conexion();
int aceptar_pedidos(int);
void manejar_cliente(int cliente_socket, int cliente_index);
void manejar_terminacion(int signo);
void autenticacion(int cliente_socket, int cliente_index);
void enviar_lista_conversaciones(int cliente_socket);
void enviar_mensajes_anteriores(int cliente_socket, const char *conversacion_nombre);

int abrir_conexion() {
    int sock_auxiliar;
    struct sockaddr_in mi_direccion;

    if ((sock_auxiliar = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "No se puede abrir la conexión\n");
        return -1;
    }

    int optval = 1;
    if (setsockopt(sock_auxiliar, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        fprintf(stderr, "No se pudo configurar la opción de socket\n");
        return -1;
    }

    mi_direccion.sin_family = AF_INET;
    mi_direccion.sin_port = htons(PUERTO_SERVIDOR);
    mi_direccion.sin_addr.s_addr = INADDR_ANY;
    memset(&(mi_direccion.sin_zero), 0, 8);

    if (bind(sock_auxiliar, (struct sockaddr *)&mi_direccion, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "No se pudo enlazar el socket\n");
        return -1;
    }

    if (listen(sock_auxiliar, CONEXIONES) == -1) {
        fprintf(stderr, "No se pudo escuchar en el socket\n");
        return -1;
    }

    return sock_auxiliar;
}

int aceptar_pedidos(int sockfd) {
    int nuevo_socket;
    struct sockaddr_in su_direccion;
    unsigned int sin_size = sizeof(struct sockaddr_in);

    if ((nuevo_socket = accept(sockfd, (struct sockaddr *)&su_direccion, &sin_size)) == -1) {
        fprintf(stderr, "Error al aceptar pedidos. Error %s\n", strerror(errno));
        return -1;
    } else {
        return nuevo_socket;
    }
}

void manejar_cliente(int cliente_socket, int cliente_index) {
    mensaje_completo ms;
    int bytes_recibidos;

    while (1) {
        bytes_recibidos = read(cliente_socket, &ms, sizeof(mensaje_completo));
   
        if (bytes_recibidos <= 0) {
            printf("Cliente %s se ha desconectado\n", usuarios[cliente_index].nombre_usuario);
            close(cliente_socket);

            pthread_mutex_lock(&mutex_usuarios);
            usuarios[cliente_index].cliente_socket = -1;
            usuarios[cliente_index].conversacion_nombre[0] = '\0';
            pthread_mutex_unlock(&mutex_usuarios);

            sem_post(&semaforo_conexiones);
            pthread_exit(NULL);
        }

        // Guardar mensaje en archivo
        FILE *archivo;
        char nombreArchivo[100];
        sprintf(nombreArchivo, "conversacion_%s.txt", ms.conversacion_nombre);
        archivo = fopen(nombreArchivo, "a");
        if (archivo != NULL) {
            fprintf(archivo, "%s: %s\n", ms.usuario, ms.mensaje_texto);
            fclose(archivo);
        }

        printf("Conversación %s - %s: %s\n", ms.conversacion_nombre, ms.usuario, ms.mensaje_texto);

        // Reenviar mensaje a todos los usuarios en la misma conversación
        pthread_mutex_lock(&mutex_usuarios);
        for (int i = 0; i < CONEXIONES; i++) {
            if (usuarios[i].cliente_socket != -1 && usuarios[i].cliente_socket != cliente_socket && strcmp(usuarios[i].conversacion_nombre, ms.conversacion_nombre) == 0) {
                if (write(usuarios[i].cliente_socket, &ms, sizeof(mensaje_completo)) == -1) {
                    perror("Error al reenviar datos a otro cliente");
                }
            }
        }
        pthread_mutex_unlock(&mutex_usuarios);
    }
}

void autenticacion(int cliente_socket, int cliente_index) {
    char nombre_usuario[50];
    char nombre_conversacion[50];

    read(cliente_socket, nombre_usuario, 50);

    pthread_mutex_lock(&mutex_usuarios);
    strcpy(usuarios[cliente_index].nombre_usuario, nombre_usuario);
    usuarios[cliente_index].cliente_socket = cliente_socket;
    pthread_mutex_unlock(&mutex_usuarios);

    enviar_lista_conversaciones(cliente_socket);

    read(cliente_socket, nombre_conversacion, 50);

    pthread_mutex_lock(&mutex_conversaciones);
    int existe = 0;
    for (int i = 0; i < num_conversaciones; i++) {
        if (strcmp(conversaciones[i], nombre_conversacion) == 0) {
            existe = 1;
            break;
        }
    }
    if (!existe) {
        strcpy(conversaciones[num_conversaciones++], nombre_conversacion);
    }
    pthread_mutex_unlock(&mutex_conversaciones);

    pthread_mutex_lock(&mutex_usuarios);
    strcpy(usuarios[cliente_index].conversacion_nombre, nombre_conversacion);
    pthread_mutex_unlock(&mutex_usuarios);

    // Enviar mensajes anteriores
    enviar_mensajes_anteriores(cliente_socket, nombre_conversacion);
}

void enviar_lista_conversaciones(int cliente_socket) {
    char lista[BUFFER_SIZE] = "Lista de conversaciones:\n";
    pthread_mutex_lock(&mutex_conversaciones);
    for (int i = 0; i < num_conversaciones; i++) {
        char temp[100];
        sprintf(temp, "%s\n", conversaciones[i]);
        strcat(lista, temp);
    }
    pthread_mutex_unlock(&mutex_conversaciones);
    write(cliente_socket, lista, BUFFER_SIZE);
}

void enviar_mensajes_anteriores(int cliente_socket, const char *conversacion_nombre) {
    char nombreArchivo[100];
    sprintf(nombreArchivo, "conversacion_%s.txt", conversacion_nombre);
    FILE *archivo = fopen(nombreArchivo, "r");
    if (archivo != NULL) {
        mensaje_completo ms;
        char linea[BUFFER_SIZE];
        while (fgets(linea, BUFFER_SIZE, archivo) != NULL) {
            linea[strcspn(linea, "\n")] = '\0';
            strcpy(ms.mensaje_texto, linea);
            strcpy(ms.usuario, "Historial");
            strcpy(ms.conversacion_nombre, conversacion_nombre);
            write(cliente_socket, &ms, sizeof(mensaje_completo));
        }
        fclose(archivo);
    }
}

void *thread_func(void *arg) {
    int cliente_socket = *(int *)arg;
    int cliente_index;

    pthread_mutex_lock(&mutex_usuarios);
    for (int i = 0; i < CONEXIONES; i++) {
        if (usuarios[i].cliente_socket == -1) {
            cliente_index = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_usuarios);

    autenticacion(cliente_socket, cliente_index);
    manejar_cliente(cliente_socket, cliente_index);
    return NULL;
}

int main() {
    int socket_servidor, socket_cliente;
    struct sockaddr_in cliente_direccion;
    unsigned int sin_size = sizeof(struct sockaddr_in);
    pthread_t threads[CONEXIONES];

    socket_servidor = abrir_conexion();
    if (socket_servidor == -1) {
        exit(1);
    }

    sem_init(&semaforo_conexiones, 0, CONEXIONES);
    pthread_mutex_init(&mutex_usuarios, NULL);
    pthread_mutex_init(&mutex_conversaciones, NULL);

    for (int i = 0; i < CONEXIONES; i++) {
        usuarios[i].cliente_socket = -1;
    }
    
    pid_t pidServidor = fork();
    
    if(pidServidor < 0){
        perror("Error para abrir el Servidor");
        exit(EXIT_FAILURE);
    }
    else if (pidServidor == 0){
           printf("\033[H\033[2J");
    printf("-----------------SERVIDOR----------------\n\n");
    printf("Presione la tecla <CTRL> + <C>\n\n");
   printf("Servidor escuchando en el puerto %d\n", PUERTO_SERVIDOR);

    while (1) {
        sem_wait(&semaforo_conexiones);
        socket_cliente = aceptar_pedidos(socket_servidor);
        if (socket_cliente == -1) {
            sem_post(&semaforo_conexiones);
            continue;
        }
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, thread_func, (void *)&socket_cliente);
    }

    }
    else{
        wait(NULL);
    }
    
    sem_destroy(&semaforo_conexiones);
    pthread_mutex_destroy(&mutex_usuarios);
    pthread_mutex_destroy(&mutex_conversaciones);
    close(socket_servidor);

    return 0;
}
