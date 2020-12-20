#!/bin/bash
esptool.py write_flash 0x0 ../bin/eagle.flash.bin 0x10000 ../bin/eagle.irom0text.bin 0x3fb000 ../bin/blank.bin 0x3fe000 ../bin/blank.bin 0x3fc000 ../bin/esp_init_data_default_v08.bin

#esptool.py write_flash 0x0 ../bin/eagle.flash.bin 0x10000 ../bin/eagle.irom0text.bin 0xfb000 ../bin/blank.bin 0xfe000 ../bin/blank.bin 0xfc000 ../bin/esp_init_data_default_v08.bin
