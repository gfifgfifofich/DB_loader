# Usage
run any .exe from archive

Docs - DOC.md

# Build
Best way to build - QtCreator
Oper CMakeLists.txt in QtCreator, setup CMake options, build

Probably possible to build using qt-cmake

# Drivers
## Oracle
Set Oracle_OCI_Driver to ON
Set Oracle_OCI_Lib_Directory to directory, containing all libs from Oracle OCI SDK
Set Oracle_OCI_Include_Directory to same diractory from Oracle OCI SDK

after build add all .dll's to same directory as executable (or whatever your OS is using). Or use any other method to inform program of their existance, like env Paths. 
