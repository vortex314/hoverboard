## remote openocd debugging and flashing
```
$ ./ocd-server.sh
$ sudo apt-get install gdb-multiarch
$ gdb-multiarch firmware.elf 
GNU gdb (Ubuntu 12.0.90-0ubuntu1) 12.0.90
Copyright (C) 2022 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from firmware.elf...
(No debugging symbols found in firmware.elf)
(gdb) target remote limero.local
limero.local: No such file or directory.
(gdb) target remote limero.local:3333
Remote debugging using limero.local:3333
0x08024f44 in ?? ()
(gdb) 

```
## 