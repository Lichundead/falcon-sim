/*
 * channel_file.h - Modelo del Channel File 291 (reproducción didáctica).
 * v1.1: el sensor entrega los 21 valores que declara la plantilla IPC
 * (remediación descrita en CrowdStrike, 2024) y el contrato se verifica
 * en tiempo de compilación.
 */
#ifndef CHANNEL_FILE_H
#define CHANNEL_FILE_H

#define TEMPLATE_FIELDS 21   /* campos que declara la plantilla IPC */
#define SENSOR_INPUTS   21   /* valores que el sensor entrega al intérprete */
#define MAX_INSTANCES   32
#define MAX_CRITERION   64

_Static_assert(SENSOR_INPUTS >= TEMPLATE_FIELDS,
               "el sensor debe entregar al menos los campos que declara la plantilla");

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
