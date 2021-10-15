Strong Extract
==============

Extract the files from StrongHelp manuals.


Introduction
------------

Strong Extract is a cross-platform tool to extract the files from StrongHelp manuals. It is written in C, and can be compiled to run on platforms other than RISC OS.


Installation
------------

To install and use Strong Extract, it will be necessary to have suitable Linux system with a working installation of the [GCCSDK](http://www.riscos.info/index.php/GCCSDK).

It will also be necessary to ensure that the `SFTOOLS_BIN` and `$SFTOOLS_MAKE` variables are set to a suitable location within the current environment. For example

	export SFTOOLS_BIN=/home/steve/sftools/bin
	export SFTOOLS_MAKE=/home/steve/sftools/make

where the path is changed to suit your local settings and installation requirements. Finally, you will also need to have installed the Shared Makefiles, ManTools and PackTools.

To install Strong Extract, use

	make install

from the root folder of the project, which will copy the necessary files in to the location indicated by `$SFTOOLS_BIN`.

A ReadMe for Strong Extract will be generated in the buildlinux folder.


Building for native use
-----------------------

To build Strong Extract for use natively on RISC OS, you can use

	make TARGET=riscos

and the resulting files (executable and ReadMe) will be generated in the buildro folder. To create a Zip archive of the release, use

	make release TARGET=riscos

and a Zip file will appear in the parent folder to the location of the project itself.


Licence
-------

Strong Extract is licensed under the EUPL, Version 1.2 only (the "Licence"); you may not use this work except in compliance with the Licence.

You may obtain a copy of the Licence at <http://joinup.ec.europa.eu/software/page/eupl>.

Unless required by applicable law or agreed to in writing, software distributed under the Licence is distributed on an "**as is**"; basis, **without warranties or conditions of any kind**, either express or implied.

See the Licence for the specific language governing permissions and limitations under the Licence.