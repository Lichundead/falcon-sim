#ifndef CONTENT_VALIDATOR_H
#define CONTENT_VALIDATOR_H

#include "channel_file.h"

/* 1 = aceptado, 0 = rechazado (motivo en reason) */
int cv_validate(const channel_file_t *cf, char *reason);

#endif
