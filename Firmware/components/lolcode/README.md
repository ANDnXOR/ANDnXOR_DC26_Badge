                    # lci - a LOLCODE interpreter written in C

# LICENSE

    Copyright (C) 2010-2014 Justin J. Meza

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

# ABOUT

AND!XOR LULZCODE is based upon lci, an LOLCODE interpreter written in C and is designed to be correct,
portable, fast, and precisely documented. LULZCODE extensions are unique to the AND!XOR DC26 badge.

LOLCODE and LCI documentation can be found here:

This project's homepage is at http://lolcode.org.  For help, visit
http://groups.google.com/group/lci-general.  To report a bug, go to
http://github.com/justinmeza/lci/issues.

Created and maintained by Justin J. Meza <justin.meza@gmail.com>.

# Execution

The badge will startup and initialize all peripherals, storage, and ram. It will also perform some self tests to ensure everything is in order. Once ready, it will execute /sdcard/main.lol. After that all execution is controlled by LULZCODE. If there is an error or file not found, execution will cease and badge will go into an upload mode.