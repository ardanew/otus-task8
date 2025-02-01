@echo off
if not DEFINED DOXYGEN_ROOT set DOXYGEN_ROOT="c:\Program Files\doxygen\bin"
cd %~dp0\..
%DOXYGEN_ROOT%\doxygen.exe Doxyfile
cd %~dp0
