#ifndef CONTENT_INTERPRETER_H
#define CONTENT_INTERPRETER_H

#include "channel_file.h"

/* Devuelve el número de instancias que coinciden con el evento. */
int ci_evaluate(const channel_file_t *cf, const char *values[], int n_values);

#endif
