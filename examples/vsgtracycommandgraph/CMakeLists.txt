set(SOURCES
    TracingCommandGraph.cpp
    vsgtracycommandgraph.cpp
)

add_executable(vsgtracycommandgraph ${SOURCES})

target_include_directories(vsgtracycommandgraph PUBLIC ${VSGTRACY_SOURCE_DIR}/3rdparty/tracy/public)

target_link_libraries(vsgtracycommandgraph vsg::vsg Tracy::TracyClient)

if (vsgXchange_FOUND)
    target_compile_definitions(vsgtracycommandgraph PRIVATE vsgXchange_FOUND)
    target_link_libraries(vsgtracycommandgraph vsgXchange::vsgXchange)
endif()

install(TARGETS vsgtracycommandgraph RUNTIME DESTINATION bin)
