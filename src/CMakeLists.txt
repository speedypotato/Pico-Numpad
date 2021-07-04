add_executable(pico-numpad
        pico-numpad.c
        usb_descriptors.c)

target_include_directories(pico-numpad PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_sources(pico-numpad PRIVATE pico-numpad.c)

target_link_libraries(pico-numpad PRIVATE
        pico_stdlib
        tinyusb_device
        tinyusb_board
        hardware_pio
        hardware_dma
        hardware_irq)

pico_add_extra_outputs(pico-numpad)