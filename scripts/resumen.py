"""Consolida los reportes de Cppcheck, Clang SA, Lizard, PyTest y llvm-cov de
cada versión en hallazgos.csv, resumen.md y figuras PNG; con dos versiones
genera además reportes/comparacion.md y las figuras de antes/después.

Uso: python scripts/resumen.py reportes/v1.0-incidente reportes/v1.1-corregida
"""
import csv
import re
import sys
import xml.etree.ElementTree as ET
from collections import Counter, defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

R = Path("reportes")  # versión en proceso; la fija resumir_version()
CCN_MAX = 10  # umbral de McCabe para complejidad ciclomática

SEVERIDADES = ["Crítico", "Alto", "Medio", "Bajo"]
CATEGORIAS = ["Seguridad", "Rendimiento", "Codificación", "Mantenibilidad"]

# Criterio de clasificación (documentado en el informe, sección Metodología).
CLANG_RULES = {
    "security.insecureAPI.strcpy": ("Seguridad", "Alto", ""),
    "security.insecureAPI.DeprecatedOrUnsafeBufferHandling":
        ("Seguridad", "Bajo", "posible falso positivo: printf/sscanf con formato fijo"),
    "security.insecureAPI.rand":
        ("Seguridad", "Bajo", "falso positivo en contexto: rand() solo genera eventos de prueba"),
    "unix.Malloc": ("Rendimiento", "Alto", "coincide con Cppcheck memleak"),
    "unix.Stream": ("Rendimiento", "Alto", "coincide con Cppcheck resourceLeak"),
}


# Categoría MISRA C:2012 de las reglas que aparecen en el análisis.
MISRA_ADVISORY = {"2.7", "5.9", "8.7", "8.9", "11.5", "12.1", "12.3", "15.5"}
MISRA_REQUIRED = {"7.4", "10.4", "14.4", "15.6", "15.7", "17.7", "21.3", "21.6", "21.7", "22.8", "22.9"}


def clasificar_cppcheck(rid, sev):
    if rid.startswith("misra"):
        regla = rid.rsplit("-", 1)[1]
        if regla not in MISRA_ADVISORY | MISRA_REQUIRED:
            print(f"   AVISO: regla MISRA {regla} sin categoría conocida, se asume Required")
        return "Codificación", "Bajo" if regla in MISRA_ADVISORY else "Medio"
    if rid in ("memleak", "resourceLeak", "leakReturnValNotUsed"):
        return "Rendimiento", "Crítico" if sev == "error" else "Alto"
    if sev == "performance":
        return "Rendimiento", "Medio"
    if sev == "portability":
        return "Codificación", "Medio"
    if sev == "error":
        return "Seguridad", "Crítico"
    if sev == "warning":
        return "Seguridad", "Alto"
    return "Mantenibilidad", "Bajo"


def leer_cppcheck():
    out = []
    for e in ET.parse(R / "cppcheck.xml").getroot().iter("error"):
        sev = e.get("severity")
        loc = e.find("location")
        if sev == "information" or loc is None:
            continue
        cat, nivel = clasificar_cppcheck(e.get("id"), sev)
        out.append(dict(herramienta="Cppcheck", regla=e.get("id"), severidad_herramienta=sev,
                        severidad=nivel, categoria=cat, archivo=loc.get("file"),
                        linea=int(loc.get("line")), cwe=e.get("cwe", ""),
                        mensaje=e.get("msg"), nota=""))
    return out


def leer_clang():
    out = []
    pat = re.compile(r"^(src/\S+?):(\d+):\d+: warning: (.*) \[([\w.]+)\]$")
    for line in (R / "clang-sa.txt").read_text().splitlines():
        m = pat.match(line)
        if not m:
            continue
        f, ln, msg, rid = m.groups()
        cat, nivel, nota = CLANG_RULES.get(rid, ("Seguridad", "Alto", ""))
        cwe = re.search(r"CWE-(\d+)", msg)
        out.append(dict(herramienta="Clang SA", regla=rid, severidad_herramienta="warning",
                        severidad=nivel, categoria=cat, archivo=f, linea=int(ln),
                        cwe=cwe.group(1) if cwe else "", mensaje=msg.split(". ")[0], nota=nota))
    return out


