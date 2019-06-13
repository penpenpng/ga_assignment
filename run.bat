@echo off
setlocal
  if exist a.exe (
    del a.exe
  )

  gcc submit.c -D _WIN64
  if not exist a.exe goto :EOF

  if "%1" == "" (
    set data=data\input_sqrt_100
  ) else (
    set data=%1
  )

  a.exe %data% > log
endlocal
