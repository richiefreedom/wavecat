# Wavecat - Parallel Numeric Tool for Computing Special Functions of Wave Catastrophes (SWC)

## Introduction

This package is a numerical system for computation of SWC - special functions
used in uniform asymptotic theory of wave catastrophes. This section of science
studies catastrophes - special critical points (singularities? I'm not O.K.
with English mathematical terminology) found in wave physical processes: waves
focusing, dispersion, diffraction, etc). All these special cases are classified
and have typical special functions that can be used in uniform asymptotics to
get a solution of a physical problem at, for example, caustic, where all other
ray solutions are impossible (go to infinity).

## Structure

The system is made of three parts:

* Computational core written in plain C with minimum of external dependencies;
* User interface written in HTML and JavaScript;
* Web-server (thirdparty) to connect two previous components together.

## Internal Algorithms

### Core

Direct computation of SWCs is almost impossible because they are represented as
multiple oscillating integrals. Thus, another method is used. All the integrals
are converted to systems of PDEs and than, such systems are translated to ODEs
(computation of SWCs by ODEs method developed by Andrew Kryukovsky).

It is known that if ODEs are known for some problem it can be claimed as
resolved. Of course, we have a theorem about uniqueness of ODE solution and a
number of simple methods for numerical solution of ODEs and systems of ODEs.
Many methods are available: Runge-Kutta (4-th order method), Kutta-Merson,
etc.

Thus, all special functions are represented as systems of ODEs, these ODEs have
a number of parameters that can be bound to values in limited ranges. Almost
all problems require to compute some SWC in a wide range of parameters' values
and computing of SWC in some point is independent from others, so it is
possible to make parallel solution.

### User Interface

Special functions computed as solutions to systems of ODEs are shown as contour
plots. These plots are drawn with an algorithm similar to "marching squares",
but uses not only lines, but also filling.

## Building

First of all the external dependencies must be satisfied:

	$ git submodule init
	$ git submodule update

These commands receive some known versions of mongoose and jsmn packages.

	$ make

This command will make all the project. Know that the code depends also on
libmath, libpthread and libdl, but all of them are standard for any Linux-based
system. They are also typically installed in the base of cygwin.

	$ make test

Execution of this command starts a local HTTP server on port 8080. Just open
your favorite WEB-browser and go to http://localhost:8080.

That is all, use the interface to compute SWCs and see the results.

	$ make stoptest

This command stops the server.