def leer_lizard():
    out, funciones = [], []
    with open(R / "lizard.csv") as fh:
        for row in csv.reader(fh):
            nloc, ccn, file, func, start = int(row[0]), int(row[1]), row[6], row[7], int(row[9])
            funciones.append((file, func, ccn, nloc))
            if ccn > CCN_MAX:
                out.append(dict(herramienta="Lizard", regla="cyclomatic_complexity",
                                severidad_herramienta="warning", severidad="Medio",
                                categoria="Mantenibilidad", archivo=file, linea=start, cwe="",
                                mensaje=f"{func}: complejidad ciclomática {ccn} > {CCN_MAX}",
                                nota=""))
    dup = re.search(r"Total duplicate rate: ([\d.]+)%", (R / "lizard.txt").read_text())
    return out, funciones, float(dup.group(1)) if dup else 0.0


def leer_pytest():
    casos = []
    for tc in ET.parse(R / "pytest-junit.xml").getroot().iter("testcase"):
        name = tc.get("name")
        m = re.match(r"(test_\w+?)(?:\[(.*)\])?$", name)
        base, params = m.group(1), (m.group(2) or "")
        escenario = next((s for s in ("previo", "incidente", "corregido") if s in params), "incidente")
        fallo = tc.find("failure")
        msg = fallo.get("message", "").splitlines()[0] if fallo is not None else ""
        casos.append(dict(prueba=base, params=params, escenario=escenario,
                          resultado="FALLA" if fallo is not None else "PASA", mensaje=msg))
    return casos


def leer_cobertura():
    for line in (R / "cobertura.txt").read_text().splitlines():
        if line.startswith("TOTAL"):
            cols = line.split()
            # TOTAL Regions Missed Cover Functions Missed Executed Lines Missed Cover ...
            return dict(regiones=cols[3], funciones=cols[6], lineas=cols[9])
    return {}


# --- Figuras --------------------------------------------------------------
INK, INK2, GRID = "#0b0b0b", "#52514e", "#e4e3df"
ORDINAL = {"Crítico": "#104281", "Alto": "#256abf", "Medio": "#5598e7", "Bajo": "#86b6ef"}
BLUE = "#2a78d6"
GOOD, CRIT = "#0ca30c", "#d03b3b"

plt.rcParams.update({"font.family": "DejaVu Sans", "font.size": 10, "text.color": INK,
                     "axes.labelcolor": INK2, "xtick.color": INK2, "ytick.color": INK,
                     "axes.edgecolor": GRID, "figure.dpi": 200})


def _ejes(ax):
    for s in ("top", "right", "left"):
        ax.spines[s].set_visible(False)
    ax.tick_params(axis="y", length=0)
    ax.xaxis.grid(True, color=GRID, linewidth=0.8)
    ax.set_axisbelow(True)


def barras(cuentas, orden, colores, titulo, archivo, xlabel="Número de hallazgos"):
    vals = [cuentas.get(k, 0) for k in orden][::-1]
    etiquetas = orden[::-1]
    fig, ax = plt.subplots(figsize=(6.5, 0.55 * len(orden) + 1.0))
    bars = ax.barh(etiquetas, vals, height=0.6,
                   color=[colores(k) for k in etiquetas], edgecolor="white", linewidth=2)
    for b, v in zip(bars, vals):
        ax.text(b.get_width() + max(vals) * 0.015, b.get_y() + b.get_height() / 2, str(v),
                va="center", color=INK, fontsize=10)
    _ejes(ax)
    ax.set_xlabel(xlabel)
    ax.set_xlim(0, max(vals) * 1.12 + 0.5)
    ax.set_title(titulo, loc="left", fontsize=11, color=INK)
    fig.tight_layout()
    fig.savefig(R / archivo, facecolor="white")
    plt.close(fig)


def apilado_modulos(hallazgos, archivo):
    mods = sorted({h["archivo"] for h in hallazgos},
                  key=lambda m: -sum(h["archivo"] == m for h in hallazgos))
    fig, ax = plt.subplots(figsize=(6.5, 0.55 * len(mods) + 1.4))
    left = [0] * len(mods)
    etiquetas = [Path(m).name for m in mods][::-1]
    mods_r = mods[::-1]
    for sev in SEVERIDADES:
        vals = [sum(h["archivo"] == m and h["severidad"] == sev for h in hallazgos) for m in mods_r]
        ax.barh(etiquetas, vals, left=left, height=0.6, color=ORDINAL[sev],
                edgecolor="white", linewidth=2, label=sev)
        left = [a + b for a, b in zip(left, vals)]
    for y, tot in enumerate(left):
        ax.text(tot + max(left) * 0.015, y, str(tot), va="center", color=INK)
    _ejes(ax)
    ax.set_xlabel("Número de hallazgos")
    ax.set_xlim(0, max(left) * 1.12)
    ax.legend(ncol=4, frameon=False, loc="upper left", bbox_to_anchor=(0, -0.18 if len(mods) > 3 else -0.3),
              fontsize=9, labelcolor=INK2)
    ax.set_title("Hallazgos por módulo y severidad", loc="left", fontsize=11)
    fig.tight_layout()
    fig.savefig(R / archivo, facecolor="white", bbox_inches="tight")
    plt.close(fig)


