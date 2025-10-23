import importlib
import itertools
import os

import matplotlib.patches as patches
import matplotlib.pyplot as plt
import numpy as np
import ROOT as r
from matplotlib import rcParams
from matplotlib.lines import Line2D
from scipy.interpolate import interp1d
from scipy.stats import chi2
from tabulate import tabulate


def load_interval(fname):
    if not os.path.exists(fname):
        raise RuntimeError("No such file", fname)
    # print('Loading configuration from file', fname)
    spec = importlib.util.spec_from_file_location("module", fname)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    if hasattr(module, "intervals"):
        return module.intervals
    return None


def print_interval(fname, nsigma=1, floatfmt=[]):
    intervals = load_interval(fname)
    print_rows = []
    for cl, cfgs in intervals.items():
        ns = 0
        conf = 0
        if cl == "0.68":
            ns = 1
        elif cl == "0.95":
            ns = 2
        elif cl == "1.00":
            ns = 3
        else:
            continue

        if ns > nsigma:
            continue

        conf = f"{chi2.cdf(ns**2, 1):4.1%}"

        for sol in cfgs:
            print_rows.append(
                [
                    ns,
                    conf,
                    sol["central"],
                    sol["neg"],
                    sol["pos"],
                    sol["min"],
                    sol["max"],
                ]
            )

    print(
        tabulate(
            print_rows,
            headers=["nSig", "cl", "val", "-err", "+err", "min", "max"],
            floatfmt=floatfmt,
        )
    )


def read1dscan(h, bf, minnll):
    assert len(bf) == 1

    x = np.array([h.GetBinCenter(b) for b in range(1, h.GetNbinsX() + 1)])
    y = np.array([h.GetBinContent(b) for b in range(1, h.GetNbinsX() + 1)])

    # get chi2 as DeltaChi2
    y -= minnll
    # convert to p-value
    y = chi2.sf(y, 1)

    # insert the best fit
    assert len(x) == len(y)
    index = np.argmax(x > bf)
    x = np.insert(x, index, bf)
    y = np.insert(y, index, 1.0)

    return x, y, None, bf


def read2dscan(h, bf, minnll):
    assert len(bf) == 2

    xcenters = np.array(
        [h.GetXaxis().GetBinCenter(b) for b in range(1, h.GetNbinsX() + 1)]
    )
    ycenters = np.array(
        [h.GetYaxis().GetBinCenter(b) for b in range(1, h.GetNbinsX() + 1)]
    )

    # get content of each bin
    entries = np.array(
        [
            h.GetBinContent(xbin + 1, ybin + 1)
            for xbin, ybin in itertools.product(
                range(h.GetNbinsX()), range(h.GetNbinsY())
            )
        ]
    )

    # convert to right format
    x, y = np.meshgrid(xcenters, ycenters)
    z = entries.reshape((h.GetNbinsX(), h.GetNbinsY()))
    z = z.T - minnll

    return x, y, z, bf


def eval1DPoint(res, x):
    f = interp1d(res[0], res[1], kind="quadratic")
    return f(x)


def read_gc_scan(scanfile, parfile, pars):
    if not os.path.exists(scanfile):
        raise RuntimeError("No such file", scanfile)
    if not os.path.exists(parfile):
        raise RuntimeError("No such file", parfile)
    if len(pars) > 2:
        raise RuntimeError("Cannot pass more than two parameters", pars)

    # get best fit point
    bf = []
    minnll = None
    pf = open(parfile)
    ls = pf.readlines()
    # iterate pars first to keep the right order
    for var in pars:
        for l in ls:
            if "SOLUTION 1" in l:
                break  ## only read the first solution
            if l.startswith("### FCN"):
                minnll = float(l.split()[2].strip(","))
            if l.startswith(var + " "):
                bf.append(float(l.split()[1]))
    pf.close()

    if not len(bf) == len(pars):
        raise RuntimeError(
            "Did not find a best fit value for all the parameters passed",
            pars,
            "in",
            parfile,
        )

    # get chi2 distribution
    tf = r.TFile(scanfile)
    # tf.ls()
    h = tf.Get("hChi2min").Clone()
    # h.Print()

    if h is None:
        raise RuntimeError("No 'hChi2min' found in", scanfile)

    if h.InheritsFrom("TH2") and len(pars) == 2:
        res = read2dscan(h, bf, minnll)
    elif h.InheritsFrom("TH1"):
        res = read1dscan(h, bf, minnll)
    else:
        raise RuntimeError("hchi2min does not appear to be a TH1 or TH2")

    tf.Close()

    return res


