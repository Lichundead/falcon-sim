# Guía para generar los archivos de soporte (capturas)

## 1. Preparar (una sola vez)

Abre la Terminal en la carpeta `falcon-sim` y ejecuta:

```bash
brew install cppcheck
xcode-select --install          # solo si clang no está instalado
rm -rf .venv && python3 -m venv .venv && .venv/bin/pip install -r requirements.txt
chmod +x analizar.sh capturas.sh
```

El `.venv` se recrea porque guarda rutas absolutas y deja de funcionar si la carpeta se mueve.

## 2. Ejecutar el análisis completo

```bash
./analizar.sh
```

Compila las dos versiones, ejecuta Cppcheck, Clang Static Analyzer, Lizard, PyTest y llvm-cov,
y deja todo en `reportes/`. Tarda menos de un minuto. Que la v1.0 tenga 12 pruebas fallidas
es lo esperado, porque esas pruebas documentan los defectos.

## 3. Tomar las capturas de consola

Pon la Terminal en pantalla completa con letra de 12-13 pt (algunas tablas son anchas) y ejecuta:

```bash
./capturas.sh
```

Cada pantalla muestra el comando ejecutado y su resultado. Toma la captura con **⌘ ⇧ 4** y
después la barra espaciadora para capturar solo la ventana. Luego presiona **Enter** para pasar a la siguiente.

| # | Captura | Sección del informe |
|---|---|---|
| 01 | Herramientas y versiones | 1. Introducción: herramientas utilizadas |
| 02 | v1.0: validador ACEPTA y el intérprete falla (`stack-buffer-overflow`) | 3. Resultados: evidencia del defecto crítico |
| 03 | v1.0: Cppcheck sin MISRA | 3. Resultados: severidad y ubicación |
| 04 | v1.0: Cppcheck + MISRA, ocurrencias por regla | 3. Resultados: frecuencia y normas de codificación |
| 05 | v1.0: Clang Static Analyzer | 3. Resultados: seguridad |
| 06 | v1.0: Lizard | 3. Resultados: mantenibilidad y duplicidad |
| 07 | v1.0: PyTest (18/30) | 3. Resultados: cobertura de pruebas |
| 08 | v1.0: cobertura llvm-cov | 3. Resultados: cobertura de pruebas |
| 09-15 | Las mismas capturas para la v1.1 corregida | 5. Recomendaciones: verificación de las acciones correctivas |
| 16 | Comparación v1.0 → v1.1 | 4. Análisis de impacto / 6. Conclusiones |

Si una pantalla no cabe completa, desplázate y toma dos capturas, o reduce la letra con **⌘ −**.

## 4. Capturas de los reportes HTML (opcional, se ven mejor en el informe)

```bash
open reportes/v1.0-incidente/pytest.html
open reportes/v1.0-incidente/cppcheck-html/index.html
open reportes/v1.0-incidente/cobertura-html/index.html
```

Haz lo mismo con `v1.1-corregida`. En el reporte de cobertura, entra a `content_interpreter.c`:
la línea 28 aparece ejecutada, que es justo donde se produce la lectura fuera de límites.

## 5. Gráficos ya generados (no requieren captura)

- `reportes/v1.0-incidente/fig_severidad.png`, `fig_categoria.png`, `fig_modulos.png`, `fig_pytest.png`
- `reportes/fig_comparacion_severidad.png`, `fig_comparacion_categoria.png`, `fig_pytest_comparacion.png`

En APA 7, cada captura o gráfico va como **Figura N**, con un título en cursiva arriba y una nota
de fuente debajo, por ejemplo: *Nota.* Salida de Cppcheck 2.22.0 ejecutado sobre falcon-sim v1.0. Elaboración propia.
