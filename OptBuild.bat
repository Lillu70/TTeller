@echo off
cls
pushd bin

set common_linker_flags= /incremental:no /opt:ref User32.lib Gdi32.lib
set defines= /DINSTRUMENTATION
:: set defines=

cl /Fe"Nalkapeli.exe" %defines% /GR- /EHa- /W3 /options:strict /sdl /nologo /Zi /fp:fast /fp:except- /GA /O2 /std:c++17 ../src/Main.cpp ../src/App/App.cpp /link %common_linker_flags%
popd