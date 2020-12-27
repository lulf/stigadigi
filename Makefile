MODULES=firmware backend

all: $(MODULES)

$(MODULES):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(MODULES)
