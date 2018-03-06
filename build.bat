@echo off

cls

mkdir build

cl.exe -nologo -Iinclude -I..\..\..\Downloads\pdcurs36 -c -O1 -DPDC_WIDE -DPDC_FORCE_UTF8 src\dungeon.c /Fobuild\dungeon.obj
cl.exe -nologo -Iinclude -I..\..\..\Downloads\pdcurs36 -c -O1 -DPDC_WIDE -DPDC_FORCE_UTF8 src\character.c /Fobuild\character.obj
cl.exe -nologo -Iinclude -I..\..\..\Downloads\pdcurs36 -c -O1 -DPDC_WIDE -DPDC_FORCE_UTF8 src\event.c /Fobuild\event.obj
cl.exe -nologo -Iinclude -I..\..\..\Downloads\pdcurs36 -c -O1 -DPDC_WIDE -DPDC_FORCE_UTF8 src\heap.c /Fobuild\heap.obj
cl.exe -nologo -Iinclude -I..\..\..\Downloads\pdcurs36 -c -O1 -DPDC_WIDE -DPDC_FORCE_UTF8 src\info.c /Fobuild\info.obj
cl.exe -nologo -Iinclude -I..\..\..\Downloads\pdcurs36 -c -O1 -DPDC_WIDE -DPDC_FORCE_UTF8 src\move.c /Fobuild\move.obj
cl.exe -nologo -Iinclude -I..\..\..\Downloads\pdcurs36 -c -O1 -DPDC_WIDE -DPDC_FORCE_UTF8 src\npc.c /Fobuild\npc.obj
cl.exe -nologo -Iinclude -I..\..\..\Downloads\pdcurs36 -c -O1 -DPDC_WIDE -DPDC_FORCE_UTF8 src\path.c /Fobuild\path.obj
cl.exe -nologo -Iinclude -I..\..\..\Downloads\pdcurs36 -c -O1 -DPDC_WIDE -DPDC_FORCE_UTF8 src\pc.c /Fobuild\pc.obj
cl.exe -nologo -Iinclude -I..\..\..\Downloads\pdcurs36 -c -O1 -DPDC_WIDE -DPDC_FORCE_UTF8 src\rlg327.c /Fobuild\rlg327.obj
cl.exe -nologo -Iinclude -I..\..\..\Downloads\pdcurs36 -c -O1 -DPDC_WIDE -DPDC_FORCE_UTF8 src\utils.c /Fobuild\utils.obj

link.exe -nologo ..\..\..\Downloads\pdcurs36\wincon\pdcurses.lib user32.lib advapi32.lib build\*.obj /out:build\rlg.exe