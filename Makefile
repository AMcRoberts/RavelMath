# Makefile - SpectreTileProject2 (top-level)
#
# Top-level build orchestration for the C++ side of the project.  See
# docs/CPP_DESIGN_PHILOSOPHY.md for the design conventions; this file
# is the entry point that wires them together.
#
# Layout (post-reorganization):
#   include/ravel/   - public C++ headers (the ravel library)
#   src/               - non-header-only C++ (rauzy_fractal, lua_bindings)
#   app/               - main()-bearing C++ programs
#   tests/             - C++ test binaries (paired with their header)
#   math/              - math library subproject (proper C++ library with
#                        its own Makefile; produces math/out/libmath.a)
#   lua/               - Lua package root (lua/lua_src/ravel/, unchanged)
#   vendor/            - (none; mini-gmp is treated as native project code
#                        and lives in math/include/mini-gmp/)
#
# Usage:
#   make              - build everything and run all C++ tests (no Lua)
#   make check        - build everything, run all C++ tests AND Lua tests
#   make math         - build just the math library
#   make apps         - build the apps in app/
#   make tests        - build the tests in tests/
#   make lua          - build the Lua shared library + Lua data pipeline
#   make clean        - remove build artifacts
#
# Long-running drivers (gkw_sturm_certify, gkw_fully_rigorous) are NOT
# in `make all` or `make check` -- build and run them by hand.  See
# docs/GKW_SCRIPTS_NOTE.md for the rationale.

CXX      ?= g++
CC       ?= gcc
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic -Wno-error -fPIC
CXXFLAGS += -MMD -MP
CFLAGS   ?= -std=c99 -O2 -Wall -Wextra -Wpedantic -Wno-error -fPIC
LUA_PKG  := lua5.4
LUA_CFLAGS := $(shell pkg-config --cflags $(LUA_PKG))
LUA_LIBS   := $(shell pkg-config --libs $(LUA_PKG))

# Directories.
BUILDDIR     := out
MATH_DIR     := math
MATH_BUILDDIR := $(MATH_DIR)/out
MATH_LIB     := $(MATH_BUILDDIR)/libmath.a

INCDIR       := include
MATH_INCDIR  := $(MATH_DIR)/include
MATH_INCGMPL := $(MATH_DIR)/include/mini-gmp
SRCDIR       := src
APPDIR       := app
TESTDIR      := tests

# Common include flags for anything in app/ or tests/ that uses the
# ravel + math libraries.
INCLUDES := -I$(INCDIR) -I$(MATH_INCDIR) -I$(MATH_INCGMPL)
LIBS    := $(MATH_LIB)

LIBNAME  := spectre_native.so
LIB      := $(BUILDDIR)/$(LIBNAME)

# Main project non-header-only C++ impls.
SRCS    := $(SRCDIR)/lua_bindings.cpp $(SRCDIR)/rauzy_fractal.cpp
OBJECTS := $(BUILDDIR)/lua_bindings.o $(BUILDDIR)/rauzy_fractal.o

# Lua data pipeline.
DATA_DIR         := lua/lua_src/ravel/data
ENRICHED_DIR     := $(DATA_DIR)/specmap_enriched
TRANSITIONS_DIR  := $(DATA_DIR)/spectre_transitions
SPECTRE_KINDS    := G D J L X P S F Y
ENRICHED_FILES   := $(addprefix $(ENRICHED_DIR)/specmap_,$(addsuffix .txt,$(SPECTRE_KINDS)))
TRANSITION_FILES := $(addprefix $(TRANSITIONS_DIR)/spectre_transitions_,$(addsuffix .txt,$(SPECTRE_KINDS)))

# ====================================================================
# Phony targets
# ====================================================================

.PHONY: all build check clean
.PHONY: math apps tests lua data tables lean-check
.PHONY: d_cont_check_test ambient_graph_test corona_test
.PHONY: contact_boundary_test exact_pisot_test spectral_general_test
.PHONY: survey_test qbeta_eigenvalue_test qbeta_eigenvalue
.PHONY: rational_transcendentals_test gkw_sturm_certify gkw_fully_rigorous
.PHONY: test_qbasis validate_exploded test_12_exploded contact_boundary_4x4
.PHONY: cylinder_measure verify_exploded_12 test_general_spectral
.PHONY: prefix_automaton_test padic_test dedekind_factorization_test
.PHONY: ideal_arithmetic_test coincidence_and_property_f_test local_field_test graph_divisor_test maximal_order_test gb_bp_hop_rule_test
.PHONY: rnd13_factor_probe rnd13_prefix_automaton_probe classify_adelic_tiling test_bp_gb_divisor adelic_boundary_spectral_radius gb_bp_involution_check
.PHONY: class_ii_symmetry_probe class_ii_boundary_family_test substitution_neighborhood_test
.PHONY: class_ii_corona_literature_probe
.PHONY: class_ii_neighbor_probe
.PHONY: return_contact_lift_probe
.PHONY: class_ii_terminal_transport_probe
.PHONY: class_ii_bp_family_probe
.PHONY: csy_carry_automaton_test
.PHONY: csy_finite_carry_automaton_test
.PHONY: lean_class_ii_catalogue_cross_check_test

all: build data apps tests
build: $(LIB)
lua: $(LIB) data

# ====================================================================
# Math library (delegated)
# ====================================================================

math: $(MATH_LIB)
	$(MAKE) -C $(MATH_DIR) check

LEAN_ENV ?= ../LEAN/free_involution_perron/free_involution_perron
lean-check:
	cd $(LEAN_ENV) && lake env lean $(abspath lean/free_involution_perron_core.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/return_contact_lift.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/bp_correction_determinant.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_affine_shells.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_neighbor_dominance.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_neighbor2_extensions.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_terminal_shells.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_balanced_pivot.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_neighbor_d_support.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_six_vertex_graduation.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_global_round_partition.lean)
	@if rg -n '^[[:space:]]*sorry([[:space:]]|$$)' \
		lean/free_involution_perron_core.lean \
		lean/return_contact_lift.lean \
		lean/bp_correction_determinant.lean \
		lean/class_ii_affine_shells.lean \
		lean/class_ii_neighbor_dominance.lean \
		lean/class_ii_neighbor2_extensions.lean \
		lean/class_ii_terminal_shells.lean \
		lean/class_ii_balanced_pivot.lean \
		lean/class_ii_neighbor_d_support.lean \
		lean/class_ii_six_vertex_graduation.lean \
		lean/class_ii_global_round_partition.lean; then \
		echo "ERROR: enrolled Lean file contains sorry"; exit 1; \
	else \
		echo "Enrolled Lean files are sorry-free."; \
	fi
