# math/Makefile
#
# Build the math library (math/): exact rational arithmetic over Q, Q[x],
# Q(beta), and Q^(n x n); Sturm sequencing and root isolation; exact Pisot
# classification (Tier 0 through Tier 3.5 of TOOLS.md); plus
# the vendored-but-native mini-gmp and mini-mpq arbitrary-precision
# integer and rational arithmetic.
#
# This is a proper C/C++ sub-library.  It produces libmath.a (static)
# and a test binary math_test that exercises every tier.  The main
# project links against libmath.a for the Sturm/Pisot primitives and
# for mini-gmp; nothing else in the main project has a direct
# dependency on mini-gmp except via libmath.a.
#
# See ../docs/CPP_DESIGN_PHILOSOPHY.md and math/TOOLS.md for the
# design rationale.

CXX      ?= g++
CC       ?= gcc
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic -Wno-error -fPIC
CFLAGS   ?= -std=c99 -O2 -Wall -Wextra -Wpedantic -Wno-error -fPIC
AR       ?= ar
ARFLAGS  ?= rcs

INCDIR  := include
SRCDIR  := src
TESTDIR := tests
BUILDDIR := out

LIBNAME  := libmath.a
LIB      := $(BUILDDIR)/$(LIBNAME)

MATH_SRCS := $(SRCDIR)/exact_pisot.c
GMP_SRCS  := $(SRCDIR)/mini-gmp.c $(SRCDIR)/mini-mpq.c

MATH_OBJS := $(BUILDDIR)/exact_pisot.o
GMP_OBJS  := $(BUILDDIR)/mini-gmp.o $(BUILDDIR)/mini-mpq.o
OBJS      := $(MATH_OBJS) $(GMP_OBJS)

# Test binary: glues the tier test files together as one TU to avoid
# linker symbol collisions across tiers.  See math/TOOLS.md for the
# tier breakdown.
MATH_TEST_BIN     := $(BUILDDIR)/math_test
MATH_TEST_SRC     := $(TESTDIR)/mathlib_test.cpp
MATH_TEST_TIER_SRCS := \
	$(TESTDIR)/test_poly.cpp \
	$(TESTDIR)/test_qbeta.cpp \
	$(TESTDIR)/test_bezout.cpp \
	$(TESTDIR)/test_linalg_qbeta.cpp \
	$(TESTDIR)/test_sturm.cpp \
	$(TESTDIR)/test_in_h_sigma.cpp \
	$(TESTDIR)/test_exploded.cpp \
	$(TESTDIR)/test_mat_q.cpp \
	$(TESTDIR)/test_ball.cpp \
	$(TESTDIR)/test_bigfloat.cpp \
	$(TESTDIR)/test_charpoly.cpp \
	$(TESTDIR)/test_perron_frobenius.cpp \
	$(TESTDIR)/test_fft.cpp

MATH_HEADERS := \
	$(INCDIR)/math/bigint.hpp \
	$(INCDIR)/math/poly_z.hpp \
	$(INCDIR)/math/poly_q.hpp \
	$(INCDIR)/math/mat_q.hpp \
	$(INCDIR)/math/qbeta.hpp \
	$(INCDIR)/math/bezout.hpp \
	$(INCDIR)/math/linalg_qbeta.hpp \
	$(INCDIR)/math/sturm.hpp \
	$(INCDIR)/math/in_h_sigma.hpp \
	$(INCDIR)/math/ball.hpp \
	$(INCDIR)/math/bigfloat.hpp \
	$(INCDIR)/math/charpoly.hpp \
	$(INCDIR)/math/perron_frobenius.hpp \
	$(INCDIR)/exact_pisot.h \
	$(INCDIR)/mini-gmp/mini-gmp.h \
	$(INCDIR)/mini-gmp/mini-mpq.h

.PHONY: all lib check clean
all: lib check
lib: $(LIB)

$(BUILDDIR):
	@mkdir -p $@

$(BUILDDIR)/mini-gmp.o: $(SRCDIR)/mini-gmp.c $(INCDIR)/mini-gmp/mini-gmp.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR)/mini-gmp -c $< -o $@

$(BUILDDIR)/mini-mpq.o: $(SRCDIR)/mini-mpq.c $(INCDIR)/mini-gmp/mini-mpq.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR)/mini-gmp -c $< -o $@

$(BUILDDIR)/exact_pisot.o: $(SRCDIR)/exact_pisot.c $(INCDIR)/exact_pisot.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(INCDIR)/mini-gmp -c $< -o $@

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

# Test target: builds libmath.a and links the math_test driver against it.
check: $(MATH_TEST_BIN)
	./$(MATH_TEST_BIN)

$(MATH_TEST_BIN): $(MATH_TEST_SRC) $(MATH_TEST_TIER_SRCS) $(MATH_HEADERS) $(LIB) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(INCDIR)/mini-gmp \
		$(MATH_TEST_SRC) $(LIB) -o $@

clean:
	rm -rf $(BUILDDIR)
