idf_component_register(
    SRCS "main.c" "ssd1306.c" "mqtt_handler.c"
    INCLUDE_DIRS "."
    REQUIRES mqtt
)