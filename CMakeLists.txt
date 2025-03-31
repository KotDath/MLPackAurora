# SPDX-FileCopyrightText: Copyright (c) 2025, Daniil Markevich
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.5)
project(ru.kotdath.MLPackAurora CXX)

find_package (Qt5 COMPONENTS Core Network Qml Gui Quick LinguistTools REQUIRED)

include(FindPkgConfig)
pkg_search_module(AURORA auroraapp REQUIRED)
pkg_search_module(AURORA auroraapp_i18n REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(short_name MLPackAurora)

file(GLOB_RECURSE HEADER_FILES "src/*.h")

file(GLOB_RECURSE SOURCES_FILES "src/*.cpp")

file(GLOB_RECURSE QML_FILES "qml/*.qml")

file(GLOB_RECURSE ICON_FILES "icons/*.png")

file(GLOB TS_FILES "translations/*.ts")

file(GLOB MD_FILES "*.md")

set(TRANSLATION_SOURCES_FILES
    ${SOURCES_FILES}
    ${QML_FILES}
)

set(GEN_TS_PATH ${CMAKE_CURRENT_BINARY_DIR}/gen_translations)


foreach(_ts_file ${TS_FILES})
    file(COPY ${_ts_file} DESTINATION ${GEN_TS_PATH})
endforeach()
file(GLOB GEN_TS_FILES "${GEN_TS_PATH}/*.ts")

add_custom_target(update_translations)
foreach(_ts_file ${GEN_TS_FILES})
    get_filename_component(_ts_name ${_ts_file} NAME_WE)
    add_custom_command(
        OUTPUT ${_ts_file}
        DEPENDS ${TRANSLATION_SOURCES_FILES}
        COMMAND ${Qt5_LUPDATE_EXECUTABLE}
        ARGS ${TRANSLATION_SOURCES_FILES} -ts ${_ts_file}
        COMMAND ${CMAKE_COMMAND} -E copy
        ARGS ${_ts_file} ${CMAKE_CURRENT_SOURCE_DIR}/translations
    )
    add_custom_target(translate_${_ts_name} DEPENDS ${_ts_file})
    add_dependencies(update_translations translate_${_ts_name})
endforeach()

qt5_add_translation(TRANSLATION_QM_FILES ${GEN_TS_FILES})

set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/lib")


add_executable(${PROJECT_NAME}
    ${HEADER_FILES}
    ${SOURCES_FILES}
    ${ICON_FILES}
    ${QML_FILES}
    ${TS_FILES}
    ${PROJECT_NAME}.desktop
    rpm/${PROJECT_NAME}.spec
    ${TRANSLATION_QM_FILES}
    ${MD_FILES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:
    ${AURORA_INCLUDE_DIRS}
>)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Quick
    ${AURORA_LDFLAGS}
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY qml
    DESTINATION share/${PROJECT_NAME}
)

foreach(_file IN LISTS TRANSLATION_QM_FILES)
    get_filename_component(_filename "${_file}" NAME)
    string(REPLACE "${short_name}" "${PROJECT_NAME}" _full_filename "${_filename}")
    install(FILES ${_file} DESTINATION share/${PROJECT_NAME}/translations RENAME ${_full_filename})
endforeach()

set(IconSize 86x86 108x108 128x128 172x172)
foreach(_size IN LISTS IconSize)
    install(FILES icons/${_size}/${PROJECT_NAME}.png DESTINATION share/icons/hicolor/${_size}/apps)
endforeach()

install(FILES ${PROJECT_NAME}.desktop
    DESTINATION share/applications
)

file(WRITE "${CMAKE_BINARY_DIR}/QtCreatorDeployment.txt"
    "${CMAKE_INSTALL_PREFIX}
${CMAKE_BINARY_DIR}/${PROJECT_NAME}:bin
")
