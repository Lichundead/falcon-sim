# Resumen del análisis - falcon-sim v1.0-incidente
Total de hallazgos estáticos: **141** (Cppcheck: 125, Clang SA: 16)
## Severidad × categoría
| Categoría | Crítico | Alto | Medio | Bajo | Total |
|---|---|---|---|---|---|
| Seguridad | 0 | 9 | 0 | 9 | 18 |
| Rendimiento | 4 | 2 | 0 | 0 | 6 |
| Codificación | 0 | 0 | 84 | 30 | 114 |
| Mantenibilidad | 0 | 0 | 0 | 3 | 3 |
| **Total** | 4 | 11 | 84 | 42 | 141 |

## Frecuencia por regla
| Herramienta | Regla | Ocurrencias | Archivos |
|---|---|---|---|
| Cppcheck | misra-c2012-17.7 | 26 | channel_file.c, content_validator.c, main.c, sensor_events.c |
| Cppcheck | misra-c2012-21.6 | 21 | channel_file.c, content_validator.c, main.c, sensor_events.c |
| Cppcheck | misra-c2012-15.6 | 19 | channel_file.c, content_interpreter.c, main.c, sensor_events.c |
| Cppcheck | misra-c2012-15.5 | 11 | channel_file.c, content_interpreter.c, content_validator.c, main.c, sensor_events.c |
| Cppcheck | misra-c2012-10.4 | 7 | content_interpreter.c, content_validator.c, sensor_events.c |
| Cppcheck | misra-c2012-12.3 | 7 | content_interpreter.c, content_validator.c, main.c |
| Clang SA | security.insecureAPI.strcpy | 5 | channel_file.c, content_validator.c, sensor_events.c |
| Clang SA | security.insecureAPI.rand | 5 | sensor_events.c |
| Cppcheck | misra-c2012-12.1 | 4 | channel_file.c, content_interpreter.c, main.c, sensor_events.c |
| Cppcheck | misra-c2012-21.3 | 4 | channel_file.c, main.c |
| Clang SA | security.insecureAPI.DeprecatedOrUnsafeBufferHandling | 4 | channel_file.c, main.c, sensor_events.c |
| Cppcheck | misra-c2012-21.7 | 3 | main.c |
| Cppcheck | resourceLeak | 2 | channel_file.c |
| Cppcheck | memleak | 2 | channel_file.c |
| Cppcheck | nullPointerOutOfMemory | 2 | channel_file.c |
| Cppcheck | misra-c2012-14.4 | 2 | content_interpreter.c, main.c |
| Cppcheck | misra-c2012-8.9 | 2 | sensor_events.c |
| Cppcheck | unusedFunction | 2 | sensor_events.c |
| Cppcheck | misra-c2012-5.9 | 2 | content_interpreter.c, sensor_events.c |
| Cppcheck | misra-c2012-8.7 | 2 | sensor_events.c |
| Cppcheck | invalidscanf | 1 | channel_file.c |
| Cppcheck | constVariablePointer | 1 | channel_file.c |
| Cppcheck | misra-c2012-11.5 | 1 | channel_file.c |
| Cppcheck | misra-c2012-2.7 | 1 | content_interpreter.c |
| Cppcheck | misra-c2012-7.4 | 1 | main.c |
| Cppcheck | misra-c2012-15.7 | 1 | main.c |
| Cppcheck | ctunullpointerOutOfMemory | 1 | sensor_events.c |
| Clang SA | unix.Stream | 1 | channel_file.c |
| Clang SA | unix.Malloc | 1 | channel_file.c |

## Hallazgos críticos y altos
| Severidad | Herramienta | Regla | Ubicación | Mensaje |
|---|---|---|---|---|
| Crítico | Cppcheck | resourceLeak | channel_file.c:41 | Resource leak: f |
| Crítico | Cppcheck | memleak | channel_file.c:41 | Memory leak: cf |
| Crítico | Cppcheck | resourceLeak | channel_file.c:44 | Resource leak: f |
| Crítico | Cppcheck | memleak | channel_file.c:44 | Memory leak: cf |
| Alto | Cppcheck | invalidscanf | channel_file.c:42 | sscanf() without field width limits can crash with huge input data. |
| Alto | Cppcheck | nullPointerOutOfMemory | channel_file.c:38 | If memory allocation fails, then there is a possible null pointer dereference: cf |
| Alto | Cppcheck | nullPointerOutOfMemory | channel_file.c:42 | If memory allocation fails, then there is a possible null pointer dereference: cf |
| Alto | Cppcheck | ctunullpointerOutOfMemory | sensor_events.c:33 | If memory allocation fails, then there is a possible null pointer dereference: values |
| Alto | Clang SA | security.insecureAPI.strcpy | channel_file.c:17 | Call to function 'strcpy' is insecure as it does not provide bounding of the memory buffer |
| Alto | Clang SA | security.insecureAPI.strcpy | channel_file.c:23 | Call to function 'strcpy' is insecure as it does not provide bounding of the memory buffer |
| Alto | Clang SA | unix.Stream | channel_file.c:41 | Opened stream never closed |
| Alto | Clang SA | unix.Malloc | channel_file.c:41 | Potential leak of memory pointed to by 'cf' |
| Alto | Clang SA | security.insecureAPI.strcpy | content_validator.c:22 | Call to function 'strcpy' is insecure as it does not provide bounding of the memory buffer |
| Alto | Clang SA | security.insecureAPI.strcpy | sensor_events.c:22 | Call to function 'strcpy' is insecure as it does not provide bounding of the memory buffer |
| Alto | Clang SA | security.insecureAPI.strcpy | sensor_events.c:23 | Call to function 'strcpy' is insecure as it does not provide bounding of the memory buffer |

