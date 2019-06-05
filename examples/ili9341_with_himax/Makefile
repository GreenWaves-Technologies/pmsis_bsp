BOARD ?= gapuino

PULP_APP = test
PULP_APP_FC_SRCS = test.c
PULP_CFLAGS += -O2
PULP_LDFLAGS += -lpibsp -lrt

ifeq '$(BOARD)' 'gapuino'
PULP_LDFLAGS += -lpibsp_gapuino
PULP_CFLAGS = -DGAPUINO -DHIMAX -DUSE_BRIDGE
endif

ifeq '$(BOARD)' 'gapoc_a'
PULP_LDFLAGS += -lpibsp_gapoc_a
PULP_CFLAGS = -DGAPOC_A -DUSE_BRIDGE
endif

USER_CONFIG = $(CURDIR)/config.ini

include $(PULP_SDK_HOME)/install/rules/pulp_rules.mk

