#!/usr/bin/env python3
"""
Write the HFLAV headline combinations made with GammaCombo (gamma, alpha, rho-eta) as
HFLAV.json files following the hflav-fair schema (the format of HFLAV.json on Zenodo, read by
ut-combo/make_hflav_json.py and by hflav-web). Each file holds one group:

  averages             the headline result first (gamma, alpha; rho-bar and eta-bar), then every
                       other free parameter of the fit at the global minimum with its HESSE error
  averages_correlation the HESSE correlation matrix at the minimum (what --printcor prints)
  fit                  chi2 at the minimum, ndf = number of observables - number of free parameters
  inputs               every observable of every measurement used, with the measurement as source:
                       value, stat/syst uncertainty and references (Inspire record, or the DOI of the
                       HFLAV Zenodo record for inputs that are HFLAV averages)
  inputs_correlation   the measurements' own total (stat+syst) correlations, block diagonal

Run from the top of the gammacombo repository after building (scripts/build-macos-conda.sh), in
the conda env with ROOT (PyROOT is used to read the fit result from the scanner file):

    python scripts/hflav_json.py                        # all three: run, write zenodo/<angle>_wa_hflav.json
    python scripts/hflav_json.py gamma alpha            # some of them
    python scripts/hflav_json.py rhoeta --no-run        # re-use the last scan; only rewrite the JSON
    python scripts/hflav_json.py --copy-to ~/Scratch/hflav/ut-combo
    python scripts/hflav_json.py --validate             # check with hflav_fair (if importable)

For each angle this runs, from the combiner directory,
  1. the scan, exactly as configured in COMBINATIONS below (Matt's headline commands);
  2. for 1D scans, the same command with "-a plot --ps 1 --digits N" (reads the scan back and writes
     the confidence intervals to plots/cl/clintervals_*.py);
  3. "-c N --info --infojson", which writes all inputs at full precision to plots/json/*_inputs.json.
Logs: <module>/plots/logs/hflav_<angle>_{scan,intervals,info}.log

References: <module>/RunScripts/HFLAV/references.json maps each measurement id (the id given to
gc.addPdf in main/<exe>.cpp) to an Inspire record ("inspire": 1838001) or a DOI with a label
("doi": "10.5281/zenodo.19371140", "pub": "HFLAV ..."). Inspire metadata (bibtex key, journal
reference, doi, arXiv) is cached in scripts/inspire_cache.json; records missing from the cache are
fetched from inspirehep.net when online.
"""

import argparse
import datetime
import json
import math
import os
import re
import shlex
import shutil
import subprocess
import sys
import urllib.request
from pathlib import Path

TOP = Path(__file__).resolve().parent.parent
SCHEMA_VERSION = "1.2.0"
CL_LEVELS = {"0.68": 0.683, "0.95": 0.954, "1.00": 0.997}

COMBINATIONS = {
    "gamma": dict(
        module="gammacombo",
        exe="gamma_wa",
        comb=360,
        args="--var g --scanrange 0.7:1.7 --printcor",
        digits=3,
        group="gamma_CKM",
        headline={"g": ("gamma_CKM", r"$\gamma$")},
        # observables that are angles (GammaCombo uses radians): written in degrees
        angle_observables=r"^dD_",
        title="HFLAV combination for the CKM angle gamma",
    ),
    "alpha": dict(
        module="alphacombo",
        exe="alpha_wa",
        comb=50,
        args="--var alpha --scanrange 1:2 --pr --printcor",
        digits=3,
        group="alpha_CKM",
        headline={"alpha": ("alpha_CKM", r"$\alpha$")},
        angle_observables=None,
        title="HFLAV combination for the CKM angle alpha",
    ),
    "rhoeta": dict(
        module="RhoEta",
        exe="rhoeta_wa",
        comb=4,
        args="--ps 1 --var rho --var eta --scanrange 0.1:0.2 --scanrangey 0.3:0.4 --npoints 100 --printcor",
        digits=None,  # 2D scan: the averages are the values at the minimum with their HESSE errors
        group="rhoeta_CKM",
        headline={
            "rho": ("rhobar_CKM", r"$\bar{\rho}$"),
            "eta": ("etabar_CKM", r"$\bar{\eta}$"),
        },
        angle_observables=r"^(alpha|beta|gamma)_obs$",
        title="HFLAV fit of rho-bar and eta-bar from the CKM angles",
    ),
}

