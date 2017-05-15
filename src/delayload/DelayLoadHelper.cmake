# DelayLoadHelper macros.
MACRO(SET_DELAY_LOAD_FLAGS)
IF(MSVC)
	UNSET(DL_DEBUG_FLAGS)
	FOREACH(_dll zlib1d libpng16d jpeg62d tinyxml2d)
		SET(DL_DEBUG_FLAGS "${DL_DEBUG_FLAGS} /DELAYLOAD:${_dll}.dll")
	ENDFOREACH()

	UNSET(DL_RELEASE_FLAGS)
	FOREACH(_dll zlib1 libpng16 jpeg62 tinyxml2)
		SET(DL_RELEASE_FLAGS "${DL_RELEASE_FLAGS} /DELAYLOAD:${_dll}.dll")
	ENDFOREACH()

	SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${DL_DEBUG_FLAGS}")
	SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${DL_DEBUG_FLAGS}")
	SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} ${DL_DEBUG_FLAGS}")

	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE    "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${DL_RELEASE_FLAGS}")
	SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${DL_RELEASE_FLAGS}")
	SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} ${DL_RELEASE_FLAGS}")
ENDIF(MSVC)
ENDMACRO(SET_DELAY_LOAD_FLAGS)
