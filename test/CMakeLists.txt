idf_component_register(
    SRCS "test_main.c"
         "test_mining.c"
         "test_ssd1306.c"
         "test_ssd1306_auto.c"
    INCLUDE_DIRS "."
    REQUIRES unity main
)
