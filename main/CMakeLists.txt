idf_component_register(
    SRCS "main.c" "ssd1306.c" "../driver/i2c_master.c"
    INCLUDE_DIRS "." ".."
)