# LaTeX for fit parameters whose GammaCombo (TLatex) title does not convert cleanly
PARAMETER_LATEX = {
    "d_dkstz": r"\delta_{B}^{DK^{*0}}",
    "r_dkstz": r"r_{B}^{DK^{*0}}",
    "k_dkstz": r"\kappa_{B}^{DK^{*0}}",
    "phis": r"\phi_s",
    "delta_alpha_pipi": r"|\alpha-\alpha_{\rm eff}|\,(\pi\pi)",
    "delta_alpha_rhorho": r"|\alpha-\alpha_{\rm eff}|\,(\rho\rho)",
}

# observable names in the GammaCombo inputs -> LaTeX (as in ut-combo/make_hflav_json.py)
OBS_PREFIX_LATEX = {
    "rcp_p": "R_{CP+}",
    "rcp_m": "R_{CP-}",
    "acp_p": "A_{CP+}",
    "acp_m": "A_{CP-}",
    "rcpp": "R_{CP+}",
    "acpp": "A_{CP+}",
    "rglw": "R_{\\text{GLW}}",
    "aglw": "A_{\\text{GLW}}",
    "rads": "R_{\\text{ADS}}",
    "aads": "A_{\\text{ADS}}",
    "rp": "R_{+}",
    "rm": "R_{-}",
    "xp": "x_{+}",
    "xm": "x_{-}",
    "yp": "y_{+}",
    "ym": "y_{-}",
    "k": "\\kappa",
    "F": "F_{+}",
    "rD": "r_D",
    "RD": "R_D",
    "dD": "\\delta_D",
    "kD": "\\kappa_D",
    "xD": "x_D",
    "yD": "y_D",
    "phis": "\\phi_s",
    "c": "C",
    "s": "S",
    "sb": "\\bar{S}",
    "d": "D",
    "db": "\\bar{D}",
    "afav": "A_{\\text{fav}}",
    "asup": "A_{\\text{sup}}",
    "rfavsup": "R_{\\text{fav/sup}}",
    "ccp": "C_{CP}",
    "scp": "S_{CP}",
    "gamma": "\\gamma",
    "beta": "\\beta",
    "alpha": "\\alpha",
    "sin2beta": "\\sin 2\\beta",
}
OBS_MODE_LATEX = {
    "dk": "DK",
    "dstk": "D^{*}K",
    "dkst": "DK^{*}",
    "dkstz": "DK^{*0}",
    "dkpipi": "DK\\pi\\pi",
    "dsk": "D_sK",
    "dskpipi": "D_sK\\pi\\pi",
    "kpi": "K\\pi",
    "k3pi": "K3\\pi",
    "kpipi0": "K\\pi\\pi^0",
    "kskpi": "K_SK\\pi",
    "pipipi0": "\\pi\\pi\\pi^0",
    "kkpi0": "KK\\pi^0",
    "pipipipi": "4\\pi",
    "4pi": "4\\pi",
    "kkpipi": "KK\\pi\\pi",
    "dpi": "D\\pi^0",
    "dg": "D\\gamma",
    "pippim": "\\pi^+\\pi^-",
    "pizpiz": "\\pi^0\\pi^0",
    "pippiz": "\\pi^+\\pi^0",
    "rhoprhom": "\\rho^+\\rho^-",
    "rhozrhoz": "\\rho^0\\rho^0",
    "rhoprhoz": "\\rho^+\\rho^0",
}
ALPHA_B = {"bd": "B^0", "bp": "B^+"}
SIGN = {"p": "+", "m": "-", "z": "0", "pm": "+-", "pz": "+0", "mz": "-0"}


def log(msg):
    print(msg, flush=True)


# ---------------------------------------------------------------------------------------------
# LaTeX
# ---------------------------------------------------------------------------------------------


