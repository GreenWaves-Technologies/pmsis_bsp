PULP_APP = test
PULP_APP_FC_SRCS = test.c
PULP_CFLAGS = -O3 -g #-DFPGA
PULP_LDFLAGS += -lpibsp -lrt -lpibsp

override CONFIG_OPT += camera/image-stream=$(CURDIR)/imgTest0.pgm
#override CONFIG_OPT += camera/image-stream=$(CURDIR)/imgTest%d.pgm
override CONFIG_OPT += runner/peripherals=true

USER_CONFIG = $(CURDIR)/config.ini

include $(PULP_SDK_HOME)/install/rules/pulp_rt.mk
