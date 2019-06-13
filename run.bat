@echo off

if exist a.exe (
  del a.exe
)

gcc submit.c -D DEV_ENV

if exist a.exe (
  a.exe %1
)