def read_external_scan(rootfile, scanvar1=None, scanvar2=None):
    """
    Read an external scan from a ROOT file containing hCL histogram

    Parameters
    ----------
    rootfile : str
        Path to the ROOT file
    scanvar1 : str, optional
        Name of x-axis variable (for axis labels)
    scanvar2 : str, optional
        Name of y-axis variable (for axis labels)

    Returns
    -------
    tuple
        (x, y, z, bf) where bf is [bfx] for 1D or [bfx, bfy] for 2D
    """
    if not os.path.exists(rootfile):
        raise FileNotFoundError(f"Cannot find external scan file {rootfile}")

    tf = r.TFile(rootfile)
    hCL = tf.Get("hCL")

    if hCL is None:
        raise RuntimeError(f"No hCL histogram found in {rootfile}")

    # Determine if 1D or 2D scan first
    is2D = hCL.InheritsFrom("TH2")

    # Try to read best-fit points
    bf = None
    bfX_obj = tf.Get("bestfit_x")

    if bfX_obj and hasattr(bfX_obj, "GetTitle"):
        try:
            bfX = float(bfX_obj.GetTitle())
            bf = [bfX]
            print(f"Found best-fit X: {bfX}")

            # Only try to read Y for 2D scans
            if is2D:
                bfY_obj = tf.Get("bestfit_y")
                if bfY_obj and hasattr(bfY_obj, "GetTitle"):
                    try:
                        bfY = float(bfY_obj.GetTitle())
                        bf.append(bfY)
                        print(f"Found best-fit Y: {bfY}")
                    except (ValueError, ReferenceError, TypeError) as e:
                        print(f"Warning: Could not read best-fit Y value: {e}")
        except (ValueError, ReferenceError, TypeError) as e:
            print(f"Warning: Could not read best-fit X value: {e}")
            bf = None
    else:
        print("Warning: No best-fit point found in external scan file")

    # Process histogram data
    if is2D:
        # 2D case
        xcenters = np.array(
            [hCL.GetXaxis().GetBinCenter(b) for b in range(1, hCL.GetNbinsX() + 1)]
        )
        ycenters = np.array(
            [hCL.GetYaxis().GetBinCenter(b) for b in range(1, hCL.GetNbinsY() + 1)]
        )

        # Get bin contents (already as p-values)
        entries = np.array(
            [
                hCL.GetBinContent(xbin + 1, ybin + 1)
                for xbin, ybin in itertools.product(
                    range(hCL.GetNbinsX()), range(hCL.GetNbinsY())
                )
            ]
        )

        # Convert to meshgrid format
        x, y = np.meshgrid(xcenters, ycenters)
        z = entries.reshape((hCL.GetNbinsX(), hCL.GetNbinsY())).T

        # Convert p-value to chi2 for contour plotting
        z = chi2.isf(z, 2)  # inverse survival function

    elif hCL.InheritsFrom("TH1"):
        # 1D case
        x = np.array([hCL.GetBinCenter(b) for b in range(1, hCL.GetNbinsX() + 1)])
        y = np.array([hCL.GetBinContent(b) for b in range(1, hCL.GetNbinsX() + 1)])
        z = None
    else:
        raise RuntimeError("hCL is not a TH1 or TH2")

    tf.Close()

    return x, y, z, bf


def getfnames(prefix, xpar, ypar=None):
    """
    For a given prefix and one or two parameter names find the scan file and the best-fit file

    Parameters
    ----------
    prefix : str
        The prefix for the scan which is normally the main file name followed by the combination name
        e.g. gamma_wa_scanner_hflav_moriond2024
    xpar : str
        The scan variable name. If `ypar` is provided this is the x-axis parameter
    ypar : str, optional
        The y-axis parameter

    Returns
    -------
    tuple of str
        A two element tuple of the scan file name and the best fit file name

    """

    if ypar is not None:
        fname = f"plots/scanner/{prefix}_{xpar}_{ypar}.root"
    else:
        fname = f"plots/scanner/{prefix}_{xpar}.root"

    bfname = (
        fname.replace("scanner/", "par/")
        .replace("_scanner", "")
        .replace(".root", ".dat")
    )

    if not os.path.exists(fname):
        raise FileNotFoundError(f"Cannot find scan file {fname}")

    if not os.path.exists(bfname):
        raise FileNotFoundError(f"Cannot find fit result file {bfname}")

    return fname, bfname


def print_cl(prefix, xpar, ypar=None, prob=True):
    if ypar is not None:
        return

    pref = prefix.split("scanner")[1]
    suff = "Prob" if prob else "Plugin"
    fname = f"plots/cl/clintervals{pref}_{xpar}_{suff}.py"
    if not os.path.exists(fname):
        return

    try:
        print_interval(fname, nsigma=1)
    except Exception:
        return


