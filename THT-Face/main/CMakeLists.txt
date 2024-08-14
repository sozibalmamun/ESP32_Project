#old 
# set(src_dirs        .)

# set(include_dirs    .)
# idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs})

#new
 idf_component_register(SRCS 
                             "app_main.cpp" 
                             "event_logic.cpp" 
                             "StomeClient/StomeClient.c" 
                             "dataHandler/dataHandler.c" 
                             "enrol/enrol.c" 
                             "Conectivity/Conectivity.c"

                             "blufi_example_main.c"
                             "blufi_security.c"
                             "blufi_init.c"

                             
                    INCLUDE_DIRS ".")

