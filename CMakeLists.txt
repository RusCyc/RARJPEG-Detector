cmake_minimum_required(VERSION 3.16)

project(untitled1 LANGUAGES C)

add_executable(untitled1 main.c)

include(GNUInstallDirs)
install(TARGETS untitled1
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
