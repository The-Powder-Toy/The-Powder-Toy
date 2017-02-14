@echo off
scons -j2 --win --luajit --nofft --output=MyMod.exe --snapshot >outputdata.txt 2>error.txt
pause>nul