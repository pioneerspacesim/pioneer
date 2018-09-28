#!/usr/bin/make -f

APP := net.pioneerspacesim.Pioneer
SRC := $(APP).json
TARGET := Pioneer.flatpak

.PHONY: all clean distclean

all: $(TARGET)

$(TARGET): repo
	flatpak build-bundle $< $@ $(APP)

repo: build
	flatpak build-export $@ $<
	flatpak build-update-repo $@

build: clean
	flatpak-builder $@ $(SRC)

clean:
	rm -rf $(TARGET) build repo

distclean: clean
	rm -rf .flatpak-builder
