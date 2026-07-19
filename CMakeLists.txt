cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# prevent installing to system directories. 
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

# Declare the project
project(chipp8)

set(SDL_SHARED ON)

set(EXECUTABLE_NAME ${PROJECT_NAME})
set(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set(VENDORED_DIR "${CMAKE_CURRENT_LIST_DIR}/vendored")
set(MACOS_DIR "${CMAKE_CURRENT_LIST_DIR}/macos")

# Create an executable and add our sources to it
add_executable(${EXECUTABLE_NAME})

target_compile_options(${EXECUTABLE_NAME}
PRIVATE
    -Wall
    -Wextra
    -Wsign-conversion
    -Wsign-compare
)

# Add your sources to the target
target_sources(${EXECUTABLE_NAME} 
PRIVATE 
    "${SOURCE_DIR}/main.cpp"
    "${SOURCE_DIR}/chip8.cpp"
    "${SOURCE_DIR}/emulator.cpp"
)

target_include_directories(${EXECUTABLE_NAME}
PRIVATE
    "${SOURCE_DIR}"
)

# Configure SDL by calling its CMake file.
# we use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory(${VENDORED_DIR}/SDL EXCLUDE_FROM_ALL)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC 
    SDL3::SDL3              
)

set_target_properties(${EXECUTABLE_NAME} PROPERTIES 
    # On macOS, make a proper .app bundle instead of a bare executable
    MACOSX_BUNDLE TRUE
    # Set the Info.plist file for macOS. Without this file, your app
    # will not launch. 
    MACOSX_BUNDLE_INFO_PLIST "${MACOS_DIR}/Info.plist.in"
    RESOURCE "${MACOS_DIR}/logo.png"
)

# On macOS Platforms, ensure that the bundle is valid for distribution by calling fixup_bundle.
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    # tell Install about the target, otherwise fixup won't know about the transitive dependencies
    install(TARGETS ${EXECUTABLE_NAME}
    	BUNDLE DESTINATION ./install COMPONENT Runtime
   	    RUNTIME DESTINATION ./install/bin COMPONENT Runtime
    )
	
    set(DEP_DIR "${CMAKE_BINARY_DIR}")  # where to look for dependencies when fixing up
    INSTALL(CODE 
        "include(BundleUtilities)
        fixup_bundle(\"$<TARGET_BUNDLE_DIR:${EXECUTABLE_NAME}>\" \"\" \"${DEP_DIR}\")
        " 
    )
    set(CPACK_GENERATOR "DragNDrop")
    include(CPack)
endif()
