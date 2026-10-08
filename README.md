# falcon-sim

Simulador en C del intérprete de contenido del sensor CrowdStrike Falcon. Lo usamos para el
análisis estático y las pruebas de la Actividad 6 de Calidad de Software (Ingeniería de Software,
Corporación Universitaria Iberoamericana).

Autores: Jhiann Macias Vargas (100154852) y Sebastian Vallejo Ricaurte (100161067).
Docente: William Ruiz.

El modelo reproduce el incidente del Channel File 291 del 19/07/2024 a partir del análisis de
causa raíz que publicó CrowdStrike (2024). No es el código real del sensor. Escribimos los defectos
a propósito para ver cómo responden las herramientas de calidad ante ese tipo de falla.

## Reproducir el análisis

Requisitos: macOS con las Command Line Tools de Xcode y Homebrew, o Linux/WSL con clang y llvm;
Python 3.10 o superior.

```bash
git clone https://github.com/Lichundead/falcon-sim.git
cd falcon-sim
brew install cppcheck
python3 -m venv .venv && .venv/bin/pip install -r requirements.txt
./analizar.sh
```

En Linux o WSL, en lugar de `brew install cppcheck`:

```bash
sudo apt install clang llvm cppcheck python3-venv
```

`analizar.sh` compila las dos versiones, ejecuta todas las herramientas y deja los resultados en
`reportes/`. Tarda menos de un minuto. Con Cppcheck 2.22.0 y Apple clang 21 el resultado es este:

| Versión        | PyTest              | Hallazgos estáticos | Cobertura de líneas |
| -------------- | ------------------- | ------------------- | ------------------- |
| v1.0-incidente | 12 fallan, 18 pasan | 141                 | 81.82 %             |
| v1.1-corregida | 30 pasan            | 47                  | 89.22 %             |

Las 12 pruebas que fallan en la v1.0 son las que detectan los defectos del incidente. Con otra
versión de Cppcheck el número de hallazgos puede cambiar un poco; los resultados de PyTest no
dependen de esa versión.

Para ver la salida de cada herramienta en consola, una pantalla a la vez:

```bash
./capturas.sh
```

[GUIA_CAPTURAS.md](GUIA_CAPTURAS.md) explica qué muestra cada pantalla.

## Estructura

```
falcon-sim/
├── src/
│   ├── v1.0-incidente/     código con los defectos del incidente
│   └── v1.1-corregida/     código con las correcciones (escenario D)
├── data/                   tres versiones del Channel File 291
├── tests/                  suite PyTest (Tabla 2 de la Actividad 4)
├── scripts/resumen.py      consolida hallazgos, tablas y gráficos
├── analizar.sh             ejecuta todo el análisis
├── capturas.sh             muestra la salida de cada herramienta en consola
├── reportes/               resultados del análisis (incluidos en el repositorio)
└── capturas/               capturas de consola usadas en el informe
```

## Versiones del código

La v1.0 reproduce los defectos que describe el análisis de causa raíz:

| Defecto                                                                       | Dónde está                                             |
| ----------------------------------------------------------------------------- | ------------------------------------------------------ |
| La plantilla IPC declara 21 campos y el sensor entrega 20 valores             | `channel_file.h` (`TEMPLATE_FIELDS` y `SENSOR_INPUTS`) |
| El intérprete lee el campo 21 sin comprobar límites                           | `content_interpreter.c:28`                             |
| El validador compara contra la plantilla y no contra lo que entrega el sensor | `content_validator.c`                                  |
| La instancia IPC-003 usa un criterio distinto del comodín en el campo 21      | `data/cf291_incidente.txt`                             |

La v1.1 aplica la remediación del análisis de causa raíz y corrige los hallazgos críticos y altos:

| Corrección                                                                              | Archivo                     |
| --------------------------------------------------------------------------------------- | --------------------------- |
| El sensor entrega 21 valores y el compilador verifica el contrato con `_Static_assert`  | `channel_file.h`            |
| El intérprete comprueba límites; un campo sin valor cuenta como "no coincide"           | `content_interpreter.c`     |
| El validador rechaza criterios que apuntan a campos que el sensor no entrega            | `content_validator.c`       |
| Sin fugas en las rutas de error, `malloc` verificado, `sscanf` con ancho máximo         | `channel_file.c`            |
| `strcpy` y `sprintf` reemplazados por copias con límite y `snprintf`; sin código muerto | `sensor_events.c`, `main.c` |
| Argumentos leídos con `strtol` y control de rango                                       | `main.c`                    |

