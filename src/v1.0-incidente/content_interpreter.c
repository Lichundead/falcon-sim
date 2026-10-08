/*
 * content_interpreter.c - Intérprete de contenido (equivalente al motor del
 * sensor que evalúa las instancias de plantilla contra cada evento IPC).
 * Defecto reproducido (RCA, hallazgos 1 y 3): itera los 21 campos de la
 * plantilla sin comprobar que existan 21 valores de entrada.
 */
#include <string.h>
#include "channel_file.h"
#include "content_interpreter.h"

static int pattern_match(const char *pattern, const char *value)
{
    size_t n = strlen(pattern);

    if (n > 0 && pattern[n - 1] == '*')
        return strncmp(pattern, value, n - 1) == 0;
    return strcmp(pattern, value) == 0;
}

static int match_instance(const template_instance_t *inst, const char *values[])
{
    int i;
    int matched;

    for (i = 0; i < TEMPLATE_FIELDS; i++) {
        if (strcmp(inst->criteria[i], "*") == 0)
            continue;                         /* comodín: no lee la entrada */
        if (!pattern_match(inst->criteria[i], values[i]))
            return 0;
        matched = 1;
    }
    return matched;
}

int ci_evaluate(const channel_file_t *cf, const char *values[], int n_values)
{
    int i, hits = 0;

    for (i = 0; i < cf->n_instances; i++) {
        if (match_instance(&cf->instances[i], values))
            hits++;
    }
    return hits;
}
