set(SOURCES
    vsgtracyprofiler.cpp
)

add_executable(vsgtracyprofiler ${SOURCES})

target_include_directories(vsgtracyprofiler PUBLIC ${VSGTRACY_SOURCE_DIR}/3rdparty/tracy/public)

target_link_libraries(vsgtracyprofiler vsg::vsg Tracy::TracyClient)

if (vsgXchange_FOUND)
    target_compile_definitions(vsgtracyprofiler PRIVATE vsgXchange_FOUND)
    target_link_libraries(vsgtracyprofiler vsgXchange::vsgXchange)
endif()

install(TARGETS vsgtracyprofiler RUNTIME DESTINATION bin)
