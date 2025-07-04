add_subdirectory(source)
add_subdirectory(include/libpreprocessor)

add_library(${PROJECT_NAME} STATIC ${LibPreprocessor_SourceFiles})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME} PREFIX "")

target_compile_definitions(
    ${PROJECT_NAME} PRIVATE
        PREFIX_ERROR="[${PROJECT_NAME}::Runtime/error]"
        PREFIX_WARN="[${PROJECT_NAME}::Runtime/warn]"
        PREFIX_INFO="[${PROJECT_NAME}::Runtime/info]"
)

if (ENABLE_CLANGTIDY)
    enable_clang_tidy(${PROJECT_NAME})
endif()

if (ENABLE_CPPCHECK)
    enable_cppcheck(${PROJECT_NAME})
endif()

target_include_directories(${PROJECT_NAME}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/libpreprocessor"
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_sources(${PROJECT_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${LibPreprocessor_HeaderFiles})

export(TARGETS      ${PROJECT_NAME}
       FILE         ${EXPORT_DIR}/${PROJECT_NAME}Targets.cmake
       NAMESPACE    ${PROJECT_NAME}::
)

install(TARGETS     ${PROJECT_NAME}
        EXPORT      ${PROJECT_NAME}Targets
        ARCHIVE
        FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT      ${PROJECT_NAME}Targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}/cmake
        NAMESPACE   ${PROJECT_NAME}::
)

install(FILES ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}/cmake)

target_link_options(${PROJECT_NAME} PRIVATE ${LibPreprocessor_LinkerOptions})
target_compile_options(${PROJECT_NAME} PRIVATE ${LibPreprocessor_CompilerOptions})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LibPreprocessor_ExternalLibraries})
