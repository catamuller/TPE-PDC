# Implementación de un servidor SMTP

El trabajo consiste en la implementación de un servidor SMTP [RFC 5321](https://datatracker.ietf.org/doc/html/rfc5321).

## Materiales

Dentro de la carpeta /src se encuentra el archivo fuente del servidor.
En /config se disponse de los archivos necesarios para compilar el cliente y dentro de /monitoring, los archivos pertenecientes al monitoreo.

El informe del proyecto se encuentra en la carpeta /docs.

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

## Ejecución del cliente de configuración

Ir a la carpeta /config y repetir el comando:

```
make clean all
```

Se creara el binario client_config que se ejecuta con:

```
./client_config
```

## Ejecución del cliente de métricas

Ir a la carpeta /monitoring, ejecutando:

```
make clean all
```

Y analogamente, ejecutar el siguiente comando para correr:

```
./metrics
```

Para ver una lista de parámetros posibles, se puede usar el flag _-h_ en sus respectivos comnados de ejecuciones.

## Requisitos

- Es necesario un entorno linux para poder compilar y ejecutar el código.
