@echo off
set common_linker_flags=-incremental:no -opt:ref
set warnings=-WX -W4 -wd4101 -wd4189 -wd4505 -wd4100 -wd4201 -wd4127
set common_flags=-nologo -GR -EHa -FC -Zi
set release_flags=-O2
set debug_flags=-Od -DHANDMADE_INTERNAL=1
set linker_flags=/DLL
if "%1"=="" (
    echo Need filename to build
) else (
    if not exist build mkdir build
    pushd build
        cl %warnings% %debug_flags% %common_flags% ..\%1 /link %common_linker_flags%
    popd
)
