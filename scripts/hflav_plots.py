"""
HFLAV-style matplotlib plots of GammaCombo scans (1-CL curves and 2D contours) with the
official HFLAV logo carrying the Zenodo DOI.

Shared by the HFLAV CKM-angle plots; the plots themselves (which scans, colours, ranges) are
defined in hflav_averages.py (PLOTS) and made with `python scripts/hflav_averages.py plots`.

The logo comes from logo.py of the HFLAV shared-material repository
(https://gitlab.cern.ch/hflav/shared/shared-material), found in $HFLAV_SHARED_MATERIAL or
~/Scratch/hflav/shared-material.
"""

import importlib.util
import os
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import Arc
from scipy.stats import chi2

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from gc_mpl_tools import (  # noqa: E402
    hflav_cols,
    plot1d,
    plot2d,
    print_interval,
    read_gc_scan,
)

plt.style.use(str(HERE / "gc.mplstyle"))

# ---------------------------------------------------------------------------
# Official HFLAV logo with the Zenodo DOI
# ---------------------------------------------------------------------------

SHARED_MATERIAL = Path(
    os.environ.get("HFLAV_SHARED_MATERIAL", "~/Scratch/hflav/shared-material")
).expanduser()

# physical size of the official HFLAV logo (shared-material logo.py defaults on an 8x6 in figure)
HFLAV_LOGO_SIZE_IN = (0.24 * 8, 0.18 * 6)

_draw_hflav_logo = None


def draw_hflav_logo_func():
    """drawHFLAVlogo from shared-material/logo.py (loaded once)."""
    global _draw_hflav_logo
    if _draw_hflav_logo is None:
        path = SHARED_MATERIAL / "logo.py"
        if not path.exists():
            raise FileNotFoundError(
                f"{path} not found: clone https://gitlab.cern.ch/hflav/shared/shared-material "
                "there or set $HFLAV_SHARED_MATERIAL"
            )
        spec = importlib.util.spec_from_file_location("hflav_logo", path)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        _draw_hflav_logo = module.drawHFLAVlogo
    return _draw_hflav_logo


def hflav_zenodo_logo(
    fig, ax, version, year, topRight=None, topLeft=None, size_in=HFLAV_LOGO_SIZE_IN
):
    """
    Official HFLAV logo with the Zenodo DOI (shared-material logo.py), drawn
    - at its official physical size: dx/dy (axes fractions) from the final axes size, so call it
      after everything else has been drawn (constrained layout moves the axes);
    - without LaTeX and in DejaVu Sans: gc.mplstyle sets text.usetex=True, which would typeset the
      logo text in Computer Modern (the rest of the plot keeps LaTeX);
    - opaque, on top of everything else in the axes (hatching, curves, contours, legend).
    Save with save_figure() so the logo font is embedded as TrueType (needed for the DOI rewriting).
    """
    fig.canvas.draw()  # let constrained layout fix the axes position
    pos = ax.get_position()
    fig_w, fig_h = fig.get_size_inches()
    dx = size_in[0] / (pos.width * fig_w)
    dy = size_in[1] / (pos.height * fig_h)
    if topRight is None and topLeft is None:
        topRight = (0.98, 0.98)
    n_patches, n_texts = len(ax.patches), len(ax.texts)
    plt.sca(ax)  # logo.py draws its text with plt.text
    with plt.rc_context({"text.usetex": False, "font.family": ["DejaVu Sans"]}):
        draw_hflav_logo_func()(
            fig,
            ax,
            topRight=topRight,
            topLeft=topLeft,
            thisVersion=version,
            thisYear=year,
            dx=dx,
            dy=dy,
        )
    for artist in ax.patches[n_patches:] + ax.texts[n_texts:]:
        artist.set_zorder(artist.get_zorder() + 10)


def save_figure(fig, path, **kwargs):
    """savefig with TrueType (Type 42) fonts, as required for the Zenodo DOI rewriting."""
    with plt.rc_context({"pdf.fonttype": 42, "ps.fonttype": 42}):
        fig.savefig(path, **kwargs)


# ---------------------------------------------------------------------------
# Plotter
# ---------------------------------------------------------------------------


