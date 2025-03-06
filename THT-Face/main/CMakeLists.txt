#old 
# set(src_dirs        .)

# set(include_dirs    .)
# idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs})

#new
 idf_component_register(SRCS 
                             "app_main.cpp" 
                             "event_logic.cpp" 
                             "WssClient/WssClient.c" 
                             "cloudEvent/cloudEvent.c" 

                             "Conectivity/blufi_example_main.c"
                             "Conectivity/blufi_security.c"
                             "Conectivity/blufi_init.c"
                             "Storage/fs.c"
                             "Storage/mynvs.c"

                             
                    INCLUDE_DIRS ".")

