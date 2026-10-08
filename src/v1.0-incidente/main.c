/*
 * main.c - CLI del simulador del sensor.
 *   falcon_sim info     <channel_file>
 *   falcon_sim validate <channel_file>
 *   falcon_sim run      <channel_file> [--events N] [--seed S] [--inputs K]
 * Códigos de salida: 0 ok/aceptado, 1 rechazado, 2 error de uso o de carga.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "channel_file.h"
#include "content_validator.h"
#include "content_interpreter.h"
#include "sensor_events.h"

static int cmd_info(const channel_file_t *cf)
{
    int i, j;

    printf("{\"template\":\"%s\",\"declared_fields\":%d,\"sensor_inputs\":%d,\"instances\":[",
           cf->template_name, cf->declared_fields, SENSOR_INPUTS);
    for (i = 0; i < cf->n_instances; i++) {
        const template_instance_t *inst = &cf->instances[i];
        int first = 1;

        printf("%s{\"id\":\"%s\",\"n_criteria\":%d,\"non_wildcard\":[",
               i ? "," : "", inst->id, inst->n_criteria);
        for (j = 0; j < inst->n_criteria; j++) {
            if (strcmp(inst->criteria[j], "*") != 0) {
                printf("%s%d", first ? "" : ",", j + 1);
                first = 0;
            }
        }
        printf("]}");
    }
    printf("]}\n");
    return 0;
}

static int cmd_validate(const channel_file_t *cf)
{
    char reason[256] = "";

    if (cv_validate(cf, reason)) {
        printf("ACEPTADO\n");
        return 0;
    }
    printf("RECHAZADO: %s\n", reason);
    return 1;
}

static int cmd_run(const channel_file_t *cf, int events, unsigned int seed, int n_inputs)
{
    ipc_event_t ev;
    int e, hits = 0;

    for (e = 0; e < events; e++) {
        sensor_synthetic_event(&ev, seed + (unsigned int)e);
        if (n_inputs > 0) {
            /* modo prueba: número de entradas controlado (valores frontera) */
            const char **values = malloc(sizeof(char *) * (size_t)n_inputs);
            sensor_build_inputs(&ev, values, n_inputs);
            hits += ci_evaluate(cf, values, n_inputs);
            free(values);
        } else {
            /* modo producción: el sensor entrega SENSOR_INPUTS valores */
            const char *values[SENSOR_INPUTS];
            sensor_build_inputs(&ev, values, SENSOR_INPUTS);
            hits += ci_evaluate(cf, values, SENSOR_INPUTS);
        }
    }
    printf("eventos=%d coincidencias=%d\n", events, hits);
    return 0;
}

int main(int argc, char **argv)
{
    channel_file_t *cf;
    int i, rc, events = 100, n_inputs = 0;
    unsigned int seed = 42;

    if (argc < 3) {
        fprintf(stderr, "uso: %s info|validate|run <channel_file> [opciones]\n", argv[0]);
        return 2;
    }
    cf = cf_load(argv[2]);
    if (cf == NULL) {
        fprintf(stderr, "no se pudo cargar %s\n", argv[2]);
        return 2;
    }
    for (i = 3; i + 1 < argc; i += 2) {
        if (strcmp(argv[i], "--events") == 0)
            events = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--seed") == 0)
            seed = (unsigned int)atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--inputs") == 0)
            n_inputs = atoi(argv[i + 1]);
    }

    if (strcmp(argv[1], "info") == 0)
        rc = cmd_info(cf);
    else if (strcmp(argv[1], "validate") == 0)
        rc = cmd_validate(cf);
    else if (strcmp(argv[1], "run") == 0)
        rc = cmd_run(cf, events, seed, n_inputs);
    else
        rc = 2;
    cf_free(cf);
    return rc;
}
