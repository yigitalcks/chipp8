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
    "${SOURCE_DIR}/LTexture.cpp"
)

target_include_directories(${EXECUTABLE_NAME}
PRIVATE
    "${SOURCE_DIR}"
)

# Configure SDL by calling its CMake file.
# we use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory(${VENDORED_DIR}/SDL EXCLUDE_FROM_ALL)

# If you don't want SDL_ttf, then remove this section.
set(SDLTTF_VENDORED ON) # tell SDL_ttf to build its own dependencies
add_subdirectory(${VENDORED_DIR}/SDL_ttf EXCLUDE_FROM_ALL)	

# SDL_mixer (used for playing audio)
set(SDLMIXER_MIDI_NATIVE OFF)     # disable formats we don't use to make the build faster and smaller. Also some of these don't work on all platforms so you'll need to do some experimentation.
set(SDLMIXER_GME OFF)
set(SDLMIXER_WAVPACK OFF)     
set(SDLMIXER_MOD OFF)
set(SDLMIXER_MOD_XMP OFF)
set(SDLMIXER_OPUS OFF)
set(SDLMIXER_MP3_MPG123 OFF)
set(SDLMIXER_VORBIS_VORBISFILE OFF)
set(SDLMIXER_FLAC_LIBFLAC OFF)
set(SDLMIXER_VENDORED ON)   # tell SDL_mixer to build its own dependencies
add_subdirectory(${VENDORED_DIR}/SDL_mixer EXCLUDE_FROM_ALL)

# SDL_image (used for loading various image formats)
set(SDLIMAGE_VENDORED ON)
set(SDLIMAGE_AVIF OFF)	# disable formats we don't use to make the build faster and smaller.
set(SDLIMAGE_BMP OFF)
set(SDLIMAGE_JPG OFF)
set(SDLIMAGE_WEBP OFF)
set(SDLIMAGE_PNG_LIBPNG OFF)    # SDL3 includes built-in support for loading PNGs. If you need more than what that supports, enable this option.
add_subdirectory(${VENDORED_DIR}/SDL_image EXCLUDE_FROM_ALL)

# Link SDL to our executable. This also makes its include directory available to us. 
target_link_libraries(${EXECUTABLE_NAME} PUBLIC 
	#SDL3_ttf::SDL3_ttf      # remove if you are not using SDL_ttf
	#SDL3_mixer::SDL3_mixer  # remove if you are not using SDL_mixer
	SDL3_image::SDL3_image	# remove if you are not using SDL_image
    SDL3::SDL3              # If using satelite libraries, SDL must be the last item in the list. 
)

# Dealing with assets
# We have some non-code resources that our application needs in order to work. How we deal with those differs per platform.
if (APPLE)
    # on Apple targets, the application bundle has a "resources" subfolder where we can place our assets.
    # SDL_GetBasePath() gives us easy access to that location.
    set(input_root "${MACOS_DIR}")
    macro(add_resource FILE)
        file(RELATIVE_PATH relpath "${input_root}" "${FILE}")
        get_filename_component(relpath "${relpath}" DIRECTORY)
        target_sources(${EXECUTABLE_NAME} PRIVATE ${FILE})
        set_property(SOURCE ${FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${relpath}")
    endmacro()
    add_resource("${MACOS_DIR}/Inter-VariableFont.ttf")
else()
    # for platforms that do not have a good packaging format, all we can do is copy the assets to the process working directory.
    macro(copy_helper filename)
        set(outname "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${filename}")
        add_custom_command(TARGET "${EXECUTABLE_NAME}" POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MACOS_DIR}/${filename}" "${outname}"
        )
    endmacro()
    copy_helper("Inter-VariableFont.ttf")
endif()

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
