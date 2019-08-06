ifndef GAP_SDK_HOME
PULP_PROPERTIES += udma/cpi/version pulp_chip_family
include $(TARGET_INSTALL_DIR)/rules/pulp_properties.mk
endif

VEGA_SRC = \
  bsp/vega.c \
  camera/camera.c \
  camera/himax/himax.c \
  flash/flash.c \
  fs/read_fs/read_fs.c \
  flash/hyperflash/hyperflash.c \
  ram/hyperram/hyperram.c \
  ram/ram.c \
  ram/alloc_extern.c

WOLFE_SRC = \
  bsp/wolfe.c \
  camera/camera.c \
  camera/himax/himax.c \
  flash/flash.c \
  fs/read_fs/read_fs.c \
  flash/hyperflash/hyperflash.c \
  ram/hyperram/hyperram.c \
  ram/ram.c \
  ram/alloc_extern.c

GAPUINO_SRC = \
  bsp/gapuino.c \
  camera/camera.c \
  camera/himax/himax.c \
  display/display.c \
  display/ili9341/ili9341.c \
  flash/flash.c \
  fs/read_fs/read_fs.c \
  flash/hyperflash/hyperflash.c \
  ram/hyperram/hyperram.c \
  ram/ram.c \
  ram/alloc_extern.c

AI_DECK_SRC = \
  bsp/ai_deck.c \
  camera/camera.c \
  camera/himax/himax.c \
  flash/flash.c \
  flash/hyperflash/hyperflash.c \
  transport/transport.c \
  transport/nina_w10/nina_w10.c \
  ram/hyperram/hyperram.c \
  ram/ram.c \
  ram/alloc_extern.c

GAPOC_A_SRC = \
  bsp/gapoc_a.c \
  camera/camera.c \
  camera/mt9v034/mt9v034.c \
  flash/flash.c \
  fs/read_fs/read_fs.c \
  flash/hyperflash/hyperflash.c \
  fs/read_fs/read_fs.c \
  transport/transport.c \
  display/display.c \
  display/ili9341/ili9341.c \
  transport/nina_w10/nina_w10.c \
  ram/hyperram/hyperram.c \
  ram/ram.c \
  ram/alloc_extern.c \
  ble/nina_b112/nina_b112.c


ifdef GAP_SDK_HOME
include $(CURDIR)/rules/gap_sdk.mk
else
include $(CURDIR)/rules/pulp_sdk.mk
endif
