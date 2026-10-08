#ifndef CONTENT_VALIDATOR_H
#define CONTENT_VALIDATOR_H

#include <stddef.h>
#include "channel_file.h"

/* 1 = aceptado, 0 = rechazado (motivo en reason, de tamaño size) */
int cv_validate(const channel_file_t *cf, char *reason, size_t size);

#endif
