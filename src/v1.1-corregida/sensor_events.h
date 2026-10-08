#ifndef SENSOR_EVENTS_H
#define SENSOR_EVENTS_H

#define VALUE_LEN      32
#define MAX_RAW_FIELDS 24

typedef struct {
    unsigned long seq;
    char pipe_name[VALUE_LEN];
    char process[VALUE_LEN];
    char user[VALUE_LEN];
    char fields[MAX_RAW_FIELDS][VALUE_LEN];   /* valores crudos capturados del sistema */
} ipc_event_t;

void sensor_synthetic_event(ipc_event_t *ev, unsigned int seed);
int  sensor_build_inputs(const ipc_event_t *ev, const char *values[], int n);

#endif