def get_scan_res(prefix, xpar, ypar=None):
    pars = [xpar]
    if ypar is not None:
        pars.append(ypar)

    fname, bfname = getfnames(prefix, xpar, ypar)

    header_str = prefix + " - " + xpar
    if ypar is not None:
        header_str += " , " + ypar

    print(header_str)
    print_cl(prefix, xpar, ypar)

    return read_gc_scan(fname, bfname, pars)


def_lopts_1d = [
    dict(c="steelblue"),
    dict(c="forestgreen"),
    dict(c="peru"),
    dict(c="hotpink"),
]

def_fopts_1d = [
    dict(fc="skyblue"),
    dict(fc="darkseagreen"),
    dict(fc="peru"),
    dict(fc="hotpink"),
]

def_mopts_1d = [
    dict(fmt="ko", ms=3),
    dict(fmt="b+", ms=3),
    dict(fmt="gx", ms=3),
    dict(fmt="r^", ms=3),
]

def_lopts_2d = [
    {"colors": ((0.09, 0.66, 0.91), (0.35, 0.33, 0.85)), "zorder": 1},
    {"colors": ((0.8, 0.6, 0.2), (0.6, 0.2, 0.0)), "zorder": 3},
    {"colors": ((0.84, 0.00, 0.99), (0.82, 0.04, 0.82)), "zorder": 5},
    {"colors": ((0.55, 0.28, 0.10), (0.60, 0.30, 0.10)), "zorder": 7},
    {"colors": ((0.0, 0.4, 0.0), (0.2, 0.4, 0.2)), "zorder": 9},
    # { 'colors' : ((0.90,0.20,0.20),(0.70,0.00,0.00))   , 'zorder': 7 },
    # { 'colors' : ((0.0,0.4,0.0)   ,(0.2,0.4,0.2)   )   , 'zorder': 9 }
]

def_fopts_2d = [
    {"colors": ("#bee7fd", "#d3e9ff"), "alpha": 0.9, "zorder": 0},
    {"colors": ((0.90, 0.78, 0.60), (0.99, 0.87, 0.71)), "alpha": 0.9, "zorder": 2},
    {"colors": ((0.96, 0.48, 1.00), (1.00, 0.60, 1.00)), "alpha": 0.9, "zorder": 4},
    {"colors": ("#d95f02", "#df823b"), "alpha": 0.9, "zorder": 6},
    {"colors": ((0.40, 1.00, 0.40), (0.40, 0.80, 0.40)), "alpha": 0.9, "zorder": 8},
]

def_mopts_2d = [
    {"marker": "o", "color": (0.09, 0.66, 0.91), "zorder": 10},
    {"marker": "o", "color": (0.8, 0.6, 0.2), "zorder": 11},
    {"marker": "o", "color": (0.84, 0.00, 0.99), "zorder": 12},
    {"marker": "o", "color": (0.90, 0.20, 0.20), "zorder": 13},
    {"marker": "o", "color": (0.0, 0.4, 0.0), "zorder": 14},
]

hflav_cols = {
    "b": (116, 112, 174),
    "r": (209, 53, 54),
    "g": (74, 155, 122),
    "p": (142, 81, 159),
    "y": (221, 173, 58),
    "lg": (117, 165, 57),
    "k": "k",
}

lhcb_cols = {
    "b": (94, 129, 185),
    "r": (203, 80, 68),
    "o": (231, 138, 104),
    "lb": (150, 186, 214),
    "y": (245, 169, 94),
    "g": (132, 181, 178),
    "p": (169, 124, 159),
    "db": (56, 81, 123),
    "dr": (138, 44, 34),
    "do": (190, 70, 35),
    "dlb": (76, 125, 167),
    "dy": (160, 84, 10),
    "dg": (82, 137, 133),
    "dp": (124, 81, 114),
    "k": "k",
}

lhcb_2d_cols = {
    # lighter to darker
    "b": [
        (114, 144, 193),
        (65, 103, 168),
    ],
    "r": [(227, 152, 144), (215, 108, 96), (203, 80, 68)],
    "o": [(244, 184, 163), (239, 153, 123), (233, 116, 76)],
    "lb": [(188, 211, 230), (159, 192, 219), (120, 167, 204)],
    "y": [
        (249, 201, 152),
        (245, 169, 94),
    ],
    "g": [
        (157, 196, 193),
        (132, 181, 178),
    ],
    "p": [
        (203, 176, 197),
        (186, 150, 178),
    ],
}


keys = hflav_cols.keys()

for key in keys:
    item = hflav_cols[key]
    if type(item) is not str:
        newval = tuple([float(v) / 255.0 for v in item])
        hflav_cols[key] = newval

keys = lhcb_cols.keys()

