/*
 * sensor_events.c - Generación de eventos IPC sintéticos y construcción de
 * los valores de entrada que el sensor entrega al intérprete.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sensor_events.h"

static const char *PIPES[] = { "\\\\.\\pipe\\spoolss", "\\\\.\\pipe\\lsass",
                               "\\\\.\\pipe\\msagent_42", "\\\\.\\pipe\\wkssvc" };
static const char *PROCS[] = { "svchost.exe", "explorer.exe", "rundll32.exe",
                               "powershell.exe" };

void sensor_synthetic_event(ipc_event_t *ev, unsigned int seed)
{
    int i;

    srand(seed);
    memset(ev, 0, sizeof(*ev));
    ev->seq = (unsigned long)rand();
    strcpy(ev->pipe_name, PIPES[rand() % 4]);
    strcpy(ev->process, PROCS[rand() % 4]);
    sprintf(ev->user, "user%d", rand() % 100);
    for (i = 0; i < 24; i++)
        sprintf(ev->fields[i], "v%02d_%d", i + 1, rand() % 10);
}

int sensor_build_inputs(const ipc_event_t *ev, const char *values[], int n)
{
    int i;

    values[0] = ev->pipe_name;
    values[1] = ev->process;
    values[2] = ev->user;
    for (i = 3; i < n; i++)
        values[i] = ev->fields[i];
    return n;
}

static int pattern_match(const char *pattern, const char *value)
{
    size_t n = strlen(pattern);

    if (n > 0 && pattern[n - 1] == '*')
        return strncmp(pattern, value, n - 1) == 0;
    return strcmp(pattern, value) == 0;
}

int sensor_filter(const ipc_event_t *ev, const char *pattern)
{
    return pattern_match(pattern, ev->pipe_name) ||
           pattern_match(pattern, ev->process);
}

void sensor_log_event(ipc_event_t ev)
{
    char line[64];

    sprintf(line, "[%lu] %s -> %s (%s)", ev.seq, ev.process, ev.pipe_name, ev.user);
    fprintf(stderr, "%s\n", line);
}
