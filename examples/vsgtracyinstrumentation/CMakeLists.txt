set(SOURCES
    vsgtracyinstrumentation.cpp
)

add_executable(vsgtracyinstrumentation ${SOURCES})

target_include_directories(vsgtracyinstrumentation PUBLIC ${VSGTRACY_SOURCE_DIR}/3rdparty/tracy/public)

target_link_libraries(vsgtracyinstrumentation vsg::vsg Tracy::TracyClient)

if (vsgXchange_FOUND)
    target_compile_definitions(vsgtracyinstrumentation PRIVATE vsgXchange_FOUND)
    target_link_libraries(vsgtracyinstrumentation vsgXchange::vsgXchange)
endif()

install(TARGETS vsgtracyinstrumentation RUNTIME DESTINATION bin)
