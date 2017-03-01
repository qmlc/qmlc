Qml Compiler
------------

The Qml Compiler can be used to convert Qml source code files into
precompiled Qml files. The precompiled Qml files are faster to load
and do not expose the source code. Normally, the Qt either compiles
the Qml files in the startup or interprets the Qml files runtime.

The Qml uses the JIT compiler shipped with Qt to create precompiled
files. It works thus in same way, as Qt when it is using JIT. Only
that the JIT compilation step is performed already when the program
launches. The loader is used to load precompiled files and link
them into the Qml Engine.

The compiler is created by creating a new version of the Qml
compilation framework, that is builtin in the Qt. The class
QQmlTypeCompiler, that is the core class handling the compilation,
is taken as a base for the compiler. Some of its dependencies
are exported on the Qt, some of the dependencies are rewritten, due
to need for modifications.

The compiler, loader and examples are all based on Qt source code.
They are licensed under LGPL 2.1 (with Digia Qt LGPL exception 1.1)
or BSD license. See source code files for license. Collectively this
component can be used under terms of LGPL 2.1 with Digia Qt LGPL
exception 1.1. See LICENSE.LGPL and LGPL_EXCEPTION.txt for details.

Please read the whole document from top to bottom to get a good overview of how
things work.

Working architectures
---------------------

* X86(32/64 bit)
* ARM Thumb2

Building the Qml Compiler/Loader
--------------------------------

<h2>Introduction</h2>

At the moment to use the compiler one has to build it first. For this one first
need to build the qt source distribution with the qtdeclarative directory
changed. After that qmlc can be build.

When the building is completed one can compile Qml and Javascript files with the
compiler and load and run with the compiled files.

<h2>Build requirements</h2>

Tested with Ubuntu 13.10 but will probably work fine with any Linux.

<h2>Building</h2>

<h3>Building Qt and QtDeclarative</h3>

* Download Qt source package,

<pre>wget http://download.qt-project.org/archive/qt/5.3/5.3.1/single/qt-everywhere-opensource-src-5.3.1.tar.gz</pre>

* Extract the archive and enter directory,

<pre>
tar zxvf qt-everywhere-opensource-src-5.3.1.tar.gz
cd qt-everywhere-opensource-src-5.3.1/
</pre>

* Move QtWebkit things so it excluded from the build process. It takes very long to build, prone to errors and we don't need it:

<pre>
mkdir bak
mv qtwebkit bak
mv qtwebkit-examples bak
</pre>

* Replace qtdeclarative directory,

<pre>
mv qtdeclarative bak
git clone https://github.com/qmlc/qtdeclarative.git
</pre>

* Configure and build

<pre>
./configure -developer-build -nomake tests -debug -opensource -confirm-license
make # use makes -j.. option for faster builds
</pre>

<h3>Building qmlc</h3>

While still in the qt-everywhere-opensource-src-5.3.1 directory

* Clone

<pre>
git clone https://github.com/qmlc/qmlc.git
</pre>

* Build

<pre>
cd qmlc
source qmlc.env
qmake
make
</pre>


Compiling and loading qml/js files
----------------------------------

* In order to use the compiler to compile qml/js files always source the
 qmlc.env of a compiled qmlc package.

<pre>
source qmlc.env
</pre>

<h3>Compiling and loading single qml/js files</h3>

* Use qmc to compile qml/js files. A .qmc/.jsc file with the same name will be created.
For example if test.qml have no dependencies on other qml/js files or c++ plugins.

<pre>
qmc test.qml
</pre>

* Use qmcloader from the tools directory to load(run) the compiled qml file.

<pre>
qmcloader test.qmc
</pre>

<h3>Compiling more complex projects</h3>

When compiling a qml/js file which have imports and elements to other
qml/js/c++ plugins all those have to be available during compiling just like if
one were going to run the qml/js as normal through qmlscene or one's program.

One can look at the source and build files of tests/manual/multipleitems/ to
see how to handle projects with multiple files, c++ plugins etc. via a .pri file.

For example with two qml files A.qml and B.qml in the same directory. A uses B
but B doesn't use A. Both have to be compiled to be loaded. The order that one
compiles them doesn't matter, only that the raw qml is available.

