cmake_minimum_required(VERSION 3.16)

project(real_coursework LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Sql)

add_executable(real_coursework
  main.cpp
  mainwindow.h
  mainwindow.cpp
  loginwidget.h
  loginwidget.cpp
  database.h database.cpp
  usercabinet.h usercabinet.cpp
  .gitignore
  booksmodel.h booksmodel.cpp
  bookcarddelegate.h bookcarddelegate.cpp
  test_interface.qml
  addbookdialog.cpp addbookdialog.h
  loansmodel.h loansmodel.cpp
  booksfilterproxymodel.h booksfilterproxymodel.cpp
  admincabinet.h admincabinet.cpp
)
target_link_libraries(real_coursework Qt${QT_VERSION_MAJOR}::Core)

target_link_libraries(real_coursework
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)


include(GNUInstallDirs)
install(TARGETS real_coursework
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
