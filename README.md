GammaCombo
==========

GammaCombo is a framework to combine measurements in order to compute
confidence intervals on parameters of interest. A global likelihood function is
constructed from the probability densitiy functions of the input observables,
which is used to derive likelihood-based intervals and frequentist intervals
based on pseudoexperiments following the Plugin method to handle nuisance
parameters.

Manual
======

A manual is provided in
* [GammaComboManual.pdf](https://github.com/mkarbach/gammacombo/blob/master/manual/GammaComboManual.pdf)

Installation
============

To build GammaCombo cmake is needed in version 3.0 or higher.

Get the source code from GitHub:

    git clone https://github.com/mkarbach/gammacombo.git
    cd gammacombo

To create the library, create a build directory and change into it. E.g.

    mkdir build; cd build
 
Now create the Makefiles:

    cmake ..

And build:

    make

To install the libraries and copy all header files into the directories lib/ and
include/ in the root directory of the project:

    make install
 
To build the Doxygen documentation, which will create an HTML class documentation in
the doc/html subdirectory, do:

    make doc

Follow the tutorial in the [manual](https://github.com/mkarbach/gammacombo/blob/master/manual/GammaComboManual.pdf):

    cd tutorial
    bin/tutorial

