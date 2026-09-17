zephyr_library()
zephyr_library_include_directories(.)
zephyr_library_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)
zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)

# Packaging. Do not add device functions to these files.
zephyr_library_sources(amu.c)

zephyr_library_sources_ifdef(CONFIG_PEROVSAT_AMU_BACKEND_HARDWARE hardware_transfer.c)
zephyr_library_sources_ifdef(CONFIG_PEROVSAT_AMU_BACKEND_LIBRARY_MOCK lib_mock_transfer.c)

# Device API: mock xor library, same symbols (amu_init, …).
zephyr_library_sources_ifdef(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK
			     ${ZEPHYR_CURRENT_MODULE_DIR}/mock/amu.c)

set(amu_LIB ${ZEPHYR_CURRENT_MODULE_DIR}/lib)

if(CONFIG_PEROVSAT_AMU_BACKEND_HARDWARE OR CONFIG_PEROVSAT_AMU_BACKEND_LIBRARY_MOCK)
	zephyr_library_include_directories(${amu_LIB})
	zephyr_library_sources(${amu_LIB}/amu.c)
endif()