def _rgb(r, g, b, a=None):
    return (
        (r / 255.0, g / 255.0, b / 255.0)
        if a is None
        else (r / 255.0, g / 255.0, b / 255.0, a)
    )


# 2D contours: line, fill (68%, 95%) and best-fit marker options per colour
CONTOUR_STYLES = {
    "r": dict(
        lopts=dict(colors=[_rgb(208, 53, 43)], linewidths=[1]),
        fopts=dict(colors=[_rgb(208, 53, 43, 0.4), _rgb(208, 53, 43, 0.2)]),
        mopts={},
    ),
    "b": dict(
        lopts=dict(colors=[_rgb(116, 112, 174)], linewidths=[1]),
        fopts=dict(colors=[_rgb(116, 112, 174, 0.4), _rgb(116, 112, 174, 0.2)]),
        mopts={},
    ),
    "g": dict(
        lopts=dict(colors=[_rgb(75, 156, 122)], linewidths=[1]),
        fopts=dict(colors=[_rgb(75, 156, 122, 0.4), _rgb(75, 156, 122, 0.2)]),
        mopts={},
    ),
    "k": dict(
        lopts=dict(colors="k", linewidths=[1]),
        fopts=dict(colors=[(0, 0, 0, 0.2), (0, 0, 0, 0.1)]),
        mopts=dict(color="k", marker="o", ms=5),
    ),
}


