PULP_PROPERTIES += udma/cpi/version pulp_chip_family

include $(TARGET_INSTALL_DIR)/rules/pulp_properties.mk

INSTALL_FILES += $(shell find include -name *.h)

ifeq '$(pulp_chip_family)' 'gap'
PULP_LIBS += pibsp_gapoc_a pibsp_gapuino pibsp_ai_deck
endif

PULP_LIB_FC_SRCS_pibsp_gapoc_a = $(GAPOC_A_SRC)
PULP_LIB_TARGET_NAME_pibsp_gapoc_a = gapoc_a/libpibsp.a
PULP_LIB_CFLAGS_pibsp_gapoc_a = -DCONFIG_GAPOC_A

PULP_LIB_FC_SRCS_pibsp_gapuino = $(GAPUINO_SRC)
PULP_LIB_TARGET_NAME_pibsp_gapuino = gapuino/libpibsp.a
PULP_LIB_CFLAGS_pibsp_gapuino = -DCONFIG_GAPUINO

PULP_LIB_FC_SRCS_pibsp_ai_deck = $(AI_DECK_SRC)
PULP_LIB_TARGET_NAME_pibsp_ai_deck = ai_deck/libpibsp.a
PULP_LIB_CFLAGS_pibsp_ai_deck = -DCONFIG_AI_DECK

PULP_CFLAGS += -Os -g -I$(CURDIR)/include
PULP_CFLAGS += -Wextra -Wall -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wundef

include $(PULP_SDK_HOME)/install/rules/pulp.mk
