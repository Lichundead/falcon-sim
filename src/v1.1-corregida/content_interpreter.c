/*
 * content_interpreter.c - Intérprete de contenido (equivalente al motor del
 * sensor que evalúa las instancias de plantilla contra cada evento IPC).
 * v1.1: verificación de límites en tiempo de ejecución; un criterio que
 * apunta a un campo sin valor se trata como "no coincide" en lugar de leer
 * fuera del arreglo.
 */
#include <string.h>
#include "channel_file.h"
#include "content_interpreter.h"

static int pattern_match(const char *pattern, const char *value)
{
    size_t n = strlen(pattern);
    int match;

    if ((n > 0U) && (pattern[n - 1U] == '*')) {
        match = (strncmp(pattern, value, n - 1U) == 0) ? 1 : 0;
    } else {
        match = (strcmp(pattern, value) == 0) ? 1 : 0;
    }
    return match;
}

static int match_instance(const template_instance_t *inst, const char *values[], int n_values)
{
    int i;
    int matched = 1;

    for (i = 0; (matched != 0) && (i < inst->n_criteria); i++) {
        if (strcmp(inst->criteria[i], "*") != 0) {
            if ((i >= n_values) || (values[i] == NULL)) {
                matched = 0;                  /* campo sin valor: no coincide */
            } else {
                matched = pattern_match(inst->criteria[i], values[i]);
            }
        }
    }
    return matched;
}

int ci_evaluate(const channel_file_t *cf, const char *values[], int n_values)
{
    int i;
    int hits = 0;

    for (i = 0; i < cf->n_instances; i++) {
        if (match_instance(&cf->instances[i], values, n_values) != 0) {
            hits++;
        }
    }
    return hits;
}
