PULP_APP = test
PULP_APP_FC_SRCS = test.c
PULP_LDFLAGS += -lpibsp


PULP_PROPERTIES += pulp_chip_family pulp_chip
include $(PULP_SDK_HOME)/install/rules/pulp_properties.mk

FLASH_TYPE ?= HYPER_FLASH

PULP_CFLAGS = -O3 -g -DFLASH_TYPE=$(FLASH_TYPE) -Werror

ifeq '$(pulp_chip_family)' 'gap'
ifeq '$(FLASH_TYPE)' 'HYPER_FLASH'
USER_CONFIG = $(CURDIR)/config.ini
endif
endif

ifeq '$(pulp_chip)' 'wolfe'
USER_CONFIG = $(CURDIR)/config-wolfe.ini
endif

include $(PULP_SDK_HOME)/install/rules/pulp_rt.mk