ESCENARIOS = [("previo", "B · previo"), ("incidente", "A · incidente"),
              ("corregido", "C · corregido")]


def matriz_pytest(columnas, archivo, destino=None, grupos=()):
    """columnas: [(título, casos, escenario)]; grupos: [(título, primera, última)]."""
    pruebas = list(dict.fromkeys(c["prueba"] for _, casos, _ in columnas for c in casos))
    titulos = [t for t, _, _ in columnas]
    fig, ax = plt.subplots(figsize=(2.3 + 1.65 * len(columnas), 0.6 * len(pruebas) + 1.3))
    for i, p in enumerate(pruebas):
        for j, (_, casos, e) in enumerate(columnas):
            cs = [c for c in casos if c["prueba"] == p and c["escenario"] == e]
            y = len(pruebas) - 1 - i
            if not cs:
                ax.text(j, y, "—", ha="center", va="center", color=INK2)
                continue
            ok = sum(c["resultado"] == "PASA" for c in cs)
            color = GOOD if ok == len(cs) else CRIT
            ax.add_patch(plt.Rectangle((j - 0.46, y - 0.4), 0.92, 0.8, color=color, linewidth=0))
            txt = ("✓ " if ok == len(cs) else "✗ ") + f"{ok}/{len(cs)} pasan"
            ax.text(j, y, txt, ha="center", va="center", color="white", fontweight="bold", fontsize=9)
    ax.set_xlim(-0.5, len(columnas) - 0.5)
    ax.set_ylim(-0.5, len(pruebas) - 0.5)
    ax.set_xticks(range(len(columnas)), titulos, fontsize=9)
    for titulo, a, b in grupos:
        ax.annotate(titulo, xy=((a + b) / 2, len(pruebas) - 0.5), xycoords="data",
                    xytext=(0, 30), textcoords="offset points", ha="center",
                    fontsize=10, fontweight="bold", color=INK)
    ax.set_yticks(range(len(pruebas)), pruebas[::-1], fontsize=9, family="DejaVu Sans Mono")
    ax.xaxis.tick_top()
    for s in ax.spines.values():
        s.set_visible(False)
    ax.tick_params(length=0)
    fig.tight_layout()
    fig.savefig((destino or R) / archivo, facecolor="white", bbox_inches="tight")
    plt.close(fig)


def barras_comparadas(series, orden, titulo, archivo, destino):
    """series: [(nombre, Counter, color)] - barras horizontales agrupadas."""
    import numpy as np
    y = np.arange(len(orden))[::-1]
    h = 0.8 / len(series)
    maximo = max(c.get(k, 0) for _, c, _ in series for k in orden)
    fig, ax = plt.subplots(figsize=(6.5, 0.75 * len(orden) + 1.3))
    for n, (nombre, cuentas, color) in enumerate(series):
        vals = [cuentas.get(k, 0) for k in orden]
        pos = y + 0.4 - h * (n + 0.5)
        ax.barh(pos, vals, height=h, color=color, edgecolor="white", linewidth=2, label=nombre)
        for p, v in zip(pos, vals):
            ax.text(v + maximo * 0.015, p, str(v), va="center", color=INK, fontsize=9)
    ax.set_yticks(y, orden)
    _ejes(ax)
    ax.set_xlabel("Número de hallazgos")
    ax.set_xlim(0, maximo * 1.12 + 0.5)
    ax.legend(frameon=False, loc="lower right", fontsize=9, labelcolor=INK2)
    ax.set_title(titulo, loc="left", fontsize=11)
    fig.tight_layout()
    fig.savefig(destino / archivo, facecolor="white")
    plt.close(fig)


# --- Tablas Markdown -------------------------------------------------------
def tabla(headers, rows):
    s = "| " + " | ".join(headers) + " |\n|" + "---|" * len(headers) + "\n"
    return s + "".join("| " + " | ".join(map(str, r)) + " |\n" for r in rows)


