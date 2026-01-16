idf_component_register(
    SRCS "main.c" "ssd1306.c" "stratum.c" "mining.c" "../driver/i2c_master.c"
    INCLUDE_DIRS "." ".."
    REQUIRES json
)