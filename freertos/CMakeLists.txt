if (NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message("Skipping FreeRTOS examples as FREERTOS_KERNEL_PATH not defined")
    return()
endif()

include(FreeRTOS_Kernel_import.cmake)

set(TARGET_NAME niji_yoji_freertos)
add_executable(${TARGET_NAME}
    niji_yoji_freertos.c
    app_dev.c
    hub75.c
    fat_access.c
    ff14b-pico/ff.c
    ff14b-pico/ffsystem.c
    ff14b-pico/ffunicode.c
    ff14b-pico/mmc_pico_spi.c
    font/read_fontx2.c
    ${PICO_SDK_PATH}/src/rp2_common/hardware_adc/adc.c
    ${PICO_SDK_PATH}/src/rp2_common/hardware_pio/pio.c
    ${PICO_SDK_PATH}/src/rp2_common/hardware_spi/spi.c
    ${PICO_SDK_PATH}/src/common/hardware_claim/claim.c
    )
target_include_directories(${TARGET_NAME} PRIVATE
    .
    ./ff14b-pico
    ${PICO_SDK_PATH}/src/rp2_common/hardware_adc/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_gpio/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_pio/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_spi/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_uart/include
    ${PICO_SDK_PATH}/src/common/hardware_claim/include
    )
target_link_libraries(${TARGET_NAME} PRIVATE
    pico_stdlib
    FreeRTOS-Kernel-Heap4
    )
target_compile_definitions(${TARGET_NAME} PRIVATE
    configNUMBER_OF_CORES=1
    )
pico_add_extra_outputs(${TARGET_NAME})
