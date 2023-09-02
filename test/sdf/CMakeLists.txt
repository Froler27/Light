set(TARGET_NAME sdf)

file(GLOB EDITOR_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB EDITOR_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${EDITOR_HEADERS} ${EDITOR_SOURCES})

add_executable(${TARGET_NAME} ${EDITOR_HEADERS} ${EDITOR_SOURCES} ${EDITOR_RESOURCE})

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/WX->")

target_link_libraries(${TARGET_NAME} renderer)

target_include_directories(
  ${TARGET_NAME} 
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
