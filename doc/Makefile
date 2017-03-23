ASCIIDOC ?= asciidoc
XMLTO ?= xmlto

all: opcit.1

%.1.xml: %.1.txt
	$(ASCIIDOC) -b docbook -d manpage $<

%.1: %.1.xml
	$(XMLTO) man $<

clean:
	rm -rf *.xml *.1

.PHONY: all clean