class Plotter:
    """
    One HFLAV plot: add the scans with add_scan(), then plot().

    dim        : 1 (1-CL vs one parameter) or 2 (contours in two parameters)
    save       : output PDF (a PNG is written next to it)
    xtitle, ytitle, xangle, yangle : axis titles; *angle: the parameter is an angle (shown in degrees)
    xrange, yrange : axis ranges
    logo       : 'l' (top left) or 'r' (top right) inside the axes
    legpos     : 'l', 'r' (top left/right; below the logo when in the same corner), 'bl' (bottom
                 left), 'ol' (above the axes, two columns) or None
    legfill    : legend on a white, semi-transparent box
    legfontsize: legend font size
    cls        : 'l' or 'r': dotted 68.3%/95.4% CL lines labelled on that side (1D)
    levels     : number of 2D contours
    cl_text    : (x, y) in axes fractions of the "contours hold ..." note (2D), None for none
    tight      : save with bbox_inches='tight'
    extras     : functions f(ax, plotter) drawing plot-specific content after the CL lines/note
    version, year : release shown in the logo (e.g. "Autumn", "2026")
    """

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
        legfontsize=12,
        cls=None,
        levels=2,
        cl_text=(0.03, 0.03),
        tight=False,
        extras=(),
        version="Month",
        year="YEAR",
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
        self.legfontsize = legfontsize
        self.cls = cls
        self.levels = levels
        self.cl_text = cl_text
        self.tight = tight
        self.extras = list(extras)
        self.version = version
        self.year = year

        self.scanpoints = []
        self.bfs = []
        self.legtitles = []
        self.lopts = []
        self.fopts = []
        self.mopts = []

    def add_scan(self, scan, label=None, bf=False, col=None, hatch=None, lw=None):
        """
        scan : (scanner ROOT file, parameter file, [scan variables]) or None for an empty entry
        (keeps the colour/legend order when a scan does not exist).
        """
        if scan is not None:
            x, y, z, pt = read_gc_scan(str(scan[0]), str(scan[1]), list(scan[2]))
        else:
            x = y = z = np.empty((2, 2)) * np.nan
            pt = None

        self.bfs.append(pt if bf else None)
        self.scanpoints.append([x, y] if self.dim == 1 else [x, y, z])
        self.legtitles.append(label)

        if self.dim == 1:
            self.lopts.append(dict(c=hflav_cols[col], lw=lw))
            self.fopts.append(dict(ec=hflav_cols[col], fc="none", hatch=hatch))
        else:
            style = CONTOUR_STYLES[col]
            self.lopts.append(dict(style["lopts"]))
            self.fopts.append(dict(style["fopts"]))
            self.mopts.append(dict(style["mopts"]))

    def legend_options(self):
        n = len(self.scanpoints)
        h = 0.07 * n
        fs = dict(fontsize=self.legfontsize)
        if (
            self.legpos == self.logo == "l"
        ):  # same corner as the logo: start below the logo
            legopts = dict(bbox_to_anchor=(0, 0.68 - h, 0.2, h), loc="upper left", **fs)
        elif self.legpos == self.logo == "r":
            legopts = dict(
                bbox_to_anchor=(0.8, 0.68 - h, 0.2, h), loc="upper right", **fs
            )
        elif self.legpos == "r":
            legopts = dict(bbox_to_anchor=(0.8, 1 - h, 0.2, h), loc="upper right", **fs)
        elif self.legpos == "l":
            legopts = dict(bbox_to_anchor=(0, 1 - h, 0.2, h), loc="upper left", **fs)
        elif self.legpos == "bl":
            legopts = dict(bbox_to_anchor=(0, 0, 0.2, h), loc="lower left", **fs)
        elif self.legpos == "ol":
            legopts = dict(
                bbox_to_anchor=(0, 1.02, 0.2, h / 2), loc="lower left", ncols=2, **fs
            )
        else:
            legopts = {}

        # right-hand 68.3%/95.4% labels: move a right-hand legend left so it does not cover them
        if self.cls == "r" and legopts.get("loc") == "upper right":
            x0, y0, w, h = legopts["bbox_to_anchor"]
            legopts["bbox_to_anchor"] = (x0 - 0.07, y0, w, h)

        if self.legfill:
            legopts.update(
                frameon=True,
                framealpha=0.8,
                facecolor="w",
                edgecolor="0.7",
                borderpad=0.3,
            )
        return legopts

    def plot(self):
        """Draw the plot and save it as PDF and PNG."""
        fig, ax = plt.subplots()
        try:
            self._draw(fig, ax)
        finally:
            plt.close(fig)

    def _draw(self, fig, ax):
        legopts = self.legend_options()
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
        else:
            plot2d(
                self.scanpoints,
                self.lopts,
                self.fopts,
                self.mopts,
                title=[self.xtitle, self.ytitle],
                levels=self.levels,
                legtitles=self.legtitles,
                angle=[self.xangle, self.yangle],
                ax=ax,
                bf=self.bfs,
                cl2d=True,
                legopts=legopts,
            )

        # limits
        if self.xrange is not None:
            ax.set_xlim(*self.xrange)
        if self.yrange is not None:
            ax.set_ylim(*self.yrange)

        # axis labels
        ax.set_xlabel(ax.get_xlabel(), loc="right")
        ax.set_ylabel("$1-CL$" if self.dim == 1 else ax.get_ylabel(), loc="top")

        # CL lines
        if self.cls is not None:
            ax.axhline(chi2.sf(1, 1), c="k", ls=":", lw=1)
            ax.axhline(chi2.sf(4, 1), c="k", ls=":", lw=1)
            x, ha = (0.02, "left") if self.cls == "l" else (0.98, "right")
            ax.text(
                x,
                0.32,
                r"$68.3\%$",
                ha=ha,
                va="bottom",
                transform=ax.transAxes,
                fontsize=16,
            )
            ax.text(
                x,
                0.05,
                r"$95.4\%$",
                ha=ha,
                va="bottom",
                transform=ax.transAxes,
                fontsize=16,
            )

        # what the 2D contours hold
        if self.dim == 2 and self.cl_text is not None:
            clstr = rf"contours hold {chi2.cdf(1, 1) * 100:4.1f}\%, {100 * chi2.cdf(4, 1):4.1f}\% CL"
            ax.text(
                *self.cl_text, clstr, color="0.4", fontsize=14, transform=ax.transAxes
            )

        for extra in self.extras:
            extra(ax, self)

        # logo last: it is sized from the final axes and drawn on top of everything
        if self.logo == "l":
            hflav_zenodo_logo(
                fig, ax, version=self.version, year=self.year, topLeft=(0.02, 0.98)
            )
        else:
            hflav_zenodo_logo(
                fig, ax, version=self.version, year=self.year, topRight=(0.98, 0.98)
            )

        kwargs = dict(bbox_inches="tight") if self.tight else {}
        save_figure(fig, self.save, **kwargs)
        save_figure(fig, str(self.save).replace(".pdf", ".png"), **kwargs)


