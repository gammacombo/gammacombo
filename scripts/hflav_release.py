#!/usr/bin/env python3
"""
The current HFLAV release of the CKM-angle averages: the one place to edit for a new iteration.

hflav_averages.py and hflav_json.py take from here
  - the tag in the combiner names of main/gamma_wa.cpp, main/alpha_wa.cpp and main/rhoeta_wa.cpp:
        hflav_<TAG>             the full combination
        hflav_<TAG>_<sub>       its sub-combinations (2body, td, bu2dk, pipi, ...; for rho-eta the
                                single angles gamma, beta, alpha, beta_alt)
    Combiner ids are looked up by name in the .cpp, so they are not repeated in the scripts;
  - the label of the HFLAV logo on the plots ("HFLAV <VERSION> <YEAR>"); the plots are written to
    <module>/paper_plots/HFLAV/<VERSION><YEAR>/.

For the next iteration:
  1. add the combiners hflav_<newtag>[_<sub>] to the .cpp, with the same <sub> names
     (rho-eta: new gamma, alpha, beta values first, as a new config of RhoEta/src/PDF_{gamma,alpha,beta}.cpp);
  2. change TAG, VERSION and YEAR below;
  3. adjust scan ranges in hflav_averages.py (SCANS) only where a result has moved.
`python scripts/hflav_averages.py list` prints the resulting commands without running anything.
"""

import re
from functools import lru_cache
from pathlib import Path

TAG = "nov26"
VERSION = "November"
YEAR = "2026"

TOP = Path(__file__).resolve().parent.parent

_COMBINER = re.compile(
    r'gc\.(?:new|clone)Combiner\(\s*(\d+)\s*,(?:\s*\d+\s*,)?\s*"([^"]+)"'
)


@lru_cache(maxsize=None)
def combiners(module, exe):
    """{combiner name: id} of <module>/main/<exe>.cpp (commented-out lines are ignored)."""
    source = TOP / module / "main" / f"{exe}.cpp"
    found = {}
    for line in source.read_text().splitlines():
        if line.lstrip().startswith("//"):
            continue
        for cid, name in _COMBINER.findall(line):
            found[name] = int(cid)
    return found


def combiner_name(sub=""):
    """hflav_<TAG> or hflav_<TAG>_<sub>."""
    return f"hflav_{TAG}_{sub}" if sub else f"hflav_{TAG}"


def combiner_id(module, exe, sub=""):
    """Id of the combiner hflav_<TAG>[_<sub>] in <module>/main/<exe>.cpp."""
    name = combiner_name(sub)
    try:
        return combiners(module, exe)[name]
    except KeyError:
        raise SystemExit(
            f'error: no combiner "{name}" in {module}/main/{exe}.cpp '
            f"(release tag {TAG!r} is set in scripts/hflav_release.py)"
        ) from None


if __name__ == "__main__":
    print(f"HFLAV {VERSION} {YEAR} (tag {TAG})")
    for module, exe in (
        ("gammacombo", "gamma_wa"),
        ("alphacombo", "alpha_wa"),
        ("RhoEta", "rhoeta_wa"),
    ):
        prefix = combiner_name()
        for name, cid in sorted(combiners(module, exe).items(), key=lambda kv: kv[1]):
            if name == prefix or name.startswith(prefix + "_"):
                print(f"  {exe} -c {cid:<4d} {name}")
