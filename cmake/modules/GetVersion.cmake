function(get_version OUTPUT)
    set(MESSAGE_ERROR_PREFIX "[get_version] could not determine project version:")
    set(VERSION_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
    if(EXISTS ${VERSION_FILE_PATH})
        message(STATUS "[get_version] reading version from VERSION file")
        file(READ ${VERSION_FILE_PATH} VERSION_STRING)
    else()
        message(STATUS "[get_version] no VERSION file found. getting version from `git describe`")
        find_package(Git)
        if(Git_FOUND)
            execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags
                RESULT_VARIABLE GIT_DESCRIBE_RESULT
                OUTPUT_VARIABLE GIT_DESCRIBE_OUTPUT)
            if(${GIT_DESCRIBE_RESULT} EQUAL "0")
                string(STRIP ${GIT_DESCRIBE_OUTPUT} VERSION_STRING)
            else()
                message(SEND_ERROR "${MESSAGE_ERROR_PREFIX} an error occurred when executing `git describe`")
                set(VERSION_STRING "v0.0.0")
            endif()
        else()
            message(SEND_ERROR "${MESSAGE_ERROR_PREFIX} git is not available")
            set(VERSION_STRING "v0.0.0")
        endif()
    endif()
    
    set(VERSION_REGEX "^v[0-9]+\\.[0-9]+\\.[0-9]+.*")
    string(REGEX MATCH ${VERSION_REGEX} VERSION_REGEX_MATCH_RESULT ${VERSION_STRING})
    if(VERSION_REGEX_MATCH_RESULT STREQUAL ${VERSION_STRING})
        string(REGEX REPLACE "^v([0-9]+).*" "\\1" VERSION_MAJOR ${VERSION_STRING})
        string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR ${VERSION_STRING})
        string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH ${VERSION_STRING})
        set(${OUTPUT} "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" PARENT_SCOPE)
    else()
        message(SEND_ERROR "${MESSAGE_ERROR_PREFIX} `${VERSION_STRING}` is not a valid version. The version must match the regex `${VERSION_REGEX}`!")
        set(${OUTPUT} "0.0.0" PARENT_SCOPE)
    endif()
endfunction()