## Mantenibilidad (Lizard)
- Funciones analizadas: 16; complejidad ciclomática máxima: 10 (main); umbral: 10
- Tasa de duplicación: 0.00 %

## Pruebas dinámicas (PyTest + AddressSanitizer)
Casos ejecutados: 30 - pasan 18, fallan 12

| Prueba | Parámetros | Resultado | Mensaje |
|---|---|---|---|
| test_contrato_campos_vs_entradas | previo | FALLA | AssertionError: plantilla declara 21 campos, el sensor entrega 20 |
| test_contrato_campos_vs_entradas | incidente | FALLA | AssertionError: plantilla declara 21 campos, el sensor entrega 20 |
| test_contrato_campos_vs_entradas | corregido | FALLA | AssertionError: plantilla declara 21 campos, el sensor entrega 20 |
| test_criterios_dentro_de_rango | previo | PASA |  |
| test_criterios_dentro_de_rango | incidente | FALLA | AssertionError: criterios sin valor de entrada (instancia, campo): [('IPC-003', 21)] |
| test_criterios_dentro_de_rango | corregido | PASA |  |
| test_interprete_con_eventos_sinteticos | previo-1 | PASA |  |
| test_interprete_con_eventos_sinteticos | previo-42 | PASA |  |
| test_interprete_con_eventos_sinteticos | previo-2024 | PASA |  |
| test_interprete_con_eventos_sinteticos | incidente-1 | FALLA | AssertionError: SUMMARY: AddressSanitizer: stack-buffer-overflow content_interpreter.c:28 in match_instance |
| test_interprete_con_eventos_sinteticos | incidente-42 | FALLA | AssertionError: SUMMARY: AddressSanitizer: stack-buffer-overflow content_interpreter.c:28 in match_instance |
| test_interprete_con_eventos_sinteticos | incidente-2024 | FALLA | AssertionError: SUMMARY: AddressSanitizer: stack-buffer-overflow content_interpreter.c:28 in match_instance |
| test_interprete_con_eventos_sinteticos | corregido-1 | PASA |  |
| test_interprete_con_eventos_sinteticos | corregido-42 | PASA |  |
| test_interprete_con_eventos_sinteticos | corregido-2024 | PASA |  |
| test_valores_frontera | previo-19 | FALLA | AssertionError: 19 entradas -> SUMMARY: AddressSanitizer: heap-buffer-overflow content_interpreter.c:28 in mat |
| test_valores_frontera | previo-20 | PASA |  |
| test_valores_frontera | previo-21 | PASA |  |
| test_valores_frontera | previo-22 | PASA |  |
| test_valores_frontera | incidente-19 | FALLA | AssertionError: 19 entradas -> SUMMARY: AddressSanitizer: heap-buffer-overflow content_interpreter.c:28 in mat |
| test_valores_frontera | incidente-20 | FALLA | AssertionError: 20 entradas -> SUMMARY: AddressSanitizer: heap-buffer-overflow content_interpreter.c:28 in mat |
| test_valores_frontera | incidente-21 | PASA |  |
| test_valores_frontera | incidente-22 | PASA |  |
| test_valores_frontera | corregido-19 | FALLA | AssertionError: 19 entradas -> SUMMARY: AddressSanitizer: heap-buffer-overflow content_interpreter.c:28 in mat |
| test_valores_frontera | corregido-20 | PASA |  |
| test_valores_frontera | corregido-21 | PASA |  |
| test_valores_frontera | corregido-22 | PASA |  |
| test_regresion_del_validador | previo | PASA |  |
| test_regresion_del_validador | incidente | FALLA | AssertionError: el validador aceptó contenido que hace fallar al intérprete: SUMMARY: AddressSanitizer: stack- |
| test_regresion_del_validador | corregido | PASA |  |

## Cobertura de código C (llvm-cov)
- Líneas: 81.82% · Funciones: 81.25% · Regiones: 81.65%
- Nota: los procesos abortados por AddressSanitizer no escriben su perfil, por lo que la cobertura es un límite inferior.
