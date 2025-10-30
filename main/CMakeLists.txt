idf_component_register(
    SRCS "main.c" "ssd1306.c"
    INCLUDE_DIRS "."
)

# Allow WiFi credentials to be overridden via environment variables for CI/CD
# Note: These environment variables should only be set in trusted CI/CD environments
# CMake automatically handles proper escaping of the environment variable values
if(DEFINED ENV{CI_WIFI_SSID})
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        WIFI_SSID="$ENV{CI_WIFI_SSID}"
    )
endif()

if(DEFINED ENV{CI_WIFI_PASS})
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        WIFI_PASS="$ENV{CI_WIFI_PASS}"
    )
endif()