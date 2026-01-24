# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

#set(ENV{PATH} "$ENV{HOME}/.pyenv/shims;$ENV{HOME}/Data/ESP8266/xtensa-lx106-elf/bin;/opt/homebrew/opt/ncurses/bin;$ENV{PATH}")
LIST(APPEND CMAKE_PROGRAM_PATH  "$ENV{HOME}/.pyenv/shims" "$ENV{HOME}/Data/ESP8266/xtensa-lx106-elf/bin" "/opt/homebrew/opt/ncurses/bin")
set(ENV{IDF_PATH} $ENV{HOME}/Data/ESP8266/ESP8266_RTOS_SDK)
set(IDF_PATH $ENV{HOME}/Data/ESP8266/ESP8266_RTOS_SDK)
set(PYTHON "$ENV{HOME}/.pyenv/shims/python")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ShutterCtrl)