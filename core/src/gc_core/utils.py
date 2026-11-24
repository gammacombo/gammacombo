from pathlib import Path
from sys import platform

import ROOT


def load_gc_core_lib():
    """Import the Core library to ROOT (needed to access the custom dictionaries)."""
    if platform == "darwin":
        ext = "dylib"
    elif platform == "linux":
        ext = "so"
    else:
        raise RuntimeError(f'Platform "{platform}" not supported')
    repo_path = Path(__file__).resolve().parents[3]
    libpath = (repo_path / "build" / f"libGammaComboCore.{ext}").resolve()
    if ROOT.gSystem.Load(str(libpath)) < 0:
        raise RuntimeError(
            f'Could not load "{libpath}" (was GammaCombo built in "build"?)'
        )
