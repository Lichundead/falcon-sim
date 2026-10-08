/*
 * main.c - CLI del simulador del sensor.
 *   falcon_sim info     <channel_file>
 *   falcon_sim validate <channel_file>
 *   falcon_sim run      <channel_file> [--events N] [--seed S] [--inputs K]
 * Códigos de salida: 0 ok/aceptado, 1 rechazado, 2 error de uso o de carga.
 * v1.1: argumentos validados con strtol, malloc verificado, snprintf.
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
    int i;
    int j;

    (void)printf("{\"template\":\"%s\",\"declared_fields\":%d,\"sensor_inputs\":%d,\"instances\":[",
                 cf->template_name, cf->declared_fields, SENSOR_INPUTS);
    for (i = 0; i < cf->n_instances; i++) {
        const template_instance_t *inst = &cf->instances[i];
        int first = 1;

        (void)printf("%s{\"id\":\"%s\",\"n_criteria\":%d,\"non_wildcard\":[",
                     (i > 0) ? "," : "", inst->id, inst->n_criteria);
        for (j = 0; j < inst->n_criteria; j++) {
            if (strcmp(inst->criteria[j], "*") != 0) {
                (void)printf("%s%d", (first != 0) ? "" : ",", j + 1);
                first = 0;
            }
        }
        (void)printf("]}");
    }
    (void)printf("]}\n");
    return 0;
}

static int cmd_validate(const channel_file_t *cf)
{
    char reason[256] = "";
    int rc = 1;

    if (cv_validate(cf, reason, sizeof(reason)) != 0) {
        (void)printf("ACEPTADO\n");
        rc = 0;
    } else {
        (void)printf("RECHAZADO: %s\n", reason);
    }
    return rc;
}

static int run_event(const channel_file_t *cf, const ipc_event_t *ev, int n_inputs)
{
    int hits = 0;

    if (n_inputs > 0) {
        /* modo prueba: número de entradas controlado (valores frontera) */
        const char **values = calloc((size_t)n_inputs, sizeof(char *));
        if (values != NULL) {
            int n = sensor_build_inputs(ev, values, n_inputs);
            hits = ci_evaluate(cf, values, n);
            free(values);
        }
    } else {
        /* modo producción: el sensor entrega SENSOR_INPUTS valores */
        const char *values[SENSOR_INPUTS] = { NULL };
        int n = sensor_build_inputs(ev, values, SENSOR_INPUTS);
        hits = ci_evaluate(cf, values, n);
    }
    return hits;
}

static int cmd_run(const channel_file_t *cf, int events, unsigned int seed, int n_inputs)
{
    ipc_event_t ev;
    int e;
    int hits = 0;

    for (e = 0; e < events; e++) {
        sensor_synthetic_event(&ev, seed + (unsigned int)e);
        hits += run_event(cf, &ev, n_inputs);
    }
    (void)printf("eventos=%d coincidencias=%d\n", events, hits);
    return 0;
}

static int parse_int(const char *s, long min, long max, long *out)
{
    char *end = NULL;
    long v = strtol(s, &end, 10);
    int ok = 0;

    if ((end != s) && (*end == '\0') && (v >= min) && (v <= max)) {
        *out = v;
        ok = 1;
    }
    return ok;
}

static int parse_options(int argc, char **argv, long *events, long *seed, long *n_inputs)
{
    int i;
    int ok = 1;

    for (i = 3; (ok != 0) && ((i + 1) < argc); i += 2) {
        if (strcmp(argv[i], "--events") == 0) {
            ok = parse_int(argv[i + 1], 0L, 1000000L, events);
        } else if (strcmp(argv[i], "--seed") == 0) {
            ok = parse_int(argv[i + 1], 0L, 2147483647L, seed);
        } else if (strcmp(argv[i], "--inputs") == 0) {
            ok = parse_int(argv[i + 1], 1L, (long)MAX_RAW_FIELDS, n_inputs);
        } else {
            ok = 0;
        }
    }
    return ok;
}

int main(int argc, char **argv)
{
    channel_file_t *cf = NULL;
    long events = 100L;
    long seed = 42L;
    long n_inputs = 0L;
    int rc = 2;

    if (argc < 3) {
        (void)fprintf(stderr, "uso: %s info|validate|run <channel_file> [opciones]\n", argv[0]);
    } else if (parse_options(argc, argv, &events, &seed, &n_inputs) == 0) {
        (void)fprintf(stderr, "opción inválida\n");
    } else {
        cf = cf_load(argv[2]);
        if (cf == NULL) {
            (void)fprintf(stderr, "no se pudo cargar %s\n", argv[2]);
        } else if (strcmp(argv[1], "info") == 0) {
            rc = cmd_info(cf);
        } else if (strcmp(argv[1], "validate") == 0) {
            rc = cmd_validate(cf);
        } else if (strcmp(argv[1], "run") == 0) {
            rc = cmd_run(cf, (int)events, (unsigned int)seed, (int)n_inputs);
        } else {
            rc = 2;
        }
        cf_free(cf);
    }
    return rc;
}
