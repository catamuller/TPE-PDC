#ifndef MAP_H
#define MAP_H

#include <stdbool.h>

#define ERROR_MAP -2

typedef struct MapValue{
    int type;
    char * initialization;
} MapValue;

typedef struct MapEntry{
    char * key;
    MapValue value;
} MapEntry;

typedef struct Map{
    struct MapEntry * map;
    long capacity;
    long size;
} Map;

Map * mapInit(long capacity);
bool mapPut(Map * map, char * key, MapValue value);
bool mapRemove(Map * map, char * key);
MapValue mapGet(Map * map, char * key);
void mapFree(Map * map);
void _mapPrint(Map * map);

#endif
