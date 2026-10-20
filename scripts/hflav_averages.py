#!/usr/bin/env python3
"""
Run the HFLAV CKM-angle averages (gamma, alpha, rho-eta) with GammaCombo,
check them against a reference set of scans, make the HFLAV plots of them,
and export the Zenodo JSON.

The scan list reproduces the published PDG 2025 scans (same combinations,
options, ranges and number of points as the files in <module>/plots/scanner),
so that "rerun as is" gives bin-by-bin identical 1-CL curves.

Run from the top of the gammacombo repository, after building and installing
(scripts/build-macos-conda.sh):

    python scripts/hflav_averages.py run                    # all scans (gamma, alpha, rhoeta)
    python scripts/hflav_averages.py run --angles alpha     # one angle only
    python scripts/hflav_averages.py run --nominal          # only the headline averages
    python scripts/hflav_averages.py check                  # compare with the backed-up reference
    python scripts/hflav_averages.py plots                  # HFLAV plots (overviews + gamma comparisons)
    python scripts/hflav_averages.py plots --copy-to ~/Scratch/hflav/ut-combo   # ... and the overviews into ut-combo
    python scripts/hflav_averages.py plots -i --only alpha  # ... and show them at the end (plt.show())
    python scripts/hflav_averages.py zenodo                 # write zenodo/*_wa_{inputs,outputs}.json
    python scripts/hflav_averages.py list                   # print the commands without running

Before the first scan of a module, `run` copies the existing scanner/cl/par
files of these combinations to <module>/plots/_reference/ (never overwritten),
which is what `check` compares against.

`plots` makes the figures in PLOTS from the scans (scripts/hflav_plots.py; needs PyROOT and
LaTeX, and the HFLAV shared-material checkout for the logo) into
<module>/paper_plots/HFLAV/<version><year>/. The beta overview shows beta_CKM read from the
ut-combo HFLAV.json (--beta-json). With --copy-to <ut-combo>, the overview figures are also
copied to <ut-combo>/<angle>/<angle>_wa_hflav_pub.pdf (gamma: the B-decay-mode comparison).
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

TOP = Path(__file__).resolve().parent.parent

# ---------------------------------------------------------------------------
# Scan definitions.
#   name   : label used in the output
#   module : combiner subdirectory (the scan runs from there: bin/<exe> ...)
#   exe    : executable
#   comb   : combination id
#   vars   : scan variables
#   args   : remaining command-line options
#   out    : combiner name + vars as used in plots/scanner/<exe>_scanner_<out>.root
#   nominal: True for the headline average of each angle
#   after  : name of a scan that must have finished first (e.g. it writes a parfile)
# ---------------------------------------------------------------------------
G = dict(module="gammacombo", exe="gamma_wa")
A = dict(module="alphacombo", exe="alpha_wa")
R = dict(module="RhoEta", exe="rhoeta_wa")

SCANS = [
    # ---- gamma -------------------------------------------------------------
    dict(
        G,
        name="gamma",
        comb=360,
        vars=["g"],
        out="hflav_pdg2025_g",
        nominal=True,
        args="--pr --ps 1 --scanrange 0.8:1.4 --npoints 200 --digits 3",
    ),
    # D-decay categories, TD/TI (comp_methods / comp_tdti)
    dict(
        G,
        name="gamma_2body",
        comb=361,
        vars=["g"],
        out="hflav_pdg2025_2body_g",
        args="--ps 1 --scanrange 0.6:1.8 --npoints 100",
    ),
    dict(
        G,
        name="gamma_3body",
        comb=362,
        vars=["g"],
        out="hflav_pdg2025_3body_g",
        args="--ps 1 --scanrange 0.6:1.8 --npoints 100",
    ),
    dict(
        G,
        name="gamma_4body",
        comb=363,
        vars=["g"],
        out="hflav_pdg2025_4body_g",
        args="--ps 1 --scanrange 0.6:1.6 --npoints 100",
    ),
    dict(
        G,
        name="gamma_td",
        comb=364,
        vars=["g"],
        out="hflav_pdg2025_td_g",
        args="--ps 1 --scanrange 0.6:2.8 --npoints 100",
    ),
    dict(
        G,
        name="gamma_ti",
        comb=375,
        vars=["g"],
        out="hflav_pdg2025_ti_g",
        args="--pr --ps 1 --scanrange 0:3.14159265359 --npoints 100",
    ),
    # B-meson species (comp_states)
    dict(
        G,
        name="gamma_bu",
        comb=365,
        vars=["g"],
        out="hflav_pdg2025_bu_g",
        args="--pr --ps 1 --scanrange 0.6:1.6 --prange k_dkpipi=-2:2",
    ),
    dict(
        G,
        name="gamma_bd",
        comb=366,
        vars=["g"],
        out="hflav_pdg2025_bd_g",
        args="--pr --ps 1 --scanrange 0.2:2.8 --npoints 400",
    ),
    dict(
        G,
        name="gamma_bs",
        comb=367,
        vars=["g"],
        out="hflav_pdg2025_bs_g",
        args="--pr --ps 1 --scanrange 0.3:3.0",
    ),
    # B decay modes (comp_modes)
    dict(
        G,
        name="gamma_bu2dk",
        comb=368,
        vars=["g"],
        out="hflav_pdg2025_bu2dk_g",
        args="--pr --ps 1 --scanrange 0.8:1.4",
    ),
    dict(
        G,
        name="gamma_bu2dkst",
        comb=369,
        vars=["g"],
        out="hflav_pdg2025_bu2dkst_g",
        args="--pr --ps 1 --scanrange 0.0:3.0",
    ),
    dict(
        G,
        name="gamma_bu2dstk",
        comb=370,
        vars=["g"],
        out="hflav_pdg2025_bu2dstk_g",
        args="--pr --ps 1 --scanrange 0.3:3.0",
    ),
    dict(
        G,
        name="gamma_bd2dkstz",
        comb=371,
        vars=["g"],
        out="hflav_pdg2025_bd2dkstz_g",
        args="--pr --ps 1 --scanrange -0.1:3.0 --npoints 400",
    ),
    dict(
        G,
        name="gamma_bs2dsk",
        comb=372,
        vars=["g"],
        out="hflav_pdg2025_bs2dsk_g",
        args="--pr --ps 1 --scanrange 0.3:3.8",
    ),
    dict(
        G,
        name="gamma_bs2dskpp",
        comb=373,
        vars=["g"],
        out="hflav_pdg2025_bs2dskpp_g",
        args="--pr --ps 1 --scanrange 0.0:3.0",
    ),
    # hadronic parameters of the combination (comp_ratios / print_res)
    dict(
        G,
        name="gamma_l_dskpipi",
        comb=360,
        vars=["l_dskpipi"],
        out="hflav_pdg2025_l_dskpipi",
        args="--ps 1 --scanrange 0.0:0.8 --npoints 100",
    ),
    dict(
        G,
        name="gamma_l_dsk",
        comb=360,
        vars=["l_dsk"],
        out="hflav_pdg2025_l_dsk",
        args="--ps 1 --scanrange 0.1:0.5 --npoints 100",
    ),
    dict(
        G,
        name="gamma_r_dkst",
        comb=360,
        vars=["r_dkst"],
        out="hflav_pdg2025_r_dkst",
        args="--ps 1 --scanrange 0.0:0.2 --npoints 100",
    ),
    dict(
        G,
        name="gamma_r_dstk",
        comb=360,
        vars=["r_dstk"],
        out="hflav_pdg2025_r_dstk",
        args="--ps 1 --scanrange 0.0:0.2 --npoints 100",
    ),
    dict(
        G,
        name="gamma_r_dkstz",
        comb=360,
        vars=["r_dkstz"],
        out="hflav_pdg2025_r_dkstz",
        args="--ps 1 --scanrange 0.1:0.4 --npoints 100",
    ),
    dict(
        G,
        name="gamma_r_dk",
        comb=360,
        vars=["r_dk"],
        out="hflav_pdg2025_r_dk",
        args="--ps 1 --scanrange 0.08:0.12 --npoints 100",
    ),
    dict(
        G,
        name="gamma_d_dk",
        comb=360,
        vars=["d_dk"],
        out="hflav_pdg2025_d_dk",
        args="--ps 1 --scanrange 2.0:2.6 --npoints 200",
    ),
    dict(
        G,
        name="gamma_d_dkst",
        comb=360,
        vars=["d_dkst"],
        out="hflav_pdg2025_d_dkst",
        args="--ps 1 --scanrange 0.0:3.0 --npoints 200",
    ),
    dict(
        G,
        name="gamma_d_dkstz",
        comb=360,
        vars=["d_dkstz"],
        out="hflav_pdg2025_d_dkstz",
        args="--ps 1 --scanrange 3.0:5.0 --npoints 200",
    ),
    dict(
        G,
        name="gamma_d_dsk",
        comb=360,
        vars=["d_dsk"],
        out="hflav_pdg2025_d_dsk",
        args="--ps 1 --scanrange 5.0:7.0 --npoints 200",
    ),
    dict(
        G,
        name="gamma_d_dstk",
        comb=360,
        vars=["d_dstk"],
        out="hflav_pdg2025_d_dstk",
        args="--ps 1 --scanrange 4.5:6.0 --npoints 200",
    ),
    # ---- alpha -------------------------------------------------------------
    dict(
        A,
        name="alpha",
        comb=60,
        vars=["alpha"],
        out="hflav_nov26_alpha",
        nominal=True,
        args="--pr --ps 1 --scanrange 0.5:2.0 --npoints 200",
    ),
    dict(
        A,
        name="alpha_pipi",
        comb=61,
        vars=["alpha"],
        out="hflav_nov26_pipi_alpha",
        # start from the eight pipi solutions of PDG 2021: without it the scan can miss mirror solutions
        args="--scanrange -0.2:3.2 --npoints 2400 --parfile plots/par/alpha_wa_hflav_pdg2021_pipi_alpha_start.dat",
    ),
    dict(
        A,
        name="alpha_rhorho",
        comb=62,
        vars=["alpha"],
        out="hflav_nov26_rhorho_alpha",
        args="--pr --scanrange -0.2:3.2 --npoints 1200",
    ),
    dict(
        A,
        name="alpha_rhopi",
        comb=63,
        vars=["alpha"],
        out="hflav_nov26_rhopi_alpha",
        # start on the C < 0 branch of the rho-pi Dalitz fit (PDG 2025 start values): from the default
        # start values (C = +0.79) the scan follows the mirror branch and misses the minima above 100 deg
        args="--pr --ps 1 --npoints 400 --parfile plots/par/alpha_wa_hflav_pdg2025_rhopi_alpha_start.dat",
    ),
    # ---- rho-eta -----------------------------------------------------------
    # combination 4 (all angles) first: it writes the parfile the others start from
    dict(
        R,
        name="rhoeta",
        comb=4,
        vars=["rho", "eta"],
        out="hflav_angles_rho_eta",
        nominal=True,
        args="--ps 1 --scanrange 0.04:0.2 --scanrangey 0.25:0.45 --npoints2dx 100 --npoints2dy 100",
    ),
    dict(
        R,
        name="rhoeta_gamma",
        comb=1,
        vars=["rho", "eta"],
        out="hflav_gamma_rho_eta",
        after="rhoeta",
        args="--ps 1 --scanrange -0.02:1.02 --scanrangey -0.02:1.02 --npoints 400 --parfile plots/par/rhoeta_wa_hflav_angles_rho_eta.dat",
    ),
    dict(
        R,
        name="rhoeta_beta",
        comb=2,
        vars=["rho", "eta"],
        out="hflav_beta_rho_eta",
        after="rhoeta",
        args="--ps 1 --scanrange -0.22:1.22 --scanrangey -0.22:1.22 --npoints 1600 --parfile plots/par/rhoeta_wa_hflav_angles_rho_eta.dat",
    ),
    dict(
        R,
        name="rhoeta_alpha",
        comb=3,
        vars=["rho", "eta"],
        out="hflav_alpha_rho_eta",
        after="rhoeta",
        args="--ps 1 --scanrange -0.02:1.02 --scanrangey -0.02:1.02 --npoints 800 --parfile plots/par/rhoeta_wa_hflav_angles_rho_eta.dat",
    ),
    dict(
        R,
        name="rhoeta_beta_alt",
        comb=6,
        vars=["rho", "eta"],
        out="hflav_beta_alt_rho_eta",
        after="rhoeta",
        args="--ps 1 --scanrange -0.22:1.22 --scanrangey -0.22:1.22 --npoints 800 --parfile plots/par/rhoeta_wa_hflav_angles_rho_eta.dat",
    ),
]

# ---------------------------------------------------------------------------
# Plot definitions (action `plots`): HFLAV figures of the scans above, written to
# <module>/paper_plots/HFLAV/<version><year>/<file>.pdf (and .png).
#   name     : label used in the output (and by --only)
#   module   : combiner subdirectory
#   file     : output file name, without extension
#   overview : the ut-combo overview figure of this angle (--copy-to <ut-combo>/<angle>/<angle>_wa_hflav_pub.pdf)
#   opts     : hflav_plots.Plotter options (axes, legend and logo position, CL lines, ...)
#   curves   : (scan name in SCANS, hflav_plots.Plotter.add_scan options: col, hatch, lw, label, bf)
#   beta_label: draw the beta solutions with beta_CKM from the ut-combo HFLAV.json (--beta-json)
# ---------------------------------------------------------------------------
GAMMA_1D = dict(dim=1, xtitle=r"$\gamma$ [$^\circ$]", xangle=True)
RHOETA_2D = dict(
    dim=2, xtitle=r"$\bar{\rho}$", ytitle=r"$\bar{\eta}$", legfontsize=14, levels=2
)
COMBINED = dict(col="k", hatch=None, lw=2, label="Combined")

PLOTS = [
    # ---- gamma -------------------------------------------------------------
    dict(
        name="gamma_mode",
        module="gammacombo",
        file="gamma_wa_hflav_pub_mode",
        overview="gamma",
        opts=dict(GAMMA_1D, xrange=(0, 165), logo="r", legpos="r", cls="r"),
        curves=[
            (
                "gamma_bs2dsk",
                dict(col="y", hatch=r"\\\\", lw=1, label=r"$B_s^0 \to D_s^- K^+$"),
            ),
            (
                "gamma_bs2dskpp",
                dict(
                    col="lg",
                    hatch="xxxx",
                    lw=1,
                    label=r"$B_s^0 \to D_s^- K^+\pi^+\pi^-$",
                ),
            ),
            (
                "gamma_bu2dkst",
                dict(col="r", hatch="////", lw=1, label=r"$B^+ \to D^{0}K^{*+}$"),
            ),
            (
                "gamma_bu2dstk",
                dict(col="p", hatch="----", lw=1, label=r"$B^+ \to D^{*0}K^+$"),
            ),
            (
                "gamma_bd2dkstz",
                dict(col="b", hatch="||||", lw=1, label=r"$B^0 \to D^0 K^{*0}$"),
            ),
            (
                "gamma_bu2dk",
                dict(col="g", hatch=".....", lw=1, label=r"$B^+ \to D^0 K^+$"),
            ),
            ("gamma", COMBINED),
        ],
    ),
    dict(
        name="gamma_method",
        module="gammacombo",
        file="gamma_wa_hflav_pub_method",
        opts=dict(
            GAMMA_1D, xrange=(0, 160), logo="l", legpos="l", legfontsize=14, cls="l"
        ),
        curves=[
            ("gamma_2body", dict(col="g", hatch=r"\\\\", lw=1, label=r"$D\to$2-body")),
            ("gamma_3body", dict(col="b", hatch="////", lw=1, label=r"$D\to$3-body")),
            ("gamma_4body", dict(col="r", hatch=".....", lw=1, label=r"$D\to$4-body")),
            ("gamma_td", dict(col="p", hatch="----", lw=1, label=r"TD")),
            ("gamma", COMBINED),
        ],
    ),
    dict(
        name="gamma_tdti",
        module="gammacombo",
        file="gamma_wa_hflav_pub_tdti",
        opts=dict(
            GAMMA_1D, xrange=(30, 130), logo="l", legpos="l", legfontsize=14, cls="l"
        ),
        curves=[
            ("gamma_ti", dict(col="g", hatch=r"\\\\", lw=1, label=r"TI")),
            ("gamma_td", dict(col="b", hatch="////", lw=1, label=r"TD")),
            ("gamma", COMBINED),
        ],
    ),
    dict(
        name="gamma_state",
        module="gammacombo",
        file="gamma_wa_hflav_pub_state",
        opts=dict(GAMMA_1D, xrange=(30, 130), logo="r", legpos="r", cls="r"),
        curves=[
            ("gamma_bs", dict(col="r", hatch=".....", lw=1, label=r"$B_s^0$ decays")),
            ("gamma_bd", dict(col="b", hatch="////", lw=1, label=r"$B^0$ decays")),
            ("gamma_bu", dict(col="g", hatch=r"\\\\", lw=1, label=r"$B^+$ decays")),
            ("gamma", COMBINED),
        ],
    ),
    dict(
        name="gamma_ratios",
        module="gammacombo",
        file="gamma_wa_hflav_pub_ratios",
        opts=dict(
            dim=1, xtitle=r"$r_B$", xrange=(0, 0.7), logo="r", legpos="r", cls="r"
        ),
        curves=[
            (
                "gamma_l_dskpipi",
                dict(
                    col="lg",
                    hatch="xxxx",
                    lw=1,
                    label=r"$r_{B_s^0}^{D_s^{\mp} K^\pm \pi^+ \pi^-}$",
                ),
            ),
            (
                "gamma_l_dsk",
                dict(
                    col="y", hatch=r"\\\\", lw=1, label=r"$r_{B_s^0}^{D_s^{\mp} K^\pm}$"
                ),
            ),
            (
                "gamma_r_dkst",
                dict(col="r", hatch="////", lw=1, label=r"$r_{B^\pm}^{DK^{*\pm}}$"),
            ),
            (
                "gamma_r_dstk",
                dict(col="p", hatch="----", lw=1, label=r"$r_{B^\pm}^{D^{*}K^{\pm}}$"),
            ),
            (
                "gamma_r_dkstz",
                dict(col="b", hatch="||||", lw=1, label=r"$r_{B^0}^{DK^{*0}}$"),
            ),
            (
                "gamma_r_dk",
                dict(col="g", hatch=".....", lw=1, label=r"$r_{B^\pm}^{DK^{\pm}}$"),
            ),
        ],
    ),
    # ---- alpha -------------------------------------------------------------
    dict(
        name="alpha",
        module="alphacombo",
        file="alpha_wa_hflav_pub",
        overview="alpha",
        opts=dict(
            dim=1,
            xtitle=r"$\alpha$ [$^\circ$]",
            xangle=True,
            xrange=(0, 180),
            logo="r",
            legpos="l",
            legfill=True,
            tight=True,
        ),
        curves=[
            ("alpha_pipi", dict(col="g", hatch="////", lw=1, label=r"$B\to\pi\pi$")),
            (
                "alpha_rhorho",
                dict(col="b", hatch=r"\\\\", lw=1, label=r"$B\to\rho\rho$"),
            ),
            (
                "alpha_rhopi",
                dict(col="r", hatch=".....", lw=1, label=r"$B^0\to(\rho\pi)^0$"),
            ),
            ("alpha", dict(COMBINED, label="Combination")),
        ],
    ),
    # ---- rho-eta (and the beta overview) -----------------------------------
    dict(
        name="rhoeta",
        module="RhoEta",
        file="rhoeta_wa_hflav_pub",
        overview="rhoeta",
        opts=dict(RHOETA_2D, xrange=(0, 1), yrange=(0, 0.744), logo="r", legpos="l"),
        curves=[
            ("rhoeta_alpha", dict(col="r", label=r"$\alpha$")),
            ("rhoeta_beta", dict(col="g", label=r"$\beta$")),
            ("rhoeta_gamma", dict(col="b", label=r"$\gamma$")),
            ("rhoeta", dict(col="k", label=None, bf=True)),
        ],
    ),
    dict(
        name="beta",
        module="RhoEta",
        file="beta_wa_hflav_pub",
        overview="beta",
        beta_label=True,
        opts=dict(
            RHOETA_2D,
            xrange=(-0.1, 1.2),
            yrange=(-0.1, 1.2),
            logo="l",
            legpos="l",
            cl_text=(0.12, 0.02),
        ),
        curves=[("rhoeta_beta", dict(col="r")), ("rhoeta_beta_alt", dict(col="r"))],
    ),
]

ANGLE_OF_MODULE = {"gammacombo": "gamma", "alphacombo": "alpha", "RhoEta": "rhoeta"}

# Zenodo export: (angle, module, exe, comb id, clintervals file of the nominal scan)
ZENODO = [
    (
        "gamma",
        "gammacombo",
        "gamma_wa",
        360,
        "plots/cl/clintervals_hflav_pdg2025_g_Prob.py",
    ),
    (
        "alpha",
        "alphacombo",
        "alpha_wa",
        50,
        "plots/cl/clintervals_hflav_pdg2025_alpha_Prob.py",
    ),
]


def command(s):
    var = " ".join(f"--var {v}" for v in s["vars"])
    return f"bin/{s['exe']} -c {s['comb']} {var} {s['args']}"


def scanner_file(s, base=None):
    base = base or TOP / s["module"] / "plots"
    return base / "scanner" / f"{s['exe']}_scanner_{s['out']}.root"


def par_file(s, base=None):
    base = base or TOP / s["module"] / "plots"
    return base / "par" / f"{s['exe']}_{s['out']}.dat"


def select(args):
    scans = [s for s in SCANS if ANGLE_OF_MODULE[s["module"]] in args.angles]
    if args.nominal:
        scans = [s for s in scans if s.get("nominal")]
    if args.only:
        scans = [s for s in scans if s["name"] in args.only]
    return scans


def backup_reference(scans):
    """Copy the current scanner/cl/par files of these scans to plots/_reference (once)."""
    for s in scans:
        plots = TOP / s["module"] / "plots"
        ref = plots / "_reference"
        for sub, pattern in [
            ("scanner", f"{s['exe']}_scanner_{s['out']}.root"),
            ("cl", f"clintervals_{s['out']}_Prob.py"),
            ("par", f"{s['exe']}_{s['out']}.dat"),
        ]:
            src = plots / sub / pattern
            dst = ref / sub / pattern
            if src.exists() and not dst.exists():
                dst.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(src, dst)


def run_one(s, verbose):
    cwd = TOP / s["module"]
    cmd = command(s)
    log = cwd / "plots" / "logs" / f"{s['name']}.log"
    log.parent.mkdir(parents=True, exist_ok=True)
    print(f"[start] {s['name']:18s} ({s['module']}) {cmd}", flush=True)
    with open(log, "w") as f:
        r = subprocess.run(cmd, shell=True, cwd=cwd, stdout=f, stderr=subprocess.STDOUT)
    ok = r.returncode == 0 and scanner_file(s).exists()
    status = "done " if ok else "FAIL "
    print(f"[{status}] {s['name']:18s} log: {log.relative_to(TOP)}", flush=True)
    if verbose or not ok:
        lines = log.read_text(errors="replace").splitlines()
        for l in lines:
            if "@0.68CL" in l or "@0.95CL" in l or "terminate" in l or "ERROR" in l:
                print("          " + l.strip())
    return s["name"], ok


def cmd_run(args):
    scans = select(args)
    missing = sorted(
        {s["exe"] for s in scans if not (TOP / s["module"] / "bin" / s["exe"]).exists()}
    )
    if missing:
        sys.exit(
            f"error: not built/installed: {', '.join(missing)} (run scripts/build-macos-conda.sh)"
        )
    backup_reference(scans)
    first = [s for s in scans if not s.get("after")]
    later = [s for s in scans if s.get("after")]
    results = {}
    with ThreadPoolExecutor(max_workers=args.jobs) as ex:
        for name, ok in ex.map(lambda s: run_one(s, args.verbose), first):
            results[name] = ok
    runnable = [s for s in later if results.get(s["after"], True)]
    with ThreadPoolExecutor(max_workers=args.jobs) as ex:
        for name, ok in ex.map(lambda s: run_one(s, args.verbose), runnable):
            results[name] = ok
    failed = [n for n, ok in results.items() if not ok]
    skipped = [s["name"] for s in later if s not in runnable]
    print(
        f"\n{len(results) - len(failed)}/{len(scans)} scans succeeded"
        + (f"; failed: {', '.join(failed)}" if failed else "")
        + (f"; skipped: {', '.join(skipped)}" if skipped else "")
    )
    return 1 if failed or skipped else 0


def cmd_list(args):
    for s in select(args):
        print(f"(cd {s['module']} && {command(s)})")
    print("\n# plots (action 'plots'): name: scans")
    for p in select_plots(args):
        print(f"# {p['name']}: {' '.join(scan for scan, _ in p['curves'])}")
    return 0


def cmd_check(args):
    try:
        import ROOT  # noqa: F401
    except ImportError:
        sys.exit(
            "error: PyROOT not available in this python (activate the gc_2026 env)"
        )
    import ROOT

    ROOT.gROOT.SetBatch(True)
    worst = 0.0
    n_bad = 0
    for s in select(args):
        ref = scanner_file(s, TOP / s["module"] / "plots" / "_reference")
        new = scanner_file(s)
        if not ref.exists() or not new.exists():
            print(
                f"{s['name']:18s} skipped (missing {'reference' if not ref.exists() else 'new scan'})"
            )
            continue
        fr, fn = ROOT.TFile.Open(str(ref)), ROOT.TFile.Open(str(new))
        hr, hn = fr.Get("hCL"), fn.Get("hCL")
        xr, xn = hr.GetXaxis(), hn.GetXaxis()
        eps = 1e-6 * (xr.GetXmax() - xr.GetXmin())
        same_bins = (
            hr.GetNbinsX() == hn.GetNbinsX()
            and hr.GetNbinsY() == hn.GetNbinsY()
            and abs(xr.GetXmin() - xn.GetXmin()) < eps
            and abs(xr.GetXmax() - xn.GetXmax()) < eps
        )
        if not same_bins:
            print(
                f"{s['name']:18s} binning differs from reference, cannot compare bin by bin"
            )
            n_bad += 1
            continue
        d = 0.0
        for i in range(1, hr.GetNbinsX() + 1):
            for j in range(1, hr.GetNbinsY() + 1):
                d = max(d, abs(hr.GetBinContent(i, j) - hn.GetBinContent(i, j)))
        worst = max(worst, d)
        flag = "identical" if d < 1e-6 else ("OK" if d < args.tol else "DIFFERENT")
        n_bad += flag == "DIFFERENT"
        print(f"{s['name']:18s} max |delta(1-CL)| = {d:.2e}  {flag}")
    print(
        f"\nworst |delta(1-CL)| = {worst:.2e}; {n_bad} scan(s) outside tolerance {args.tol}"
    )
    return 1 if n_bad else 0


def select_plots(args):
    plots = [p for p in PLOTS if ANGLE_OF_MODULE[p["module"]] in args.angles]
    if args.only:
        plots = [p for p in plots if p["name"] in args.only]
    return plots


def beta_ckm(path):
    """beta_CKM (degrees) and its uncertainty from an HFLAV.json (ut-combo hflav/HFLAV.json)."""
    path = Path(path).expanduser()
    if not path.exists():
        raise FileNotFoundError(
            f"{path} not found (the beta value comes from ut-combo; see --beta-json)"
        )
    groups = json.loads(path.read_text())["groups"]
    group = next((g for g in groups if g["name"] == "beta_CKM"), None)
    if group is None:
        raise KeyError(f"no beta_CKM group in {path}")
    value = group["averages"][0]["average"]["value"]
    return value["central"], value["uncertainty"]


def cmd_plots(args):
    import matplotlib

    if not args.interactive:
        matplotlib.use("Agg")
    try:
        import hflav_plots
        import matplotlib.pyplot as plt
    except ImportError as exc:
        sys.exit(
            f"error: cannot import the plotting code ({exc}); activate the gc_2026 env (PyROOT, matplotlib)"
        )
    scans = {s["name"]: s for s in SCANS}
    outdir = Path("paper_plots") / "HFLAV" / f"{args.version}{args.year}"
    plots = select_plots(args)
    failed = []
    figures = []  # kept open for plt.show() with --interactive
    for p in plots:
        out = TOP / p["module"] / outdir / f"{p['file']}.pdf"
        out.parent.mkdir(parents=True, exist_ok=True)
        try:
            opts = dict(p["opts"], save=str(out), version=args.version, year=args.year)
            if p.get("beta_label"):
                beta, beta_err = beta_ckm(args.beta_json)
                opts["extras"] = [hflav_plots.beta_decorations(beta, beta_err)]
            plotter = hflav_plots.Plotter(**opts)
            for scan_name, curve in p["curves"]:
                s = scans[scan_name]
                files = [scanner_file(s), par_file(s)]
                missing = [str(f.relative_to(TOP)) for f in files if not f.exists()]
                if missing:
                    raise FileNotFoundError(
                        f"missing {', '.join(missing)} "
                        f"(run: python scripts/hflav_averages.py run --only {scan_name})"
                    )
                if args.verbose:
                    print(f"          {scan_name}:")
                    hflav_plots.print_cl_interval(
                        TOP
                        / s["module"]
                        / "plots"
                        / "cl"
                        / f"clintervals_{s['out']}_Prob.py"
                    )
                plotter.add_scan((*files, s["vars"]), **curve)
            fig = plotter.plot(keep_open=args.interactive)
            if fig is not None:
                manager = fig.canvas.manager
                if manager is not None:
                    manager.set_window_title(p["name"])
                figures.append(fig)
        except Exception as exc:
            failed.append(p["name"])
            print(f"[FAIL ] {p['name']:18s} {type(exc).__name__}: {exc}", flush=True)
            continue
        print(f"[done ] {p['name']:18s} {out.relative_to(TOP)} (+ .png)", flush=True)
        if args.copy_to and p.get("overview"):
            angle = p["overview"]
            dest = Path(args.copy_to).expanduser() / angle / f"{angle}_wa_hflav_pub.pdf"
            if dest.parent.is_dir():
                shutil.copyfile(out, dest)
                print(f"          copied to {dest}")
            else:
                print(
                    f"          WARNING: {dest.parent} does not exist, overview not copied"
                )
                failed.append(f"{p['name']} (copy)")
    n_failed = len([f for f in failed if not f.endswith("(copy)")])
    print(
        f"\n{len(plots) - n_failed}/{len(plots)} plots made"
        + (f"; failed: {', '.join(failed)}" if failed else "")
    )
    if figures:
        print(f"showing {len(figures)} plot(s); close the windows to finish", flush=True)
        plt.show()
    return 1 if failed else 0


def cmd_zenodo(args):
    out = TOP / "zenodo"
    out.mkdir(exist_ok=True)
    for angle, module, exe, comb, clfile in ZENODO:
        if angle not in args.angles:
            continue
        cwd = TOP / module
        raw = out / f"{exe}_inputs.txt"
        with open(raw, "w") as f:
            subprocess.run(
                f"bin/{exe} -c {comb} -v --info",
                shell=True,
                cwd=cwd,
                stdout=f,
                stderr=subprocess.STDOUT,
                check=True,
            )
        subprocess.run(
            [
                sys.executable,
                str(TOP / "scripts" / "info_to_json.py"),
                "-i",
                str(raw),
                "-o",
                str(out / f"{exe}_inputs.json"),
                "-c",
                str(comb),
                "--source",
                f"{module}/main/{exe}.cpp",
            ],
            check=True,
        )
        subprocess.run(
            [
                sys.executable,
                str(TOP / "scripts" / "output_to_json.py"),
                "-i",
                str(cwd / clfile),
                "-o",
                str(out / f"{exe}_outputs.json"),
            ],
            check=True,
        )
    return 0


def main():
    p = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    p.add_argument("action", choices=["run", "check", "plots", "zenodo", "list"])
    p.add_argument(
        "--angles",
        nargs="+",
        default=["gamma", "alpha", "rhoeta"],
        choices=["gamma", "alpha", "rhoeta"],
    )
    p.add_argument(
        "--nominal", action="store_true", help="only the headline average of each angle"
    )
    p.add_argument(
        "--only",
        nargs="+",
        help="only these scans (run/check/list) or plots (plots), by name as printed by 'list'",
    )
    p.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=max(1, (os.cpu_count() or 2) // 2),
        help="parallel scans",
    )
    p.add_argument(
        "--tol", type=float, default=0.01, help="check: allowed max |delta(1-CL)|"
    )
    p.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="print the CL intervals of each scan",
    )
    p.add_argument(
        "--version",
        default="November",
        help="plots: release shown in the logo (default: November)",
    )
    p.add_argument(
        "--year", default="2026", help="plots: year shown in the logo (default: 2026)"
    )
    p.add_argument(
        "--beta-json",
        default="~/Scratch/hflav/ut-combo/hflav/HFLAV.json",
        help="plots: HFLAV.json with beta_CKM, shown in the beta overview (default: the ut-combo one)",
    )
    p.add_argument(
        "-i",
        "--interactive",
        action="store_true",
        help="plots: also show the plots at the end (plt.show()); use --only to limit the number of windows",
    )
    p.add_argument(
        "--copy-to",
        help="plots: ut-combo checkout; also write the overviews to <dir>/<angle>/<angle>_wa_hflav_pub.pdf",
    )
    args = p.parse_args()
    sys.exit(
        {
            "run": cmd_run,
            "check": cmd_check,
            "plots": cmd_plots,
            "zenodo": cmd_zenodo,
            "list": cmd_list,
        }[args.action](args)
    )


if __name__ == "__main__":
    main()
