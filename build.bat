@ECHO ON

RMDIR /Q /S build
MKDIR build
PUSHD build

cmake -G "Visual Studio 16" ..
