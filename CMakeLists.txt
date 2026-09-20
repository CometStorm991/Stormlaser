cmake_minimum_required (VERSION 3.29)

project (Stormlaser)

message(STATUS "CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
message(STATUS "VCPKG_TARGET_TRIPLET = ${VCPKG_TARGET_TRIPLET}")
message(STATUS "VCPKG_INSTALLED_DIR = ${VCPKG_INSTALLED_DIR}")
message(STATUS "Z_VCPKG_ROOT_DIR = ${Z_VCPKG_ROOT_DIR}")

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan 1.4.335 REQUIRED)
find_package(Ktx     CONFIG REQUIRED)
find_package(Stb     REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator-hpp CONFIG REQUIRED)

set(PROJECT_DIR ${CMAKE_BINARY_DIR}/${PROJECT_NAME})

# -------------------- VULKAN --------------------
# Set up Vulkan C++ module only if enabled
add_library(Stormlaser_VulkanHppConfig INTERFACE)
target_link_libraries(Stormlaser_VulkanHppConfig INTERFACE Vulkan::Vulkan)
target_compile_definitions(Stormlaser_VulkanHppConfig INTERFACE
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

add_executable (${PROJECT_NAME} 
    src/Application.cpp
    src/Camera.cpp
    src/CameraController.cpp
    src/Main.cpp
    src/TextureProcessor.cpp
    src/VulkanDispatch.cpp
    src/Window.cpp
    vendors/StbImage.cpp)
set_target_properties (${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_DIR})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Stormlaser_VulkanHppConfig
    glfw
    glm::glm
    KTX::ktx
    fastgltf::fastgltf
    GPUOpen::VulkanMemoryAllocator
    unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp)
target_include_directories(${PROJECT_NAME} BEFORE PRIVATE
    ${Vulkan_INCLUDE_DIR}   # win the header-ordering fight from vcpkg package and SDK
    ${Stb_INCLUDE_DIR})

# Define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS to treat VK_ERROR_OUT_OF_DATE_KHR as a success code
target_compile_definitions(${PROJECT_NAME} PRIVATE "VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS" )

# -------------------- Slang shader --------------------

find_program(SLANGC_EXECUTABLE
    NAMES slangc
    HINTS "$ENV{VULKAN_SDK}/bin"
    REQUIRED
    DOC "Slang shader compiler from the Vulkan SDK")

message(STATUS "slangc: ${SLANGC_EXECUTABLE}")

function(add_slang_shader_target TARGET)
  cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})
  set(SHADERS_DIR ${PROJECT_DIR}/shaders)
  set(ENTRY_POINTS -entry vertMain -entry fragMain)

  add_custom_command(
      OUTPUT ${SHADERS_DIR}/slang.spv
      COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
      COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4
              -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o slang.spv
      WORKING_DIRECTORY ${SHADERS_DIR}
      DEPENDS ${SHADER_SOURCES} ${SLANGC_EXECUTABLE}
      COMMENT "Compiling Slang shaders"
      VERBATIM)

  add_custom_target(${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
endfunction()

set(SHADER_SLANG_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/shaders/shader.slang)
add_slang_shader_target(slang_shaders SOURCES ${SHADER_SLANG_SOURCES})
add_dependencies(${PROJECT_NAME} slang_shaders)

# Assets
set (ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets/")
set (ASSET_DESTINATION_DIR "${PROJECT_DIR}/assets/")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
          ${ASSET_DIR} "${ASSET_DESTINATION_DIR}"
  VERBATIM)