def obs_latex(name):
    """LaTeX for a GammaCombo observable name, e.g. 'rcp_p_dkst_obs' -> R_{CP+}(DK^{*})."""
    n = re.sub(r"(_obs)?(UID\d+)?$", "", name)
    toks = n.split("_")
    if toks[0] in ALPHA_B:
        b = ALPHA_B[toks[0]]
        if toks[-1] == "tau":
            return f"\\tau({b})"
        mode = OBS_MODE_LATEX.get(toks[1], toks[1])
        kind = {"br": "\\mathcal{B}", "fl": "f_L"}.get(
            toks[-1], f"\\text{{{toks[-1]}}}"
        )
        return f"{kind}({b} \\to {mode})"
    if toks[0] in ("u", "i") and len(toks) >= 2:
        sym = toks[0].upper()
        sub = SIGN.get(toks[1], toks[1])
        rest = toks[2:]
        sup = []
        if sym == "U" and rest and rest[0] in SIGN:
            sup.append(SIGN[rest.pop(0)])
        sup += rest
        return f"{sym}_{{{sub}}}" + (f"^{{{','.join(sup)}}}" if sup else "")
    for k in range(len(toks), 0, -1):
        prefix = "_".join(toks[:k])
        if prefix in OBS_PREFIX_LATEX:
            mode = [OBS_MODE_LATEX.get(t, t) for t in toks[k:]]
            sym = OBS_PREFIX_LATEX[prefix]
            if not mode:
                return sym
            return (
                f"{sym}({mode[0]}"
                + (f", D \\to {', '.join(mode[1:])}" if len(mode) > 1 else "")
                + ")"
            )
    return "\\text{" + n.replace("_", "\\_") + "}"


def _strip_command(text, command):
    """'#it{a#it{b}}' -> 'ab' for command '#it' (also with an [option] after it)."""
    while True:
        m = re.search(re.escape(command) + r"(\[[^\]]*\])?\{", text)
        if not m:
            return text
        depth, i = 1, m.end()
        while i < len(text) and depth:
            depth += {"{": 1, "}": -1}.get(text[i], 0)
            i += 1
        text = text[: m.start()] + text[m.end() : i - 1] + text[i:]


def tlatex_to_latex(title):
    """GammaCombo parameter title (ROOT TLatex) -> LaTeX, or None if it does not look like math."""
    if not title or not re.search(r"[#_^{]", title):
        return None
    s = title
    for cmd in ("#it", "#bf", "#rm", "#scale", "#lower", "#raise", "#kern"):
        s = _strip_command(s, cmd)
    s = s.replace("#rightarrow", "\\to ").replace("#plus", "+").replace("#minus", "-")
    s = re.sub(r"#([A-Za-z]+)", r"\\\1", s)
    s = re.sub(r"\*\^\{([^{}]*)\}", r"^{*\1}", s)  # K*^{+} -> K^{*+}
    s = re.sub(r"(?<!\^\{)\*", "^{*}", s)  # D*K -> D^{*}K
    if re.search(
        r"\^\{[^{}]*\}\^|\}\^\{[^{}]*\}\^", s
    ):  # double superscript: let the caller fall back
        return None
    return s


def as_math(latex):
    latex = (latex or "").strip()
    if not latex or latex.startswith("$"):
        return latex
    return f"${latex}$"


# ---------------------------------------------------------------------------------------------
# References
# ---------------------------------------------------------------------------------------------


class Inspire:
    def __init__(self, path, offline=False):
        self.path = path
        self.offline = offline
        self.cache = json.loads(path.read_text()) if path.exists() else {}
        self.dirty = False

    def _fetch(self, rid):
        url = (
            f"https://inspirehep.net/api/literature/{rid}"
            "?fields=texkeys,dois,arxiv_eprints,publication_info"
        )
        with urllib.request.urlopen(url, timeout=15) as resp:
            meta = json.load(resp)["metadata"]
        e = {}
        if meta.get("texkeys"):
            e["bibtex"] = meta["texkeys"][0]
        if meta.get("dois"):
            e["doi"] = meta["dois"][0]["value"]
        if meta.get("arxiv_eprints"):
            e["arxiv"] = meta["arxiv_eprints"][0]["value"]
        for pi in meta.get("publication_info", []):
            if "journal_title" in pi:
                page = pi.get("artid") or pi.get("page_start", "")
                e["pub"] = (
                    f"{pi['journal_title']} {pi.get('journal_volume', '')} ({pi.get('year', '')}) {page}"
                )
                e["pub"] = " ".join(e["pub"].split())
                break
        if "pub" not in e and e.get("arxiv"):
            e["pub"] = f"arXiv:{e['arxiv']}"
        return e

    def get(self, rid):
        key = str(rid)
        if key not in self.cache and not self.offline:
            try:
                self.cache[key] = self._fetch(key)
                self.dirty = True
            except Exception as exc:  # offline, blocked, ...
                log(
                    f"  WARNING: Inspire lookup of {key} failed ({exc}); continuing offline"
                )
                self.offline = True
        e = dict(self.cache.get(key, {}))
        e["url"] = f"https://inspirehep.net/literature/{key}"
        return e

    def save(self):
        if self.dirty:
            self.path.write_text(
                json.dumps(self.cache, indent=2, sort_keys=True) + "\n"
            )
            log(f"  updated {self.path.relative_to(TOP)}")