def print_cl_interval(clfile):
    """Print the CL intervals of a scan (GammaCombo plots/cl/clintervals_*.py), if the file exists."""
    if os.path.exists(clfile):
        try:
            print_interval(str(clfile), nsigma=1)
        except Exception:
            pass


# ---------------------------------------------------------------------------
# Plot-specific content
# ---------------------------------------------------------------------------

RED = _rgb(208, 53, 43)
BLUE = _rgb(116, 112, 174)


def fmt_value(central, uncertainty, ndigits=2):
    """'22.61^{+0.44}_{-0.43}' (or '22.61 \\pm 0.43') for an HFLAV.json value."""
    if isinstance(uncertainty, (list, tuple)):
        lo, hi = abs(uncertainty[0]), abs(uncertainty[1])
    else:
        lo = hi = abs(uncertainty)
    if f"{lo:.{ndigits}f}" == f"{hi:.{ndigits}f}":
        return rf"{central:.{ndigits}f} \pm {hi:.{ndigits}f}"
    return rf"{central:.{ndigits}f}^{{+{hi:.{ndigits}f}}}_{{-{lo:.{ndigits}f}}}"


def beta_decorations(beta, beta_err):
    """
    Content of the beta overview plot in the (rhobar, etabar) plane: the axes through the origin,
    the arcs and values of the two solutions beta and 90 deg - beta (beta in degrees, beta_err its
    uncertainty as in HFLAV.json: number or [-low, +high]), and the region excluded by B0 -> D h0.
    """
    lo, hi = (
        (abs(beta_err[0]), abs(beta_err[1]))
        if isinstance(beta_err, (list, tuple))
        else (abs(beta_err),) * 2
    )
    beta2 = 90.0 - beta  # second solution: errors swap

    def draw(ax, plotter):
        ax.axvline(0, c="k", ls="--", lw=1, zorder=0)
        ax.axhline(0, c="k", ls="--", lw=1, zorder=1)

        ax.add_patch(
            Arc(
                xy=(1, 0),
                width=0.58,
                height=0.58 * 1.1,
                theta1=180 - beta,
                theta2=180,
                color="k",
                lw=1.5,
            )
        )
        ax.add_patch(
            Arc(
                xy=(1, 0),
                width=0.38,
                height=0.38 * 1.4,
                theta1=180 - beta2,
                theta2=180,
                color="k",
                lw=1.5,
            )
        )

        ax.text(
            0.4,
            0.32,
            rf"$\beta\equiv\phi_1 = ({fmt_value(beta, [-lo, hi])})^\circ$",
            fontsize=14,
            c=RED,
            rotation=-18,
            va="center",
            ha="center",
        )
        ax.text(
            0.68,
            0.62,
            rf"$\beta\equiv\phi_1 = ({fmt_value(beta2, [-hi, lo])})^\circ$",
            fontsize=14,
            c=RED,
            rotation=-59,
            va="center",
            ha="center",
        )

        # region excluded by B0 -> D h0 (cos 2beta > 0)
        y = np.linspace(*plotter.yrange, 200)
        xlow = -y + 1
        xup = np.ones_like(y)
        ax.fill_betweenx(y, xlow, xup, facecolor="none", edgecolor=BLUE, hatch="xx")
        ax.plot(xlow, y, c="k")
        ax.plot(xup, y, c="k")
        ax.text(
            1.1,
            0.5,
            r"EXCLUDED BY $Dh^0$",
            fontsize=14,
            c=BLUE,
            rotation=-90,
            va="center",
            ha="center",
        )
        ax.text(
            1.04,
            0.5,
            r"(also by $J/\psi K^{*0}$ \& $D^{*+}D^{*-}K_S^{0}$)",
            fontsize=14,
            c=BLUE,
            rotation=-90,
            va="center",
            ha="center",
        )

    return draw
