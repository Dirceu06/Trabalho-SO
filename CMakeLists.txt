cmake_minimum_required(VERSION 3.19)
project(projeto-so LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(projeto-so
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    escalonador.h escalonador.cpp escalonador.ui
    Escalonador/auxiliares.cpp
    Escalonador/auxiliares.h
    Escalonador/base.cpp
    Escalonador/base.h
    Escalonador/priori.cpp
    Escalonador/rr.cpp
    Escalonador/sjf.cpp
    Memoria/GerenciadorMemoria.cpp
    Memoria/GerenciadorMemoria.h
    relatoriowindow.h relatoriowindow.cpp relatoriowindow.ui
)

target_include_directories(projeto-so PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Escalonador
    ${CMAKE_CURRENT_SOURCE_DIR}/Memoria
)

target_link_libraries(projeto-so
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS projeto-so
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET projeto-so
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
