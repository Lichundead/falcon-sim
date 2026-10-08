/*
 * channel_file.c - Carga del Channel File desde texto.
 * Formato:
 *   #template=IPC fields=21
 *   IPC-001|c1|c2|...|c21
 * v1.1: copias acotadas, verificación de malloc, sin fugas en rutas de error
 * y límite de MAX_INSTANCES.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "channel_file.h"

static int copy_field(char *dst, size_t size, const char *src)
{
    int ok = 0;

    if (src != NULL) {
        size_t len = strlen(src);
        if (len < size) {
            (void)memcpy(dst, src, len + 1U);
            ok = 1;
        }
    }
    return ok;
}

static int split_criteria(char *line, template_instance_t *inst)
{
    const char *tok = strtok(line, "|\n");
    int ok = copy_field(inst->id, sizeof(inst->id), tok);

    inst->n_criteria = 0;
    while ((ok != 0) && (inst->n_criteria < TEMPLATE_FIELDS)) {
        tok = strtok(NULL, "|\n");
        if (tok == NULL) {
            break;
        }
        ok = copy_field(inst->criteria[inst->n_criteria], MAX_CRITERION, tok);
        if (ok != 0) {
            inst->n_criteria++;
        }
    }
    return ok;
}

static int load_body(FILE *f, channel_file_t *cf)
{
    char line[2048];
    int ok = 0;

    if ((fgets(line, (int)sizeof(line), f) != NULL) &&
        (sscanf(line, "#template=%15s fields=%d", cf->template_name,
                &cf->declared_fields) == 2)) {
        ok = 1;
        while ((ok != 0) && (fgets(line, (int)sizeof(line), f) != NULL)) {
            if ((line[0] != '#') && (line[0] != '\n')) {
                if (cf->n_instances >= MAX_INSTANCES) {
                    ok = 0;
                } else {
                    ok = split_criteria(line, &cf->instances[cf->n_instances]);
                    cf->n_instances++;
                }
            }
        }
    }
    return ok;
}

channel_file_t *cf_load(const char *path)
{
    channel_file_t *cf = NULL;
    FILE *f = fopen(path, "r");

    if (f != NULL) {
        cf = calloc(1U, sizeof(channel_file_t));
        if ((cf != NULL) && (load_body(f, cf) == 0)) {
            free(cf);
            cf = NULL;
        }
        (void)fclose(f);
    }
    return cf;
}

void cf_free(channel_file_t *cf)
{
    free(cf);
}
