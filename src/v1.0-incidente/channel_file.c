/*
 * channel_file.c - Carga del Channel File desde texto.
 * Formato:
 *   #template=IPC fields=21
 *   IPC-001|c1|c2|...|c21
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "channel_file.h"

static void split_criteria(char *line, template_instance_t *inst)
{
    char *tok = strtok(line, "|\n");
    int i;

    strcpy(inst->id, tok);
    inst->n_criteria = 0;
    for (i = 0; i < TEMPLATE_FIELDS; i++) {
        tok = strtok(NULL, "|\n");
        if (tok == NULL)
            break;
        strcpy(inst->criteria[i], tok);
        inst->n_criteria++;
    }
}

channel_file_t *cf_load(const char *path)
{
    char line[2048];
    FILE *f = fopen(path, "r");
    channel_file_t *cf;

    if (f == NULL)
        return NULL;

    cf = malloc(sizeof(channel_file_t));
    memset(cf, 0, sizeof(channel_file_t));

    if (fgets(line, sizeof(line), f) == NULL)
        return NULL;
    if (sscanf(line, "#template=%s fields=%d", cf->template_name,
               &cf->declared_fields) != 2)
        return NULL;

    while (fgets(line, sizeof(line), f) != NULL) {
        if (line[0] == '#' || line[0] == '\n')
            continue;
        split_criteria(line, &cf->instances[cf->n_instances]);
        cf->n_instances++;
    }
    fclose(f);
    return cf;
}

void cf_free(channel_file_t *cf)
{
    free(cf);
}