def references_for(entry, inspire):
    """References object (hflav-fair schema) of one measurement from its references.json entry."""
    if not entry:
        return None
    if entry.get("inspire"):
        ids = (
            entry["inspire"]
            if isinstance(entry["inspire"], list)
            else [entry["inspire"]]
        )
        refs = inspire.get(ids[0])
    elif entry.get("doi"):
        refs = {"doi": entry["doi"], "url": f"https://doi.org/{entry['doi']}"}
    else:
        return None
    for key in ("pub", "bibtex", "arxiv"):
        if entry.get(key):
            refs[key] = entry[key]
    return {k: v for k, v in refs.items() if v}


# ---------------------------------------------------------------------------------------------
# Running GammaCombo
# ---------------------------------------------------------------------------------------------


def run(cmd, cwd, logfile):
    logfile.parent.mkdir(parents=True, exist_ok=True)
    log(f"  ({cwd.name}) {cmd}")
    with open(logfile, "w") as f:
        r = subprocess.run(cmd, shell=True, cwd=cwd, stdout=f, stderr=subprocess.STDOUT)
    if r.returncode:
        tail = "\n".join(logfile.read_text(errors="replace").splitlines()[-15:])
        sys.exit(
            f"error: '{cmd}' failed (exit {r.returncode}); end of {logfile}:\n{tail}"
        )


def plot_args(args, digits):
    """Scan arguments -> arguments that read the scan back and compute the CL intervals."""
    toks = shlex.split(args)
    out, skip = [], False
    for t in toks:
        if skip:
            skip = False
            continue
        if t in ("--printcor",):
            continue
        if t in ("--ps", "--digits", "-a"):
            skip = True
            continue
        out.append(t)
    return " ".join(out + ["-a", "plot", "--ps", "1", "--digits", str(digits)])


def find_core_library():
    names = ["libGammaComboCore.dylib", "libGammaComboCore.so"]
    roots = (
        [Path(os.environ["GAMMACOMBO_BUILD"])]
        if os.environ.get("GAMMACOMBO_BUILD")
        else []
    )
    roots += [TOP / "build", TOP / "build" / "lib"]
    for root in roots:
        for n in names:
            if (root / n).exists():
                return root / n
    hits = sorted(TOP.glob("build*/**/libGammaComboCore.*"))
    hits = [h for h in hits if h.suffix in (".dylib", ".so")]
    if hits:
        return hits[0]
    sys.exit(
        "error: libGammaComboCore not found (build first, or set GAMMACOMBO_BUILD)"
    )


def read_solution(scanner_file):
    """Global minimum stored in the scanner file (RooSlimFitResult 'sol0')."""
    import ROOT

    ROOT.gROOT.SetBatch(True)
    if not hasattr(ROOT, "RooSlimFitResult"):
        lib = find_core_library()
        if ROOT.gSystem.Load(str(lib)) < 0:
            sys.exit(f"error: could not load {lib}")
    f = ROOT.TFile.Open(str(scanner_file))
    if not f or f.IsZombie():
        sys.exit(f"error: cannot open {scanner_file}")
    sol = f.Get("sol0")
    if not sol:
        sys.exit(f"error: no solution 'sol0' in {scanner_file}")
    pars = []
    for p in sol.floatParsFinal():
        pars.append({"name": p.GetName(), "value": p.getVal(), "error": p.getError()})
    cor = sol.correlationMatrix()
    n = cor.GetNrows()
    if n != len(pars):
        sys.exit(
            f"error: correlation matrix of {scanner_file} has {n} rows for {len(pars)} parameters"
            " (was the scan run with this GammaCombo version?)"
        )
    matrix = [[cor(i, j) for j in range(n)] for i in range(n)]
    result = {
        "fcn": sol.minNll(),
        "edm": sol.edm(),
        "status": sol.status(),
        "covqual": sol.covQual(),
        "confirmed": bool(sol.isConfirmed()),
        "parameters": pars,
        "correlation": matrix,
    }
    f.Close()
    return result


