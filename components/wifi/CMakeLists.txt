sdk_generate_library(oblfr_wifi)
sdk_add_include_directories(include)

#Prebuilt Wifi includes
sdk_add_include_directories(prebuilt_libs/wifi/include)
#Prebuilt Wifi links
sdk_add_link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/prebuilt_libs/wifi/lib/libwifi.a ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt_libs/bl606p_phyrf/lib/libbl606p_phyrf.a )

sdk_library_add_sources(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/oblfr_wifi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wifi_ops_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bl_os_hal.c

    #Embedded Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/utils_crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/utils_list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/utils_tlv_bl.c

    #Bits needed from the original driver
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wifi_hosal/wifi_hosal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bl60x_drvr/bl_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bl60x_drvr/bl_wifi_tx.c
)
