cmake_minimum_required(VERSION 3.20)
project(WebsiteVulkanApplication LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(app
  src/main.cpp
  src/gpu/context.cpp
  src/gpu/window.cpp
  src/gpu/renderer.cpp
)

target_include_directories(app
  PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(app PRIVATE Vulkan::Vulkan glfw)


# find_program(SLANGC slangc HINTS $ENV{VULKAN_SDK}/bin)

# if(SLANGC)
#     set(SHADER_DIR ${CMAKE_SOURCE_DIR}/shaders)
#     set(SPV_DIR ${CMAKE_BINARY_DIR}/shaders)
#     file(MAKE_DIRECTORY ${SPV_DIR})

#     add_custom_command(OUTPUT ${SPV_DIR}/shader.spv
#         COMMAND ${SLANGC} ${SHADER_DIR}/shader.slang
#             -target spirv
#             -o ${SPV_DIR}/shader.spv
#             -entry vertMain -stage vertex
#             -entry fragMain -stage fragment
#         DEPENDS ${SHADER_DIR}/shader.slang
#         COMMENT "Compiling shader.slang -> shader.spv"
#     )

#     add_custom_target(shaders ALL DEPENDS ${SPV_DIR}/shader.spv)
#     add_dependencies(app shaders)
# else()
#     message(WARNING "slangc not found — install Slang (https://github.com/shader-slang/slang) or add it to PATH")
# endif()