for key in keys:
    item = lhcb_cols[key]
    if type(item) is not str:
        newval = tuple([float(v) / 255.0 for v in item])
        lhcb_cols[key] = newval

keys = lhcb_2d_cols.keys()

for key in keys:
    items = lhcb_2d_cols[key]
    for i, item in enumerate(items):
        if type(item) is not str:
            newval = tuple([float(v) / 255.0 for v in item])
            lhcb_2d_cols[key][i] = newval

lhcb_ls = {
    "-": "-",
    "longdash": (0, (4, 4)),
    "shortdash": (0, (2, 2)),
    "dashdot": (0, (4, 2, 1, 2)),
    "dash2dots": (0, (4, 2, 1, 2, 1, 2)),
    "dash3dots": (0, (4, 2, 1, 2, 1, 2, 1, 2)),
    "dots": (0, (1, 1)),
}


def get_lopts(nscans, lopts, dim=1):
    if dim == 1:
        defs = def_lopts_1d
    else:
        defs = def_lopts_2d

    ret = [None for i in range(nscans)]
    for i in range(nscans):
        if i < len(lopts):
            ret[i] = lopts[i]
        elif i < len(defs):
            ret[i] = defs[i]
        else:
            raise RuntimeError(f"Can't find a default lopt for iscanner = {i}")
    return ret


def get_fopts(nscans, fopts, dim=1):
    if dim == 1:
        defs = def_fopts_1d
    else:
        defs = def_fopts_2d

    ret = [None for i in range(nscans)]
    for i in range(nscans):
        if i < len(fopts):
            ret[i] = fopts[i]
        elif i < len(defs):
            ret[i] = defs[i]
        else:
            raise RuntimeError(f"Can't find a default fopt for iscanner = {i}")
    return ret


def get_mopts(nscans, mopts, dim=1):
    if dim == 1:
        defs = def_mopts_1d
    else:
        defs = def_mopts_2d

    ret = [None for i in range(nscans)]
    for i in range(nscans):
        if i < len(mopts):
            ret[i] = mopts[i]
        elif i < len(defs):
            ret[i] = defs[i]
        else:
            raise RuntimeError(f"Can't find a default fopt for iscanner = {i}")
    return ret


def addLHCbLogo(ax, x=0.02, y=0.95, prelim=False):
    ax.text(
        x,
        y,
        "LHCb",
        transform=ax.transAxes,
        fontname="Times New Roman",
        fontsize=24,
        ha="left",
        va="top",
    )
    if prelim:
        ax.text(
            x,
            y - 0.08,
            "Preliminary",
            transform=ax.transAxes,
            fontname="Times New Roman",
            fontsize=14,
            ha="left",
            va="top",
        )


def addContoursLine(ax, x=0.01, y=0.01, nlevs=2, etc=False, cl2d=False):
    text = "contours hold"
    for lev in range(nlevs):
        if cl2d:
            fc = chi2.cdf((lev + 1) ** 2, 1)
        else:
            fc = chi2.cdf((lev + 1) ** 2, 2)
        text += rf" {fc * 100:4.1f}\%"
        if lev < nlevs - 1:
            text += ","
    if etc:
        text += " etc."
    ax.text(
        x,
        y,
        text,
        transform=ax.transAxes,
        ha="left",
        va="bottom",
        fontsize=13,
        fontname="Times New Roman",
        color="0.5",
    )


