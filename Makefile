#!/usr/bin/make -f

RXT_SOURCE_PATH ?=
BUILDDIR := build
TMP := $(shell mktemp -d)

all: garden

$(BUILDDIR):
	mkdir -p $@

garden: $(BUILDDIR)
	cmake -S $(CURDIR) -B $(BUILDDIR) -D RXT_AS_SUBDIR=$(shell readlink -f $(RXT_SOURCE_PATH))
	cmake --build $(BUILDDIR) -t garden

tags:
	ctags -Re src

.PHONY: garden tags