<pre>
qmc A.qml
qmc B.qml
qmcloader A.qmc
</pre>

If one is importing and using a c++ plugin extension that contains qml/js those
have to be compiled first and the resulting .so, qmldir and qml/js files has to
be available just as when those dependencies are installed.

For example with A.qml having an 'import Plugin 2.0' import with Plugin
containing qml/js, there has to be a Plugin directory with the .so and a qmldir
file as usual. Then the following should succeed.


<pre>
# Plugin directory contains at least qmldir, libplugin.so. If not using the
# resource system qml/js files have to be there as well.
qmc A.qml
</pre>

To run/load an application that uses a plugin that contains qml/js one needs to
add a  qmldir_loader file to the plugins directory that is similar to the
qmldir file with the qml/js extensions replaces with qmc/js. Note if a plugin
doesn't contain qml/js it doesn't need any changes in order to be used from
compiled files.

<pre>
# Plugin directory contains at least qmldir_loader, libplugin.so and qmc/jsc
# files.  All other qml/js it depends on has to be available as compiled
# qmc/jsc files.
qmcloader A.qmc
</pre>

Using the compiler from qmake
-----------------------------

This explains how to change existing applications or plugins to use the compiler.

There is a qmlc.pri file in qmlc that can be included in .pro files to
facilitate application and plugin extension qmlc/qmc building. It has comments
at the top explaining possible variables one can set.

In tests/manual/multipleitems/ there is a plugin and an application that uses
it that can be checked for example usage.

The application in tests/manual/multipleitems/ depends on the c++ plugin in
tests/manual/multipleitems/plugin so we build the plugin first with
tests/manual/multipleitems/plugin/plugin.pro

<h4>Plugin</h4>

Example tests/manual/multipleitems/plugin/plugin.pro.

Basic steps are,

* Make a copy of qmldir and name it qmldir_loader and in it change qml/js
  extensions to qmc/jsc extensions. This is used by the loader program to load
the qmc/jsc files. Check for example the difference between
tests/manual/multipleitems/plugin/qmldir and
tests/manual/multipleitems/plugin/qmldir_loader.

* If one were already building an uncompiled version of the plugin one might
have a file like tests/manual/multipleitems/plugin/plugin.pro up to the 'qmc
start' comment. To compile the qml/js files in of the plugin one would add
something like between the 'qmc start' and 'qmc end' comments.

<h4>Application</h4>

For an existing application with a tests/manual/multipleitems/main.cpp that contains
the main() entry pint and a tests/manual/multipleitems/app.pro file to build the application.

* Create a main_loader.cpp file, it would be very similar to
tests/manual/multipleitems/main_loader.cpp except the headers and
qRegister.. calls will be different for different applications and
loader.loadComponent will have the applications root qml file as argument.

* We create a new .pro file that has the same content as the
uncompiled programs .pro file with changes so the loader gets build. We also
add QMLC_.. variables and include qmlc.pri to compile it's qml/js. See,
tests/manual/multipleitems/app.pro and
tests/manual/multipleitems/app_compiled.pro.


Developing the compiler and loader further
------------------------------------------

* In order to work on the compiler after it was build one has to source the
 qmlc.env file that setup the paths.

<pre>
source qmlc.env
</pre>

* There are test cases in the compiletest directory. One can run it like this,

<pre>
./compiletest/compiletest
</pre>

* The multipleitems manual test is currently used to test complex application
 layouts with multiple files/c++ plugins etc.

<pre>
cd tests/manual/multipleitems/
./multipleitems
./multipleitems_loader
</pre>

Known limitations
-----------------

There are few limitations with the compiler. The compiler requires
a modified version of the Qt. The modifications are available in the
qtdeclarative repository. Other than that, the target is to
remove these limitations. There might be also a way to no require
modified Qt. The reason for the other limitations is the lack of
implementation.

- It is not possible to mix precompiled and source Qml files
- It is not possible to load Qml from network
- Few structures are still unsupported, for example composite
  singleton
- The test cases are still not very comprehensive

List of current issues can be found at:
https://github.com/qmlc/qmlc/issues
