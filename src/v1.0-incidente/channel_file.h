/*
 * channel_file.h - Modelo del Channel File 291 (reproducción didáctica).
 * Basado en CrowdStrike (2024), External Technical Root Cause Analysis.
 */
#ifndef CHANNEL_FILE_H
#define CHANNEL_FILE_H

#define TEMPLATE_FIELDS 21   /* campos que declara la plantilla IPC */
#define SENSOR_INPUTS   20   /* valores que el sensor entrega al intérprete */
#define MAX_INSTANCES   32
#define MAX_CRITERION   64

typedef struct {
    char id[16];
    char criteria[TEMPLATE_FIELDS][MAX_CRITERION];
    int  n_criteria;
} template_instance_t;

typedef struct {
    char template_name[16];
    int  declared_fields;
    template_instance_t instances[MAX_INSTANCES];
    int  n_instances;
} channel_file_t;

channel_file_t *cf_load(const char *path);
void cf_free(channel_file_t *cf);

#endif