def plot1d(
    scanpoints,
    lopts=[],
    fopts=[],
    xtitle=None,
    legtitles=None,
    angle=False,
    ax=None,
    save=None,
    logo=False,
    prelim=False,
    legopts={},
    axes_origin=(0.12, 0.16),
    righttop_padding=(0.04, 0.05),
):
    """parameters
    scanpoints : 2D array-like
        - expect an array of (x,y) points for each scanner [ (x1,y1), (x2,y2), ... , (xn,yn) ]
    lopts : list of dict (default: None)
        - expect a list of dictionaries giving the line options for each scanner
    fopts : list of dict (default: None)
        - expect a list of dictionaries giving the fill options for each scanner
    axes_origin : tuple
        - Origin of the axes, as a fraction of the figure size.
          Needed to keep the space between the figure margin and the axis constant, for plots that need to appear side-by-side in papers.
    righttop_padding : tuple
        - Space to be left between the top and right axes and the margins of the figure, as a fraction of the figure size.
          Needed to keep the space between the figure margin and the axis constant, for plots that need to appear side-by-side in papers.
    """
    ax = ax or plt.gca()

    nscanners = len(scanpoints)
    fopts = get_fopts(nscanners, fopts, dim=1)
    lopts = get_lopts(nscanners, lopts, dim=1)

    # convert if angle
    if angle:
        scanpoints = [[np.degrees(x), y] for x, y in scanpoints]

    # do the fills first
    for i, (x, y) in enumerate(scanpoints):
        # plot the fill
        ax.fill_between(x, 0, y, **fopts[i])

    # overlay the lines
    for i, (x, y) in enumerate(scanpoints):
        # plot the line
        ax.plot(x, y, **lopts[i])

    # add legend
    leg_els = []
    if legtitles:
        for i, title in enumerate(legtitles):
            lopt = dict(**lopts[i])
            fopt = dict(**fopts[i])
            if "c" in lopt.keys():
                lopt["ec"] = lopt.pop("c")
            if "c" in fopt.keys():
                fopt["fc"] = fopt.pop("c")
            if "color" in lopt.keys():
                lopt["ec"] = lopt.pop("color")
            if "color" in fopt.keys():
                fopt["fc"] = fopt.pop("color")
            if "ec" in fopt.keys():
                fopt.pop("ec")
            if "fc" in lopt.keys():
                lopt.pop("fc")
            leg_opts = {**lopt, **fopt, **dict(label=title)}

            leg_els.append(patches.Patch(**leg_opts))

        # if 'prop' not in legopts:
        #     legopts['prop'] = {'family': 'Times New Roman'}
        ax.legend(handles=leg_els, **legopts)

    # style
    ax.set_ylim(0, 1)
    ax.autoscale(enable=True, axis="x", tight=True)

    # xaxis label
    if xtitle:
        ax.set_xlabel(xtitle)

    # yaxis label
    ax.set_ylabel("$p=1-CL$")

    # add logos
    if logo:
        addLHCbLogo(ax, prelim=prelim)

    if axes_origin and righttop_padding:
        ax.set_position(
            [
                axes_origin[0],
                axes_origin[1],
                1 - righttop_padding[0] - axes_origin[0],
                1 - righttop_padding[1] - axes_origin[1],
            ]
        )

    if save:
        fig = plt.gcf()
        fig.savefig(save)


