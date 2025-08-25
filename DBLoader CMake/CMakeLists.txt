cmake_minimum_required(VERSION 3.16)

project(untitled VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Sql Test)
find_package(Qt6 REQUIRED COMPONENTS Charts)
find_package(Qt6 REQUIRED COMPONENTS Graphs)
find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS AxContainer)
find_package(Qt6 REQUIRED COMPONENTS QuickControls2)
find_package(Qt6 COMPONENTS Core LinguistTools)

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES en ru)

add_subdirectory(QXlsx-master/QXlsx)
add_subdirectory(OpenXLSX)

#oracle
if(Oracle_OCI_Driver)
    add_compile_definitions(Oracle_OCI_Driver)
    LINK_DIRECTORIES(${Oracle_OCI_Lib_Directory})
    include_directories(${Oracle_OCI_Include_Directory})
endif()

#oracle
if(Postgres_PSQL_Driver)
    add_compile_definitions(Postgres_PSQL_Driver)
    LINK_DIRECTORIES(${Postgres_PSQL_Lib_Directory})
    include_directories(${Postgres_PSQL_Include_Directory})
endif()
#SimpleMail for mail automation
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/SimpleMail)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/lib)


# add sqlite source directly
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sqlite_amalgamation)

enable_testing(true)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
    qt_add_executable(untitled
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        main.cpp
        codeeditor.cpp codeeditor.h datastorage.cpp datastorage.h highlighter.cpp highlighter.h linenumberarea.cpp linenumberarea.h Patterns.cpp Patterns.h
        codeeditor.cpp

        linenumberarea.h linenumberarea.cpp
        codeeditor.h codeeditor.cpp
        highlighter.h highlighter.cpp
        Patterns.cpp
        Patterns.h
        datastorage.h datastorage.cpp
        tabledata.h tabledata.cpp
        databaseconnection.h databaseconnection.cpp
        loaderwidnow.h loaderwidnow.cpp loaderwidnow.ui
        table.h table.cpp
        replacedialog.h replacedialog.cpp replacedialog.ui


        qtcomputeshader/glwrappers.h
        qtcomputeshader/glwrappers.cpp

        tokenprocessor.h tokenprocessor.cpp
        sqlSubfunctions.h
        NeuralNetwork.h NeuralNetwork.cpp

        ${app_icon_resource_windows}

        oracledriver.h oracledriver.cpp

        colorbutton.h colorbutton.cpp
        sqlite_amalgamation/sqlite3.c


        settingswindow.h settingswindow.cpp settingswindow.ui
        structuredescriptor.h structuredescriptor.cpp structuredescriptor.ui
        docwindow.h docwindow.cpp docwindow.ui
        replacewindow.h replacewindow.cpp replacewindow.ui
        testing.h
        bpe.cpp





    )
    qt_add_translations(untitled)

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET untitled APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(untitled SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(untitled
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(untitled PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(untitled PRIVATE Qt${QT_VERSION_MAJOR}::Sql)
target_link_libraries(untitled PRIVATE QXlsx::QXlsx)
target_link_libraries(untitled PRIVATE OpenXLSX)
target_link_libraries(untitled PRIVATE Qt6::Charts)
target_link_libraries(untitled PRIVATE Qt6::Graphs)
target_link_libraries(untitled PRIVATE Qt6::Quick)
target_link_libraries(untitled PRIVATE Qt6::QuickControls2)
target_link_libraries(untitled PRIVATE Qt6::Test)
target_link_libraries(untitled PRIVATE Qt6::AxContainer)


add_test(test_infr untitled test)
add_test(test_const_fail untitled test fail)
add_test(test_xlsxReadWrite untitled test xlsxReadWrite)
add_test(test_csvReadWrite untitled test csvReadWrite)
add_test(test_LocalDBReadWrite untitled test LocalDBReadWrite)
add_test(test_CycleTest untitled test CycleTest)
add_test(test_SubexecTest untitled test SubexecTest)
add_test(test_DateFunctionTest untitled test DateFunctionTest)
add_test(test_ParallelExecTest untitled test ParallelExecTest)
add_test(test_Amalgamation untitled test Amalgamation)


if(Oracle_OCI_Driver)
    target_link_libraries(untitled PRIVATE oci oraocci12)
endif()

if(Postgres_PSQL_Driver)
    target_link_libraries(untitled PRIVATE libpq libssl postgres pgevent)
endif()


target_link_libraries(untitled PRIVATE SimpleMail3Qt6)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.untitled)
endif()
set_target_properties(untitled PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS untitled
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(untitled)
endif()
