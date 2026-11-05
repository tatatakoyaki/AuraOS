cmake_minimum_required(VERSION 3.14)

project(AuraOS VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Multimedia Network WebSockets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Multimedia Network WebSockets REQUIRED)

set(PROJECT_SOURCES
    main.cpp
    include/Services/hvachandler.h
    include/Services/mediahandler.h
    include/Services/appmanager.h
    include/Services/widgetmanager.h
    include/Services/systemmanager.h
    include/Services/audiorecorder.h
    include/Services/AgentGuard.h

    include/Agent/agentbridge.h

    include/Core/IMessageBus.h
    include/Core/ServiceRegistry.h
    include/Core/AgentRegistry.h
    include/Core/RuleEngine.h
    include/Core/Blackboard.h

    include/Platform/LocalEventBus.h

    include/HAL/IVehicleHardware.h
    include/HAL/IMediaHardware.h

    include/Platform/Mock/MockHvacHardware.h
    include/Platform/Mock/MockMediaHardware.h

    src/Services/hvachandler.cpp
    src/Services/mediahandler.cpp
    src/Services/appmanager.cpp
    src/Services/widgetmanager.cpp
    src/Services/systemmanager.cpp
    src/Services/audiorecorder.cpp
    src/Services/AgentGuard.cpp

    src/Agent/agentbridge.cpp

    src/Platform/Mock/MockHvacHardware.cpp
    src/Platform/Mock/MockMediaHardware.cpp

    qml.qrc
    )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AuraOS
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET AuraOS APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(AuraOS SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(AuraOS
          ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(AuraOS PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(AuraOS
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(AuraOS
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::WebSockets)

set_target_properties(AuraOS PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(AuraOS)
    qt_finalize_executable(AuraOS)
endif()