def plot2d(
    scanpoints,
    lopts=[],
    fopts=[],
    mopts=[],
    title=[None, None],
    levels=1,
    legtitles=None,
    angle=[False, False],
    ax=None,
    save=None,
    bf=None,
    cl2d=False,
    logo=False,
    prelim=False,
    contourline=False,
    legopts={},
    axes_origin=(0.14, 0.16),
    righttop_padding=(0.04, 0.04),
):
    """parameters
    scanpoints : 2D array-like
        - expect an array of (x,y,z) points for each scanner [ (x1,y1,z1), (x2,y2,z2), ... , (xn,yn,zn) ]
    lopts : list of dict (default: None)
        - expect a list of dictionaries giving the line options for each scanner
    fopts : list of dict (default: None)
        - expect a list of dictionaries giving the fill options for each scanner
    title : list of str
        - must be two item list or tuple of [xtitle, ytitle]
    angle : list of bool
        - must be two item list or tuple of [xangle, yangle]
    legtitles : list of str
        - list of legend titles
    levels : int or list of int or list of float
        - if int then will plot this number of sigma contours at -2DLL=1,4,9,..,N**2
        - if list of int will plot this number of sigma contours as above
        - if list of float will plot this number containing the fraction
    axes_origin : tuple
        - Origin of the axes, as a fraction of the figure size.
          Needed to keep the space between the figure margin and the axis constant, for plots that need to appear side-by-side in papers.
    righttop_padding : tuple
        - Space to be left between the top and right axes and the margins of the figure, as a fraction of the figure size.
          Needed to keep the space between the figure margin and the axis constant, for plots that need to appear side-by-side in papers.
    """
    ax = plt.gca()

    # figure out the levels
    if type(levels) is int:
        levels = [lev + 1 for lev in range(levels)]
    if type(levels[0]) is int:
        levels = [lev**2 for lev in levels]
        if cl2d:
            levels = [chi2.ppf(chi2.cdf(lev, 1), 2) for lev in levels]
    else:
        levels = [chi2.ppf(lev, 2) for lev in levels]

    # # figure out frac contained
    # fc = [chi2.cdf(lev, 2) for lev in levels]

    nscanners = len(scanpoints)
    fopts = get_fopts(nscanners, fopts, dim=2)
    lopts = get_lopts(nscanners, lopts, dim=2)
    mopts = get_mopts(nscanners, mopts, dim=2)

    # convert if angle
    if angle[0]:
        scanpoints = [[np.degrees(x), y, z] for x, y, z in scanpoints]
        if bf is not None:
            for i, pt in enumerate(bf):
                if pt is not None:
                    bf[i] = [np.degrees(pt[0]), pt[1]]
    if angle[1]:
        scanpoints = [[x, np.degrees(y), z] for x, y, z in scanpoints]
        if bf is not None:
            for i, pt in enumerate(bf):
                if pt is not None:
                    bf[i] = [pt[0], np.degrees(pt[1])]

    # do the fills first
    for i, (x, y, z) in enumerate(scanpoints):
        # workaround to avoid that contours are drawn for points where the scanner failed to converge
        if np.any(z < 0):
            print(
                "WARNING: There are negative values in the likelihood scan. this implies that the scan failed to converge and the results could be unreliable"
            )
            z[z < 0] = 1e9

        # plot the fill
        ax.contourf(x, y, z, levels=[0] + levels, **fopts[i])

    # overlay the lines
    for i, (x, y, z) in enumerate(scanpoints):
        # plot the line
        ax.contour(x, y, z, levels=levels, **lopts[i])

    # add the best fit
    if bf is not None:
        for i, pt in enumerate(bf):
            if pt is not None and pt is not False:
                ax.plot(pt[0], pt[1], **mopts[i])

    # add legend
    leg_handles = []
    leg_labels = []
    if legtitles:
        for i, ltitle in enumerate(legtitles):
            lopt = dict(**lopts[i])
            fopt = dict(**fopts[i])
            mopt = dict(**mopts[i])
            if "c" in lopt.keys():
                lopt["ec"] = lopt.pop("c")
            if "color" in lopt.keys():
                lopt["ec"] = lopt.pop("color")
            if "colors" in lopt.keys():
                c = lopt.pop("colors")
                lopt["ec"] = c[0]
            if "linewidths" in lopt.keys():
                lw = lopt.pop("linewidths")
                if lw is not None:
                    lopt["linewidth"] = lw[0]
            if "linestyles" in lopt.keys():
                ls = lopt.pop("linestyles")
                if ls is not None:
                    lopt["linestyle"] = ls[0]
            if "c" in fopt.keys():
                fopt["fc"] = fopt.pop("c")
            if "color" in fopt.keys():
                fopt["fc"] = fopt.pop("color")
            if "colors" in fopt.keys():
                c = fopt.pop("colors")
                fopt["fc"] = c[0]

            if ltitle is not None:
                leg_labels.append(ltitle)
                leg_opts = {**lopt, **fopt}
                leg_handle = (
                    (patches.Patch(**leg_opts), Line2D([0], [0], lw=0, **mopt))
                    if mopt
                    else patches.Patch(**leg_opts)
                )
                leg_handles.append(leg_handle)

        # if 'prop' not in legopts:
        #     legopts['prop'] = {'family': 'Times New Roman'}
        ax.legend(leg_handles, leg_labels, **legopts)

    # style
    if title[0]:
        ax.set_xlabel(title[0])
    if title[1]:
        ax.set_ylabel(title[1])

    # add logos
    if logo:
        addLHCbLogo(ax, prelim=prelim)
    if contourline:
        addContoursLine(ax, nlevs=len(levels), cl2d=cl2d)

    if axes_origin and righttop_padding:
        ax.set_position(
            [
                axes_origin[0],
                axes_origin[1],
                1 - righttop_padding[0] - axes_origin[0],
                1 - righttop_padding[1] - axes_origin[1],
            ]
        )

    if save:
        fig = plt.gcf()
        fig.savefig(save)


def lhcb_logo(pos=[0.02, 0.88], prelim=False, date=None, ax=None):
    ax = ax or plt.gca()
    props = dict(fc="none", ec="none", boxstyle="square,pad=0.1")
    font = {"family": "Times New Roman", "weight": 400}
    ax.text(
        *pos,
        "LHCb",
        transform=ax.transAxes,
        size=28,
        ha="left",
        bbox=props,
        fontdict=font,
        usetex=False,
    )
    if prelim:
        ax.text(
            pos[0],
            pos[1] - 0.05,
            "Preliminary",
            transform=ax.transAxes,
            size=14.7,
            ha="left",
            bbox=props,
            fontdict=font,
            usetex=False,
        )
    if date is not None:
        ax.text(
            pos[0],
            pos[1] - 0.10,
            date,
            transform=ax.transAxes,
            size=12.2,
            ha="left",
            bbox=props,
            fontdict=font,
            usetex=False,
        )


