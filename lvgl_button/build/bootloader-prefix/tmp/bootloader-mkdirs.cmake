# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/Espressif/frameworks/esp-idf-v4.4.2/components/bootloader/subproject"
  "D:/Project/ESP32_lvgl/lvgl_button/build/bootloader"
  "D:/Project/ESP32_lvgl/lvgl_button/build/bootloader-prefix"
  "D:/Project/ESP32_lvgl/lvgl_button/build/bootloader-prefix/tmp"
  "D:/Project/ESP32_lvgl/lvgl_button/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Project/ESP32_lvgl/lvgl_button/build/bootloader-prefix/src"
  "D:/Project/ESP32_lvgl/lvgl_button/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Project/ESP32_lvgl/lvgl_button/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
