@echo off

if "%1"=="" ( 
  goto :default
) else ( 
  goto %*
)
goto :EOF

:default
  echo * %C_YELLOW% Locating VS2022... %C_DEF%
  if /i "%PROCESSOR_ARCHITECTURE%"=="x86" (
    set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
  ) else (
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
  )
  if exist "%VSWHERE%" (
    echo * vswhere.exe located, path = "%VSWHERE%"
  ) else (
    set RET=102
    echo * vswhere.exe not found
    goto :EXITCMD
  )
  for /d %%I in ("%VSWHERE%") do set VSWHERE=%%~sI
  for /F "tokens=*" %%I in ('%VSWHERE% -version "[17.0,18.0)" -property installationPath') do set "VS170COMNTOOLS=%%I"

  if not exist "%VS170COMNTOOLS%" (
    echo * Required Visual Studio or its component was not found
    set RET=103
    goto :EXITCMD
  )
  set "VS170COMNTOOLS=%VS170COMNTOOLS%\Common7\Tools"
  echo * Launching VS command prompt %VS170COMNTOOLS%\VsDevCmd.bat
  if exist "%VS170COMNTOOLS%\VsDevCmd.bat" (
    call "%VS170COMNTOOLS%\VsDevCmd.bat"
  )
  echo.
  set VS_GENERATOR="Visual Studio 17 2022"
  goto :EOF


:EXITCMD
if !RET! NEQ 0 (
  echo %RED% Can't locate VS2017 %COLOR_DEFAULT%
  exit /b %RET%
)