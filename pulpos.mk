PULP_PROPERTIES += udma/cpi/version pulp_chip_family

include $(TARGET_INSTALL_DIR)/rules/pulp_properties.mk

include common.mk

INSTALL_FILES += $(shell find include -name *.h)

ifeq '$(pulp_chip_family)' 'gap'
PULP_LIBS += pibsp_gapoc_a pibsp_gapuino
endif

PULP_LIB_FC_SRCS_pibsp_gapoc_a = $(gapoc_a_SRC)
PULP_LIB_TARGET_NAME_pibsp_gapoc_a = gapoc_a/libpibsp.a
PULP_LIB_CFLAGS_pibsp_gapoc_a = -DCONFIG_GAPOC_A

PULP_LIB_FC_SRCS_pibsp_gapuino = $(gapuino_SRC)
PULP_LIB_TARGET_NAME_pibsp_gapuino = gapuino/libpibsp.a
PULP_LIB_CFLAGS_pibsp_gapuino = -DCONFIG_GAPUINO

PULP_CFLAGS += -Os -g -Werror -Wall -I$(CURDIR)/include

include $(PULP_SDK_HOME)/install/rules/pulp.mk
