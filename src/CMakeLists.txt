zephyr_library()
zephyr_library_include_directories(.)
zephyr_include_directories(.)
zephyr_library_sources(amu.c)
zephyr_library_sources_ifdef(CONFIG_PEROVSAT_AMU_BACKEND_HARDWARE hardware_transfer.c)
zephyr_library_sources_ifdef(CONFIG_PEROVSAT_AMU_BACKEND_SIMULATION simulation_transfer.c)
zephyr_library_sources_ifdef(CONFIG_PEROVSAT_AMU_BACKEND_LIBRARY_MOCK lib_mock_transfer.c)
set(amu_LIB ${ZEPHYR_CURRENT_MODULE_DIR}/lib)
zephyr_library_sources_ifdef(CONFIG_PEROVSAT_AMU_BACKEND_HARDWARE
			     ${amu_LIB}/amu_lib.c)
zephyr_library_sources_ifdef(CONFIG_PEROVSAT_AMU_BACKEND_SIMULATION
			     ${amu_LIB}/amu_lib.c)
zephyr_library_sources_ifdef(CONFIG_PEROVSAT_AMU_BACKEND_LIBRARY_MOCK
			     ${amu_LIB}/amu_lib.c)
# amu_lib.h declares iv_sweep_t, which amu.h's public API needs regardless of
# backend, so this include path is always exposed even though amu_lib.c
# (the NDA-restricted implementation) is only compiled for the backends above.
zephyr_include_directories(${amu_LIB})
