export GAPUINO_SRC
export GAPOC_A_SRC
export AI_DECK_SRC

all:
ifdef PULPOS
	make -f rules/pulpos.mk build install
endif

clean:
ifdef PULPOS
	make -f rules/pulpos.mk clean
endif

header:
ifdef PULPOS
	make -f rules/pulpos.mk header
endif
ifdef SRC
	mkdir -p $(TARGET_INSTALL_DIR)/src/pmsis_bsp
	cp -r bsp camera display flash include zephyr $(TARGET_INSTALL_DIR)/src/pmsis_bsp
endif