## Escenarios de datos

| Archivo               | Escenario              | Contenido                                               |
| --------------------- | ---------------------- | ------------------------------------------------------- |
| `cf291_incidente.txt` | A                      | Contenido del 19/07/2024: IPC-003 usa el campo 21       |
| `cf291_previo.txt`    | B                      | Instancias anteriores, todas con comodín en el campo 21 |
| `cf291_corregido.txt` | C con v1.0, D con v1.1 | IPC-003 con comodín en el campo 21                      |

El binario se compila con AddressSanitizer. La lectura fuera de límites que en Windows produjo la
pantalla azul aquí detiene el proceso con un `stack-buffer-overflow`.

## Herramientas y clasificación

| Herramienta                  | Versión usada      | Para qué                                 |
| ---------------------------- | ------------------ | ---------------------------------------- |
| Cppcheck + complemento MISRA | 2.22.0             | Análisis estático y norma MISRA C:2012   |
| Clang Static Analyzer        | Apple clang 21.0.0 | Segunda revisión de seguridad y fugas    |
| Lizard                       | 1.24.1             | Complejidad ciclomática y duplicación    |
| PyTest                       | 9.1.1              | Pruebas automatizadas                    |
| AddressSanitizer             | clang 21.0.0       | Detección de accesos inválidos a memoria |
| llvm-cov                     | clang 21.0.0       | Cobertura del código C                   |

Cada herramienta usa su propia escala de severidad. `scripts/resumen.py` las lleva a una escala común:

| Severidad | Incluye                                                                       |
| --------- | ----------------------------------------------------------------------------- |
| Crítico   | Errores de Cppcheck (fugas de memoria o recursos, accesos inválidos)          |
| Alto      | Advertencias de Cppcheck; `strcpy` (CWE-119) y fugas según Clang              |
| Medio     | Reglas MISRA obligatorias; rendimiento y portabilidad; complejidad mayor a 10 |
| Bajo      | Reglas MISRA recomendadas; estilo; avisos de Clang sobre `rand` y `fprintf`   |

Las categorías son seguridad (memoria, punteros y funciones inseguras), rendimiento (fugas),
codificación (MISRA y portabilidad) y mantenibilidad (estilo, código muerto y complejidad).

## Evidencias

`reportes/` contiene los resultados de la ejecución del 07/10/2026, los mismos que aparecen en el
informe. `analizar.sh` los vuelve a generar.

| Archivo en `reportes/<versión>/`             | Contenido                                     |
| -------------------------------------------- | --------------------------------------------- |
| `cppcheck.xml`, `cppcheck-html/index.html`   | Reporte de Cppcheck                           |
| `clang-sa.txt`                               | Reporte de Clang Static Analyzer              |
| `lizard.txt`                                 | Complejidad y duplicación                     |
| `pytest.html`, `pytest-junit.xml`            | Resultado de las pruebas                      |
| `cobertura.txt`, `cobertura-html/index.html` | Cobertura por archivo y por línea             |
| `hallazgos.csv`                              | Todos los hallazgos con severidad y categoría |
| `resumen.md`, `fig_*.png`                    | Tablas y gráficos de la versión               |

La comparación entre versiones está en `reportes/comparacion.md` y en `reportes/fig_comparacion_*.png`.

En `capturas/` están las 16 capturas de consola del informe: la 01 muestra las versiones de las
herramientas, de la 02 a la 08 la salida sobre la v1.0, de la 09 a la 15 la salida sobre la v1.1 y
la 16 la comparación entre las dos.

## Uso del simulador

```bash
build/v1.0-incidente/falcon_sim info     data/cf291_incidente.txt   # metadatos en JSON
build/v1.0-incidente/falcon_sim validate data/cf291_incidente.txt   # validador de contenido
build/v1.0-incidente/falcon_sim run      data/cf291_incidente.txt --events 50 [--seed 42] [--inputs 21]
```

Códigos de salida: 0 aceptado o ejecución correcta, 1 rechazado, 2 error de uso o de carga.

## Referencia

CrowdStrike. (2024). _External technical root cause analysis: Channel File 291_.
https://www.crowdstrike.com/en-us/blog/channel-file-291-rca-available/
