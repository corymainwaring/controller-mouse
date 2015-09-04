@echo off
set common_linker_flags=-incremental:no -opt:ref
set warnings=-WX -W4 -wd4101 -wd4189 -wd4505 -wd4100 -wd4201 -wd4127
set common_flags=-nologo -GR -EHa -FC -Zi
set release_flags=-O2
set debug_flags=-Od -DHANDMADE_INTERNAL=1
set linker_flags=/DLL
set filename=controller_mouse.cpp
if not "%1" == "" (
    set filename=controller_mouse.cpp

) else (

    if not exist build mkdir build
    pushd build
        cl %warnings% %debug_flags% %common_flags% ..\controller_mouse.cpp /link %common_linker_flags% ^
             Shell32.lib User32.lib Gdiplus.lib Gdi32.lib
    popd
)