$(MATH_LIB): $(wildcard $(MATH_DIR)/src/*.[ch]) \
		$(wildcard $(MATH_DIR)/include/math/*.hpp) \
		$(wildcard $(MATH_DIR)/include/mini-gmp/*.h)
	$(MAKE) -C $(MATH_DIR) lib

# ====================================================================
# Lua shared library
# ====================================================================

$(BUILDDIR):
	@mkdir -p $@

$(BUILDDIR)/lua_bindings.o: $(SRCDIR)/lua_bindings.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(LUA_CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDDIR)/rauzy_fractal.o: $(SRCDIR)/rauzy_fractal.cpp \
		$(INCDIR)/ravel/rauzy_fractal.hpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(LIB): $(OBJECTS) $(MATH_LIB)
	$(CXX) -shared -o $@ $(OBJECTS) $(LUA_LIBS) -L$(MATH_BUILDDIR) -lmath

# ====================================================================
# Data pipeline (Lua side)
# ====================================================================

data: $(ENRICHED_DIR) $(TRANSITIONS_DIR)
$(ENRICHED_DIR):
	@mkdir -p $@
$(TRANSITIONS_DIR):
	@mkdir -p $@

$(ENRICHED_DIR)/specmap_%.txt: lua/scripts/parse_specmap.lua
	lua5.4 lua/scripts/parse_specmap.lua

$(TRANSITIONS_DIR)/spectre_transitions_%.txt: \
		$(ENRICHED_DIR)/specmap_%.txt lua/scripts/build_spectre_transition_table.lua \
		$(DATA_DIR)/hex_outer_table.lua
	lua5.4 lua/scripts/build_spectre_transition_table.lua

tables: $(ENRICHED_FILES) $(TRANSITION_FILES)

# ====================================================================
# Apps (main()-bearing C++ programs)
# ====================================================================

# Default: build all the in-tree apps (those not marked slow/unfinished).
APPS_DEFAULT := \
	cylinder_measure \
	contact_boundary_4x4 \
	verify_exploded_12 \
	test_general_spectral \
	rational_transcendentals_test

apps: $(APPS_DEFAULT)

# cylinder_measure: empirical cylinder-measure / factorization driver.
# See app/cylinder_measure.cpp for the FIX 1/FIX 2 history (the
# exponential-blowup bug and the always-additive "factorization" check).
CYLINDER_MEASURE_BIN := $(BUILDDIR)/cylinder_measure
cylinder_measure: $(CYLINDER_MEASURE_BIN)
$(CYLINDER_MEASURE_BIN): $(APPDIR)/cylinder_measure.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# contact_boundary_4x4: full contact-boundary pipeline + in-process
# exact Q(beta) eigenvalue for the G_B adjacency matrix.
CONTACT_BOUNDARY_4X4_BIN := $(BUILDDIR)/contact_boundary_4x4
contact_boundary_4x4: $(CONTACT_BOUNDARY_4X4_BIN)
$(CONTACT_BOUNDARY_4X4_BIN): $(APPDIR)/contact_boundary_4x4.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# verify_exploded_12: forward-branch driver that uses the exact
# Sturm-based Pisot classifier on the 12 EXPLODED substitutions.  Kept
# as a small, independent check alongside tests/validate_exploded.cpp.
VERIFY_EXPLODED_12_BIN := $(BUILDDIR)/verify_exploded_12
verify_exploded_12: $(VERIFY_EXPLODED_12_BIN)
$(VERIFY_EXPLODED_12_BIN): $(APPDIR)/verify_exploded_12.cpp $(MATH_INCDIR)/exact_pisot.h | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) -I$(MATH_INCDIR) -I$(MATH_INCGMPL) $< $(MATH_LIB) -o $@

# test_general_spectral: quick smoke test for spectral_invariants_general.
# The canonical regression test is tests/spectral_general_test.cpp (which
# this app predates); keep this around as a fast visual check.
TEST_GENERAL_SPECTRAL_BIN := $(BUILDDIR)/test_general_spectral
test_general_spectral: $(TEST_GENERAL_SPECTRAL_BIN)
$(TEST_GENERAL_SPECTRAL_BIN): $(APPDIR)/test_general_spectral.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# rational_transcendentals_test: verifies rational_transcendentals.hpp
# (Machin's formula for pi, Taylor + Lagrange remainder for sin/cos)
# to 40 decimal places.
RATIONAL_TRANSCENDENTALS_TEST_BIN := $(BUILDDIR)/rational_transcendentals_test
rational_transcendentals_test: $(RATIONAL_TRANSCENDENTALS_TEST_BIN)
$(RATIONAL_TRANSCENDENTALS_TEST_BIN): $(APPDIR)/rational_transcendentals_test.cpp $(INCDIR)/ravel/rational_transcendentals.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# gkw_sturm_certify: Sturm-certified GKW eigenvalues for the M-node
# Chebyshev collocation matrix (default M=12).  NOT in `make all` because
# the per-run cost is several seconds and the result is a single fixed
# numeric fact; build and run on demand.  See docs/GKW_SCRIPTS_NOTE.md.
GKW_STURM_CERTIFY_BIN := $(BUILDDIR)/gkw_sturm_certify
gkw_sturm_certify: $(GKW_STURM_CERTIFY_BIN)
	./$(GKW_STURM_CERTIFY_BIN)
$(GKW_STURM_CERTIFY_BIN): $(APPDIR)/gkw_sturm_certify.cpp $(MATH_INCDIR)/exact_pisot.h | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) -I$(MATH_INCDIR) -I$(MATH_INCGMPL) $< $(MATH_LIB) -o $@

# gkw_fully_rigorous: the fully-rigorous (no floating point anywhere)
# version of the GKW matrix construction.  NOT in `make all` and not
# in `make check` -- it is unfinished (per SESSION_NOTES.md in the
# forward work branch: times out at M=8) and is left in the tree as a
# forward-step reference.  Build manually when working on it.
GKW_FULLY_RIGOROUS_BIN := $(BUILDDIR)/gkw_fully_rigorous
gkw_fully_rigorous: $(GKW_FULLY_RIGOROUS_BIN)
$(GKW_FULLY_RIGOROUS_BIN): $(APPDIR)/gkw_fully_rigorous.cpp $(INCDIR)/ravel/rational_transcendentals.hpp $(MATH_INCDIR)/exact_pisot.h | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# smooth_relaxation_search: prototype driver for smooth-relaxation on
# matrix entries (RESEARCH_VECTORS.md vector 2).  Uses spectral_dual
# for gradient computation.  NOT in `make all` -- a prototype, run by hand.
SMOOTH_RELAXATION_SEARCH_BIN := $(BUILDDIR)/smooth_relaxation_search
smooth_relaxation_search: $(SMOOTH_RELAXATION_SEARCH_BIN)
$(SMOOTH_RELAXATION_SEARCH_BIN): $(APPDIR)/smooth_relaxation_search.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) \
		$(wildcard $(MATH_INCDIR)/math/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# hyperplane_progression: hyperplane-first view of the Pisot polytope
# H_sigma for several small 3-letter Pisot substitutions.  Enumerates
# integer points in the stepped-hyperplane acceptance strip and
# tabulates width ratios and polytope sizes — see
# docs/HYPERPLANE_PROGRESSION_NOTES.md.  NOT in `make all` -- run by
# hand when comparing Pisot families.
HYPERPLANE_PROGRESSION_BIN := $(BUILDDIR)/hyperplane_progression
hyperplane_progression: $(HYPERPLANE_PROGRESSION_BIN)
$(HYPERPLANE_PROGRESSION_BIN): $(APPDIR)/hyperplane_progression.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) \
		$(wildcard $(MATH_INCDIR)/math/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# ====================================================================
# Tests (paired _test.cpp files; standalone binaries with main())
# ====================================================================

TESTS_DEFAULT := \
	d_cont_check_test \
	ambient_graph_test \
	corona_test \
	contact_boundary_test \
	exact_pisot_test \
	spectral_general_test \
	survey_test \
	qbeta_eigenvalue_test \
	test_qbasis \
	validate_exploded \
	test_12_exploded \
	prefix_automaton_test \
	padic_test \
	classify_adelic_test \
	dedekind_factorization_test \
	ideal_arithmetic_test \
	coincidence_and_property_f_test \
	local_field_test \
	graph_divisor_test \
	maximal_order_test \
	gb_bp_hop_rule_test \
	dual_format_test \
	dual_test \
	spectral_dual_test \
	weighted_digraph_file_test \
	involution_helpers_test \
	bp_dump_analysis_test \
	fibonacci_selection_test \
	fibonacci_finite_test \
	packed_binary_dynamics_test \
	return_substitution_test \
	return_contact_lift_test \
	class_ii_boundary_family_test \
	substitution_neighborhood_test \
	family_of_families_test \
	pisot_numeration_topology_test \
	openai_unit_distance_test \
	csy_carry_automaton_test \
	csy_finite_carry_automaton_test \
	class_ii_neighbor_d_matrix_test \
	lean_class_ii_catalogue_cross_check_test

tests: $(TESTS_DEFAULT)

# --- C++ test binaries ---

D_CONT_TEST_BIN := $(BUILDDIR)/d_cont_check_test
d_cont_check_test: $(D_CONT_TEST_BIN)
$(D_CONT_TEST_BIN): $(TESTDIR)/d_cont_check_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

AMBIENT_GRAPH_TEST_BIN := $(BUILDDIR)/ambient_graph_test
ambient_graph_test: $(AMBIENT_GRAPH_TEST_BIN)
$(AMBIENT_GRAPH_TEST_BIN): $(TESTDIR)/ambient_graph_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

CORONA_TEST_BIN := $(BUILDDIR)/corona_test
corona_test: $(CORONA_TEST_BIN)
$(CORONA_TEST_BIN): $(TESTDIR)/corona_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

CLASS_II_BOUNDARY_FAMILY_TEST_BIN := $(BUILDDIR)/class_ii_boundary_family_test
class_ii_boundary_family_test: $(CLASS_II_BOUNDARY_FAMILY_TEST_BIN)
$(CLASS_II_BOUNDARY_FAMILY_TEST_BIN): $(TESTDIR)/class_ii_boundary_family_test.cpp \
		$(INCDIR)/ravel/class_ii_boundary_family.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

SUBSTITUTION_NEIGHBORHOOD_TEST_BIN := $(BUILDDIR)/substitution_neighborhood_test
substitution_neighborhood_test: $(SUBSTITUTION_NEIGHBORHOOD_TEST_BIN)
$(SUBSTITUTION_NEIGHBORHOOD_TEST_BIN): $(TESTDIR)/substitution_neighborhood_test.cpp \
		$(INCDIR)/ravel/substitution_neighborhood.hpp \
		$(INCDIR)/ravel/class_ii_neighbor_family.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

CONTACT_BOUNDARY_TEST_BIN := $(BUILDDIR)/contact_boundary_test
contact_boundary_test: $(CONTACT_BOUNDARY_TEST_BIN)
$(CONTACT_BOUNDARY_TEST_BIN): $(TESTDIR)/contact_boundary_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

EXACT_PISOT_TEST_BIN := $(BUILDDIR)/exact_pisot_test
exact_pisot_test: $(EXACT_PISOT_TEST_BIN)
$(EXACT_PISOT_TEST_BIN): $(TESTDIR)/exact_pisot_test.cpp $(MATH_INCDIR)/exact_pisot.h | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) -I$(MATH_INCDIR) -I$(MATH_INCGMPL) $< $(MATH_LIB) -o $@

SPECTRAL_GENERAL_TEST_BIN := $(BUILDDIR)/spectral_general_test
spectral_general_test: $(SPECTRAL_GENERAL_TEST_BIN)
$(SPECTRAL_GENERAL_TEST_BIN): $(TESTDIR)/spectral_general_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

SURVEY_TEST_BIN := $(BUILDDIR)/survey_test
survey_test: $(SURVEY_TEST_BIN)
$(SURVEY_TEST_BIN): $(TESTDIR)/survey_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

QBETA_EIGENVALUE_TEST_BIN := $(BUILDDIR)/qbeta_eigenvalue_test
qbeta_eigenvalue_test: $(QBETA_EIGENVALUE_TEST_BIN)
$(QBETA_EIGENVALUE_TEST_BIN): $(TESTDIR)/qbeta_eigenvalue_test.cpp $(INCDIR)/ravel/qbeta_eigenvalue.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# CLI driver for exact Q(beta) eigenvalue (was lua/scripts/qbeta_eigenvalue.cpp
# pre-reorganization; kept as an app so the shell-orchestrated workflows
# scripts/run_qbeta_survey.sh drives can still invoke it).
QBETA_EIGENVALUE_BIN := $(BUILDDIR)/qbeta_eigenvalue
qbeta_eigenvalue: $(QBETA_EIGENVALUE_BIN)
$(QBETA_EIGENVALUE_BIN): $(APPDIR)/qbeta_eigenvalue.cpp $(INCDIR)/ravel/qbeta_eigenvalue.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# Self-test for qbasis.hpp: bit-exact Q(beta) primitives.
TEST_QBASIS_BIN := $(BUILDDIR)/test_qbasis
test_qbasis: $(TEST_QBASIS_BIN)
$(TEST_QBASIS_BIN): $(TESTDIR)/test_qbasis.cpp $(INCDIR)/ravel/qbasis.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# Validate the 12 EXPLODED entries from the 39-substitution survey.
# Uses the Sturm-based exact Pisot classifier (math/exact_pisot.c).
VALIDATE_EXPLODED_BIN := $(BUILDDIR)/validate_exploded
validate_exploded: $(VALIDATE_EXPLODED_BIN)
$(VALIDATE_EXPLODED_BIN): $(TESTDIR)/validate_exploded.cpp $(MATH_INCDIR)/exact_pisot.h | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) -I$(MATH_INCDIR) -I$(MATH_INCGMPL) $< $(MATH_LIB) -o $@

# Re-run the 12 EXPLODED candidates against the new exact in_H_sigma path.
TEST_12_EXPLODED_BIN := $(BUILDDIR)/test_12_exploded
test_12_exploded: $(TEST_12_EXPLODED_BIN)
$(TEST_12_EXPLODED_BIN): $(TESTDIR)/test_12_exploded.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

RAUZY_FRACTAL_TEST_BIN := $(BUILDDIR)/rauzy_fractal_test
rauzy_fractal_test: $(RAUZY_FRACTAL_TEST_BIN)
$(RAUZY_FRACTAL_TEST_BIN): $(TESTDIR)/rauzy_fractal_test.cpp \
		$(BUILDDIR)/rauzy_fractal.o
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

OPENAI_UNIT_DISTANCE_BIN := $(BUILDDIR)/openai_unit_distance_test
openai_unit_distance_test: $(OPENAI_UNIT_DISTANCE_BIN)
$(OPENAI_UNIT_DISTANCE_BIN): $(TESTDIR)/openai_unit_distance_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

LEAN_CLASS_II_CATALOGUE_CROSS_CHECK_TEST_BIN := $(BUILDDIR)/lean_class_ii_catalogue_cross_check_test
lean_class_ii_catalogue_cross_check_test: $(LEAN_CLASS_II_CATALOGUE_CROSS_CHECK_TEST_BIN)
$(LEAN_CLASS_II_CATALOGUE_CROSS_CHECK_TEST_BIN): $(TESTDIR)/lean_class_ii_catalogue_cross_check_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

CSY_FINITE_CARRY_AUTOMATON_TEST_BIN := $(BUILDDIR)/csy_finite_carry_automaton_test
csy_finite_carry_automaton_test: $(CSY_FINITE_CARRY_AUTOMATON_TEST_BIN)
$(CSY_FINITE_CARRY_AUTOMATON_TEST_BIN): $(TESTDIR)/csy_finite_carry_automaton_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

FAMILY_OF_FAMILIES_TEST_BIN := $(BUILDDIR)/family_of_families_test
family_of_families_test: $(FAMILY_OF_FAMILIES_TEST_BIN)
$(FAMILY_OF_FAMILIES_TEST_BIN): $(TESTDIR)/family_of_families_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

PISOT_NUMERATION_TOPOLOGY_TEST_BIN := $(BUILDDIR)/pisot_numeration_topology_test
pisot_numeration_topology_test: $(PISOT_NUMERATION_TOPOLOGY_TEST_BIN)
$(PISOT_NUMERATION_TOPOLOGY_TEST_BIN): $(TESTDIR)/pisot_numeration_topology_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# csy_carry_automaton_test: tests for the Carton-Sudbery-Yassawi
# carry automaton on Pisot numerations (arXiv:2606.30496).
# Header-only; depends only on include/adelic/csy_carry_automaton.hpp.
CSY_CARRY_AUTOMATON_TEST_BIN := $(BUILDDIR)/csy_carry_automaton_test
csy_carry_automaton_test: $(CSY_CARRY_AUTOMATON_TEST_BIN)
$(CSY_CARRY_AUTOMATON_TEST_BIN): $(TESTDIR)/csy_carry_automaton_test.cpp \
		$(INCDIR)/adelic/csy_carry_automaton.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_neighbor_d_matrix_test: boundary-layer support for the three
# Class-II adjacent-swap neighbor dominant cores.  Verifies the affine
# structure A(a) = A(3) + (a-3)*D from class_ii_neighbor_affine_edges,
# extracts D as the slope matrix, identifies boundary-layer roles,
# and uses the qexpansion machinery in math/qexpansion.hpp to
# cross-check the closed-form asymptotic structure.
CLASS_II_NEIGHBOR_D_MATRIX_TEST_BIN := $(BUILDDIR)/class_ii_neighbor_d_matrix_test
.PHONY: class_ii_neighbor_d_matrix_test
class_ii_neighbor_d_matrix_test: $(CLASS_II_NEIGHBOR_D_MATRIX_TEST_BIN)
$(CLASS_II_NEIGHBOR_D_MATRIX_TEST_BIN): $(TESTDIR)/class_ii_neighbor_d_matrix_test.cpp \
		$(INCDIR)/ravel/class_ii_neighbor_family.hpp \
		$(MATH_INCDIR)/math/qexpansion.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

CLASS_II_TERMINAL_TRANSPORT_PROBE_BIN := $(BUILDDIR)/class_ii_terminal_transport_probe
class_ii_terminal_transport_probe: $(CLASS_II_TERMINAL_TRANSPORT_PROBE_BIN)
$(CLASS_II_TERMINAL_TRANSPORT_PROBE_BIN): $(APPDIR)/class_ii_terminal_transport_probe.cpp \
		$(INCDIR)/ravel/class_ii_neighbor2_pruning.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

RETURN_SUBSTITUTION_TEST_BIN := $(BUILDDIR)/return_substitution_test
return_substitution_test: $(RETURN_SUBSTITUTION_TEST_BIN)
$(RETURN_SUBSTITUTION_TEST_BIN): $(TESTDIR)/return_substitution_test.cpp \
		$(INCDIR)/ravel/return_substitution.hpp \
		$(INCDIR)/ravel/substitution.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

RETURN_CONTACT_LIFT_TEST_BIN := $(BUILDDIR)/return_contact_lift_test
return_contact_lift_test: $(RETURN_CONTACT_LIFT_TEST_BIN)
$(RETURN_CONTACT_LIFT_TEST_BIN): $(TESTDIR)/return_contact_lift_test.cpp \
		$(INCDIR)/ravel/return_contact_lift.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

BP_DUMP_ANALYSIS_TEST_BIN := $(BUILDDIR)/bp_dump_analysis_test
bp_dump_analysis_test: $(BP_DUMP_ANALYSIS_TEST_BIN)
$(BP_DUMP_ANALYSIS_TEST_BIN): $(TESTDIR)/bp_dump_analysis_test.cpp \
		$(INCDIR)/ravel/bp_dump_analysis.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

FIBONACCI_SELECTION_TEST_BIN := $(BUILDDIR)/fibonacci_selection_test
fibonacci_selection_test: $(FIBONACCI_SELECTION_TEST_BIN)
$(FIBONACCI_SELECTION_TEST_BIN): $(TESTDIR)/fibonacci_selection_test.cpp \
		$(INCDIR)/ravel/fibonacci_selection.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

FIBONACCI_FINITE_TEST_BIN := $(BUILDDIR)/fibonacci_finite_test
fibonacci_finite_test: $(FIBONACCI_FINITE_TEST_BIN)
$(FIBONACCI_FINITE_TEST_BIN): $(TESTDIR)/fibonacci_finite_test.cpp \
		$(INCDIR)/ravel/fibonacci_finite.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

PACKED_BINARY_DYNAMICS_TEST_BIN := $(BUILDDIR)/packed_binary_dynamics_test
packed_binary_dynamics_test: $(PACKED_BINARY_DYNAMICS_TEST_BIN)
$(PACKED_BINARY_DYNAMICS_TEST_BIN): \
		$(TESTDIR)/packed_binary_dynamics_test.cpp \
		$(INCDIR)/ravel/packed_binary_dynamics.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# ====================================================================
# `make check` -- build everything and run all tests
# ====================================================================

# ====================================================================
# Check-run stamps: `make check` used to unconditionally re-execute
# every test/app/Lua binary on every invocation, regardless of what
# had actually changed (compilation was already incremental via the
# generated .d files; execution was not). Each stamp file re-runs its
# target only when the target's binary -- or, for Lua, its sources --
# is newer than the last passing run, mirroring ordinary Make
# incremental-build semantics. Run `make retest` to force a full
# re-run (clears all stamps without touching build artifacts).
# ====================================================================
STAMPDIR := $(BUILDDIR)/.stamps

$(STAMPDIR):
	@mkdir -p $(STAMPDIR)

TEST_STAMPS := $(addprefix $(STAMPDIR)/,$(TESTS_DEFAULT))
$(STAMPDIR)/%: $(BUILDDIR)/% | $(STAMPDIR)
	@echo "[check] $*"
	@./$(BUILDDIR)/$* && touch $@

# Only rational_transcendentals_test is actually executed as part of
# `make check` from APPS_DEFAULT; the others are build-only checks
# (already covered by the `apps` prerequisite of `all`).
APP_RUN_STAMPS := $(STAMPDIR)/rational_transcendentals_test

LUA_SOURCES := $(shell find lua -type f \( -name '*.lua' -o -name '*.txt' \) 2>/dev/null)
$(STAMPDIR)/lua_tests: $(LUA_SOURCES) $(LIB) | $(STAMPDIR)
	@echo "[check] lua tests"
	@cd lua && lua5.4 scripts/run_lua_tests.lua
	@touch $@

.PHONY: retest
retest:
	rm -f $(TEST_STAMPS) $(APP_RUN_STAMPS) $(STAMPDIR)/lua_tests
	@echo "Cleared check-run stamps; next 'make check' re-runs everything."

check: all $(LIB)
	@$(MAKE) -C $(MATH_DIR) check
	@echo "--- Running C++ tests ---"
	@$(MAKE) $(TEST_STAMPS)
	@echo "--- Running app verifications ---"
	@$(MAKE) $(APP_RUN_STAMPS)
	@echo "--- Running Lua tests ---"
	@$(MAKE) $(STAMPDIR)/lua_tests
	@echo ""
	@echo "All checks passed."

# ====================================================================
# Adelic classifier tests (the new prefix_automaton, padic, and
# dedekind_factorization tests).  These test the building blocks of
# the adelic tiling classifier (the spec'd work item).
# ====================================================================

PREFIX_AUTOMATON_TEST_BIN := $(BUILDDIR)/prefix_automaton_test
prefix_automaton_test: $(PREFIX_AUTOMATON_TEST_BIN)
$(PREFIX_AUTOMATON_TEST_BIN): $(TESTDIR)/prefix_automaton_test.cpp \
		$(INCDIR)/adelic/prefix_automaton.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

PADIC_TEST_BIN := $(BUILDDIR)/padic_test
padic_test: $(PADIC_TEST_BIN)
$(PADIC_TEST_BIN): $(TESTDIR)/padic_test.cpp $(INCDIR)/adelic/padic.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

CLASSIFY_ADELIC_TEST_BIN := $(BUILDDIR)/classify_adelic_test
classify_adelic_test: $(CLASSIFY_ADELIC_TEST_BIN)
$(CLASSIFY_ADELIC_TEST_BIN): $(TESTDIR)/classify_adelic_test.cpp \
		$(INCDIR)/adelic/classify_adelic.hpp \
		$(INCDIR)/adelic/coincidence_and_property_f.hpp \
		$(INCDIR)/adelic/prefix_automaton.hpp \
		$(INCDIR)/adelic/dedekind_factorization.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

DEDEKIND_FACTORIZATION_TEST_BIN := $(BUILDDIR)/dedekind_factorization_test
dedekind_factorization_test: $(DEDEKIND_FACTORIZATION_TEST_BIN)
$(DEDEKIND_FACTORIZATION_TEST_BIN): $(TESTDIR)/dedekind_factorization_test.cpp \
		$(INCDIR)/adelic/dedekind_factorization.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

IDEAL_ARITHMETIC_TEST_BIN := $(BUILDDIR)/ideal_arithmetic_test
ideal_arithmetic_test: $(IDEAL_ARITHMETIC_TEST_BIN)
$(IDEAL_ARITHMETIC_TEST_BIN): $(TESTDIR)/ideal_arithmetic_test.cpp \
		$(INCDIR)/adelic/ideal_arithmetic.hpp $(INCDIR)/adelic/dedekind_factorization.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

COINCIDENCE_PROPERTY_F_TEST_BIN := $(BUILDDIR)/coincidence_and_property_f_test
coincidence_and_property_f_test: $(COINCIDENCE_PROPERTY_F_TEST_BIN)
$(COINCIDENCE_PROPERTY_F_TEST_BIN): $(TESTDIR)/coincidence_and_property_f_test.cpp \
		$(INCDIR)/adelic/coincidence_and_property_f.hpp $(INCDIR)/adelic/prefix_automaton.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# local_field_test: tests for include/adelic/local_field.hpp -- the
# general local field K_p construction for arbitrary (e, f) above p
# (ZpPoly arithmetic, Newton polygon, Newton iteration on roots,
# local polynomial construction via three approaches, QpLocalField,
# valuation via the norm, integrality predicate, Ore's algorithm).
# Cross-validates Approach A (Newton) and Approach C (cofactor) on
# the worked example's p_1 (e=2, f=1), and verifies the totally
# ramified case (rnd13, e=4, f=1) and the unramified case (Q(i)
# at p=3, e=1, f=2) still work.
LOCAL_FIELD_TEST_BIN := $(BUILDDIR)/local_field_test
local_field_test: $(LOCAL_FIELD_TEST_BIN)
$(LOCAL_FIELD_TEST_BIN): $(TESTDIR)/local_field_test.cpp \
		$(INCDIR)/adelic/local_field.hpp \
		$(INCDIR)/adelic/dedekind_factorization.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# graph_divisor_test: tests for include/ravel/graph_divisor.hpp --
# coarsest equitable partition / quotient-matrix machinery used to
# test whether the balanced-pair automaton and G_B admit a common
# equitable-partition structure (which would force equal Perron
# roots by elementary Perron-Frobenius theory, rather than the
# equality bp_rho_nc == lambda(G_B) needing per-candidate empirical
# re-checking).  See app/test_bp_gb_divisor.cpp for the actual
# experiment run against Tetrabonacci and rnd13.
GRAPH_DIVISOR_TEST_BIN := $(BUILDDIR)/graph_divisor_test
graph_divisor_test: $(GRAPH_DIVISOR_TEST_BIN)
$(GRAPH_DIVISOR_TEST_BIN): $(TESTDIR)/graph_divisor_test.cpp \
		$(INCDIR)/ravel/graph_divisor.hpp $(INCDIR)/ravel/barge.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# maximal_order_test: tests for include/adelic/maximal_order.hpp --
# one round of the Pohst-Zassenhaus p-maximal-order enlargement,
# tested against Dedekind's classic non-monogenic cubic
# (docs/ADELIC_TILING_PLAN.md §6 names this exact example as the
# test-ladder gap): disc(Z[theta]) = -2012 = -4*503 enlarges in one
# round to disc(O') = -503, the true field discriminant.
MAXIMAL_ORDER_TEST_BIN := $(BUILDDIR)/maximal_order_test
maximal_order_test: $(MAXIMAL_ORDER_TEST_BIN)
$(MAXIMAL_ORDER_TEST_BIN): $(TESTDIR)/maximal_order_test.cpp \
		$(INCDIR)/adelic/maximal_order.hpp $(INCDIR)/adelic/dedekind_factorization.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# gb_bp_hop_rule_test: tests for include/ravel/gb_bp_hop_rule.hpp --
# the hop-distance rule discovered cross-checking BP-core's candidate
# bijection against G_B's real dominant recurrent core (thread A, item
# (1)); turns app/gb_bp_crosscheck.cpp's exploratory "COMBINED RULE:
# X/Y match" printout into an actual asserted regression check. See
# docs/RESEARCH_STATUS.md "The G_B cross-check and the hop-distance
# dichotomy" for the derivation. Runs n=3..6 (~80s total -- the
# slowest item in the default suite, but a real, specific, checkable
# claim, not a smoke test).
GB_BP_HOP_RULE_TEST_BIN := $(BUILDDIR)/gb_bp_hop_rule_test
gb_bp_hop_rule_test: $(GB_BP_HOP_RULE_TEST_BIN)
$(GB_BP_HOP_RULE_TEST_BIN): $(TESTDIR)/gb_bp_hop_rule_test.cpp \
		$(INCDIR)/ravel/gb_bp_hop_rule.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# dual_format_test: tests for include/ravel/dual_format.hpp -- the
# rational-assessment / dual-view plumbing for smooth-relaxation.
DUAL_FORMAT_TEST_BIN := $(BUILDDIR)/dual_format_test
dual_format_test: $(DUAL_FORMAT_TEST_BIN)
$(DUAL_FORMAT_TEST_BIN): $(TESTDIR)/dual_format_test.cpp \
		$(INCDIR)/ravel/dual_format.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# dual_test: tests for include/ravel/dual.hpp -- forward-mode autodiff.
DUAL_TEST_BIN := $(BUILDDIR)/dual_test
dual_test: $(DUAL_TEST_BIN)
$(DUAL_TEST_BIN): $(TESTDIR)/dual_test.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) \
		$(wildcard $(MATH_INCDIR)/math/*.hpp) \
		$(wildcard $(MATH_INCDIR)/mini-gmp/*.h) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# spectral_dual_test: tests for include/ravel/spectral_dual.hpp --
# differentiable 3x3 spectral invariants.
SPECTRAL_DUAL_TEST_BIN := $(BUILDDIR)/spectral_dual_test
spectral_dual_test: $(SPECTRAL_DUAL_TEST_BIN)
$(SPECTRAL_DUAL_TEST_BIN): $(TESTDIR)/spectral_dual_test.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) \
		$(wildcard $(MATH_INCDIR)/math/*.hpp) \
		$(wildcard $(MATH_INCDIR)/mini-gmp/*.h) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# weighted_digraph_file_test: round-trip tests for
# WeightedDigraph::from_dense_file (the streaming GBMT0002 loader).
WEIGHTED_DIGRAPH_FILE_TEST_BIN := $(BUILDDIR)/weighted_digraph_file_test
weighted_digraph_file_test: $(WEIGHTED_DIGRAPH_FILE_TEST_BIN)
$(WEIGHTED_DIGRAPH_FILE_TEST_BIN): $(TESTDIR)/weighted_digraph_file_test.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# involution_helpers_test: round-trip tests for the extracted
# involution_helpers.hpp shared between the n-bonacci driver and the
# new σ_{a,b}-family probe (probe_a1_a2_unimodular).
INVOLUTION_HELPERS_TEST_BIN := $(BUILDDIR)/involution_helpers_test
involution_helpers_test: $(INVOLUTION_HELPERS_TEST_BIN)
$(INVOLUTION_HELPERS_TEST_BIN): $(TESTDIR)/involution_helpers_test.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# probe_a1_a2_unimodular: σ_{a,b}-family + σ_1 + σ_2 + Tribonacci
# nilpotent-cofactor and involution probes.  Built by hand (default
# 'make' doesn't run it; the test under default run is involution_helpers_test).
PROBE_A1_A2_BIN := $(BUILDDIR)/probe_a1_a2_unimodular
probe_a1_a2_unimodular: $(PROBE_A1_A2_BIN)
$(PROBE_A1_A2_BIN): $(APPDIR)/probe_a1_a2_unimodular.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

CLASS_II_SYMMETRY_PROBE_BIN := $(BUILDDIR)/class_ii_symmetry_probe
class_ii_symmetry_probe: $(CLASS_II_SYMMETRY_PROBE_BIN)
	./$(CLASS_II_SYMMETRY_PROBE_BIN)
$(CLASS_II_SYMMETRY_PROBE_BIN): $(APPDIR)/class_ii_symmetry_probe.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

CLASS_II_CORONA_LITERATURE_PROBE_BIN := $(BUILDDIR)/class_ii_corona_literature_probe
class_ii_corona_literature_probe: $(CLASS_II_CORONA_LITERATURE_PROBE_BIN)
	./$(CLASS_II_CORONA_LITERATURE_PROBE_BIN)
$(CLASS_II_CORONA_LITERATURE_PROBE_BIN): $(APPDIR)/class_ii_corona_literature_probe.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

CLASS_II_NEIGHBOR_PROBE_BIN := $(BUILDDIR)/class_ii_neighbor_probe
class_ii_neighbor_probe: $(CLASS_II_NEIGHBOR_PROBE_BIN)
	./$(CLASS_II_NEIGHBOR_PROBE_BIN)
$(CLASS_II_NEIGHBOR_PROBE_BIN): $(APPDIR)/class_ii_neighbor_probe.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_neighbor2_round1_window_certificate: abstract (not per-a)
# window-validity certificate for Round 1's 27-state raw pre-Red
# self-closure. Exits nonzero if the certificate stops closing.
CLASS_II_NEIGHBOR2_ROUND1_WINDOW_CERTIFICATE_BIN := $(BUILDDIR)/class_ii_neighbor2_round1_window_certificate
class_ii_neighbor2_round1_window_certificate: $(CLASS_II_NEIGHBOR2_ROUND1_WINDOW_CERTIFICATE_BIN)
	./$(CLASS_II_NEIGHBOR2_ROUND1_WINDOW_CERTIFICATE_BIN)
$(CLASS_II_NEIGHBOR2_ROUND1_WINDOW_CERTIFICATE_BIN): $(APPDIR)/class_ii_neighbor2_round1_window_certificate.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_neighbor2_round1_red_identity: exact finite check (not a
# symbolic proof) of whether Red's 2 pruned states, for a=3..60, equal
# the 2 D_cont seeds shown to need no backward predecessor above.
CLASS_II_NEIGHBOR2_ROUND1_RED_IDENTITY_BIN := $(BUILDDIR)/class_ii_neighbor2_round1_red_identity
class_ii_neighbor2_round1_red_identity: $(CLASS_II_NEIGHBOR2_ROUND1_RED_IDENTITY_BIN)
	./$(CLASS_II_NEIGHBOR2_ROUND1_RED_IDENTITY_BIN)
$(CLASS_II_NEIGHBOR2_ROUND1_RED_IDENTITY_BIN): $(APPDIR)/class_ii_neighbor2_round1_red_identity.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_neighbor2_round1_red_forward_check: dense+sparse numeric
# confirmation (a=3..200, then 500/1000/5000/20000) of the closed-form
# forward images of the two Round-1 D_cont seeds, plus the exact bound
# (2) on the 27-target coordinates.
CLASS_II_NEIGHBOR2_ROUND1_RED_FORWARD_CHECK_BIN := $(BUILDDIR)/class_ii_neighbor2_round1_red_forward_check
class_ii_neighbor2_round1_red_forward_check: $(CLASS_II_NEIGHBOR2_ROUND1_RED_FORWARD_CHECK_BIN)
	./$(CLASS_II_NEIGHBOR2_ROUND1_RED_FORWARD_CHECK_BIN)
$(CLASS_II_NEIGHBOR2_ROUND1_RED_FORWARD_CHECK_BIN): $(APPDIR)/class_ii_neighbor2_round1_red_forward_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_neighbor2_round1_red_symbolic: the actual symbolic proof --
# every raw forward-edge candidate from the two Round-1 D_cont seeds has
# an x2-coordinate affine in a with nonzero slope (derived from tau_a's
# fixed word forms and incidence matrix, not sampled), so none can ever
# match a bounded (|coord|<=2) target. Closes Round 1's Red-pruning gap.
CLASS_II_NEIGHBOR2_ROUND1_RED_SYMBOLIC_BIN := $(BUILDDIR)/class_ii_neighbor2_round1_red_symbolic
class_ii_neighbor2_round1_red_symbolic: $(CLASS_II_NEIGHBOR2_ROUND1_RED_SYMBOLIC_BIN)
	./$(CLASS_II_NEIGHBOR2_ROUND1_RED_SYMBOLIC_BIN)
$(CLASS_II_NEIGHBOR2_ROUND1_RED_SYMBOLIC_BIN): $(APPDIR)/class_ii_neighbor2_round1_red_symbolic.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_neighbor2_round1_signed_relation: checks the exact
# relationship (not just cardinality) between the unsigned Round-1
# survivor set this investigation proves and
# class_ii_neighbor2_signed_contact_set(), the object the base-premises
# table names as Round 1's actual target.
CLASS_II_NEIGHBOR2_ROUND1_SIGNED_RELATION_BIN := $(BUILDDIR)/class_ii_neighbor2_round1_signed_relation
class_ii_neighbor2_round1_signed_relation: $(CLASS_II_NEIGHBOR2_ROUND1_SIGNED_RELATION_BIN)
	./$(CLASS_II_NEIGHBOR2_ROUND1_SIGNED_RELATION_BIN)
$(CLASS_II_NEIGHBOR2_ROUND1_SIGNED_RELATION_BIN): $(APPDIR)/class_ii_neighbor2_round1_signed_relation.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_abstract_window_classify_regression: checks that
# class_ii_abstract_window_classify() (include/ravel/class_ii_neighbor_family.hpp,
# factored out of class_ii_neighbor2_round1_window_certificate.cpp for
# reuse by future Round 2/3/4 attempts) reproduces that file's own
# already-verified result exactly.
CLASS_II_ABSTRACT_WINDOW_CLASSIFY_REGRESSION_BIN := $(BUILDDIR)/class_ii_abstract_window_classify_regression
class_ii_abstract_window_classify_regression: $(CLASS_II_ABSTRACT_WINDOW_CLASSIFY_REGRESSION_BIN)
	./$(CLASS_II_ABSTRACT_WINDOW_CLASSIFY_REGRESSION_BIN)
$(CLASS_II_ABSTRACT_WINDOW_CLASSIFY_REGRESSION_BIN): $(APPDIR)/class_ii_abstract_window_classify_regression.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round2_recon: confirms Round 2's E_2 = 25-state size against
# the trusted corona-trace pipeline at a=7,8. Reconnaissance only, not
# a reverse-inclusion or Red-exclusion proof -- see
# docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md's "Round 2:
# reconnaissance only" note.
CLASS_II_ROUND2_RECON_BIN := $(BUILDDIR)/class_ii_round2_recon
class_ii_round2_recon: $(CLASS_II_ROUND2_RECON_BIN)
	./$(CLASS_II_ROUND2_RECON_BIN)
$(CLASS_II_ROUND2_RECON_BIN): $(APPDIR)/class_ii_round2_recon.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round2_ccorona_stability: confirms (calling the real,
# trusted c_corona() directly, not a reimplementation) that Round 2's
# raw candidate count is identical across a=6,7,8,15. Strengthens the
# "unverified lead" note in
# docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md into something more
# concrete; still not a reverse-inclusion proof.
CLASS_II_ROUND2_CCORONA_STABILITY_BIN := $(BUILDDIR)/class_ii_round2_ccorona_stability
class_ii_round2_ccorona_stability: $(CLASS_II_ROUND2_CCORONA_STABILITY_BIN)
	./$(CLASS_II_ROUND2_CCORONA_STABILITY_BIN)
$(CLASS_II_ROUND2_CCORONA_STABILITY_BIN): $(APPDIR)/class_ii_round2_ccorona_stability.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round2_structure: precise structural findings about Round 2
# (T_2=B_2+E_2 identity, E_2's window validity reducing to
# already-established bounds, the exact 3-rank/123-state Red-exclusion
# shape), replacing the vaguer earlier lead.
CLASS_II_ROUND2_STRUCTURE_BIN := $(BUILDDIR)/class_ii_round2_structure
class_ii_round2_structure: $(CLASS_II_ROUND2_STRUCTURE_BIN)
	./$(CLASS_II_ROUND2_STRUCTURE_BIN)
$(CLASS_II_ROUND2_STRUCTURE_BIN): $(APPDIR)/class_ii_round2_structure.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round2_red_exclusion_check: independently re-verifies (via
# simple_forward_targets_exact, not red_anode's own bookkeeping) that
# every one of Round 2's 123 pruned states has all its in-195-set edges
# landing on an earlier-or-equal rank, never a survivor or later rank.
# Exact finite certificate at a=6,7,8, not yet a universal proof.
CLASS_II_ROUND2_RED_EXCLUSION_CHECK_BIN := $(BUILDDIR)/class_ii_round2_red_exclusion_check
class_ii_round2_red_exclusion_check: $(CLASS_II_ROUND2_RED_EXCLUSION_CHECK_BIN)
	./$(CLASS_II_ROUND2_RED_EXCLUSION_CHECK_BIN)
$(CLASS_II_ROUND2_RED_EXCLUSION_CHECK_BIN): $(APPDIR)/class_ii_round2_red_exclusion_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round3_structure: the Round 2 treatment (T_r=B_r+E_r
# identity, independent Red-exclusion re-derivation via
# simple_forward_targets_exact) applied to Round 3.
CLASS_II_ROUND3_STRUCTURE_BIN := $(BUILDDIR)/class_ii_round3_structure
class_ii_round3_structure: $(CLASS_II_ROUND3_STRUCTURE_BIN)
	./$(CLASS_II_ROUND3_STRUCTURE_BIN)
$(CLASS_II_ROUND3_STRUCTURE_BIN): $(APPDIR)/class_ii_round3_structure.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

BP_DUMP_PROVENANCE_BIN := $(BUILDDIR)/bp_dump_provenance
bp_dump_provenance: $(BP_DUMP_PROVENANCE_BIN)
$(BP_DUMP_PROVENANCE_BIN): $(APPDIR)/bp_dump_provenance.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

RETURN_CONTACT_LIFT_PROBE_BIN := $(BUILDDIR)/return_contact_lift_probe
return_contact_lift_probe: $(RETURN_CONTACT_LIFT_PROBE_BIN)
	./$(RETURN_CONTACT_LIFT_PROBE_BIN)
$(RETURN_CONTACT_LIFT_PROBE_BIN): $(APPDIR)/return_contact_lift_probe.cpp \
		$(INCDIR)/ravel/return_contact_lift.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

CLASS_II_BP_FAMILY_PROBE_BIN := $(BUILDDIR)/class_ii_bp_family_probe
class_ii_bp_family_probe: $(CLASS_II_BP_FAMILY_PROBE_BIN)
	./$(CLASS_II_BP_FAMILY_PROBE_BIN)
$(CLASS_II_BP_FAMILY_PROBE_BIN): $(APPDIR)/class_ii_bp_family_probe.cpp \
		$(INCDIR)/ravel/balanced_pair.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# tabulate_pisot_properties: tabulate first-principles Pisot properties
# (spectrum, frequencies, dual σ*, return words, carry sequence, A1/A2)
# across our 10 Pisot test set.  Run by hand (not in make check).
TABULATE_BIN := $(BUILDDIR)/tabulate_pisot_properties
tabulate_pisot_properties: $(TABULATE_BIN)
$(TABULATE_BIN): $(APPDIR)/tabulate_pisot_properties.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# test_bp_gb_divisor: the actual graph-divisor experiment (session 5)
# against the balanced-pair automaton and G_B for Tetrabonacci and
# rnd13.  NOT in make all/check -- a one-off structural experiment,
# same "exercised by hand, not wired into CI" policy as the other
# probes below.  Result: negative (see docs/RESEARCH_STATUS.md) -- neither
# graph's own coarsest equitable partition finds a common quotient.
TEST_BP_GB_DIVISOR_BIN := $(BUILDDIR)/test_bp_gb_divisor
test_bp_gb_divisor: $(TEST_BP_GB_DIVISOR_BIN)
	./$(TEST_BP_GB_DIVISOR_BIN)
$(TEST_BP_GB_DIVISOR_BIN): $(APPDIR)/test_bp_gb_divisor.cpp \
		$(INCDIR)/ravel/graph_divisor.hpp $(INCDIR)/ravel/balanced_pair.hpp \
		$(INCDIR)/ravel/contact_boundary.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# adelic_boundary_spectral_radius: tests whether check_property_f's
# internal "zero-expansion graph" (the combined archimedean + p-adic
# bounded region) gives an adelic-corrected lambda(G_B) closer to
# BP-rho_nc than the plain real-space G_B does, for rnd13.  NOT in
# make all/check -- one-off structural experiment (session 5).
# Result: negative, and diagnosed -- that graph's dominant eigenvalue
# is exactly 1.0 (trivial self-loops only; it's a decision-procedure
# graph, not a metric one).  See docs/RESEARCH_STATUS.md.
ADELIC_BOUNDARY_SPECTRAL_RADIUS_BIN := $(BUILDDIR)/adelic_boundary_spectral_radius
adelic_boundary_spectral_radius: $(ADELIC_BOUNDARY_SPECTRAL_RADIUS_BIN)
	./$(ADELIC_BOUNDARY_SPECTRAL_RADIUS_BIN)
$(ADELIC_BOUNDARY_SPECTRAL_RADIUS_BIN): $(APPDIR)/adelic_boundary_spectral_radius.cpp \
		$(INCDIR)/adelic/coincidence_and_property_f.hpp $(INCDIR)/adelic/prefix_automaton.hpp \
		$(INCDIR)/ravel/graph_divisor.hpp $(INCDIR)/ravel/contact_boundary.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# gb_bp_involution_check: session 7's central finding as reproducible
# code -- the exact free involution [i,x,j] <-> [j,-x,i] on G_B's
# recurrent core (graph_divisor.hpp's extract_recurrent_core, added
# this session), and the exact polynomial-divisibility mechanism
# (mathlib::divmod) explaining BP-rho_nc == lambda(G_B) for the
# n-bonacci family.  NOT in make all/check -- a research experiment,
# not a regression test, though every assertion in it is exact
# (integer arithmetic), not a numerical survey.  See
# docs/FINDINGS_FOR_CITATION.md Finding 1 and
# docs/DIRECTION_AND_OPEN_THREADS.md thread A.
GB_BP_INVOLUTION_CHECK_BIN := $(BUILDDIR)/gb_bp_involution_check
gb_bp_involution_check: $(GB_BP_INVOLUTION_CHECK_BIN)
	./$(GB_BP_INVOLUTION_CHECK_BIN)
$(GB_BP_INVOLUTION_CHECK_BIN): $(APPDIR)/gb_bp_involution_check.cpp \
		$(INCDIR)/ravel/graph_divisor.hpp $(INCDIR)/ravel/balanced_pair.hpp \
		$(INCDIR)/ravel/contact_boundary.hpp $(MATH_INCDIR)/math/poly_z.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# rnd13_factor_probe: ADELIC_TILING_PLAN.md §4 steps 1-2 applied to
# rnd13's actual quartic charpoly.  NOT in make all/check yet -- a
# single-purpose probe, exercised by hand once so far (result:
# Z[beta] is 2-maximal, (2) = p^4 totally ramified, e=4 f=1,
# cross-checked against sympy's independent Round 2 implementation).
# Same "don't wire in until exercised" policy as gkw_sturm_certify;
# see docs/GKW_SCRIPTS_NOTE.md.
RND13_FACTOR_PROBE_BIN := $(BUILDDIR)/rnd13_factor_probe
rnd13_factor_probe: $(RND13_FACTOR_PROBE_BIN)
	./$(RND13_FACTOR_PROBE_BIN)
$(RND13_FACTOR_PROBE_BIN): $(APPDIR)/rnd13_factor_probe.cpp \
		$(INCDIR)/adelic/dedekind_factorization.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# rnd13_prefix_automaton_probe: ADELIC_TILING_PLAN.md §4 step 3 --
# builds the actual prefix automaton + digit set for rnd13 (not the
# degree-3 worked example prefix_automaton_test.cpp uses).  Result
# cross-checked against an independent NumPy eigenvector computation
# (exact match at double precision).  Same "exercised by hand, not
# yet in make all/check" policy as the other probes above.
RND13_PREFIX_AUTOMATON_PROBE_BIN := $(BUILDDIR)/rnd13_prefix_automaton_probe
rnd13_prefix_automaton_probe: $(RND13_PREFIX_AUTOMATON_PROBE_BIN)
	./$(RND13_PREFIX_AUTOMATON_PROBE_BIN)
$(RND13_PREFIX_AUTOMATON_PROBE_BIN): $(APPDIR)/rnd13_prefix_automaton_probe.cpp \
		$(INCDIR)/adelic/prefix_automaton.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# classify_adelic_tiling: ADELIC_TILING_PLAN.md §3.5 driver, tying
# together factorization + ideal cross-check + prefix automaton +
# strong coincidence + property (F).  Property (F)'s result is
# UNVALIDATED (see coincidence_and_property_f.hpp STATUS) so the
# verdict logic never reports TILES on its strength; it can report a
# sound DOES-NOT-TILE off a definitive strong-coincidence failure, or
# INCONCLUSIVE otherwise.  NOT in make all/check -- run by hand.
CLASSIFY_ADELIC_TILING_BIN := $(BUILDDIR)/classify_adelic_tiling
classify_adelic_tiling: $(CLASSIFY_ADELIC_TILING_BIN)
	./$(CLASSIFY_ADELIC_TILING_BIN)
$(CLASSIFY_ADELIC_TILING_BIN): $(APPDIR)/classify_adelic_tiling.cpp \
		$(INCDIR)/adelic/dedekind_factorization.hpp \
		$(INCDIR)/adelic/ideal_arithmetic.hpp \
		$(INCDIR)/adelic/prefix_automaton.hpp \
		$(INCDIR)/adelic/coincidence_and_property_f.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# ====================================================================
# Cleanup
# ====================================================================

clean:
	rm -rf $(BUILDDIR)
	$(MAKE) -C $(MATH_DIR) clean

# Compiler-generated transitive header dependencies. Missing files are
# harmless on the first build; every subsequent build tracks only the
# headers each translation unit actually includes.
-include $(wildcard $(BUILDDIR)/*.d)
