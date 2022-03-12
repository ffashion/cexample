project-y := $(filter-out ./Makefile, $(shell find -name "Makefile"))
all: $(project-y)
$(project-y):
	cd $(dir $@); make -f ./Makefile
.PHONY: $(project-y)
