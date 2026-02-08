cmake_minimum_required(VERSION 3.16)

project(untitled LANGUAGES C)

add_executable(untitled main.c)

include(GNUInstallDirs)
install(TARGETS untitled
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
