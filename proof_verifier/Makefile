# Zero Knowledge Verifier Makefile

# Toolchain & Paths
CC ?= $(CROSS_COMPILE)gcc
WOLFSSL_INSTALL_DIR ?= /usr/local

# Compiler/Linker flags
CFLAGS   += -Wall -I$(WOLFSSL_INSTALL_DIR)/include
LDFLAGS  += -Wl,--hash-style=gnu
LIBS     += -L$(WOLFSSL_INSTALL_DIR)/lib -lwolfssl -lm

# Optional debug/optimization
DEBUG_FLAGS = -g -DDEBUG
OPTIMIZE    = -Os
# Uncomment to use optimization
# CFLAGS += $(OPTIMIZE)

# Binary target
TARGETS = proof_verifier

.PHONY: all clean debug install

# Default build
all: $(TARGETS)

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: all

# Build rule
proof_verifier: proof_verifier.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS) $(LIBS)

# Clean rule
clean:
	rm -f $(TARGETS)

# Install to /usr/bin or DESTDIR (for Buildroot)
install: $(TARGETS)
	install -d $(DESTDIR)/usr/bin
	install -m 0755 $(TARGETS) $(DESTDIR)/usr/bin/
