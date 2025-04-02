add_library(utf8 STATIC)
luastg_target_common_options(utf8)
luastg_target_more_warning(utf8)
target_include_directories(utf8 PUBLIC .)
target_sources(utf8 PRIVATE utf8.hpp utf8.cpp)

set_target_properties(utf8 PROPERTIES FOLDER utility)
