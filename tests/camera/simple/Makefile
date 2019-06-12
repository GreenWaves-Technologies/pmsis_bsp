APP = test
SRCS += test.c
CFLAGS += -O3 -g
USE_PMSIS_BSP=1

RUNNER_CONFIG = $(CURDIR)/config.ini

override runner_args += --config-opt=camera/image-stream=$(CURDIR)/imgTest0.pgm
override runner_args += --config-opt=runner/peripherals=true


include $(PULP_SDK_HOME)/install/rules/pmsis.mk
