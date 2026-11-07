cmake_minimum_required(VERSION 3.16)
project(actividad_integral_2 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(actividad_integral_2 Node.h)

# Copia la bitacora de la Actividad Integral 1 junto al ejecutable para que el
# programa la encuentre sin importar desde donde se compile
configure_file(bitacora_ordenada.csv ${CMAKE_CURRENT_BINARY_DIR}/bitacora_ordenada.csv COPYONLY)
