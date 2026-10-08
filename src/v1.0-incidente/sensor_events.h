#ifndef SENSOR_EVENTS_H
#define SENSOR_EVENTS_H

#define VALUE_LEN 32

typedef struct {
    unsigned long seq;
    char pipe_name[VALUE_LEN];
    char process[VALUE_LEN];
    char user[VALUE_LEN];
    char fields[24][VALUE_LEN];   /* valores crudos capturados del sistema */
} ipc_event_t;

void sensor_synthetic_event(ipc_event_t *ev, unsigned int seed);
int  sensor_build_inputs(const ipc_event_t *ev, const char *values[], int n);
int  sensor_filter(const ipc_event_t *ev, const char *pattern);
void sensor_log_event(ipc_event_t ev);

#endif
