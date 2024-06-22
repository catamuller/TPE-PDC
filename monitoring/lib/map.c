#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "headers/map.h"

/**
 * Code taken from https://github.com/catamuller/TP-TLA/blob/development/src/main/c/backend/symbol-table/map.c
 * Original created by mquesada02 (contributor to this repo)
 * Modified by tm-sm
 */

/* PRIVATE FUNCTIONS */
int _calcHash(char * key, long mod) {
    long h = 0;
    for (int i = 0; key[i]; i++) {
        h = 31 * h + key[i];
    }
    return h % mod;
}

bool _mapPut(Map * map, char * key, MapValue value, int index, int startingIndex) {
    if (index == startingIndex) return false;
    if (map->map[index].key != NULL)
        return _mapPut(map, key, value, index+1%map->capacity, startingIndex);
    map->map[index].key = key;
    map->map[index].value = value;
    map->size++;
    return true;
}

void _mapPrint(Map * map) {
    printf("Map:\n");
    for(int i=0;i<map->capacity;i++) {
        printf("{Key: %s; Value: {%d,%s}}\n", map->map[i].key, map->map[i].value.type, map->map[i].value.initialization);
    }
}

bool _mapRemove(Map * map, char * key, int index, int startingIndex) {
    if (index == startingIndex || map->map[index].key == NULL) return false;
    if (strcmp(map->map[index].key, key) != 0)
        return _mapRemove(map, key, index+1%map->capacity, startingIndex);
    map->map[index].key = NULL;
    map->size--;
    return true;
}

MapValue _mapGet(Map * map, char * key, int index, int startingIndex) {
    if (index == startingIndex || map->map[index].key == NULL) return (MapValue) {0, NULL};
    if (strcmp(map->map[index].key, key) != 0)
        return _mapGet(map, key, index+1%map->capacity, startingIndex);
    return map->map[index].value;
}

/* PUBLIC FUNCTIONS */

Map * mapInit(long capacity) {
    Map * map = malloc(sizeof(Map));
    map->capacity = capacity;
    map->size = 0;
    MapEntry * mapEntry = calloc(capacity, sizeof(MapEntry));
    map->map = mapEntry;

    return map;
}

void mapFree(Map * map) {
    free(map->map);
    free(map);
}

bool mapPut(Map * map, char * key, MapValue value) {
    if (key == NULL || *key == '\0' || map->size == map->capacity) return false;
    int hash = _calcHash(key, map->capacity);
    if (map->map[hash].key != NULL) {
        return _mapPut(map, key, value, hash+1%map->capacity, hash);
    }
    map->map[hash].key = key;
    map->map[hash].value = value;
    map->size++;
    return true;
}

bool mapRemove(Map * map, char * key) {
    if (key == NULL || *key == '\0' || map->size == 0) return false;
    int hash = _calcHash(key, map->capacity);
    if (strcmp(map->map[hash].key, key) != 0)
        return _mapRemove(map, key, hash+1%map->capacity, hash);
    map->map[hash].key = NULL;
    map->size--;
    return true;
}

MapValue mapGet(Map * map, char * key) {
    if (key == NULL || *key == '\0' || map->size == 0) return (MapValue) {0, NULL};
    int hash = _calcHash(key, map->capacity);
    if (map->map[hash].key == NULL) return (MapValue) {0, NULL};
    if (strcmp(map->map[hash].key, key) != 0)
        return _mapGet(map, key, hash+1%map->capacity, hash);
    return map->map[hash].value;
}