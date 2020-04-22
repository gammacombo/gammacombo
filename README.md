GammaCombo
==========

GammaCombo is a framework to combine measurements in order to compute
confidence intervals on parameters of interest. A global likelihood function is
constructed from the probability densitiy functions of the input observables,
which is used to derive likelihood-based intervals and frequentist intervals
based on pseudoexperiments following the Plugin method to handle nuisance
parameters.

WebLink
======

You can find information about the package at our website
* [gammacombo.github.io](https://gammacombo.github.io)

This also includes a quick start guide, some tutorials and the details on reproducing results


Manual
======

A manual is provided in
* [GammaComboManual.pdf](https://gammacombo.github.io/manual.pdf)

Installation
============

To build GammaCombo cmake is needed in version 3.0 or higher.

Get the source code from GitHub:

    git clone https://github.com/gammacombo/gammacombo.git
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

Follow the tutorial in the [manual](https://gammacombo.github.io/manual.pdf):

    cd ../tutorial
    bin/tutorial

Pushing Changes
============

We prefer you to use the "push from fork" method for committing your changes.
 * Please make a fork of the gammacombo repository. 
 * Please always make a Pull Request from your fork into the `development` branch
 
From your local `gammacombo` directory (cloned from here above) you can do the following:

```bash
git remote add <username> https://github.com/<username>/gammacombo.git
git checkout development
## make whatever changes ##
git add ..
git commit ..
git push <username> development
## then use the web interface to make a pull request
```

Continous Integration
============

Some continuous integration (CI) checks will run automatically when you make a pull request or push directly to `gammacombo/master` or `gammacombo/development` (in practise pushing directly to these location is restricted for most users).

An advanced part of the CI can upload the results of the test to the web at (https://gammacombo.github.io/ci) but in order to do this you will need to setup the appropriate access keys between your fork of `<username>/gammacombo` (which is where the CI test will run) and `gammacombo/gammacombo.github.io` which is the repository for the website. This feature won't run by default but if you want to enable it you can do the following:

 * Generate an ssh key on your local machine and convert it to PEM format. Do not enter any password otherwise the CI action will fail (in general this is a bad idea but as this will be stored in your github Secrets it should be relatively safe).
 ``` bash
 ssh-keygen -t ed25519 -a 100 -f mynewkey
 ssh-keygen -p -f mynewkey -m pem
```

* Put the private part of the key in your fork of `gammacombo`'s "Secrets"
  * Go to `https://github.com/<username>/gammacombo`
  * Go to the "Settings" tab
  * Click on "Secrets"
  * Click "Add a new secret"
  * Ensure the secret name is exactly "SSH_PRIVATE_KEY"
  * Add the private part of the key generated above into the "Value" box. This should start with `-----BEGIN ... PRIVATE KEY-----` have many lines and end with `-----END ... PRIVATE KEY-----`
   
 * Put the public part of the key in `gammacombo/gammacombo.github.io`'s "Deploy Keys"
   * You will need to be an admin to do this. If you are not then please contact us to have your key added.
   * Go to https://github.com/gammacombo/gammacombo.github.io 
   * Go to the "Settings" tab
   * Click on "Deploy Keys" and then `Add deploy key`
   
 * Finally tell the CI system to add your `<username>` to the list of users that will execute this action
   * As part of your commit edit the file `.github/workflows/push_users.yml` to include your `username`
