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

set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")

message(${CMAKE_SYSTEM_PROCESSOR})

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(OPENBLAS_LIB libopenblas.so)
else()
    set(OPENBLAS_LIB libopenblas_d.so)
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
    message(STATUS "Detected x86_64 architecture")
    set(TARGET_ARCH GENERIC)
    set(BINARY 64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386|i686")
    message(STATUS "Detected Linux x86 architecture")
    set(TARGET_ARCH GENERIC)
    set(BINARY 32)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7l")
    message(STATUS "Detected Linux arm32 architecture")
    set(TARGET_ARCH ARMV7)
    set(BINARY 32)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    message(STATUS "Detected Linux arm64 architecture")
    set(TARGET_ARCH ARMV8)
    set(BINARY 64)
endif()

#openblas build
set(openblas_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openblas)
set(BUILD_SHARED_LIBS ON CACHE INTERNAL "Build SHARED libraries")
set(DYNAMIC_ARCH ON CACHE INTERNAL "Dynamic arch")
set(TARGET ${TARGET_ARCH} CACHE INTERNAL "Architecute")
set(BINARY ${BINARY} CACHE INTERNAL "64 or 32 bit")
set(C_LAPACK ON CACHE INTERNAL "Use c impl of lapack")
add_subdirectory(${openblas_SOURCE_DIR} ${openblas_SOURCE_DIR} EXCLUDE_FROM_ALL)
include_directories(${openblas_SOURCE_DIR})

#cereal build
add_library(cereal_interface INTERFACE)
target_include_directories(cereal_interface INTERFACE thirdparty/cereal/include)

#ensmallen build
add_library(ensmallen_interface INTERFACE)
target_include_directories(ensmallen_interface INTERFACE thirdparty/ensmallen/include)

#armadillo build

set(OPENBLAS_PATH thirdparty/openblas/lib)
configure_file(${CMAKE_SOURCE_DIR}/thirdparty/patches/ARMA_FindOpenBLAS.cmake.in ${CMAKE_SOURCE_DIR}/thirdparty/armadillo/cmake_aux/Modules/ARMA_FindOpenBLAS.cmake)
set(armadillo_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/armadillo)
set(BUILD_SHARED_LIBS ON CACHE INTERNAL "Build SHARED libraries")
set(DETECT_HDF5 OFF CACHE INTERNAL "DETECT_HDF5")
set(OPENBLAS_PROVIDES_LAPACK ON CACHE INTERNAL "openblas provides lapack")
set(ARMA_USE_LAPACK ON CACHE INTERNAL "armadillo use lapack")
set(ARMA_USE_BLAS ON CACHE INTERNAL "armadillo use blas")
add_subdirectory(${armadillo_SOURCE_DIR} ${armadillo_SOURCE_DIR} EXCLUDE_FROM_ALL)
include_directories(${armadillo_SOURCE_DIR}/include)

#mlpack build
add_library(mlpack_interface INTERFACE)
target_include_directories(mlpack_interface INTERFACE thirdparty/mlpack/src)


add_library(OpenBLAS::OpenBLAS SHARED IMPORTED)
set_target_properties(OpenBLAS::OpenBLAS PROPERTIES
    IMPORTED_LOCATION "${openblas_SOURCE_DIR}/lib/${OPENBLAS_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${openblas_SOURCE_DIR}"
    )

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
    OpenBLAS::OpenBLAS
    cereal_interface
    ensmallen_interface
    mlpack_interface
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
