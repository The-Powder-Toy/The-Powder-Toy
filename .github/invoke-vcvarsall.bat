@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x86
echo ::set-env name=PATH::%PATH%
echo ::set-env name=CC::cl
echo ::set-env name=CXX::cl
exit
