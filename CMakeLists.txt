## Minimum is 3.16 for PCH support
cmake_minimum_required( VERSION 3.16 )

## Project name
project( Elegy.DevConsoleApp )

## Some property stuff
set_property( GLOBAL PROPERTY USE_FOLDERS ON )
## FTXUI uses C++17
set( CMAKE_CXX_STANDARD 17 )

set( ELG_ROOT ${CMAKE_CURRENT_SOURCE_DIR} )
set( ELG_BIN_DIRECTORY ${ELG_ROOT}/bin )

## FTXUI
option( FTXUI_BUILD_TESTS OFF )
option( FTXUI_BUILD_EXAMPLES OFF )
option( FTXUI_BUILD_DOCS OFF )
option( FTXUI_ENABLE_INSTALL OFF )

## We really only need to link against 'component' here,
## so we don't need to link against all 3 individually
add_subdirectory( extern/FTXUI )
set_target_properties( screen PROPERTIES FOLDER "FTXUI" )
set_target_properties( dom PROPERTIES FOLDER "FTXUI" )
set_target_properties( component PROPERTIES FOLDER "FTXUI" )

## ENet (LSalzman's ENet 1.3.17, the one Godot uses - an important detail)
## In the near future we might as well just directly copy Godot's ENet, but this works for now
add_subdirectory( extern/enet )
set_target_properties( enet PROPERTIES FOLDER "ENet" )

## Elegy.DevConsoleApp stuff
set( DEVCONAPP_SOURCES
	${ELG_ROOT}/src/Main.cpp )

## Folder organisation
source_group( TREE ${ELG_ROOT} FILES ${DEVCONAPP_SOURCES} )

## Project
add_executable( Elegy.DevConsoleApp ${DEVCONAPP_SOURCES} )

## Include ENet
target_include_directories( Elegy.DevConsoleApp PRIVATE
	${ELG_ROOT}
	${ELG_ROOT}/src
	${ELG_ROOT}/extern/enet/include )

## Link against FTXUI
target_link_libraries( Elegy.DevConsoleApp enet ftxui::component )

## Output here
install( TARGETS Elegy.DevConsoleApp
	RUNTIME DESTINATION ${ELG_BIN_DIRECTORY}
	LIBRARY DESTINATION ${ELG_BIN_DIRECTORY} )

## On Windows, place PDB files as well
if ( WIN32 )
	install( FILES $<TARGET_PDB_FILE:Elegy.DevConsoleApp>
		DESTINATION ${ELG_BIN_DIRECTORY} OPTIONAL )
endif()