def read_intervals(clfile):
    ns = {}
    exec(clfile.read_text(), ns)
    out = []
    for key, entries in ns["intervals"].items():
        for e in entries:
            out.append(
                {
                    "cl": CL_LEVELS.get(key, float(key)),
                    "var": e["var"],
                    "central": float(e["central"]),
                    "min": float(e["min"]),
                    "max": float(e["max"]),
                    "neg": float(e["neg"]),
                    "pos": float(e["pos"]),
                    "unit": e.get("unit", ""),
                    "method": e.get("method", ""),
                }
            )
    return sorted(out, key=lambda c: (c["cl"], c["min"]))


def chi2_prob(chi2, ndf):
    try:
        import ROOT

        return float(ROOT.TMath.Prob(chi2, ndf))
    except Exception:
        from scipy.stats import chi2 as c2

        return float(c2.sf(chi2, ndf))


# ---------------------------------------------------------------------------------------------
# HFLAV.json
# ---------------------------------------------------------------------------------------------


def fmt(x):
    return f"{x:g}"


def sig(x, n=6):
    """Round to n significant digits (fit results are stored in single precision)."""
    return float(f"{x:.{n}g}") if math.isfinite(x) else x


def build(angle, cfg, info, sol, intervals, refmap, inspire, commands):
    pdeg = 180.0 / math.pi
    par_info = {p["name"]: p for p in info["parameters"]}
    angle_obs = (
        re.compile(cfg["angle_observables"]) if cfg.get("angle_observables") else None
    )

    # ---- averages: headline first, then the other free parameters in GammaCombo order
    pars = sol["parameters"]
    order = [i for v in cfg["headline"] for i, p in enumerate(pars) if p["name"] == v]
    order += [i for i in range(len(pars)) if i not in order]
    averages = []
    for i in order:
        p = pars[i]
        meta = par_info.get(p["name"], {})
        is_angle = meta.get("unit", "").lower() == "rad"
        scale = pdeg if is_angle else 1.0
        value = {
            "central": sig(p["value"] * scale),
            "uncertainty": sig(p["error"] * scale),
        }
        unit = "deg" if is_angle else meta.get("unit", "")
        if unit:
            value["unit"] = unit
        if p["name"] in cfg["headline"]:
            name, latex = cfg["headline"][p["name"]]
        else:
            name = p["name"]
            latex = (
                PARAMETER_LATEX.get(p["name"])
                or tlatex_to_latex(meta.get("title", ""))
                or "\\text{" + p["name"].replace("_", "\\_") + "}"
            )
        avg = {
            "name": name,
            "latexname": as_math(latex),
            "PDGcode": "None",
            "comment": "value at the global minimum of the fit, HESSE uncertainty",
            "average": {"value": value},
        }
        mine = [c for c in intervals if c["var"] == p["name"]]
        if mine:
            # profile-likelihood scan of this parameter: the result is the 68.3% CL interval
            # containing the minimum; the other intervals go to the comment (read by the website)
            x0 = p["value"] * scale
            main = min(
                (c for c in mine if abs(c["cl"] - 0.683) < 0.01),
                key=lambda c: (
                    0 if c["min"] <= x0 <= c["max"] else abs(c["central"] - x0)
                ),
            )
            value["central"] = main["central"]
            value["uncertainty"] = [-abs(main["neg"]), abs(main["pos"])]
            u = " deg" if unit == "deg" else ""
            parts = [
                f"{100 * main['cl']:.1f}% CL interval [{fmt(main['min'])}, {fmt(main['max'])}] ({main['method']})"
            ]
            parts += [
                f"{100 * c['cl']:.1f}% CL: [{fmt(c['min'])}, {fmt(c['max'])}]{u}"
                for c in mine
                if c is not main
            ]
            parts.append(
                f"at the global minimum {p['value'] * scale:.6g} +- {p['error'] * scale:.3g}{u} (HESSE)"
            )
            avg["comment"] = "; ".join(parts)
        averages.append(avg)
    corr = [[sig(sol["correlation"][a][b]) for b in order] for a in order]

    # ---- inputs: one per observable, one source per measurement
    inputs, input_order, flat, blocks, unresolved = {}, [], [], [], []
    for m in info["measurements"]:
        desc = " ".join(m["title"].split())
        entry = refmap.get(str(m["measurement_id"]))
        refs = references_for(entry, inspire)
        if not refs:
            unresolved.append(f"{m['measurement_id']}: {desc} [{m['values_source']}]")
            refs = {"pub": m["values_source"] or "unknown"}
        src_name = refs.get("bibtex") or f"{m['pdf_name']}_{m['measurement_id']}"
        names = []
        for o in m["observables"]:
            name = o["name"]
            scale = pdeg if angle_obs and angle_obs.search(name) else 1.0
            value = {"central": o["value"] * scale}
            if o["syst"]:
                value.update(
                    {"statistical": o["stat"] * scale, "systematic": o["syst"] * scale}
                )
            else:
                value["uncertainty"] = o["stat"] * scale
            if scale != 1.0:
                value["unit"] = "deg"
            if name not in inputs:
                inputs[name] = {
                    "name": name,
                    "latexname": as_math(obs_latex(name)),
                    "sources": [],
                }
                input_order.append(name)
            inputs[name]["sources"].append(
                {"name": src_name, "comment": desc, "value": value, "references": refs}
            )
            names.append(name)
            flat.append((name, m["index"]))
        blocks.append((m["index"], names, m["correlation"]))
    # sources in the order they appear in "inputs"; correlations from each measurement's own matrix
    flat = [(name, idx) for name in input_order for (n2, idx) in flat if n2 == name]
    pos = {k: i for i, k in enumerate(flat)}
    icorr = [
        [1.0 if a == b else 0.0 for b in range(len(flat))] for a in range(len(flat))
    ]
    for idx, names, c in blocks:
        for a, na in enumerate(names):
            for b, nb in enumerate(names):
                if c and a < len(c) and b < len(c[a]):
                    icorr[pos[(na, idx)]][pos[(nb, idx)]] = c[a][b]

    nobs, npar = len(info["observables"]), len(info["parameters"])
    ndf = nobs - npar
    fit = {
        "chi2": sol["fcn"],
        "ndf": ndf,
        "p": chi2_prob(sol["fcn"], ndf) if ndf > 0 else 1.0,
    }
    group = {
        "name": cfg["group"],
        "comment": (
            f"HFLAV combination '{info['combination_title']}' ({info['executable']}, combination "
            f"{info['combination_id']}, {info['software']}): {commands['scan']}. "
            "Averages: the headline result from the profile-likelihood scan, then all other free "
            "parameters at the global minimum with their HESSE uncertainties; averages_correlation is "
            "the HESSE correlation matrix at the minimum. Inputs: the measurements used with their "
            "total (stat+syst) correlations. Angles in degrees."
        ),
        "fit": fit,
        "averages": averages,
        "averages_correlation": corr,
        "inputs": [inputs[n] for n in input_order],
        "inputs_correlation": icorr,
    }
    return group, unresolved


