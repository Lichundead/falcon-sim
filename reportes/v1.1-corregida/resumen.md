# Resumen del análisis - falcon-sim v1.1-corregida
Total de hallazgos estáticos: **47** (Cppcheck: 38, Clang SA: 9)
## Severidad × categoría
| Categoría | Crítico | Alto | Medio | Bajo | Total |
|---|---|---|---|---|---|
| Seguridad | 0 | 1 | 0 | 9 | 10 |
| Rendimiento | 0 | 0 | 0 | 0 | 0 |
| Codificación | 0 | 0 | 32 | 3 | 35 |
| Mantenibilidad | 0 | 0 | 0 | 2 | 2 |
| **Total** | 0 | 1 | 32 | 14 | 47 |

## Frecuencia por regla
| Herramienta | Regla | Ocurrencias | Archivos |
|---|---|---|---|
| Cppcheck | misra-c2012-21.6 | 24 | channel_file.c, content_validator.c, main.c, sensor_events.c |
| Cppcheck | misra-c2012-21.3 | 5 | channel_file.c, main.c |
| Clang SA | security.insecureAPI.rand | 5 | sensor_events.c |
| Clang SA | security.insecureAPI.DeprecatedOrUnsafeBufferHandling | 4 | channel_file.c, main.c |
| Cppcheck | variableScope | 2 | main.c, sensor_events.c |
| Cppcheck | misra-c2012-8.9 | 2 | sensor_events.c |
| Cppcheck | misra-c2012-11.5 | 1 | channel_file.c |
| Cppcheck | arrayIndexOutOfBoundsCond | 1 | content_validator.c |
| Cppcheck | misra-c2012-7.4 | 1 | main.c |
| Cppcheck | misra-c2012-22.8 | 1 | main.c |
| Cppcheck | misra-c2012-22.9 | 1 | main.c |

## Hallazgos críticos y altos
| Severidad | Herramienta | Regla | Ubicación | Mensaje |
|---|---|---|---|---|
| Alto | Cppcheck | arrayIndexOutOfBoundsCond | content_validator.c:26 | Either the condition 'j>=21' is redundant or the array 'inst->criteria[21][64]' is accessed at index 21, which is out of bounds. |

## Mantenibilidad (Lizard)
- Funciones analizadas: 19; complejidad ciclomática máxima: 8 (load_body); umbral: 10
- Tasa de duplicación: 0.00 %

## Pruebas dinámicas (PyTest + AddressSanitizer)
Casos ejecutados: 30 - pasan 30, fallan 0

| Prueba | Parámetros | Resultado | Mensaje |
|---|---|---|---|
| test_contrato_campos_vs_entradas | previo | PASA |  |
| test_contrato_campos_vs_entradas | incidente | PASA |  |
| test_contrato_campos_vs_entradas | corregido | PASA |  |
| test_criterios_dentro_de_rango | previo | PASA |  |
| test_criterios_dentro_de_rango | incidente | PASA |  |
| test_criterios_dentro_de_rango | corregido | PASA |  |
| test_interprete_con_eventos_sinteticos | previo-1 | PASA |  |
| test_interprete_con_eventos_sinteticos | previo-42 | PASA |  |
| test_interprete_con_eventos_sinteticos | previo-2024 | PASA |  |
| test_interprete_con_eventos_sinteticos | incidente-1 | PASA |  |
| test_interprete_con_eventos_sinteticos | incidente-42 | PASA |  |
| test_interprete_con_eventos_sinteticos | incidente-2024 | PASA |  |
| test_interprete_con_eventos_sinteticos | corregido-1 | PASA |  |
| test_interprete_con_eventos_sinteticos | corregido-42 | PASA |  |
| test_interprete_con_eventos_sinteticos | corregido-2024 | PASA |  |
| test_valores_frontera | previo-19 | PASA |  |
| test_valores_frontera | previo-20 | PASA |  |
| test_valores_frontera | previo-21 | PASA |  |
| test_valores_frontera | previo-22 | PASA |  |
| test_valores_frontera | incidente-19 | PASA |  |
| test_valores_frontera | incidente-20 | PASA |  |
| test_valores_frontera | incidente-21 | PASA |  |
| test_valores_frontera | incidente-22 | PASA |  |
| test_valores_frontera | corregido-19 | PASA |  |
| test_valores_frontera | corregido-20 | PASA |  |
| test_valores_frontera | corregido-21 | PASA |  |
| test_valores_frontera | corregido-22 | PASA |  |
| test_regresion_del_validador | previo | PASA |  |
| test_regresion_del_validador | incidente | PASA |  |
| test_regresion_del_validador | corregido | PASA |  |

## Cobertura de código C (llvm-cov)
- Líneas: 89.22% · Funciones: 100.00% · Regiones: 89.95%
- Nota: los procesos abortados por AddressSanitizer no escriben su perfil, por lo que la cobertura es un límite inferior.
