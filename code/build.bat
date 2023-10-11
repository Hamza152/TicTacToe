@echo off

pushd ..\build\
cl -Zi ..\code\TicTacToe.cpp user32.lib gdi32.lib Winmm.lib
popd
