TOPTARGETS = all clean

SUBDIRS = src doc

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

build:
	$(MAKE) -C src

install: build
	install -d bin
	install -p -m "0755" "src/opcit" "bin"

.PHONY: $(TARGETS) $(SUBDIRS)