def hflav_logo(subtitle, pos=[0.02, 0.98], ax=None, scale=1):
    ax = ax or plt.gca()

    xwidth = 0.22  # width of logo in units of axis
    yratio = 0.7  # relative height of y to xwidth (in axis units)
    ysub = 0.8  # relative size of subtitle to HFLAV title
    fontsize = 16  # font size of HFLAV bit
    fontsub = 0.7  # relative size of subtitle font
    font = {
        "family": "sans-serif",
        "style": "italic",
        "color": "white",
        "weight": 500,
        "size": scale * fontsize,
        "stretch": "condensed",
    }

    fraction = 1 - ysub / (1 + ysub)

    # black part with white HFLAV
    x = (pos[0], pos[0] + xwidth)
    y = (pos[1] - yratio * fraction * xwidth, pos[1])
    xc = (x[0] + x[1]) / 2
    yc = (y[0] + y[1]) / 2
    ax.fill(
        [x[0], x[1], x[1], x[0]],
        [y[0], y[0], y[1], y[1]],
        "k",
        ec="k",
        lw=0.5,
        transform=ax.transAxes,
        clip_on=False,
        zorder=100,
    )
    ax.text(
        xc,
        yc - 0.01,
        "HFLAV",
        fontdict=font,
        ha="center",
        va="center",
        transform=ax.transAxes,
        usetex=False,
        clip_on=False,
        zorder=110,
    )

    # white part with black subtitle
    y = (pos[1] - yratio * xwidth, pos[1] - yratio * fraction * xwidth)
    yc = (y[0] + y[1]) / 2
    font["color"] = "black"
    font["size"] *= fontsub
    ax.fill(
        [x[0], x[1], x[1], x[0]],
        [y[0], y[0], y[1], y[1]],
        "w",
        ec="k",
        lw=0.5,
        transform=ax.transAxes,
        clip_on=False,
        zorder=120,
    )
    ax.text(
        xc,
        yc - 0.005,
        subtitle,
        fontdict=font,
        ha="center",
        va="center",
        transform=ax.transAxes,
        usetex=False,
        clip_on=False,
        zorder=130,
    )


def corr_plot(df, savef=None, names=None):
    # symmetrise
    if names == "columns":
        names = df.columns.values

    corr = df.values

    for (j, i), value in np.ndenumerate(corr):
        if j > i:
            corr[j, i] = corr[i, j]

    scale = len(names) / 12
    fig, ax = plt.subplots(figsize=(scale * 6.4, scale * 4.8))
    im = ax.imshow(
        corr,
        cmap="coolwarm",
        interpolation="none",
        aspect="auto",
        vmin=-1,
        vmax=1,
        rasterized=True,
    )
    cb = fig.colorbar(
        im,
        ax=ax,
        format=lambda x, pos: f"${x:+3.1f}$",
        pad=0.03 / scale,
        aspect=15 * scale,
    )
    font_size = rcParams["font.size"] * scale
    cb.set_label("Correlation", size=font_size)

    for (j, i), value in np.ndenumerate(corr):
        if not np.isnan(value) and abs(value) >= 0.01:
            ax.text(i, j, f"${value:+4.2f}$", ha="center", va="center", fontsize=8)

    if names is not None:
        labels = [
            name.replace(r"\,[^\circ]", "").replace(r"\, [\%]", "") for name in names
        ]
        ax.set_xticks(np.arange(len(labels)))
        ax.set_xticklabels(labels)
        ax.set_yticks(np.arange(len(labels)))
        ax.set_yticklabels(labels)
        ax.tick_params(axis="x", labelrotation=45)
        ax.tick_params(direction="out")
        ax.minorticks_off()

    if savef is not None:
        fig.savefig(savef)
        if ".pdf" in savef:
            for ext in [".png", ".ps"]:
                fig.savefig(savef.replace(".pdf", ext))


