/*
 * content_validator.c - Validador de contenido previo a la publicación.
 * v1.1: además de la plantilla, verifica que ningún criterio distinto del
 * comodín apunte a un campo que el sensor no entrega.
 */
#include <stdio.h>
#include <string.h>
#include "channel_file.h"
#include "content_validator.h"

static int validate_instance(const template_instance_t *inst, int declared,
                             char *reason, size_t size)
{
    int j;
    int ok = 1;

    if (inst->n_criteria != declared) {
        (void)snprintf(reason, size, "instancia %s: %d criterios, plantilla declara %d",
                       inst->id, inst->n_criteria, declared);
        ok = 0;
    }
    for (j = 0; (ok != 0) && (j < inst->n_criteria); j++) {
        if (inst->criteria[j][0] == '\0') {
            (void)snprintf(reason, size, "instancia %s: criterio %d vacío", inst->id, j + 1);
            ok = 0;
        } else if ((j >= SENSOR_INPUTS) && (strcmp(inst->criteria[j], "*") != 0)) {
            (void)snprintf(reason, size, "instancia %s: campo %d sin valor de entrada en el sensor",
                           inst->id, j + 1);
            ok = 0;
        } else {
            /* criterio válido */
        }
    }
    return ok;
}

int cv_validate(const channel_file_t *cf, char *reason, size_t size)
{
    int i;
    int ok = 1;

    if (cf->declared_fields > SENSOR_INPUTS) {
        (void)snprintf(reason, size, "plantilla declara %d campos, el sensor entrega %d",
                       cf->declared_fields, SENSOR_INPUTS);
        ok = 0;
    }
    for (i = 0; (ok != 0) && (i < cf->n_instances); i++) {
        ok = validate_instance(&cf->instances[i], cf->declared_fields, reason, size);
    }
    return ok;
}
