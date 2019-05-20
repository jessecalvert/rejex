@echo off

set CommonCompilerFlags=/EHsc -MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4127 -wd4201 -wd4100 -wd4189 -wd4505 -FC -Z7

pushd ..\build
cl %CommonCompilerFlags% ..\code\rejex.cpp -Fmrejex.map /link -incremental:no -opt:ref
popd