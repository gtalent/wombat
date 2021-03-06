cmake_minimum_required(VERSION 2.8.8)

#setup libraries

find_package(Jansson REQUIRED)
include_directories(${JANSSON_INCLUDE_DIR})
set(LIBS ${LIBS} ${JANSSON_LIBRARY})

if(WOMBAT_BUILD_TYPE STREQUAL "Native")
	find_package(SDL REQUIRED)
	include_directories(${SDL_INCLUDE_DIR})
	set(LIBS ${LIBS} ${SDL_LIBRARY})

	find_package(SDL_IMAGE REQUIRED)
	include_directories(${SDL_IMAGE_INCLUDE_DIR})
	set(LIBS ${LIBS} ${SDL_IMAGE_LIBRARY})

	find_package(SDL_ttf REQUIRED)
	include_directories(${SDL_TTF_INCLUDE_DIR})
	set(LIBS ${LIBS} ${SDL_TTF_LIBRARY})
elseif(WOMBAT_BUILD_TYPE STREQUAL "GBA")
	set(LIBS ${LIBS} ${GBA_LIBRARY})
	set(LIBS ${LIBS} ${FILESYSTEM_LIBRARY})
	set(LIBS ${LIBS} ${FAT_LIBRARY})
	include_directories(${GBA_INCLUDE_DIR})
endif()


#project packages

include_directories(".")

add_subdirectory(models)
set(OBJS ${OBJS} $<TARGET_OBJECTS:WombatModels>)

add_subdirectory(common)
set(OBJS ${OBJS} $<TARGET_OBJECTS:WombatCommon>)

add_subdirectory(core)
set(OBJS ${OBJS} $<TARGET_OBJECTS:WombatCore>)

add_subdirectory(ui)
set(OBJS ${OBJS} $<TARGET_OBJECTS:WombatUI>)

add_subdirectory(entities)
set(OBJS ${OBJS} $<TARGET_OBJECTS:WombatEntities>)

add_subdirectory(world)
set(OBJS ${OBJS} $<TARGET_OBJECTS:WombatWorld>)

add_subdirectory(wombat)
