INSTALL_FILES += $(shell find include -name *.h)

ifeq '$(pulp_chip_family)' 'gap'
PULP_LIBS += pibsp_gapoc_a pibsp_gapuino
endif

PULP_LIB_FC_SRCS_pibsp_gapoc_a = $(GAPOC_A_SRC)
PULP_LIB_TARGET_NAME_pibsp_gapoc_a = gapoc_a/libpibsp.a
PULP_LIB_CFLAGS_pibsp_gapoc_a = -DCONFIG_GAPOC_A

PULP_LIB_FC_SRCS_pibsp_gapuino = $(GAPUINO_SRC)
PULP_LIB_TARGET_NAME_pibsp_gapuino = gapuino/libpibsp.a
PULP_LIB_CFLAGS_pibsp_gapuino = -DCONFIG_GAPUINO

PULP_CFLAGS += -Os -g -Werror -Wall -I$(CURDIR)/include

include $(PULP_SDK_HOME)/install/rules/pulp.mk