def metadata(cfg, args, info):
    return {
        "title": cfg["title"],
        "description": (
            f"{info['combination_title']}: GammaCombo combination {info['combination_id']} of "
            f"{info['executable']}, {len(info['measurements'])} measurements, "
            f"{len(info['observables'])} observables, {len(info['parameters'])} free parameters."
        ),
        "version": args.version,
        "date": datetime.date.today().isoformat(),
        "author": "HFLAV Collaboration",
        "schema": SCHEMA_VERSION,
    }


def validate(path):
    try:
        from hflav_fair import validator
    except ImportError:
        log(
            "  (hflav_fair not importable here: validate with hflav_fair-validate from the hflav-web venv)"
        )
        return None
    return validator.validate(str(path))


# ---------------------------------------------------------------------------------------------


def process(angle, args, inspire):
    cfg = COMBINATIONS[angle]
    cwd = TOP / cfg["module"]
    exe, comb = cfg["exe"], cfg["comb"]
    if not (cwd / "bin" / exe).exists():
        sys.exit(
            f"error: {cfg['module']}/bin/{exe} not found (build and install first)"
        )
    logs = cwd / "plots" / "logs"
    commands = {"scan": f"bin/{exe} -c {comb} {cfg['args']}"}
    one_d = cfg["digits"] is not None
    if one_d:
        commands["intervals"] = (
            f"bin/{exe} -c {comb} {plot_args(cfg['args'], cfg['digits'])}"
        )
    commands["info"] = f"bin/{exe} -c {comb} --info --infojson"
    log(f"{angle}:")
    if not args.no_run:
        run(commands["scan"], cwd, logs / f"hflav_{angle}_scan.log")
        if one_d:
            run(commands["intervals"], cwd, logs / f"hflav_{angle}_intervals.log")
    run(commands["info"], cwd, logs / f"hflav_{angle}_info.log")

    info_files = sorted(
        (cwd / "plots" / "json").glob(f"{exe}_*_inputs.json"), key=os.path.getmtime
    )
    info = None
    for f in reversed(info_files):
        d = json.loads(f.read_text())
        if d.get("combination_id") == comb:
            info = d
            break
    if not info:
        sys.exit(f"error: no plots/json/{exe}_*_inputs.json for combination {comb}")
    vars_ = re.findall(r"--var\s+(\S+)", cfg["args"])
    scanner = (
        cwd
        / "plots"
        / "scanner"
        / f"{exe}_scanner_{info['combination_name']}_{'_'.join(vars_)}.root"
    )
    sol = read_solution(scanner)
    intervals = []
    if one_d:
        clfile = (
            cwd
            / "plots"
            / "cl"
            / f"clintervals_{info['combination_name']}_{vars_[0]}_Prob.py"
        )
        if not clfile.exists():
            sys.exit(f"error: {clfile} not found (the intervals step writes it)")
        intervals = read_intervals(clfile)
    refpath = cwd / "RunScripts" / "HFLAV" / "references.json"
    refmap = json.loads(refpath.read_text()) if refpath.exists() else {}
    group, unresolved = build(
        angle, cfg, info, sol, intervals, refmap, inspire, commands
    )
    doc = {"metadata": metadata(cfg, args, info), "groups": [group]}

    out = TOP / "zenodo" / f"{exe}_hflav.json"
    out.parent.mkdir(exist_ok=True)
    out.write_text(json.dumps(doc, indent=2) + "\n")
    head = group["averages"][: len(cfg["headline"])]
    summary = ", ".join(
        f"{a['name']} = {a['average']['value']['central']:.6g} {a['average']['value']['uncertainty']}"
        f" {a['average']['value'].get('unit', '')}".rstrip()
        for a in head
    )
    log(
        f"  wrote {out.relative_to(TOP)}: {summary}; chi2/ndf = {group['fit']['chi2']:.2f}/{group['fit']['ndf']}, "
        f"p = {group['fit']['p']:.3f}; {len(group['averages'])} parameters, "
        f"{len(info['measurements'])} measurements, {len(group['inputs'])} observables"
    )
    if unresolved:
        log(
            f"  WARNING: no reference for {len(unresolved)} measurement(s); add them to {refpath.relative_to(TOP)}:"
        )
        for u in unresolved:
            log(f"      {u}")
    if args.copy_to:
        dest = Path(args.copy_to).expanduser() / angle / f"{angle}_wa_hflav.json"
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(out, dest)
        log(f"  copied to {dest}")
    if args.validate:
        n = validate(out)
        if n is not None:
            log(f"  hflav_fair: {n} error(s)")
    return out


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument(
        "angles",
        nargs="*",
        metavar="angle",
        help=f"combinations to write: {', '.join(COMBINATIONS)} (default: all)",
    )
    ap.add_argument(
        "--no-run",
        action="store_true",
        help="do not rerun the scan; use the last scanner file",
    )
    ap.add_argument(
        "--offline",
        action="store_true",
        help="do not query Inspire (use the cache only)",
    )
    ap.add_argument("--version", default="1.0.0", help="metadata version of the files")
    ap.add_argument(
        "--copy-to",
        help="ut-combo checkout: also write <dir>/<angle>/<angle>_wa_hflav.json",
    )
    ap.add_argument(
        "--validate",
        action="store_true",
        help="validate with hflav_fair (if importable)",
    )
    args = ap.parse_args()
    unknown = [a for a in args.angles if a not in COMBINATIONS]
    if unknown:
        ap.error(
            f"unknown combination(s) {', '.join(unknown)}; choose from {', '.join(COMBINATIONS)}"
        )
    inspire = Inspire(TOP / "scripts" / "inspire_cache.json", offline=args.offline)
    for angle in args.angles or list(COMBINATIONS):
        process(angle, args, inspire)
    inspire.save()


if __name__ == "__main__":
    main()
