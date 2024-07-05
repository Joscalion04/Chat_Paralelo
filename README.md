# Chat_Paralelo
Repositorio para un sistema de chat desarrollado en C utilizando sockets TCP/IP, diseñado para permitir a múltiples usuarios conectarse simultáneamente, participar en conversaciones y enviar mensajes en tiempo real.

## Objetivos
El objetivo de este proyecto es desarrollar un sistema de chat utilizando programación en C y sockets, tanto para el servidor como para el cliente. El sistema permitirá a múltiples usuarios conectarse simultáneamente, participar en diferentes conversaciones y enviar mensajes en tiempo real.

## Características

### Servidor
- **Gestión de conexiones:** Maneja múltiples conexiones de clientes utilizando sockets TCP/IP.
- **Autenticación:** Permite a los usuarios autenticarse proporcionando un nombre de usuario y seleccionando una conversación existente o creando una nueva.
- **Gestión de conversaciones:** Administra múltiples conversaciones, permitiendo a los usuarios enviar y recibir mensajes en cada una de ellas.
- **Persistencia de mensajes:** Almacena mensajes anteriores en archivos locales para cada conversación.

### Cliente
- **Interfaz de usuario simple:** Proporciona una interfaz de línea de comandos para enviar mensajes y recibir mensajes entrantes.
- **Conexión al servidor:** Conecta al servidor a través de sockets TCP/IP y maneja la comunicación bidireccional.
- **Selección de conversación:** Permite al usuario unirse a una conversación existente o crear una nueva ingresando el nombre de la conversación.
- **Finalización de sesión:** Permite al usuario salir de la conversación y desconectarse del servidor.

## Funcionalidades
- **Chat en tiempo real:** Los mensajes enviados por un usuario son recibidos instantáneamente por otros usuarios en la misma conversación.
- **Gestión de errores:** Maneja posibles errores de red y de conexión de manera adecuada para garantizar la estabilidad del sistema.
- **Seguridad básica:** Aunque no se implementan funciones avanzadas de seguridad, el sistema básico asegura que los usuarios puedan comunicarse de manera confiable y eficiente.

## Tecnologías Utilizadas
- **Lenguaje de Programación:** C
- **Bibliotecas y APIs:** POSIX Sockets, pthreads (para hilos), semáforos y mutexes para sincronización.

## Creditos

Este repositorio fue realizado como proyecto universitario.
Integrantes:
 - Raquel Hernández Campos
 - David Serrano Medrano
 - Joseph Leon Cabezas (Joscalion04)
