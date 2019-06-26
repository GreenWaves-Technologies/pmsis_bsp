ifndef GAP_SDK_HOME
PULP_PROPERTIES += udma/cpi/version pulp_chip_family
include $(TARGET_INSTALL_DIR)/rules/pulp_properties.mk
endif

GAPUINO_SRC = \
  bsp/gapuino.c \
  camera/camera.c \
  camera/himax/himax.c \
  display/display.c \
  display/ili9341/ili9341.c \
  flash/flash.c \
  flash/hyperflash/hyperflash.c

AI_DECK_SRC = \
  bsp/ai_deck.c \
  camera/camera.c \
  camera/himax/himax.c \
  flash/flash.c \
  flash/hyperflash/hyperflash.c \
  transport/transport.c \
  transport/nina_w10/nina_w10.c

GAPOC_A_SRC = \
  bsp/gapoc_a.c \
  camera/camera.c \
  camera/mt9v034/mt9v034.c \
  flash/flash.c \
  flash/hyperflash/hyperflash.c


ifdef GAP_SDK_HOME
include $(CURDIR)/rules/gap_sdk.mk
else
include $(CURDIR)/rules/pulp_sdk.mk
endif