class plotter:
    def __init__(
        self,
        dim=1,
        save=None,
        xtitle=None,
        ytitle=None,
        xangle=False,
        yangle=False,
        xrange=None,
        yrange=None,
        logo="l",
        legpos=None,
        legfill=False,
        cls=None,
    ):
        self.dim = dim
        self.save = save
        self.xtitle = xtitle
        self.ytitle = ytitle
        self.xangle = xangle
        self.yangle = yangle
        self.xrange = xrange
        self.yrange = yrange
        self.logo = logo
        self.legpos = legpos
        self.legfill = legfill
        self.cls = cls

        self.scanpoints = []
        self.bfs = []
        self.legtitles = []
        self.lopts = []
        self.fopts = []
        self.mopts = []

    def add_scan(self, scanname, pars, label, bf=False, col=None, hatch=None, lw=None):
        if scanname is not None:
            print("FIXME: args should not visible here")
            # x, y, z, pt = get_scan_res(f"{args.prefix}{scanname}", *pars)
            x = y = z = np.empty((2, 2)) * np.nan  # tricks a fake
            pt = None
        else:
            x = y = z = np.empty((2, 2)) * np.nan  # tricks a fake
            pt = None

        # best fit point
        if bf:
            self.bfs.append(pt)
        else:
            self.bfs.append(None)

        # scan points
        if self.dim == 1:
            self.scanpoints.append([x, y])
        elif self.dim == 2:
            self.scanpoints.append([x, y, z])

        # label
        self.legtitles.append(label)

        # 1d opts
        if self.dim == 1:
            self.lopts.append(dict(c=hflav_cols[col], lw=lw))
            self.fopts.append(dict(ec=hflav_cols[col], fc="none", hatch=hatch))

        # 2d opts
        # elif self.dim==2:
        # self.mopts.append(

    def plot(self):
        legopts = {}

        if self.logo == "l" and self.legpos == "l":
            legopts = dict(
                bbox_to_anchor=(
                    0,
                    0.83 - 0.07 * len(self.scanpoints),
                    0.2,
                    0.07 * len(self.scanpoints),
                ),
                loc="upper left",
            )
        elif self.logo == "r" and self.legpos == "r":
            legopts = dict(
                bbox_to_anchor=(
                    0.8,
                    0.83 - 0.07 * len(self.scanpoints),
                    0.2,
                    0.07 * len(self.scanpoints),
                ),
                loc="upper right",
                fontsize=12,
            )
        elif self.logo == "l" and self.legpos == "r":
            legopts = dict(
                bbox_to_anchor=(
                    0.8,
                    1 - 0.07 * len(self.scanpoints),
                    0.2,
                    0.07 * len(self.scanpoints),
                ),
                loc="upper right",
                fontsize=12,
            )
        elif self.logo == "r" and self.legpos == "l":
            legopts = dict(
                bbox_to_anchor=(
                    0,
                    1 - 0.07 * len(self.scanpoints),
                    0.2,
                    0.07 * len(self.scanpoints),
                ),
                loc="upper left",
                fontsize=12,
            )
        elif self.legpos == "bl":
            legopts = dict(
                bbox_to_anchor=(0, 0, 0.2, 0.07 * len(self.scanpoints)),
                loc="lower left",
                fontsize=12,
            )

        if self.legfill:
            legopts["frameon"] = True
            legopts["framealpha"] = 0.8
            legopts["facecolor"] = "w"
            legopts["edgecolor"] = "0.7"
            legopts["borderpad"] = 0.3

        fig, ax = plt.subplots()
        if self.dim == 1:
            plot1d(
                self.scanpoints,
                self.lopts,
                self.fopts,
                xtitle=self.xtitle,
                legtitles=self.legtitles,
                angle=self.xangle,
                ax=ax,
                legopts=legopts,
            )
        elif self.dim == 2:
            plot2d(
                self.scanpoints,
                self.lopts,
                self.fopts,
                self.mopts,
                title=[self.xtitle, self.ytitle],
                levels=2,
                legtitles=self.legtitles,
                angle=[self.xangle, self.yangle],
                ax=ax,
                bf=self.bfs,
                cl2d=True,
                legopts=legopts,
            )

        if self.logo == "l":
            hflav_logo("Moriond 2024", ax=ax)
        else:
            hflav_logo("Moriond 2024", ax=ax, pos=[0.76, 0.98])

        # limits
        if self.xrange is not None:
            ax.set_xlim(*self.xrange)
        if self.yrange is not None:
            ax.set_ylim(*self.yrange)

        # axis labels
        ax.set_xlabel(ax.get_xlabel(), loc="right")
        if self.dim == 1:
            ax.set_ylabel("$1-CL$", loc="top")
        elif self.dim == 2:
            ax.set_ylabel(ax.get_ylabel(), loc="top")

        # CL lines
        if self.cls is not None:
            ax.axhline(chi2.sf(1, 1), c="k", ls=":", lw=1)
            ax.axhline(chi2.sf(4, 1), c="k", ls=":", lw=1)
            if self.cls == "l":
                ax.text(
                    0.02,
                    0.32,
                    r"$68.3\%$",
                    ha="left",
                    va="bottom",
                    transform=ax.transAxes,
                    fontsize=16,
                )
                ax.text(
                    0.02,
                    0.05,
                    r"$95.4\%$",
                    ha="left",
                    va="bottom",
                    transform=ax.transAxes,
                    fontsize=16,
                )
            elif self.cls == "r":
                ax.text(
                    0.98,
                    0.32,
                    r"$68.3\%$",
                    ha="right",
                    va="bottom",
                    transform=ax.transAxes,
                    fontsize=16,
                )
                ax.text(
                    0.98,
                    0.05,
                    r"$95.4\%$",
                    ha="right",
                    va="bottom",
                    transform=ax.transAxes,
                    fontsize=16,
                )

        fig.savefig(self.save)
        fig.savefig(self.save.replace("pdf", "png"))

        # if not args.interactive:
        #     fig.clf()
