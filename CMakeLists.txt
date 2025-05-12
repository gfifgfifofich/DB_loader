cmake_minimum_required(VERSION 3.16)

project(untitled VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Sql)
find_package(Qt6 REQUIRED COMPONENTS Charts)
find_package(Qt6 REQUIRED COMPONENTS Graphs)
find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS QuickControls2)
add_subdirectory(QXlsx-master/QXlsx)

LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Oracle/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Oracle/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/SimpleMail)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/lib)



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
        DBLoadScript.qml
        databasetask.h databasetask.cpp
        tokenprocessor.h tokenprocessor.cpp
        sqlSubfunctions.h
        NeuralNetwork.h NeuralNetwork.cpp

        ${app_icon_resource_windows}
        oracledriver.h oracledriver.cpp

    )
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
target_link_libraries(untitled PRIVATE Qt6::Charts)
target_link_libraries(untitled PRIVATE Qt6::Graphs)
target_link_libraries(untitled PRIVATE Qt6::Quick)
target_link_libraries(untitled PRIVATE Qt6::QuickControls2)
target_link_libraries(untitled PRIVATE oci oraocci12)
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
