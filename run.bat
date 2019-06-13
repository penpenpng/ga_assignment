@echo off

if exist a.exe (
  del a.exe
)

gcc submit.c -D _WIN64

if exist a.exe (
  a.exe %1
)
