set(example_name "Core.Graphics.Example.HelloTriangle")

add_executable(${example_name})
luastg_target_common_options(${example_name})
luastg_target_more_warning(${example_name})
target_sources(${example_name} PRIVATE
    HelloTriangle.cpp
    Core.Graphics.Example.HelloTriangle.manifest
)
target_link_libraries(${example_name} PRIVATE
    Core.WindowSystem
    Core.Graphics
)

set_target_properties(${example_name} PROPERTIES FOLDER engine/example)
