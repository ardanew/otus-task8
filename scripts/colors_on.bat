@echo off

:: For windows versions >= Win10 there are support for color output
for /f "tokens=4-5 delims=. " %%i in ('ver') do set WIN_VERSION=%%i.%%j
if "%WIN_VERSION%" == "10.0" set COLOR_SUPPORT="ON"
if "%WIN_VERSION%" == "11.0" set COLOR_SUPPORT="ON"
if %COLOR_SUPPORT% == "ON" (
    set C_YELLOW=[33m
    set C_RED=[31m
    set C_DEF=[0m
)