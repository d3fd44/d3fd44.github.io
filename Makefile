RAYLIB_PATH    ?= $(HOME)/etc/raylib
RAYLIB_NATIVE  ?= $(RAYLIB_PATH)/raylib-6.2_linux_amd64
RAYLIB_WEB     ?= $(RAYLIB_PATH)/raylib-6.2_webassembly

SRC            := main.c
BUILD_DIR      ?= build
TARGET         := $(BUILD_DIR)/bg
WEB_TARGET     := $(BUILD_DIR)/bg.js

CC              = gcc
CFLAGS         ?= -O2 -g
WARNINGS       := -Wall -Wextra
NATIVE_INCLUDE := -I$(RAYLIB_NATIVE)/include
NATIVE_LIB     := $(RAYLIB_NATIVE)/lib/libraylib.a
NATIVE_LDLIBS  := -lGL -lm -lpthread -ldl -lrt -lX11

EMCC           ?= emcc
WEB_INCLUDE    := -I$(RAYLIB_WEB)/include
WEB_LIB        := $(RAYLIB_WEB)/lib/libraylib.web.a
EMCC_CFLAGS    := -Os -Wall -Wextra -DPLATFORM_WEB
EMCC_LDFLAGS   := \
    -sUSE_GLFW=3                    \
    -sGL_ENABLE_GET_PROC_ADDRESS=1  \
    -sINITIAL_MEMORY=67108864       \
    -sINCOMING_MODULE_JS_API=canvas \
    -sASSERTIONS=1

.PHONY: all native run web wasm clean


default native: $(TARGET)

all: native wasm

web wasm: $(WEB_TARGET)

$(BUILD_DIR):
	mkdir -p $@

$(TARGET): $(SRC) $(NATIVE_LIB) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(WARNINGS) $(NATIVE_INCLUDE) -o $@ $(SRC) $(NATIVE_LIB) $(NATIVE_LDLIBS)

$(WEB_TARGET): $(SRC) $(WEB_LIB) | $(BUILD_DIR)
	$(EMCC) $(EMCC_CFLAGS) $(WEB_INCLUDE) -o $@ $(SRC) $(WEB_LIB) $(EMCC_LDFLAGS)

run: native
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

