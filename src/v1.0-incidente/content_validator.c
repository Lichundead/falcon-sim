/*
 * content_validator.c - Validador de contenido previo a la publicación.
 * Defecto reproducido (RCA, hallazgo 2): compara cada instancia contra los
 * campos que declara la plantilla, no contra los valores que el sensor entrega.
 */
#include <stdio.h>
#include <string.h>
#include "channel_file.h"
#include "content_validator.h"

int cv_validate(const channel_file_t *cf, char *reason)
{
    int i, j;
    char msg[128];

    for (i = 0; i < cf->n_instances; i++) {
        const template_instance_t *inst = &cf->instances[i];

        if (inst->n_criteria != cf->declared_fields) {
            sprintf(msg, "instancia %s: %d criterios, plantilla declara %d",
                    inst->id, inst->n_criteria, cf->declared_fields);
            strcpy(reason, msg);
            return 0;
        }
        for (j = 0; j < inst->n_criteria; j++) {
            if (strlen(inst->criteria[j]) == 0) {
                sprintf(reason, "instancia %s: criterio %d vacío", inst->id, j + 1);
                return 0;
            }
        }
    }
    return 1;
}
