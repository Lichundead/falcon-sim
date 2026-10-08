# Comparación v1.0-incidente → v1.1-corregida

## Hallazgos estáticos
| Dimensión | v1.0-incidente | v1.1-corregida | Variación |
|---|---|---|---|
| Crítico | 4 | 0 | -100 % |
| Alto | 11 | 1 | -91 % |
| Medio | 84 | 32 | -62 % |
| Bajo | 42 | 14 | -67 % |
| Seguridad | 18 | 10 | -44 % |
| Rendimiento | 6 | 0 | -100 % |
| Codificación | 114 | 35 | -69 % |
| Mantenibilidad | 3 | 2 | -33 % |
| **Total hallazgos estáticos** | 141 | 47 | -67 % |

## Por herramienta
| Herramienta | v1.0-incidente | v1.1-corregida | Variación |
|---|---|---|---|
| Clang SA | 16 | 9 | -44 % |
| Cppcheck | 125 | 38 | -70 % |

## Reglas
- Resueltas (22): constVariablePointer, ctunullpointerOutOfMemory, invalidscanf, memleak, misra-c2012-10.4, misra-c2012-12.1, misra-c2012-12.3, misra-c2012-14.4, misra-c2012-15.5, misra-c2012-15.6, misra-c2012-15.7, misra-c2012-17.7, misra-c2012-2.7, misra-c2012-21.7, misra-c2012-5.9, misra-c2012-8.7, nullPointerOutOfMemory, resourceLeak, security.insecureAPI.strcpy, unix.Malloc, unix.Stream, unusedFunction
- Persisten (7): misra-c2012-11.5 (1→1), misra-c2012-21.3 (4→5), misra-c2012-21.6 (21→24), misra-c2012-7.4 (1→1), misra-c2012-8.9 (2→2), security.insecureAPI.DeprecatedOrUnsafeBufferHandling (4→4), security.insecureAPI.rand (5→5)
- Nuevas (4): arrayIndexOutOfBoundsCond (1), misra-c2012-22.8 (1), misra-c2012-22.9 (1), variableScope (2)

## Pruebas y cobertura
| Indicador | v1.0-incidente | v1.1-corregida |
|---|---|---|
| Casos PyTest que pasan | 18/30 | 30/30 |
| Accesos inválidos a memoria (ASan) | 8 | 0 |
| Cobertura de líneas | 81.82% | 89.22% |
| Cobertura de funciones | 81.25% | 100.00% |
| Complejidad ciclomática máxima | 10 | 8 |
| Funciones | 16 | 19 |
