#-----------------------------------------
# @Author: YNJin YNJin0902@icloud.com
# @Date: 2026-03-23
# @LastEditors: YNJin YNJin0902@icloud.com
#-----------------------------------------

VERSION ?= 0.0.0

TARGET := ./Application/Panel-Detection_$(VERSION)

RKNN_LIB := ./runtime

CC := aarch64-linux-gnu-gcc
# for GTK
PKG_CONFIG := pkg-config

CFLAGS := -Iinclude \
		  -Wall -Wextra -g \
		  $(shell $(PKG_CONFIG) --cflags gtk+-3.0) \
		  $(shell pkg-config --cflags libdrm) \
		  -I/usr/include/libusb-1.0

LDFLAGS := -L/usr/lib/aarch64-linux-gnu -lusb-1.0 \
			-L$(RKNN_LIB) -lrknnrt \
			$(shell $(PKG_CONFIG) --libs gtk+-3.0) \
			$(shell pkg-config --libs libdrm) \
			-lm

# ---------------- Sources ----------------
SRC_DIR := src
SRCS    := main.c $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(SRCS:.c=.o)

# ---------------- Rules ----------------
all: $(TARGET)
	@echo "Building version: $(VERSION)"

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
