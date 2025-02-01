@echo off
setlocal EnableDelayedExpansion && pushd .& set RET=0

:: ---------- Init
if defined HW3_DEPS (
:: for local builds use additional cmake find modules directory and local boost version
  set HW3_SCRIPTS_CMAKE_DIR=!HW3_DEPS!/scripts/cmake
  set "HW3_SCRIPTS_CMAKE_DIR=!HW3_SCRIPTS_CMAKE_DIR:\=/!"
  set LIBDIR=lib
  set Boost_VERSION=1.87.0
)

cd %~dp0\..

call scripts\colors_on.bat
call scripts\locate_vs2022.bat
if %RET% NEQ 0 (goto :EXITCMD)

set PLATFORM_BUILD_DIR=x64-win-vs-debug
call scripts\cleanup.bat build\%PLATFORM_BUILD_DIR%
set INSTALL_TO=%CD%\output\%PLATFORM_BUILD_DIR%
call scripts\cleanup.bat %INSTALL_TO%
set BUILD_TYPE=Debug

:: -------- CMake
echo *%C_YELLOW% Run cmake %PLATFORM_BUILD_DIR% %C_DEF%
cmake -DCMAKE_INSTALL_PREFIX=%INSTALL_TO% -DCMAKE_GENERATOR_PLATFORM=x64^
 -DCMAKE_BUILD_TYPE=Debug^
 -B build/%PLATFORM_BUILD_DIR% -S .^
 -DPLATFORM_BUILD_DIR=%PLATFORM_BUILD_DIR%^
 -DHW3_SCRIPTS_CMAKE_DIR=%HW3_SCRIPTS_CMAKE_DIR% -DLIBDIR=%LIBDIR%^
 -G %VS_GENERATOR%
if ERRORLEVEL 1 ( set RET=104& goto :EXITCMD )
echo.

:: -------- Build
echo *%C_YELLOW% Building %PLATFORM_BUILD_DIR% %C_DEF%
set PreferredToolArchitecture=x64
cmake --build build/%PLATFORM_BUILD_DIR% -- /property:Configuration=Debug
if ERRORLEVEL 1 ( set RET=105& goto :EXITCMD )
echo.

:: -------- Install
echo *%C_YELLOW% Installing %PLATFORM_BUILD_DIR% %C_DEF%
cmake --install build/%PLATFORM_BUILD_DIR% --config=Debug
if ERRORLEVEL 1 ( set RET=106& goto :EXITCMD )
echo.

:: ---------- Exit
:EXITCMD
POPD
if !RET! NEQ 0 (
  echo %C_RED% Error: %SCRIPTNAME% failed with code %RET% %C_DEF%
)
exit /b %RET%
