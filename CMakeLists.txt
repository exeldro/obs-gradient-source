project(gradient-source VERSION 0.0.1)
set(PROJECT_FULL_NAME "Gradient Source")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

set(gradient-source_HEADERS
	gradient-source.h
	version.h)
set(gradient-source_SOURCES
	gradient-source.c)

if(WIN32)
	get_filename_component(ISS_FILES_DIR "${CMAKE_BINARY_DIR}\\..\\package" ABSOLUTE)
	file(TO_NATIVE_PATH "${ISS_FILES_DIR}" ISS_FILES_DIR)
	get_filename_component(ISS_PACKAGE_DIR "${CMAKE_PACKAGE_PREFIX}\\.." ABSOLUTE)
	file(TO_NATIVE_PATH "${ISS_PACKAGE_DIR}" ISS_PACKAGE_DIR)
	get_filename_component(ISS_SOURCE_DIR "${PROJECT_SOURCE_DIR}" ABSOLUTE)
	file(TO_NATIVE_PATH "${ISS_SOURCE_DIR}" ISS_SOURCE_DIR)
	configure_file("installer.iss.in"
			"${PROJECT_BINARY_DIR}/installer.iss"
		)
	configure_file(resource.rc.in gradient-source.rc)
	list(APPEND gradient-source_SOURCES
		gradient-source.rc)
endif()

add_library(gradient-source MODULE
	${gradient-source_HEADERS}
	${gradient-source_SOURCES})
target_link_libraries(gradient-source
	libobs)

set_target_properties(gradient-source PROPERTIES FOLDER "plugins/exeldro")
install_obs_plugin_with_data(gradient-source data)
