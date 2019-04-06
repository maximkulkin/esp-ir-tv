PROGRAM = esp-ir-insignia-tv
PROGRAM_SRC_DIR = main

EXTRA_COMPONENTS = \
	extras/http-parser \
	extras/dhcpserver \
	$(abspath components/ir) \
	$(abspath components/wifi-config) \
	$(abspath components/wolfssl) \
	$(abspath components/cjson) \
	$(abspath components/homekit)

FLASH_SIZE ?= 32

EXTRA_CFLAGS += -I../.. -DHOMEKIT_SHORT_APPLE_UUIDS

include $(SDK_PATH)/common.mk

monitor:
	$(FILTEROUTPUT) --port $(ESPPORT) --baud 115200 --elf $(PROGRAM_OUT)
