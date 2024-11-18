
set(src_dirs   GPIO)

set(include_dirs   GPIO)

set(requires    lvgl_esp32_drivers
		lvgl)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires} EMBED_FILES ${embed_files})

component_compile_options(-ffast-math -O3)