def resumir_version(r):
    global R
    R = Path(r)
    hallazgos = leer_cppcheck() + leer_clang()
    complejidad, funciones, dup = leer_lizard()
    hallazgos += complejidad
    casos = leer_pytest()
    cov = leer_cobertura()

    with open(R / "hallazgos.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(hallazgos[0]))
        w.writeheader()
        w.writerows(sorted(hallazgos, key=lambda h: (SEVERIDADES.index(h["severidad"]),
                                                      h["archivo"], h["linea"])))

    por_sev = Counter(h["severidad"] for h in hallazgos)
    por_cat = Counter(h["categoria"] for h in hallazgos)
    por_herr = Counter(h["herramienta"] for h in hallazgos)
    cruce = defaultdict(Counter)
    for h in hallazgos:
        cruce[h["categoria"]][h["severidad"]] += 1
    reglas = Counter((h["herramienta"], h["regla"]) for h in hallazgos)
    archivos_por_regla = defaultdict(set)
    for h in hallazgos:
        archivos_por_regla[(h["herramienta"], h["regla"])].add(Path(h["archivo"]).name)

    barras(por_sev, SEVERIDADES, lambda k: ORDINAL[k], "Hallazgos por severidad", "fig_severidad.png")
    barras(por_cat, CATEGORIAS, lambda k: BLUE, "Hallazgos por categoría", "fig_categoria.png")
    apilado_modulos(hallazgos, "fig_modulos.png")
    matriz_pytest([(t, casos, e) for e, t in ESCENARIOS], "fig_pytest.png")

    n_ok = sum(c["resultado"] == "PASA" for c in casos)
    md = [f"# Resumen del análisis - falcon-sim {R.name}\n",
          f"Total de hallazgos estáticos: **{len(hallazgos)}** "
          f"({', '.join(f'{k}: {v}' for k, v in por_herr.items())})\n",
          "## Severidad × categoría\n",
          tabla(["Categoría"] + SEVERIDADES + ["Total"],
                [[c] + [cruce[c][s] for s in SEVERIDADES] + [por_cat[c]] for c in CATEGORIAS]
                + [["**Total**"] + [por_sev[s] for s in SEVERIDADES] + [len(hallazgos)]]),
          "\n## Frecuencia por regla\n",
          tabla(["Herramienta", "Regla", "Ocurrencias", "Archivos"],
                [[t, r, n, ", ".join(sorted(archivos_por_regla[(t, r)]))]
                 for (t, r), n in reglas.most_common()]),
          "\n## Hallazgos críticos y altos\n",
          tabla(["Severidad", "Herramienta", "Regla", "Ubicación", "Mensaje"],
                [[h["severidad"], h["herramienta"], h["regla"],
                  f"{Path(h['archivo']).name}:{h['linea']}", h["mensaje"]]
                 for h in sorted(hallazgos, key=lambda h: SEVERIDADES.index(h["severidad"]))
                 if h["severidad"] in ("Crítico", "Alto")]),
          "\n## Mantenibilidad (Lizard)\n",
          f"- Funciones analizadas: {len(funciones)}; complejidad ciclomática máxima: "
          f"{max(f[2] for f in funciones)} ({max(funciones, key=lambda f: f[2])[1]}); "
          f"umbral: {CCN_MAX}\n- Tasa de duplicación: {dup:.2f} %\n",
          "\n## Pruebas dinámicas (PyTest + AddressSanitizer)\n",
          f"Casos ejecutados: {len(casos)} - pasan {n_ok}, fallan {len(casos) - n_ok}\n\n",
          tabla(["Prueba", "Parámetros", "Resultado", "Mensaje"],
                [[c["prueba"], c["params"], c["resultado"], c["mensaje"].replace("|", "/")[:110]]
                 for c in casos]),
          "\n## Cobertura de código C (llvm-cov)\n",
          f"- Líneas: {cov.get('lineas')} · Funciones: {cov.get('funciones')} · "
          f"Regiones: {cov.get('regiones')}\n"
          "- Nota: los procesos abortados por AddressSanitizer no escriben su perfil, "
          "por lo que la cobertura es un límite inferior.\n"]
    (R / "resumen.md").write_text("".join(md))
    print(f"   {len(hallazgos)} hallazgos estáticos, {len(casos)} casos de prueba "
          f"({len(casos) - n_ok} fallan), cobertura de líneas {cov.get('lineas')}")
    print(f"   -> {R}/resumen.md, {R}/hallazgos.csv, {R}/fig_*.png")
    return dict(nombre=R.name, hallazgos=hallazgos, casos=casos, cov=cov,
                funciones=funciones, dup=dup)


def comparar(a, b, destino):
    """Antes/después entre dos versiones: comparacion.md y figuras."""
    def cuenta(v, campo):
        return Counter(h[campo] for h in v["hallazgos"])

    def pasan(v):
        return sum(c["resultado"] == "PASA" for c in v["casos"])

    def delta(x, y):
        return f"{(y - x) / x * 100:+.0f} %" if x else "—"

    filas = []
    for campo, orden in (("severidad", SEVERIDADES), ("categoria", CATEGORIAS)):
        ca, cb = cuenta(a, campo), cuenta(b, campo)
        filas += [[k, ca[k], cb[k], delta(ca[k], cb[k])] for k in orden]
    na, nb = len(a["hallazgos"]), len(b["hallazgos"])
    filas.append(["**Total hallazgos estáticos**", na, nb, delta(na, nb)])

    herr = sorted({h["herramienta"] for v in (a, b) for h in v["hallazgos"]})
    ha, hb = cuenta(a, "herramienta"), cuenta(b, "herramienta")

    # reglas resueltas / persistentes / nuevas
    ra = Counter(h["regla"] for h in a["hallazgos"])
    rb = Counter(h["regla"] for h in b["hallazgos"])
    resueltas = sorted(set(ra) - set(rb))
    persisten = sorted(set(ra) & set(rb))
    nuevas = sorted(set(rb) - set(ra))

    columnas = ([(t, a["casos"], e) for e, t in ESCENARIOS] +
                [(t.replace("C · corregido", "D · corregido"), b["casos"], e) for e, t in ESCENARIOS])
    matriz_pytest(columnas, "fig_pytest_comparacion.png", destino,
                  grupos=[(a["nombre"], 0, 2), (b["nombre"], 3, 5)])
    barras_comparadas([(a["nombre"], cuenta(a, "severidad"), "#2a78d6"),
                       (b["nombre"], cuenta(b, "severidad"), "#eb6834")],
                      SEVERIDADES, "Hallazgos por severidad: antes y después",
                      "fig_comparacion_severidad.png", destino)
    barras_comparadas([(a["nombre"], cuenta(a, "categoria"), "#2a78d6"),
                       (b["nombre"], cuenta(b, "categoria"), "#eb6834")],
                      CATEGORIAS, "Hallazgos por categoría: antes y después",
                      "fig_comparacion_categoria.png", destino)

    md = [f"# Comparación {a['nombre']} → {b['nombre']}\n\n",
          "## Hallazgos estáticos\n",
          tabla(["Dimensión", a["nombre"], b["nombre"], "Variación"], filas),
          "\n## Por herramienta\n",
          tabla(["Herramienta", a["nombre"], b["nombre"], "Variación"],
                [[t, ha[t], hb[t], delta(ha[t], hb[t])] for t in herr]),
          "\n## Reglas\n",
          f"- Resueltas ({len(resueltas)}): {', '.join(resueltas) or '—'}\n",
          f"- Persisten ({len(persisten)}): "
          f"{', '.join(f'{r} ({ra[r]}→{rb[r]})' for r in persisten) or '—'}\n",
          f"- Nuevas ({len(nuevas)}): {', '.join(f'{r} ({rb[r]})' for r in nuevas) or '—'}\n",
          "\n## Pruebas y cobertura\n",
          tabla(["Indicador", a["nombre"], b["nombre"]],
                [["Casos PyTest que pasan", f"{pasan(a)}/{len(a['casos'])}",
                  f"{pasan(b)}/{len(b['casos'])}"],
                 ["Accesos inválidos a memoria (ASan)",
                  sum("AddressSanitizer" in c["mensaje"] for c in a["casos"]),
                  sum("AddressSanitizer" in c["mensaje"] for c in b["casos"])],
                 ["Cobertura de líneas", a["cov"].get("lineas"), b["cov"].get("lineas")],
                 ["Cobertura de funciones", a["cov"].get("funciones"), b["cov"].get("funciones")],
                 ["Complejidad ciclomática máxima", max(f[2] for f in a["funciones"]),
                  max(f[2] for f in b["funciones"])],
                 ["Funciones", len(a["funciones"]), len(b["funciones"])]])]
    (destino / "comparacion.md").write_text("".join(md))
    print(f"   -> {destino}/comparacion.md, {destino}/fig_comparacion_*.png, "
          f"{destino}/fig_pytest_comparacion.png")


if __name__ == "__main__":
    versiones = [resumir_version(r) for r in sys.argv[1:]]
    if len(versiones) == 2:
        comparar(*versiones, Path(sys.argv[1]).parent)
