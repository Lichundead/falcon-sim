/*
 * sensor_events.c - Generación de eventos IPC sintéticos y construcción de
 * los valores de entrada que el sensor entrega al intérprete.
 * v1.1: snprintf acotado, sin código muerto (sensor_filter, sensor_log_event)
 * y número de entradas limitado a MAX_RAW_FIELDS.
 * rand() se mantiene: solo genera datos de prueba reproducibles (desviación
 * documentada, no se usa con fines criptográficos).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sensor_events.h"

static const char *const PIPES[] = { "\\\\.\\pipe\\spoolss", "\\\\.\\pipe\\lsass",
                                     "\\\\.\\pipe\\msagent_42", "\\\\.\\pipe\\wkssvc" };
static const char *const PROCS[] = { "svchost.exe", "explorer.exe", "rundll32.exe",
                                     "powershell.exe" };

void sensor_synthetic_event(ipc_event_t *ev, unsigned int seed)
{
    int i;

    srand(seed);
    (void)memset(ev, 0, sizeof(*ev));
    ev->seq = (unsigned long)rand();
    (void)snprintf(ev->pipe_name, sizeof(ev->pipe_name), "%s", PIPES[rand() % 4]);
    (void)snprintf(ev->process, sizeof(ev->process), "%s", PROCS[rand() % 4]);
    (void)snprintf(ev->user, sizeof(ev->user), "user%d", rand() % 100);
    for (i = 0; i < MAX_RAW_FIELDS; i++) {
        (void)snprintf(ev->fields[i], sizeof(ev->fields[i]), "v%02d_%d", i + 1, rand() % 10);
    }
}

int sensor_build_inputs(const ipc_event_t *ev, const char *values[], int n)
{
    int i;
    int count = (n < MAX_RAW_FIELDS) ? n : MAX_RAW_FIELDS;

    if (count >= 3) {
        values[0] = ev->pipe_name;
        values[1] = ev->process;
        values[2] = ev->user;
        for (i = 3; i < count; i++) {
            values[i] = ev->fields[i];
        }
    } else {
        count = 0;
    }
    return count;
}
