// Tarea Programada #2
// Grupo#2
// Estudiantes:
// David Serrano Medrano
// Joseph León Cabezas
// Raquel Hernandez Campos

// Usuario
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 4096
#define PUERTO_SERVIDOR 7000

typedef struct {
    char mensaje_texto[BUFFER_SIZE];
    char usuario[50];
    char conversacion_nombre[50];
} mensaje_completo;

int conectar();
void *recibir_mensajes(void *socket_usuario);
void *enviar_mensajes(void *arg);

pthread_mutex_t mutex_envio;

int conectar() {
    int sockfd;
    struct hostent *he;
    struct sockaddr_in su_direccion;

    if ((he = gethostbyname("localhost")) == NULL) {
        herror("Error en el nombre del host");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error en la creación del socket");
        exit(1);
    }

    su_direccion.sin_family = AF_INET;
    su_direccion.sin_port = htons(PUERTO_SERVIDOR);
    su_direccion.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(su_direccion.sin_zero), 0, 8);

    if (connect(sockfd, (struct sockaddr *)&su_direccion, sizeof(struct sockaddr)) == -1) {
        perror("Error en la conexión al servidor");
        exit(1);
    }

    return sockfd;
}

void *recibir_mensajes(void *socket_usuario) {
    int socket_fd = *((int *)socket_usuario);
    mensaje_completo ms;
    int bytes_recibidos;

    while (1) {
        bytes_recibidos = read(socket_fd, &ms, sizeof(mensaje_completo));
    
        if (bytes_recibidos <= 0) {
              close(socket_fd);
              pthread_exit(NULL);
        }

        pthread_mutex_lock(&mutex_envio);
        if (strcmp(ms.usuario, "Historial") == 0) {
            printf("[Historial] %s\n", ms.mensaje_texto);
            pthread_mutex_unlock(&mutex_envio);
            continue;
        }
        else{
           printf("%s: %s\n", ms.usuario, ms.mensaje_texto);
        } 
        pthread_mutex_unlock(&mutex_envio);
    }
}

void *enviar_mensajes(void *arg) {
    int socket_fd = *((int *)((void **)arg)[0]);
    char *nombre_usuario = (char *)((void **)arg)[1];
    char *nombre_conversacion = (char *)((void **)arg)[2];
    mensaje_completo ms;
    strcpy(ms.usuario, nombre_usuario);
    strcpy(ms.conversacion_nombre, nombre_conversacion);
    char buffer[BUFFER_SIZE];
    
    while (1) {
       if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            close(socket_fd);
            pthread_exit(NULL);
        }
    
        if (buffer[0] == 27 && buffer[1] == '\n') {
           close(socket_fd);
           pthread_exit(NULL);
        }
    
        strncpy(ms.mensaje_texto, buffer, BUFFER_SIZE - 1);
        ms.mensaje_texto[BUFFER_SIZE - 1] = '\0';
        ms.mensaje_texto[strcspn(ms.mensaje_texto, "\n")] = '\0';
  
        pthread_mutex_lock(&mutex_envio);
        if (write(socket_fd, &ms, sizeof(mensaje_completo)) == -1) {
             close(socket_fd);
             pthread_mutex_unlock(&mutex_envio);
             pthread_exit(NULL);
        }
        pthread_mutex_unlock(&mutex_envio);
        memset(buffer, 0, BUFFER_SIZE);
    }
}

int main() {
    int socket_usuario;
    pthread_t hilo_recibir, hilo_enviar;
    mensaje_completo ms;
    char nombre_usuario[50];
    char nombre_conversacion[50];
    void *args[3];
    
    pthread_mutex_init(&mutex_envio, NULL);

     printf("\033[H\033[2J");
    printf("--------------CHAT--------------\n");
    printf("------Bienvenido al chat PELUCHE-----\n\n");

    printf("Ingrese su nombre de usuario (Nota: No más de 50 caracteres): ");
    fgets(nombre_usuario, 50, stdin);
    nombre_usuario[strcspn(nombre_usuario, "\n")] = '\0';
    
    socket_usuario = conectar();
    
      pid_t pidUsuario = fork();
    
    if(pidUsuario < 0){
        perror("Error para abrir el Servidor");
        exit(EXIT_FAILURE);
    }
    else if (pidUsuario == 0){
             
    while(1){
       printf("\033[H\033[2J");        
       printf("--------------MENU CONVERSACIONES--------------\n");
       
          // Enviar nombre de usuario al servidor
          write(socket_usuario, nombre_usuario, 50);

          // Recibir lista de conversaciones
          char buffer[BUFFER_SIZE];
          read(socket_usuario, buffer, BUFFER_SIZE);
          printf("%s\n\n", buffer);

      printf("Ingrese el nombre de la conversación a unirse o crear (Nota: No más de 50 caracteres): ");
      fgets(nombre_conversacion, 50, stdin);
      nombre_conversacion[strcspn(nombre_conversacion, "\n")] = '\0';

    // Enviar nombre de conversación al servidor
    write(socket_usuario, nombre_conversacion, 50);

    args[0] = &socket_usuario;
    args[1] = nombre_usuario;
    args[2] = nombre_conversacion;
    
     printf("\033[H\033[2J");
     printf("----------------%s-----------------\n\n",nombre_conversacion);
    printf("Presione la tecla <ESC> y <ENTER> para salir de la conversacion\n\n");

    pthread_mutex_init(&mutex_envio, NULL);

    pthread_create(&hilo_recibir, NULL, recibir_mensajes, (void *)&socket_usuario);
    pthread_create(&hilo_enviar, NULL, enviar_mensajes, (void *)args);

    pthread_join(hilo_enviar, NULL);
    pthread_cancel(hilo_recibir);
    pthread_join(hilo_recibir, NULL);
    close(socket_usuario);
    socket_usuario = conectar();
   
    }
           
           
           
           
           
           
    }
    else{
        wait(NULL);
    }
    
     pthread_mutex_destroy(&mutex_envio);
    close(socket_usuario);
    return 0;
}

