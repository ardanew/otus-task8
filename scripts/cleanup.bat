@echo off
:: Script to clean directory and then create empty one
:: Usage: cleanup.bat <directory>
:: or after executing common_init.bat: !cleanup! <directory>

del /f /s /q %1 >NUL 2>&1
rd /s /q %1 >NUL 2>&1
mkdir %1 >NUL 2>&1

exit /b

