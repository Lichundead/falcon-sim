"""Suite de la Tabla 2 (Actividad 4) ejecutada sobre el simulador del sensor."""
import pytest

from conftest import crashed, falcon


def test_contrato_campos_vs_entradas(channel_file):
    """Prueba de contrato: la plantilla y el sensor manejan el mismo número de campos."""
    _, meta = channel_file
    assert meta["declared_fields"] == meta["sensor_inputs"], (
        f"plantilla declara {meta['declared_fields']} campos, "
        f"el sensor entrega {meta['sensor_inputs']}"
    )


def test_criterios_dentro_de_rango(channel_file):
    """Conformidad estructural: ningún criterio distinto del comodín apunta a un campo sin entrada."""
    _, meta = channel_file
    fuera = [(i["id"], c) for i in meta["instances"]
             for c in i["non_wildcard"] if c > meta["sensor_inputs"]]
    assert not fuera, f"criterios sin valor de entrada (instancia, campo): {fuera}"


@pytest.mark.parametrize("seed", [1, 42, 2024])
def test_interprete_con_eventos_sinteticos(channel_file, seed):
    """Datos sintéticos: el intérprete procesa eventos generados sin leer fuera de rango."""
    path, _ = channel_file
    rc, out = falcon("run", path, "--events", 200, "--seed", seed)
    asan = crashed(out)
    assert not asan, asan
    assert rc == 0


@pytest.mark.parametrize("n_inputs", [19, 20, 21, 22])
def test_valores_frontera(channel_file, n_inputs):
    """Valores frontera: con 19-22 entradas el intérprete procesa o rechaza de forma controlada."""
    path, _ = channel_file
    rc, out = falcon("run", path, "--events", 20, "--inputs", n_inputs)
    asan = crashed(out)
    assert not asan, f"{n_inputs} entradas -> {asan}"
    assert rc in (0, 1)


def test_regresion_del_validador(channel_file):
    """Regresión: el validador no acepta contenido que haga fallar al intérprete
    (caso conocido: IPC-003 del 19/07/2024)."""
    path, _ = channel_file
    rc, _ = falcon("validate", path)
    if rc == 0:
        _, run_out = falcon("run", path, "--events", 200)
        asan = crashed(run_out)
        assert not asan, f"el validador aceptó contenido que hace fallar al intérprete: {asan}"
