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

To improve performance of computing over typical parameters' values the
computing core uses a simple caching system based on forest of red-black trees.
Every element of these trees describes the result of SWC computing in some point
(corresponding to some set of parameter's values) and has a kind of compound key
made of values of the parameters and the variables in this point. Such keys can
be compared lexicographically.

### User Interface

Special functions computed as solutions to systems of ODEs are shown as contour
plots. These plots are drawn with an algorithm similar to "marching squares",
but uses not only lines, but also filling.

## Building

First of all the external dependencies must be satisfied:

	$ git submodule init
	$ git submodule update

These commands receive some known versions of mongoose, sigie and jsmn packages.

	$ make

This command will make all the project. Know that the code depends also on
libmath, libpthread and libdl, but all of them are standard for any Linux-based
system. They are also typically installed in the base of cygwin.

## Running

There are three possibilities to run the computing core.

### SCGI mode

Most of the functions are supported in SCGI mode. To run the system in this mode
you also need any web-server supporting SCGI protocol. The author uses Nginx for
tests.

Modify configuration of your Nginx:

	$ sudo vim /etc/nginx/nginx.conf

Add the following text in your "server" section:

	location ~ \wavecat.exe {
		include scgi_params;
		scgi_pass 127.0.0.1:8000;
	}

Such modification of the configuration file points the server to locally started
computing core.

Start previously built computing core:

	$ ./wavecat.exe --scgi

Copy contents of the "web" subdirectory of the project tree to "htdocs" of the
web server. The author uses default configuration:

	$ cp web/* /var/www/localhost/htdocs/ 

Point your browser to http://localhost

### CGI mode

Old mode that lacks some features, but good for fast testing. In such case you
don't need to configure any server, but WaveCat should be rebuilt with other
options. Be careful, without proper configuration you are not protected from
performance degradation.

Open the file "include/kernel/core/config.h"

	$ vim include/kernel/core/config.h

Comment out the line with definition of CONFIG\_CACHE\_RESULT.

	/* Define the macro to perform parallel computation */
	#define CONFIG_PARALLEL_COMP
	/* Define the macro to perform profiling */
	#define CONFIG_PROFILING
	/* Define the macro to perform result caching */
	//#define CONFIG_CACHE_RESULT

Start the system:

	$ make test

Execution of this command starts a local HTTP server on port 8080. Just open
your favorite WEB-browser and go to http://localhost:8080.

That is all, use the interface to compute SWCs and see the results.

	$ make stoptest

This command stops the server.

### Plain Mode

This mode was the first implemented one. In this mode you can run WaveCat as a
standalone application and pass it a string as a parameter. The string should
be correct WaveCat job description (JSON object). The mode is also not perfect
with new features requiring permanent execution of WaveCat (as, for example,
caching), so you have to configure the build as it is done in the previous
chapter about CGI mode.
