# Implementación de un servidor SMTP

El trabajo consiste en la implementación de un servidor SMTP [RFC 5321](https://datatracker.ietf.org/doc/html/rfc5321). 

## Ejecución del servidor

Para poder compilar y correr el servidor hace falta correr el comando:

```
make clean all
```
desde la carpeta /src.

Una vez compilado se creará el archivo smtpd dentro de la carpeta /src. Para ejecutarlo hace falta el comando:

```
./smtpd
```

Para ver una lista de parámetros posibles, se puede usar el flag _-h_

## Requisitos

- Es necesario un entorno linux para poder compilar y ejecutar el código.
