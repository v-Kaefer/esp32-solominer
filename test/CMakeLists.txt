cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_btc_miner_test)
idf_component_register(
    SRCS "test_main.c"
         "test_mining.c"
         "test_ssd1306.c"
         "test_ssd1306_auto.c"
         "test_i2c_master.c"
         "../driver/i2c_master.c"
    INCLUDE_DIRS "." ".."
    REQUIRES unity main
)
