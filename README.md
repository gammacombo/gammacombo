# GammaCombo

GammaCombo is a framework to combine measurements in order to compute
confidence intervals on parameters of interest. A global likelihood function is
constructed from the probability densitiy functions of the input observables,
which is used to derive likelihood-based intervals and frequentist intervals
based on pseudoexperiments following the Plugin method to handle nuisance
parameters.

You can find information about the package at our website
[gammacombo.github.io](https://gammacombo.github.io).
This also includes a quick start guide, some tutorials and the details on
reproducing results.

## Manual

A (not up-to-date) manual is provided in
[GammaComboManual.pdf](https://gammacombo.github.io/manual.pdf)

## Installation

Installing `GammaCombo` is straightforward if you have a working installation of
[CVMFS](https://cernvm.cern.ch/fs/) and your computer OS is `RHEL 8`, `RHEL 9`
or `AlmaLinux 9`:

    git clone https://github.com/gammacombo/gammacombo.git
    cd gammacombo
    source scripts/setup-env-cvmfs.sh
    cmake -B <build-dir>
    cmake --build <build-dir> [-j <n-cores>]
    cmake --install <build-dir>

Then, you will need to source `scripts/setup-env-cvmfs.sh` in each new shell
session before running any executables.
If `CVMFS` is not available or your OS is not supported,
`scripts/setup-env-cvmfs.sh` will not work and you will have to setup a suitable
working environment yourself (requires `CMake` 3.19 or higher, `ROOT` higher
than v6.18 but below v34, `Boost`, and a compiler supporting the `C++17`
standard).

To build the Doxygen documentation, which will create an HTML class documentation in
the doc/html subdirectory, do:

    cd <build-dir>
    make doc

Follow the tutorial in the [manual](http://gammacombo.hepforge.org/web/HTML/GammaComboManual.pdf):

    cd ../tutorial
    bin/tutorial
