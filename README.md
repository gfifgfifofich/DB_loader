# Purpose
There is no tool to just do stuff easily with relational, table data.

This tool provides ability to "swim" in data, by giving an ability to transfer data form one source to another and do intermediate processing/saving. Best usecase: when you have 3 different databases, with 12 devided tables each +random csv, and a task to process all of this data into one excel table. This tool allows to even automate this sorts of workflows, add ForLoops to sql and ease the pain of beeing an analytic in company with 10 year old architecture. For more sophisticated automation python is still a prefferable option

Regular tasks, after 1-2 months of use, will also be much easier, due to TokenProcessor (works, but still somewhat WIP). Writing simple sql queries becomes just pressing tab untill result with minor corrections. After enough sql queries was ran, sqlBackup directory can be processed using Misc/Run token processor. This will cram all data into Markow chains and use this data when suggesting next words to paste.

And last but not least, charts/Graphs. Windows/Graph, select requered columns, data separators and make a graph. Useful to check if data is correct or not, and where it all goes.

![github3](https://github.com/gfifgfifofich/DB_loader/blob/main/Assets/Demo1.png)
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
