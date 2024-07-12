idf_component_register(SRCS "stomp_client.c" "app_main.c"
                    INCLUDE_DIRS ".")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")