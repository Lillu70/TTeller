@echo off
cls
pushd bin

set common_linker_flags= /incremental:no /opt:ref User32.lib Gdi32.lib Comdlg32.lib
set defines= /D_DB

cl /Fe"StoryToTheDeath.exe" %defines% /Od /GR- /EHa- /W3 /nologo /Zi /std:c++17 ../src/Main.cpp ../src/App/App.cpp /link %common_linker_flags%
popd


