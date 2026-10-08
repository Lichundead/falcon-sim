import json
import os
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parent.parent
BIN = Path(os.environ.get("FALCON_BIN", ROOT / "build" / "v1.0-incidente" / "falcon_sim"))
DATA = ROOT / "data"


def falcon(*args):
    """Ejecuta el simulador y devuelve (código de salida, salida combinada)."""
    p = subprocess.run([str(BIN), *map(str, args)], capture_output=True, text=True, timeout=30)
    return p.returncode, p.stdout + p.stderr


def crashed(out):
    """Línea SUMMARY de AddressSanitizer si hubo acceso inválido a memoria, si no ''."""
    return next((l for l in out.splitlines() if l.startswith("SUMMARY: AddressSanitizer")), "")


@pytest.fixture(params=["previo", "incidente", "corregido"])
def channel_file(request):
    """Channel File 291 en cada escenario, con su metadato (salida de `info`)."""
    path = DATA / f"cf291_{request.param}.txt"
    rc, out = falcon("info", path)
    assert rc == 0, out
    return path, json.loads(out)
