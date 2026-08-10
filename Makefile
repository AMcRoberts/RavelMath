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
EXTRA_SRCS := $(SRCDIR)/nbonacci_covering_witness.cpp
EXTRA_OBJECTS := $(BUILDDIR)/nbonacci_covering_witness.o

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

.PHONY: all build check clean shoot-the-moon ravel_proof_runner
.PHONY: math apps tests lua data tables lean-check
.PHONY: d_cont_check_test tile_faces_test number_field_parity_test poly_discriminant_bigint_test ambient_graph_test corona_test property_f_reflection_test
.PHONY: contact_boundary_test exact_pisot_test spectral_general_test
.PHONY: survey_test qbeta_eigenvalue_test qbeta_eigenvalue
.PHONY: rational_transcendentals_test gkw_sturm_certify gkw_fully_rigorous
.PHONY: test_qbasis validate_exploded test_12_exploded contact_boundary_4x4
.PHONY: cylinder_measure verify_exploded_12 test_general_spectral
.PHONY: prefix_automaton_test padic_test dedekind_factorization_test
.PHONY: ideal_arithmetic_test coincidence_and_property_f_test local_field_test graph_divisor_test maximal_order_test gb_bp_hop_rule_test nbonacci_margin_invariant_test nbonacci_margin_core_graph_test nbonacci_block_identity_test nbonacci_covering_witness_test
.PHONY: rnd13_factor_probe rnd13_prefix_automaton_probe classify_adelic_tiling test_bp_gb_divisor adelic_boundary_spectral_radius gb_bp_involution_check
.PHONY: class_ii_symmetry_probe class_ii_boundary_family_test substitution_neighborhood_test family_closed_forms_test
.PHONY: class_ii_corona_literature_probe
.PHONY: nbonacci_periodic_carry_probe nbonacci_carry_cycle_probe nbonacci_sign_chamber_probe nbonacci_chamber_stability nbonacci_chamber_merge nbonacci_rank_feature_search nbonacci_sector_gap_rank nbonacci_block_spectrum_probe nbonacci_block_forcing_probe nbonacci_block_l1_growth_probe nbonacci_max_shell_return_probe nbonacci_max_shell_return_stability nbonacci_shell_word_probe nbonacci_conjugate_height_probe nbonacci_conjugate_height_bound nbonacci_dominance_theorem_pipeline nbonacci_homogeneous_shell_smt nbonacci_homogeneous_shell_unsat_core nbonacci_shell_covering_search nbonacci_shell_covering_proof nbonacci_covering_witness_enumerator nbonacci_covering_witness_enumerate nbonacci_data_shaker nbonacci_padic_fingerprint nbonacci_csy_state_count nbonacci_csy_dynamics nbonacci_charmpoly_proof_probe nbonacci_homogeneous_shell_core_enumerate
.PHONY: strict_shell_pump_test
.PHONY: class_ii_neighbor_probe
.PHONY: return_contact_lift_probe
.PHONY: class_ii_terminal_transport_probe
.PHONY: class_ii_bp_family_probe
.PHONY: csy_carry_automaton_test
.PHONY: csy_finite_carry_automaton_test
.PHONY: lean_class_ii_catalogue_cross_check_test
.PHONY: supergolden_qrs_audit
.PHONY: theta5_contact_boundary_probe property_f_family_autopsy property_f_family_artifact property_f_transport_certificate_test property_f_contact_transport_bridge_test property_f_escape_rank_test property_f_birth_round_grammar_test property_f_class_ii_birth_round_test stratified_escape_certificate_test canonical_nonunit_property_f_test nonunit_property_f_theorem_test marker_power_return_core_test return_core_spectral_certificate_test nonar_return_contact_lift_test nonar_property_f_bridge_test nonunit_property_f_bridge_test rnd13_property_f_bridge_test nonar_sigma02_long_probe generalized_multinacci_property_f_transport_test generalized_multinacci_property_f_transport_extended_test generalized_multinacci_growth_profile_test generalized_multinacci_block_affine_test generalized_multinacci_structural_sweep_test generalized_multinacci_cross_dimension_test generalized_multinacci_cross_dimension_property_f_test generalized_multinacci_fiber_interaction_test adelic_cocycle_extension_test

all: build data apps tests
build: $(LIB)
lua: $(LIB) data

# ====================================================================
# Math library (delegated)
# ====================================================================

math: $(MATH_LIB)
	$(MAKE) -C $(MATH_DIR) check

LEAN_ENV ?= ../LEAN/projects/nbonacci_charmpoly
lean-check: nbonacci_charmpoly_proof_probe supergolden_qrs_audit
	cd $(LEAN_ENV) && lake env lean $(abspath lean/free_involution_perron_core.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/return_contact_lift.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/bp_correction_determinant.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_affine_shells.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_neighbor_dominance.lean)
	@if [ -f out/nbonacci_charmpoly_proof.lean ]; then \
		cd $(LEAN_ENV) && lake env lean $(abspath out/nbonacci_charmpoly_proof.lean); \
	fi
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_neighbor2_extensions.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_terminal_shells.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_balanced_pivot.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_neighbor_d_support.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_six_vertex_graduation.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_global_round_partition.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_round1_red_pruning.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/class_ii_round234_shape_closure.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/perron_column_difference.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/nbonacci_margin_catalogue.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/Ravel/Matrix/EraseIndex.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/barge_diamond_lattice_line.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/barge_diamond_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/barge_diamond_wide_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/barge_diamond_finding37_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/period_rotation_forces_equal_modulus.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/period_rotation_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/constant_first_letter_forces_prefix_coincidence.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/constant_first_letter_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/constant_last_letter_forces_suffix_coincidence.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/constant_last_letter_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/substitution_iteration_infrastructure.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/zero_run_same_chain_coincidence.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/zero_run_same_chain_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/first_letter_orbit_coincidence.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/first_letter_orbit_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/first_letter_orbit_finding41_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/last_letter_orbit_coincidence.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/last_letter_orbit_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/property_f_zero_walk.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/periodic_point_repetition.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/leftmost_loop_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_shell_round_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_fixed_tables_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_terminal_shell_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_round1_raw27_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_neighbor2_fixed_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_neighbor_d_support_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/cayley_hamilton_cubic_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/pisot_root_ordering_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_six_vertex_graduation_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_backward_layers_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_terminal_sextet_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_penultimate_pair_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_interior_tip_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_global_round_phase_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_both_fixed_affine_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/class_ii_d_cont_in_pre_contact_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/sturm_sequence_root_counting.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/depressed_cubic_complex_pair_modulus.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/depressed_cubic_not_pisot_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/adjacent_swap_count_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/cycle_charpoly_campaign_validation.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/graph_cycle_charpoly_factor_validation.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/cyclic_controller_pumping.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/continuation_controller_family_algebra.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/nbonacci_margin_catalogue.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/feeder_cycle_charpoly_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/regular_shell_charpoly_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/strict_shell_pump_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/realized_first_return_completeness.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/recurrent_family_exhaustion_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/predicted_core_scc_exhaustion_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/coupled_winning_predicate_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/radial_translation_defect_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/defect_splice_step_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/condition_f_joint_dominance_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/condition_f_joint_qr_playground.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/finite_positive_grammar_majorant.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/finite_positive_grammar_majorant_batch.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/nbonacci_universal_n.kernel_checked.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/characteristic_minor_maps_direct.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/coefficient_profile_parity_obstruction.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/cyclotomic_obstruction.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/first_return_obligations_discharged.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/generalized_multinacci_admissible_subgrammar.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/generalized_multinacci_general_m.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/generalized_multinacci_general_m_intertwiner.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/generalized_multinacci_primitive_intertwiner.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/generalized_multinacci_signed_renewal_twist.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/generalized_multinacci_symbolic_embedding.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/generic_residual_formula.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/monotone_profile_corridor_closure.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/norm_weighted_qr_majorant.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/multi_compartment_closure_validation.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/plastic_three_generator_intertwiner.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/quartic_fourth_generator_quotient.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/shared_polynomial_closure_validation.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/shift_branch_three_generator_continuation.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/supergolden_three_generator_intertwiner.lean)
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/third_smallest_pisot_parry_factorization_batch.lean)
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
		lean/class_ii_global_round_partition.lean \
		lean/class_ii_round1_red_pruning.lean \
		lean/class_ii_round234_shape_closure.lean \
		lean/perron_column_difference.lean \
		lean/nbonacci_margin_catalogue.lean \
		lean/Ravel/Matrix/EraseIndex.lean \
		lean/barge_diamond_lattice_line.lean \
		lean/generated/barge_diamond_batch.lean \
		lean/generated/barge_diamond_wide_batch.lean \
		lean/generated/barge_diamond_finding37_batch.lean \
		lean/period_rotation_forces_equal_modulus.lean \
		lean/constant_first_letter_forces_prefix_coincidence.lean \
		lean/generated/constant_first_letter_batch.lean \
		lean/constant_last_letter_forces_suffix_coincidence.lean \
		lean/generated/constant_last_letter_batch.lean \
		lean/substitution_iteration_infrastructure.lean \
		lean/zero_run_same_chain_coincidence.lean \
		lean/generated/zero_run_same_chain_batch.lean \
		lean/first_letter_orbit_coincidence.lean \
		lean/generated/first_letter_orbit_batch.lean \
		lean/generated/first_letter_orbit_finding41_batch.lean \
		lean/last_letter_orbit_coincidence.lean \
		lean/generated/last_letter_orbit_batch.lean \
		lean/generated/property_f_zero_walk.lean \
		lean/periodic_point_repetition.lean \
		lean/generated/leftmost_loop_batch.lean \
		lean/generated/class_ii_shell_round_batch.lean \
		lean/generated/class_ii_fixed_tables_batch.lean \
		lean/generated/class_ii_terminal_shell_batch.lean \
		lean/generated/class_ii_round1_raw27_batch.lean \
		lean/generated/class_ii_neighbor2_fixed_batch.lean \
		lean/generated/class_ii_neighbor_d_support_batch.lean \
		lean/generated/cayley_hamilton_cubic_batch.lean \
		lean/generated/pisot_root_ordering_batch.lean \
		lean/generated/class_ii_six_vertex_graduation_batch.lean \
		lean/generated/class_ii_backward_layers_batch.lean \
		lean/generated/class_ii_terminal_sextet_batch.lean \
		lean/generated/class_ii_penultimate_pair_batch.lean \
		lean/generated/class_ii_interior_tip_batch.lean \
		lean/generated/class_ii_global_round_phase_batch.lean \
		lean/generated/class_ii_both_fixed_affine_batch.lean \
		lean/generated/class_ii_d_cont_in_pre_contact_batch.lean \
		lean/sturm_sequence_root_counting.lean \
		lean/depressed_cubic_complex_pair_modulus.lean \
		lean/generated/depressed_cubic_not_pisot_batch.lean \
		lean/generated/adjacent_swap_count_batch.lean \
		lean/cycle_charpoly_campaign_validation.lean \
		lean/graph_cycle_charpoly_factor_validation.lean \
		lean/cyclic_controller_pumping.lean \
		lean/continuation_controller_family_algebra.lean \
		lean/nbonacci_margin_catalogue.lean \
		lean/generated/feeder_cycle_charpoly_batch.lean \
		lean/generated/regular_shell_charpoly_batch.lean \
		lean/generated/strict_shell_pump_batch.lean \
		lean/generated/realized_first_return_completeness.lean \
		lean/generated/recurrent_family_exhaustion_batch.lean \
		lean/generated/predicted_core_scc_exhaustion_batch.lean \
		lean/generated/coupled_winning_predicate_batch.lean \
		lean/generated/radial_translation_defect_batch.lean \
		lean/generated/defect_splice_step_batch.lean \
		lean/generated/condition_f_joint_dominance_batch.lean \
		lean/generated/condition_f_joint_qr_playground.lean \
		lean/generated/finite_positive_grammar_majorant.lean \
		lean/generated/finite_positive_grammar_majorant_batch.lean \
		lean/generated/nbonacci_universal_n.kernel_checked.lean \
		lean/generated/characteristic_minor_maps_direct.lean \
		lean/generated/coefficient_profile_parity_obstruction.lean \
		lean/generated/cyclotomic_obstruction.lean \
		lean/generated/first_return_obligations_discharged.lean \
		lean/generated/generalized_multinacci_admissible_subgrammar.lean \
		lean/generated/generalized_multinacci_general_m.lean \
		lean/generated/generalized_multinacci_general_m_intertwiner.lean \
		lean/generated/generalized_multinacci_primitive_intertwiner.lean \
		lean/generated/generalized_multinacci_signed_renewal_twist.lean \
		lean/generated/generalized_multinacci_symbolic_embedding.lean \
		lean/generated/generic_residual_formula.lean \
		lean/generated/monotone_profile_corridor_closure.lean \
		lean/generated/multi_compartment_closure_validation.lean \
		lean/generated/norm_weighted_qr_majorant.lean \
		lean/generated/plastic_three_generator_intertwiner.lean \
		lean/generated/quartic_fourth_generator_quotient.lean \
		lean/generated/shared_polynomial_closure_validation.lean \
		lean/generated/shift_branch_three_generator_continuation.lean \
		lean/generated/supergolden_three_generator_intertwiner.lean \
		lean/generated/supergolden_qrs_audit.lean \
		lean/generated/third_smallest_pisot_parry_factorization_batch.lean \
		lean/generated/period_rotation_batch.lean; then \
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

$(BUILDDIR)/nbonacci_covering_witness.o: $(SRCDIR)/nbonacci_covering_witness.cpp \
		$(INCDIR)/ravel/nbonacci_covering_witness.hpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(LIB): $(OBJECTS) $(EXTRA_OBJECTS) $(MATH_LIB)
	$(CXX) -shared -o $@ $(OBJECTS) $(EXTRA_OBJECTS) $(LUA_LIBS) -L$(MATH_BUILDDIR) -lmath
	$(CXX) -c -o $(BUILDDIR)/nbonacci_covering_witness.o $(SRCDIR)/nbonacci_covering_witness.cpp $(CXXFLAGS) $(INCLUDES)

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

# ultrametric_chsh / ultrametric_orientation: C++ port of
# python/ultrametric_chsh.py, ultrametric_orientation.py, and
# ultrametric_scaling.py (the Spectre-tiling substitution-ultrametric
# vs. CA-lightcone locality experiments). No math/ library dependency
# -- pure geometry + graph BFS, see include/ravel/spectre_lineage.hpp
# and include/ravel/spectre_ca.hpp.
ULTRAMETRIC_CHSH_BIN := $(BUILDDIR)/ultrametric_chsh
ultrametric_chsh: $(ULTRAMETRIC_CHSH_BIN)
$(ULTRAMETRIC_CHSH_BIN): $(APPDIR)/ultrametric_chsh.cpp $(INCDIR)/ravel/spectre_geometry.hpp $(INCDIR)/ravel/spectre_ca.hpp $(INCDIR)/ravel/substitution_lineage.hpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

ULTRAMETRIC_ORIENTATION_BIN := $(BUILDDIR)/ultrametric_orientation
ultrametric_orientation: $(ULTRAMETRIC_ORIENTATION_BIN)
$(ULTRAMETRIC_ORIENTATION_BIN): $(APPDIR)/ultrametric_orientation.cpp $(INCDIR)/ravel/spectre_geometry.hpp $(INCDIR)/ravel/spectre_ca.hpp $(INCDIR)/ravel/substitution_lineage.hpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

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

# class_ii_identity_transport_probe: proves same-letter signed-strip
# validity for every regular-shell parameter of both remaining Class-II
# adjacent-swap neighbors, then crosses an incidence-matrix wall to show
# exactly what changes at Tribonacci: the sparse quotient-ring identity,
# not the characteristic-polynomial reduction method.  Research probe;
# explicit target, not part of the default app bundle.
CLASS_II_IDENTITY_TRANSPORT_PROBE_BIN := $(BUILDDIR)/class_ii_identity_transport_probe
class_ii_identity_transport_probe: $(CLASS_II_IDENTITY_TRANSPORT_PROBE_BIN)
	./$(CLASS_II_IDENTITY_TRANSPORT_PROBE_BIN)
$(CLASS_II_IDENTITY_TRANSPORT_PROBE_BIN): $(APPDIR)/class_ii_identity_transport_probe.cpp \
		$(MATH_INCDIR)/math/poly_z.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# item1_per_pair_check: exact Q(beta) certification of the existing
# n-bonacci dominant-core per-face-pair max-|t| formulas.  Equality,
# left-eigenvector sidedness, exact dominant-core transition agreement, the
# predicted sparse argmax witness, every residual margin sign, and the global
# worst formula are hard failures; doubles are diagnostic only.
ITEM1_PER_PAIR_CHECK_BIN := $(BUILDDIR)/item1_per_pair_check
item1_per_pair_check: $(ITEM1_PER_PAIR_CHECK_BIN)
	./$(ITEM1_PER_PAIR_CHECK_BIN)
$(ITEM1_PER_PAIR_CHECK_BIN): $(APPDIR)/item1_per_pair_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) \
		$(wildcard $(MATH_INCDIR)/math/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

NBONACCI_MARGIN_INVARIANT_SYNTHESIZER_BIN := $(BUILDDIR)/nbonacci_margin_invariant_synthesizer
nbonacci_margin_invariant_synthesizer: $(NBONACCI_MARGIN_INVARIANT_SYNTHESIZER_BIN)
	./$(NBONACCI_MARGIN_INVARIANT_SYNTHESIZER_BIN)
$(NBONACCI_MARGIN_INVARIANT_SYNTHESIZER_BIN): \
		$(APPDIR)/nbonacci_margin_invariant_synthesizer.cpp \
		$(INCDIR)/ravel/nbonacci_margin_invariant.hpp \
		$(wildcard $(MATH_INCDIR)/math/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# Non-circular full-corona SCC ledger for the open n-bonacci dominance seam.
# The default finite certificate stops at n=6; n=7/8 are available explicitly
# but their corona construction is intentionally not enrolled in make check.
NBONACCI_DOMINANCE_LEDGER_BIN := $(BUILDDIR)/nbonacci_dominance_ledger
NBONACCI_DOMINANCE_ARGS ?= --exact 3 4 5 6
nbonacci_dominance_ledger: $(NBONACCI_DOMINANCE_LEDGER_BIN)
	./$(NBONACCI_DOMINANCE_LEDGER_BIN) $(NBONACCI_DOMINANCE_ARGS)
$(NBONACCI_DOMINANCE_LEDGER_BIN): \
		$(APPDIR)/nbonacci_dominance_ledger.cpp \
		$(INCDIR)/ravel/nbonacci_margin_invariant.hpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) \
		$(wildcard $(MATH_INCDIR)/math/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# Corona-free exact Q(beta) supergraph experiment.  Its coefficient bound is
# explicit until the carry automaton supplies the universal containment lemma.
NBONACCI_ARITHMETIC_HULL_BIN := $(BUILDDIR)/nbonacci_arithmetic_hull
NBONACCI_ARITHMETIC_HULL_ARGS ?= --exact --bound=1 3 4 5 6
nbonacci_arithmetic_hull: $(NBONACCI_ARITHMETIC_HULL_BIN)
	./$(NBONACCI_ARITHMETIC_HULL_BIN) $(NBONACCI_ARITHMETIC_HULL_ARGS)
nbonacci_intertwiner_search: $(NBONACCI_ARITHMETIC_HULL_BIN)
	./$(NBONACCI_ARITHMETIC_HULL_BIN) --bound=1 --dump-quotients 4 5 6 | \
		python3 python/nbonacci_intertwiner_search.py
nbonacci_restricted_intertwiner_search: $(NBONACCI_ARITHMETIC_HULL_BIN)
	./$(NBONACCI_ARITHMETIC_HULL_BIN) --bound=1 --dump-relations 4 5 6 | \
		python3 python/nbonacci_restricted_intertwiner_search.py
nbonacci_passive_intertwiner_search: $(NBONACCI_ARITHMETIC_HULL_BIN)
	./$(NBONACCI_ARITHMETIC_HULL_BIN) --bound=1 --dump-passive-relations 4 5 6 | \
		python3 python/nbonacci_restricted_intertwiner_search.py
nbonacci_exterior_spectrum_probe: $(NBONACCI_ARITHMETIC_HULL_BIN)
	./$(NBONACCI_ARITHMETIC_HULL_BIN) --bound=1 --dump-quotients 4 5 6 7 | \
		PYTHONPATH=python python3 python/nbonacci_exterior_spectrum_probe.py
nbonacci_periodic_carry_probe:
	RAVEL_PROBE_MEMORY_MB=10240 PYTHONPATH=python python3 python/nbonacci_periodic_carry_probe.py
NBONACCI_CARRY_CYCLE_PROBE_BIN := $(BUILDDIR)/nbonacci_carry_cycle_probe
nbonacci_carry_cycle_probe: $(NBONACCI_CARRY_CYCLE_PROBE_BIN)
	ulimit -v 10485760; ./$(NBONACCI_CARRY_CYCLE_PROBE_BIN) --n=3 --bound=2
$(NBONACCI_CARRY_CYCLE_PROBE_BIN): \
		$(APPDIR)/nbonacci_carry_cycle_probe.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

# C++ covering-witness enumerator: the durable C++ layer for the
# Python nbonacci_shell_covering_proof.py enumeration.  Same
# JSON output format, ~10-50x faster (C++ is integer LCM-of-
# denominators, Python is Fraction).  No header dependencies
# beyond the standard library; built and run by
# `make nbonacci_covering_witness_enumerate` which iterates n.
NBONACCI_COVERING_WITNESS_ENUMERATOR_BIN := $(BUILDDIR)/nbonacci_covering_witness_enumerator
$(NBONACCI_COVERING_WITNESS_ENUMERATOR_BIN): \
		$(APPDIR)/nbonacci_covering_witness_enumerator.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

# nbonacci_data_shaker: C++ cross-tool data shaker.  Loads the C++
# covering witness enumerator's JSON sidecars, runs polynomial
# fits, holds out the largest n, emits difference tables, and
# flags anomalies.  This is the C++ durable layer for the
# pattern-mining step; the Python nbonacci_shell_covering_proof.py
# still drives the candidate enumeration but the shaker
# itself lives in C++.
NBONACCI_DATA_SHAKER_BIN := $(BUILDDIR)/nbonacci_data_shaker
$(NBONACCI_DATA_SHAKER_BIN): \
		$(APPDIR)/nbonacci_data_shaker.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

NBONACCI_DATA_SHAKER_DIR ?= out/nbonacci_data_shaker
nbonacci_data_shaker: $(NBONACCI_DATA_SHAKER_BIN) nbonacci_covering_witness_enumerate
	./$(NBONACCI_DATA_SHAKER_BIN) \
		--enumerator-dir=out/nbonacci_covering_enumerator \
		--output-dir=$(NBONACCI_DATA_SHAKER_DIR) \
		--n-min=2 --n-max=8

# nbonacci_padic_fingerprint: p-adic ramification fingerprint for
# the n-bonacci polynomial at small primes.  Tests the
# conjecture "the only ramified primes in n=2..8 are 2 and 3"
# (which would explain the 1/2- and 1/3-denominators observed
# in the simplest covering witness's free-parameter vector).
# Uses adelic/fp_poly_factor.hpp's general F_p[x] factoring.
NBONACCI_PADIC_FINGERPRINT_BIN := $(BUILDDIR)/nbonacci_padic_fingerprint
$(NBONACCI_PADIC_FINGERPRINT_BIN): \
		$(APPDIR)/nbonacci_padic_fingerprint.cpp \
		$(INCDIR)/adelic/fp_poly.hpp \
		$(INCDIR)/adelic/fp_poly_factor.hpp \
		$(INCDIR)/adelic/dedekind_factorization.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

nbonacci_padic_fingerprint: $(NBONACCI_PADIC_FINGERPRINT_BIN)
	./$(NBONACCI_PADIC_FINGERPRINT_BIN) --n-min=2 --n-max=8 \
		--primes=2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113 \
		--out=out/nbonacci_padic_fingerprint.json

# nbonacci_csy_state_count: build the CSY finite zero-expansion
# automaton on the n-bonacci PisotPoly and report the state count.
# A finite state count is the direct test of CSY Theorem 3 (the
# "preserves zeros" / Frougny-Solomyak Condition F for the
# n-bonacci Pisot β-numeration); a state count > n+1 means CSY
# Theorem 3's bound is coarser than the homogeneous-shell
# covering witness.
NBONACCI_CSY_STATE_COUNT_BIN := $(BUILDDIR)/nbonacci_csy_state_count
$(NBONACCI_CSY_STATE_COUNT_BIN): \
		$(APPDIR)/nbonacci_csy_state_count.cpp \
		$(INCDIR)/adelic/csy_carry_automaton.hpp \
		$(INCDIR)/adelic/csy_finite_carry_automaton.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

nbonacci_csy_state_count: $(NBONACCI_CSY_STATE_COUNT_BIN)
	./$(NBONACCI_CSY_STATE_COUNT_BIN) --n-min=2 --n-max=8 \
		--max-prefix=8 --bound-bits=14 \
		--out=out/nbonacci_csy_state_count.json

# nbonacci_csy_dynamics: the "complete classification and
# dynamics" of the CSY state count for the n-bonacci Pisot
# numeration.  For each (n, max_prefix), report states, raw_nodes,
# transitions, truncated.  Run at multiple max_prefixes per n to
# extract the saturation pattern and the "what does the state
# count converge to" question.
NBONACCI_CSY_DYNAMICS_BIN := $(BUILDDIR)/nbonacci_csy_dynamics
$(NBONACCI_CSY_DYNAMICS_BIN): \
		$(APPDIR)/nbonacci_csy_dynamics.cpp \
		$(INCDIR)/adelic/csy_carry_automaton.hpp \
		$(INCDIR)/adelic/csy_finite_carry_automaton.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

nbonacci_csy_dynamics: $(NBONACCI_CSY_DYNAMICS_BIN)
	./$(NBONACCI_CSY_DYNAMICS_BIN) --n-min=2 --n-max=8 \
		--max-prefixes=4,6,8,10 \
		--bound-bits=10 --out=out/nbonacci_csy_dynamics.json

# nbonacci_charmpoly_proof_probe: C++ symbolic-determinant probe
# that verifies the cofactor expansion of the n-bonacci charmatrix
# for n=2..8 and emits a Lean source file of native_decide
# examples.  The C++ side computes the witness (the symbolic
# det of the r-matrix and q-matrix, the cofactor sum); Lean's
# kernel checks the equality.  This is the "operational statement
# of the facts as they are observed in the C++ code" the user
# asked for: the C++ code observes that the cofactor formula
# holds for n=2..8, the Lean file records those observations as
# machine-checked facts.
NBONACCI_CHARMPOLY_PROOF_PROBE_BIN := $(BUILDDIR)/nbonacci_charmpoly_proof_probe
$(NBONACCI_CHARMPOLY_PROOF_PROBE_BIN): \
		$(APPDIR)/nbonacci_charmpoly_proof_probe.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

nbonacci_charmpoly_proof_probe: $(NBONACCI_CHARMPOLY_PROOF_PROBE_BIN)
	@mkdir -p out
	./$(NBONACCI_CHARMPOLY_PROOF_PROBE_BIN) --n-min=2 --n-max=8 \
		--out=out/nbonacci_charmpoly_proof.lean

NBONACCI_CHARMPOLY_PROOF_GENERAL_BIN := $(BUILDDIR)/nbonacci_charmpoly_proof_general
$(NBONACCI_CHARMPOLY_PROOF_GENERAL_BIN): \
		$(APPDIR)/nbonacci_charmpoly_proof_general.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

nbonacci_charmpoly_proof_general: $(NBONACCI_CHARMPOLY_PROOF_GENERAL_BIN)
	@mkdir -p out
	./$(NBONACCI_CHARMPOLY_PROOF_GENERAL_BIN) --n=8 \
		--trace-out=out/nbonacci_reflective_trace.txt \
		--lean-out=out/nbonacci_reflective_trace.lean

NBONACCI_COVERING_ENUM_DIR ?= out/nbonacci_covering_enumerator
nbonacci_covering_witness_enumerate: $(NBONACCI_COVERING_WITNESS_ENUMERATOR_BIN)
	@mkdir -p $(NBONACCI_COVERING_ENUM_DIR)
	@for n in 2 3 4 5 6 7 8; do \
		echo "=== enumerating n=$$n ==="; \
		./$(NBONACCI_COVERING_WITNESS_ENUMERATOR_BIN) --n=$$n --out-dir=$(NBONACCI_COVERING_ENUM_DIR) || exit 1; \
		./$(NBONACCI_COVERING_WITNESS_ENUMERATOR_BIN) --n=$$n --L=$$(($$n + 2)) --out-dir=$(NBONACCI_COVERING_ENUM_DIR) || exit 1; \
	done
	@echo "wrote n2..n8 (L=n+1 and L=n+2) JSON sidecars to $(NBONACCI_COVERING_ENUM_DIR)/"
nbonacci_sign_chamber_probe:
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_sign_chamber_probe.py --n=3 --bound=6 --mode=sign
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_sign_chamber_probe.py --n=3 --bound=6 --mode=ordered
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_sign_chamber_probe.py --n=3 --bound=6 --mode=gaps-mod --modulus=auto --rank-base=sum

nbonacci_chamber_stability:
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_sign_chamber_probe.py --n=3 --bound=8 --mode=gaps-mod --modulus=auto --rank-base=sum --emit-certificate=/tmp/n3_b8_rank.json
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_sign_chamber_probe.py --n=3 --bound=12 --mode=gaps-mod --modulus=auto --rank-base=sum --emit-certificate=/tmp/n3_b12_rank.json
	python3 python/nbonacci_chamber_certificate_check.py /tmp/n3_b8_rank.json
	python3 python/nbonacci_chamber_certificate_check.py /tmp/n3_b12_rank.json
	python3 python/nbonacci_chamber_stability.py /tmp/n3_b8_rank.json /tmp/n3_b12_rank.json

nbonacci_chamber_merge: nbonacci_chamber_stability
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_sign_chamber_probe.py --n=3 --bound=16 --mode=gaps-mod --modulus=auto --rank-base=sum --emit-certificate=/tmp/n3_b16_rank.json
	python3 python/nbonacci_chamber_certificate_check.py /tmp/n3_b16_rank.json
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_sign_chamber_probe.py --n=3 --bound=32 --mode=gaps-mod --modulus=auto --rank-base=sum --emit-certificate=/tmp/n3_b32_rank.json
	python3 python/nbonacci_chamber_certificate_check.py /tmp/n3_b32_rank.json
	python3 python/nbonacci_chamber_merge.py /tmp/n3_b8_rank.json /tmp/n3_b12_rank.json /tmp/n3_b16_rank.json /tmp/n3_b32_rank.json --emit=/tmp/n3_b8_b12_b16_b32_merged_rank.json
	python3 python/nbonacci_chamber_certificate_check.py /tmp/n3_b8_b12_b16_b32_merged_rank.json

nbonacci_rank_feature_search: nbonacci_chamber_stability
	python3 python/nbonacci_rank_feature_search.py /tmp/n3_b8_rank.json

nbonacci_sector_gap_rank:
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_sign_chamber_probe.py --n=3 --bound=8 --mode=gaps --rank-base=sum --emit-certificate=/tmp/n3_b8_gaps.json
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_sign_chamber_probe.py --n=3 --bound=12 --mode=gaps --rank-base=sum --emit-certificate=/tmp/n3_b12_gaps.json
	python3 python/nbonacci_chamber_merge.py /tmp/n3_b8_gaps.json /tmp/n3_b12_gaps.json --emit=/tmp/n3_gaps_merged.json
	python3 python/nbonacci_rank_feature_search.py /tmp/n3_gaps_merged.json --family=sector-gaps --emit=/tmp/n3_sector_gap_rank.json
	python3 python/nbonacci_rank_feature_certificate_check.py /tmp/n3_sector_gap_rank.json
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_sign_chamber_probe.py --n=3 --bound=8 --mode=gaps-mod-scale --rank-base=sum --emit-certificate=/tmp/n3_b8_scale_rank.json
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_sign_chamber_probe.py --n=3 --bound=12 --mode=gaps-mod-scale --rank-base=sum --emit-certificate=/tmp/n3_b12_scale_rank.json
	python3 python/nbonacci_chamber_merge.py /tmp/n3_b8_scale_rank.json /tmp/n3_b12_scale_rank.json --emit=/tmp/n3_scale_merged.json
	python3 python/nbonacci_rank_feature_search.py /tmp/n3_scale_merged.json --family=sector-gaps-scale --emit=/tmp/n3_sector_scale_rank.json
	python3 python/nbonacci_rank_feature_certificate_check.py /tmp/n3_sector_scale_rank.json
nbonacci_block_spectrum_probe:
	RAVEL_PROBE_MEMORY_MB=10240 OPENBLAS_NUM_THREADS=1 python3 python/nbonacci_block_spectrum_probe.py
nbonacci_block_forcing_probe:
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_block_forcing_probe.py
nbonacci_block_l1_growth_probe:
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_block_l1_growth_probe.py --n=3 --bound=8
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_block_l1_growth_probe.py --n=4 --bound=6

nbonacci_max_shell_return_probe:
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_max_shell_return_probe.py --n=3 --bound=8 --emit-rank=/tmp/n3_shell8_return.json
	python3 python/nbonacci_max_shell_return_check.py /tmp/n3_shell8_return.json
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_max_shell_return_probe.py --n=4 --bound=8 --emit-rank=/tmp/n4_shell8_return.json
	python3 python/nbonacci_max_shell_return_check.py /tmp/n4_shell8_return.json

nbonacci_max_shell_return_stability:
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_max_shell_return_stability.py --n=4 --min-bound=2 --max-bound=12

nbonacci_shell_word_probe:
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_shell_word_probe.py --n=4 --bound=10 --max-steps=7
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_shell_word_probe.py --n=5 --bound=6 --max-steps=8

nbonacci_conjugate_height_probe:
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_conjugate_height_probe.py --min-n=2 --max-n=10

nbonacci_conjugate_height_bound: $(NBONACCI_CARRY_CYCLE_PROBE_BIN)
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_conjugate_height_bound.py --n-min=2 --n-max=12 --precision=60 --identity-trials=1000
	RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_conjugate_slack_diagnostic.py --n-min=3 --n-max=7
	ulimit -v 10485760; ./out/nbonacci_carry_cycle_probe --n=2 --bound=2
	ulimit -v 10485760; ./out/nbonacci_carry_cycle_probe --n=3 --bound=3
	ulimit -v 10485760; ./out/nbonacci_carry_cycle_probe --n=4 --bound=3
	ulimit -v 10485760; ./out/nbonacci_carry_cycle_probe --n=5 --bound=4
	ulimit -v 10485760; ./out/nbonacci_carry_cycle_probe --n=6 --bound=5
	ulimit -v 10485760; ./out/nbonacci_carry_cycle_probe --n=7 --bound=6

nbonacci_homogeneous_shell_smt:
	RAVEL_PROBE_MEMORY_MB=4096 python3 python/nbonacci_homogeneous_shell_smt.py --n=14 --max-n

nbonacci_homogeneous_shell_unsat_core:
	RAVEL_PROBE_MEMORY_MB=4096 python3 python/nbonacci_homogeneous_shell_unsat_core.py --n-min=2 --n-max=6

nbonacci_shell_covering_search:
	python3 python/nbonacci_shell_covering_search.py --n-min=2 --n-max=5

nbonacci_shell_covering_proof:
	python3 python/nbonacci_shell_covering_proof.py --n-min=2 --n-max=8 \
	    --emit-json-dir=out/nbonacci_covering_proof/ \
	    --mine-gap-formula --attempt-promotion
	python3 python/nbonacci_shell_covering_proof_check.py out/nbonacci_covering_proof/

nbonacci_homogeneous_shell_core_enumerate:
	RAVEL_PROBE_MEMORY_MB=4096 python3 python/nbonacci_homogeneous_shell_core_enumerate.py --n-min=2 --n-max=7 --max-cores=6

nbonacci_dominance_theorem_pipeline: $(NBONACCI_CARRY_CYCLE_PROBE_BIN) $(NBONACCI_ARITHMETIC_HULL_BIN)
	python3 python/nbonacci_dominance_theorem_pipeline.py --n-min=3 --n-max=8 --carry-timeout=120 --dominance-timeout=120

nbonacci_scc_pattern_miner: $(NBONACCI_ARITHMETIC_HULL_BIN)
	python3 python/nbonacci_scc_pattern_miner.py --n-min=3 --n-max=9 --total-budget-mb=9000 --base-mb=32 --timeout=900

$(NBONACCI_ARITHMETIC_HULL_BIN): \
		$(APPDIR)/nbonacci_arithmetic_hull.cpp \
		$(INCDIR)/ravel/nbonacci_margin_invariant.hpp \
		$(wildcard $(MATH_INCDIR)/math/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# ====================================================================
# Tests (paired _test.cpp files; standalone binaries with main())
# ====================================================================

TESTS_DEFAULT := \
	d_cont_check_test \
	tile_faces_test \
	number_field_parity_test \
	poly_discriminant_bigint_test \
	fp_poly_factor_test \
	fp_poly_extgcd_test \
	zp_poly_extended_gcd_test \
	maximal_order_bigint_test \
	general_order_radical_test \
	golod_shafarevich_test \
	sawin_exponent_test \
	quadratic_class_group_test \
	sawin_lemma9_test \
	irreducibility_test \
	ambient_graph_test \
	corona_test \
	contact_boundary_test \
	exact_pisot_test \
	pisot_degree_bound_safety_test \
	pisot_classify_degree_n_test \
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
	coincidence_closure_prefix_test \
	single_junction_coincidence_composition_test \
	second_genuine_fourth_generator_property_f_test \
	property_f_family_autopsy \
	property_f_transport_certificate_test \
	property_f_contact_transport_bridge_test \
	property_f_escape_rank_test \
	property_f_birth_round_grammar_test \
	property_f_class_ii_birth_round_test \
	stratified_escape_certificate_test \
	canonical_nonunit_property_f_test \
	nonunit_property_f_theorem_test \
	generalized_multinacci_property_f_transport_test \
	local_field_test \
	graph_divisor_test \
	maximal_order_test \
	gb_bp_hop_rule_test \
	nbonacci_margin_invariant_test \
	nbonacci_margin_core_graph_test \
	nbonacci_block_identity_test \
	nbonacci_covering_witness_test \
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
	marker_power_return_core_test \
	return_core_spectral_certificate_test \
	nonar_return_contact_lift_test \
	nonar_property_f_bridge_test \
	nonunit_property_f_bridge_test \
	rnd13_property_f_bridge_test \
	return_contact_lift_test \
	class_ii_boundary_family_test \
	substitution_neighborhood_test \
	family_of_families_test \
	family_closed_forms_test \
	pisot_numeration_topology_test \
	openai_unit_distance_test \
	csy_carry_automaton_test \
	csy_finite_carry_automaton_test \
	class_ii_neighbor_d_matrix_test \
	lean_class_ii_catalogue_cross_check_test \
	proof_reflection_test \
	proof_campaign_engine_test \
	generalized_campaign_test \
	legacy_campaign_bridge_test \
	qmatrix_typed_ir_test \
	class_ii_backward_layers_reflection_test \
	cayley_hamilton_cubic_reflection_test \
	class_ii_shell_citation_test \
	class_ii_penultimate_pair_reflection_test \
	class_ii_both_fixed_affine_reflection_test \
	class_ii_six_vertex_graduation_reflection_test \
	class_ii_neighbor_d_support_reflection_test \
	constant_last_letter_reflection_test \
	class_ii_interior_tip_reflection_test \
	class_ii_global_round_phase_reflection_test \
	barge_diamond_finding37_test \
	class_ii_d_cont_in_pre_contact_reflection_test \
	class_ii_fixed_tables_reflection_test \
	last_letter_orbit_reflection_test \
	depressed_cubic_not_pisot_reflection_test \
	class_ii_terminal_shell_reflection_test \
	barge_diamond_certificate_test \
	class_ii_neighbor2_fixed_reflection_test \
	class_ii_round1_raw27_reflection_test \
	barge_diamond_wide_batch_test \
	period_rotation_certificate_test \
	constant_first_letter_reflection_test \
	first_letter_orbit_finding41_test \
	first_letter_orbit_reflection_test \
	pisot_root_ordering_reflection_test \
	sturm_chain_reflection_test \
	leftmost_loop_reflection_test \
	zero_run_same_chain_reflection_test \
	branching_walk_composition_test \
	class_ii_terminal_sextet_reflection_test \
	adjacent_swap_count_reflection_test \
	feeder_cycle_charpoly_reflection_test \
	regular_shell_charpoly_reflection_test \
	strict_shell_pump_reflection_test \
	realized_first_return_completeness_test \
	recurrent_family_exhaustion_reflection_test \
	predicted_core_scc_exhaustion_reflection_test \
	coupled_winning_predicate_reflection_test \
	radial_translation_defect_reflection_test \
	defect_splice_step_reflection_test \
	condition_f_joint_dominance_reflection_test \
	finite_positive_grammar_majorant_reflection_test \
	third_smallest_pisot_parry_factorization_reflection_test \
	first_genuine_fourth_generator_intertwiner_test

tests: $(TESTS_DEFAULT)

# --- C++ test binaries ---

D_CONT_TEST_BIN := $(BUILDDIR)/d_cont_check_test
d_cont_check_test: $(D_CONT_TEST_BIN)
$(D_CONT_TEST_BIN): $(TESTDIR)/d_cont_check_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

TILE_FACES_TEST_BIN := $(BUILDDIR)/tile_faces_test
tile_faces_test: $(TILE_FACES_TEST_BIN)
$(TILE_FACES_TEST_BIN): $(TESTDIR)/tile_faces_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

NUMBER_FIELD_PARITY_TEST_BIN := $(BUILDDIR)/number_field_parity_test
number_field_parity_test: $(NUMBER_FIELD_PARITY_TEST_BIN)
$(NUMBER_FIELD_PARITY_TEST_BIN): $(TESTDIR)/number_field_parity_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

POLY_DISCRIMINANT_BIGINT_TEST_BIN := $(BUILDDIR)/poly_discriminant_bigint_test
poly_discriminant_bigint_test: $(POLY_DISCRIMINANT_BIGINT_TEST_BIN)
$(POLY_DISCRIMINANT_BIGINT_TEST_BIN): $(TESTDIR)/poly_discriminant_bigint_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

FP_POLY_FACTOR_TEST_BIN := $(BUILDDIR)/fp_poly_factor_test
fp_poly_factor_test: $(FP_POLY_FACTOR_TEST_BIN)
$(FP_POLY_FACTOR_TEST_BIN): $(TESTDIR)/fp_poly_factor_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

FP_POLY_EXTGCD_TEST_BIN := $(BUILDDIR)/fp_poly_extgcd_test
fp_poly_extgcd_test: $(FP_POLY_EXTGCD_TEST_BIN)
$(FP_POLY_EXTGCD_TEST_BIN): $(TESTDIR)/fp_poly_extgcd_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

ZP_POLY_EXTENDED_GCD_TEST_BIN := $(BUILDDIR)/zp_poly_extended_gcd_test
zp_poly_extended_gcd_test: $(ZP_POLY_EXTENDED_GCD_TEST_BIN)
$(ZP_POLY_EXTENDED_GCD_TEST_BIN): $(TESTDIR)/zp_poly_extended_gcd_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

MAXIMAL_ORDER_BIGINT_TEST_BIN := $(BUILDDIR)/maximal_order_bigint_test
maximal_order_bigint_test: $(MAXIMAL_ORDER_BIGINT_TEST_BIN)
$(MAXIMAL_ORDER_BIGINT_TEST_BIN): $(TESTDIR)/maximal_order_bigint_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

GENERAL_ORDER_RADICAL_TEST_BIN := $(BUILDDIR)/general_order_radical_test
general_order_radical_test: $(GENERAL_ORDER_RADICAL_TEST_BIN)
$(GENERAL_ORDER_RADICAL_TEST_BIN): $(TESTDIR)/general_order_radical_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

GOLOD_SHAFAREVICH_TEST_BIN := $(BUILDDIR)/golod_shafarevich_test
golod_shafarevich_test: $(GOLOD_SHAFAREVICH_TEST_BIN)
$(GOLOD_SHAFAREVICH_TEST_BIN): $(TESTDIR)/golod_shafarevich_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

SAWIN_EXPONENT_TEST_BIN := $(BUILDDIR)/sawin_exponent_test
sawin_exponent_test: $(SAWIN_EXPONENT_TEST_BIN)
$(SAWIN_EXPONENT_TEST_BIN): $(TESTDIR)/sawin_exponent_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

QUADRATIC_CLASS_GROUP_TEST_BIN := $(BUILDDIR)/quadratic_class_group_test
quadratic_class_group_test: $(QUADRATIC_CLASS_GROUP_TEST_BIN)
$(QUADRATIC_CLASS_GROUP_TEST_BIN): $(TESTDIR)/quadratic_class_group_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

SAWIN_LEMMA9_TEST_BIN := $(BUILDDIR)/sawin_lemma9_test
sawin_lemma9_test: $(SAWIN_LEMMA9_TEST_BIN)
$(SAWIN_LEMMA9_TEST_BIN): $(TESTDIR)/sawin_lemma9_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

IRREDUCIBILITY_TEST_BIN := $(BUILDDIR)/irreducibility_test
irreducibility_test: $(IRREDUCIBILITY_TEST_BIN)
$(IRREDUCIBILITY_TEST_BIN): $(TESTDIR)/irreducibility_test.cpp | $(BUILDDIR) $(MATH_LIB)
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

PISOT_DEGREE_BOUND_SAFETY_TEST_BIN := $(BUILDDIR)/pisot_degree_bound_safety_test
pisot_degree_bound_safety_test: $(PISOT_DEGREE_BOUND_SAFETY_TEST_BIN)
$(PISOT_DEGREE_BOUND_SAFETY_TEST_BIN): $(TESTDIR)/pisot_degree_bound_safety_test.cpp $(MATH_INCDIR)/exact_pisot.h | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) -I$(MATH_INCDIR) -I$(MATH_INCGMPL) $< $(MATH_LIB) -o $@

PISOT_CLASSIFY_DEGREE_N_TEST_BIN := $(BUILDDIR)/pisot_classify_degree_n_test
pisot_classify_degree_n_test: $(PISOT_CLASSIFY_DEGREE_N_TEST_BIN)
$(PISOT_CLASSIFY_DEGREE_N_TEST_BIN): $(TESTDIR)/pisot_classify_degree_n_test.cpp $(MATH_INCDIR)/exact_pisot.h | $(BUILDDIR) $(MATH_LIB)
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

FAMILY_CLOSED_FORMS_TEST_BIN := $(BUILDDIR)/family_closed_forms_test
family_closed_forms_test: $(FAMILY_CLOSED_FORMS_TEST_BIN)
$(FAMILY_CLOSED_FORMS_TEST_BIN): $(TESTDIR)/family_closed_forms_test.cpp \
		$(INCDIR)/ravel/family_closed_forms.hpp \
		$(INCDIR)/ravel/substitution_neighborhood.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

NBONACCI_MARGIN_INVARIANT_TEST_BIN := $(BUILDDIR)/nbonacci_margin_invariant_test
nbonacci_margin_invariant_test: $(NBONACCI_MARGIN_INVARIANT_TEST_BIN)
$(NBONACCI_MARGIN_INVARIANT_TEST_BIN): $(TESTDIR)/nbonacci_margin_invariant_test.cpp \
		$(INCDIR)/ravel/nbonacci_margin_invariant.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

NBONACCI_MARGIN_CORE_GRAPH_TEST_BIN := $(BUILDDIR)/nbonacci_margin_core_graph_test
nbonacci_margin_core_graph_test: $(NBONACCI_MARGIN_CORE_GRAPH_TEST_BIN)
$(NBONACCI_MARGIN_CORE_GRAPH_TEST_BIN): \
		$(TESTDIR)/nbonacci_margin_core_graph_test.cpp \
		$(INCDIR)/ravel/nbonacci_margin_invariant.hpp \
		$(INCDIR)/ravel/graph_divisor.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

NBONACCI_BLOCK_IDENTITY_TEST_BIN := $(BUILDDIR)/nbonacci_block_identity_test
nbonacci_block_identity_test: $(NBONACCI_BLOCK_IDENTITY_TEST_BIN)
$(NBONACCI_BLOCK_IDENTITY_TEST_BIN): $(TESTDIR)/nbonacci_block_identity_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# nbonacci_covering_witness_test: regression test for the n=2..8
# homogeneous-shell covering witnesses emitted by
# python/nbonacci_shell_covering_proof.py.  Each witness is
# (pins, signs, free_params, denom, full_sequence); the test
# reconstructs the sequence from the free params via the
# homogeneous recurrence and verifies the box and cover
# properties.  Exact integer arithmetic (no floats, no Z3).
# Also exercises the reusable C++ core and therefore links its object.
NBONACCI_COVERING_WITNESS_TEST_BIN := $(BUILDDIR)/nbonacci_covering_witness_test
nbonacci_covering_witness_test: $(NBONACCI_COVERING_WITNESS_TEST_BIN)
$(NBONACCI_COVERING_WITNESS_TEST_BIN): $(TESTDIR)/nbonacci_covering_witness_test.cpp \
		$(BUILDDIR)/nbonacci_covering_witness.o | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(BUILDDIR)/nbonacci_covering_witness.o -o $@

CMATRIX_TEST_BIN := $(BUILDDIR)/characteristic_matrix_source_test
characteristic_matrix_source_test: $(CMATRIX_TEST_BIN)
$(CMATRIX_TEST_BIN): tests/characteristic_matrix_source_test.cpp \
		$(INCDIR)/ravel/proof/characteristic_matrix_source.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

nbonacci_covering_witness_enumerator: $(NBONACCI_COVERING_WITNESS_ENUMERATOR_BIN)
nbonacci_data_shaker: $(NBONACCI_DATA_SHAKER_BIN) nbonacci_covering_witness_enumerate

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

MARKER_POWER_RETURN_CORE_TEST_BIN := $(BUILDDIR)/marker_power_return_core_test
marker_power_return_core_test: $(MARKER_POWER_RETURN_CORE_TEST_BIN)
$(MARKER_POWER_RETURN_CORE_TEST_BIN): $(TESTDIR)/marker_power_return_core_test.cpp \
		$(INCDIR)/ravel/marker_power_return_core.hpp \
		$(INCDIR)/ravel/return_substitution.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

RETURN_CORE_SPECTRAL_CERTIFICATE_TEST_BIN := $(BUILDDIR)/return_core_spectral_certificate_test
return_core_spectral_certificate_test: $(RETURN_CORE_SPECTRAL_CERTIFICATE_TEST_BIN)
$(RETURN_CORE_SPECTRAL_CERTIFICATE_TEST_BIN): $(TESTDIR)/return_core_spectral_certificate_test.cpp \
		$(INCDIR)/ravel/return_core_spectral_certificate.hpp \
		$(INCDIR)/ravel/marker_power_return_core.hpp \
		$(INCDIR)/ravel/return_substitution.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

NONAR_RETURN_CONTACT_LIFT_TEST_BIN := $(BUILDDIR)/nonar_return_contact_lift_test
nonar_return_contact_lift_test: $(NONAR_RETURN_CONTACT_LIFT_TEST_BIN)
$(NONAR_RETURN_CONTACT_LIFT_TEST_BIN): $(TESTDIR)/nonar_return_contact_lift_test.cpp \
		$(INCDIR)/ravel/return_contact_lift.hpp \
		$(INCDIR)/ravel/return_history_factor.hpp \
		$(INCDIR)/ravel/return_offset_fibre_certificate.hpp \
		$(INCDIR)/ravel/marker_power_return_core.hpp \
		$(INCDIR)/adelic/property_f_birth_round_grammar.hpp \
		$(INCDIR)/adelic/property_f_escape_rank.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

NONAR_PROPERTY_F_BRIDGE_TEST_BIN := $(BUILDDIR)/nonar_property_f_bridge_test
nonar_property_f_bridge_test: $(NONAR_PROPERTY_F_BRIDGE_TEST_BIN)
$(NONAR_PROPERTY_F_BRIDGE_TEST_BIN): $(TESTDIR)/nonar_property_f_bridge_test.cpp \
		$(INCDIR)/adelic/property_f_contact_transport_bridge.hpp \
		$(INCDIR)/adelic/prefix_automaton.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

PROPERTY_F_ESCAPE_RANK_TEST_BIN := $(BUILDDIR)/property_f_escape_rank_test
property_f_escape_rank_test: $(PROPERTY_F_ESCAPE_RANK_TEST_BIN)
$(PROPERTY_F_ESCAPE_RANK_TEST_BIN): $(TESTDIR)/property_f_escape_rank_test.cpp \
		$(INCDIR)/adelic/property_f_escape_rank.hpp \
		$(INCDIR)/adelic/property_f_types.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

PROPERTY_F_BIRTH_ROUND_GRAMMAR_TEST_BIN := $(BUILDDIR)/property_f_birth_round_grammar_test
property_f_birth_round_grammar_test: $(PROPERTY_F_BIRTH_ROUND_GRAMMAR_TEST_BIN)
$(PROPERTY_F_BIRTH_ROUND_GRAMMAR_TEST_BIN): $(TESTDIR)/property_f_birth_round_grammar_test.cpp \
		$(INCDIR)/adelic/property_f_birth_round_grammar.hpp \
		$(INCDIR)/adelic/property_f_escape_rank.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

PROPERTY_F_CLASS_II_BIRTH_ROUND_TEST_BIN := $(BUILDDIR)/property_f_class_ii_birth_round_test
property_f_class_ii_birth_round_test: $(PROPERTY_F_CLASS_II_BIRTH_ROUND_TEST_BIN)
$(PROPERTY_F_CLASS_II_BIRTH_ROUND_TEST_BIN): $(TESTDIR)/property_f_class_ii_birth_round_test.cpp \
		$(INCDIR)/adelic/property_f_birth_round_grammar.hpp \
		$(INCDIR)/adelic/property_f_class_ii_collar_grammar.hpp \
		$(INCDIR)/adelic/property_f_class_ii_affine_tail.hpp \
		$(INCDIR)/adelic/property_f_class_ii_branch_census.hpp \
		$(INCDIR)/adelic/property_f_class_ii_tail_candidate_census.hpp \
		$(INCDIR)/adelic/property_f_class_ii_prefix_role_grammar.hpp \
		$(INCDIR)/adelic/property_f_class_ii_phase_strip.hpp \
		$(INCDIR)/adelic/property_f_class_ii_phase_absorption.hpp \
		$(INCDIR)/adelic/property_f_class_ii_phase_carrier.hpp \
		$(INCDIR)/adelic/property_f_class_ii_carry_deviation.hpp \
		$(INCDIR)/adelic/property_f_class_ii_akiyama_spine.hpp \
		$(INCDIR)/adelic/property_f_class_ii_prefix_correction.hpp \
		$(INCDIR)/adelic/property_f_class_ii_akiyama_window.hpp \
		$(INCDIR)/adelic/property_f_class_ii_akiyama_window_census.hpp \
		$(INCDIR)/adelic/property_f_class_ii_correction_absorption.hpp \
		$(INCDIR)/adelic/property_f_class_ii_akiyama_correction_cases.hpp \
		$(INCDIR)/adelic/property_f_class_ii_symbolic_tail_grammar.hpp \
		$(INCDIR)/adelic/property_f_class_ii_rank_spine.hpp \
		$(INCDIR)/adelic/coincidence_and_property_f.hpp \
		$(INCDIR)/adelic/prefix_automaton.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

STRATIFIED_ESCAPE_CERTIFICATE_TEST_BIN := $(BUILDDIR)/stratified_escape_certificate_test
stratified_escape_certificate_test: $(STRATIFIED_ESCAPE_CERTIFICATE_TEST_BIN)
	./$(STRATIFIED_ESCAPE_CERTIFICATE_TEST_BIN)
$(STRATIFIED_ESCAPE_CERTIFICATE_TEST_BIN): $(TESTDIR)/stratified_escape_certificate_test.cpp \
		$(INCDIR)/ravel/proof/finite_graph_correspondence.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

NONUNIT_PROPERTY_F_BRIDGE_TEST_BIN := $(BUILDDIR)/nonunit_property_f_bridge_test
nonunit_property_f_bridge_test: $(NONUNIT_PROPERTY_F_BRIDGE_TEST_BIN)
$(NONUNIT_PROPERTY_F_BRIDGE_TEST_BIN): $(TESTDIR)/nonunit_property_f_bridge_test.cpp \
		$(INCDIR)/adelic/property_f_contact_transport_bridge.hpp \
		$(INCDIR)/adelic/prefix_automaton.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

RND13_PROPERTY_F_BRIDGE_TEST_BIN := $(BUILDDIR)/rnd13_property_f_bridge_test
rnd13_property_f_bridge_test: $(RND13_PROPERTY_F_BRIDGE_TEST_BIN)
$(RND13_PROPERTY_F_BRIDGE_TEST_BIN): $(TESTDIR)/rnd13_property_f_bridge_test.cpp \
		$(INCDIR)/adelic/property_f_contact_transport_bridge.hpp \
		$(INCDIR)/adelic/prefix_automaton.hpp | $(BUILDDIR) $(MATH_LIB)
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

PROPERTY_F_REFLECTION_TEST_BIN := $(BUILDDIR)/property_f_reflection_test
property_f_reflection_test: $(PROPERTY_F_REFLECTION_TEST_BIN)
	./$(PROPERTY_F_REFLECTION_TEST_BIN)
$(PROPERTY_F_REFLECTION_TEST_BIN): $(TESTDIR)/property_f_reflection_test.cpp \
		$(INCDIR)/adelic/coincidence_and_property_f.hpp \
		$(INCDIR)/ravel/proof/property_f_finite_run_certificate.hpp \
		$(INCDIR)/ravel/proof/reflective_lean_renderer.hpp | $(BUILDDIR) $(MATH_LIB)
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

# class_ii_neighbor2_symmetric_seed_closure: tests whether
# class_ii_neighbor2_signed_contact_set() (the hardcoded 50-state ±C)
# is the backward_closure/red_anode result of the SYMMETRIC seed
# D_cont union mirror(D_cont), as opposed to the already-refuted
# "mirror the seeds alone" hypothesis or the merely-definitional
# union identity.
CLASS_II_NEIGHBOR2_SYMMETRIC_SEED_CLOSURE_BIN := $(BUILDDIR)/class_ii_neighbor2_symmetric_seed_closure
class_ii_neighbor2_symmetric_seed_closure: $(CLASS_II_NEIGHBOR2_SYMMETRIC_SEED_CLOSURE_BIN)
	./$(CLASS_II_NEIGHBOR2_SYMMETRIC_SEED_CLOSURE_BIN)
$(CLASS_II_NEIGHBOR2_SYMMETRIC_SEED_CLOSURE_BIN): $(APPDIR)/class_ii_neighbor2_symmetric_seed_closure.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round4_stable_bridge_check: the actual remaining base-
# premises gap. Checks whether the generic stable-machinery source
# formula at round 4 (class_ii_center_layer_candidate(a,4) union
# class_ii_neighbor2_layer_extension(a,4)) equals the independently
# ground-truth-computed round-4 post-Red survivor set.
CLASS_II_ROUND4_STABLE_BRIDGE_CHECK_BIN := $(BUILDDIR)/class_ii_round4_stable_bridge_check
class_ii_round4_stable_bridge_check: $(CLASS_II_ROUND4_STABLE_BRIDGE_CHECK_BIN)
	./$(CLASS_II_ROUND4_STABLE_BRIDGE_CHECK_BIN)
$(CLASS_II_ROUND4_STABLE_BRIDGE_CHECK_BIN): $(APPDIR)/class_ii_round4_stable_bridge_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_neighbor2_recurrent_exhaustion_check: recurrent-SCC
# exhaustion, the layer of work after the base-premises seam. Extends
# the existing (a<=8) catalogue-vs-Tarjan comparison into the a>=7
# regime and adds an explicit zero-cross-block-edge check.
CLASS_II_NEIGHBOR2_RECURRENT_EXHAUSTION_CHECK_BIN := $(BUILDDIR)/class_ii_neighbor2_recurrent_exhaustion_check
class_ii_neighbor2_recurrent_exhaustion_check: $(CLASS_II_NEIGHBOR2_RECURRENT_EXHAUSTION_CHECK_BIN)
	./$(CLASS_II_NEIGHBOR2_RECURRENT_EXHAUSTION_CHECK_BIN)
$(CLASS_II_NEIGHBOR2_RECURRENT_EXHAUSTION_CHECK_BIN): $(APPDIR)/class_ii_neighbor2_recurrent_exhaustion_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_neighbor2_round_stratified_transient_check: recurrent-SCC
# exhaustion items 3+4. Assigns every boundary state a birth round
# from the real corona trace, checks whether recurrent blocks have a
# single consistent birth round, then scans real edges for an escape
# witness per transient round-group (item 3) and zero returns to an
# earlier transient stratum (item 4).
CLASS_II_NEIGHBOR2_ROUND_STRATIFIED_TRANSIENT_CHECK_BIN := $(BUILDDIR)/class_ii_neighbor2_round_stratified_transient_check
class_ii_neighbor2_round_stratified_transient_check: $(CLASS_II_NEIGHBOR2_ROUND_STRATIFIED_TRANSIENT_CHECK_BIN)
	./$(CLASS_II_NEIGHBOR2_ROUND_STRATIFIED_TRANSIENT_CHECK_BIN)
$(CLASS_II_NEIGHBOR2_ROUND_STRATIFIED_TRANSIENT_CHECK_BIN): $(APPDIR)/class_ii_neighbor2_round_stratified_transient_check.cpp \
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

# class_ii_neighbor2_round1_union_hypothesis: exact-checked (a=3..60
# dense, plus 100/500/1000 outliers) confirmation that
# class_ii_neighbor2_signed_contact_set() equals the union of the
# D_cont(tau_a)-union-mirror-seeded backward_closure/red_anode result,
# E_1, and mirror(sigma_a's own contact set) -- a specific, concrete
# answer to Round 1's previously-open "what is the actual reverse-
# inclusion target" question. Not yet a symbolic (all-a) proof.
CLASS_II_NEIGHBOR2_ROUND1_UNION_HYPOTHESIS_BIN := $(BUILDDIR)/class_ii_neighbor2_round1_union_hypothesis
class_ii_neighbor2_round1_union_hypothesis: $(CLASS_II_NEIGHBOR2_ROUND1_UNION_HYPOTHESIS_BIN)
	./$(CLASS_II_NEIGHBOR2_ROUND1_UNION_HYPOTHESIS_BIN)
$(CLASS_II_NEIGHBOR2_ROUND1_UNION_HYPOTHESIS_BIN): $(APPDIR)/class_ii_neighbor2_round1_union_hypothesis.cpp \
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

# class_ii_round2_pruned_states_a_independence_probe: recon for the
# still-open symbolic Red-exclusion proof -- checks whether the 123
# pruned STATES (not just counts) are the literal same coordinate set
# across a, so a symbolic proof only needs per-category M-matrix
# arguments rather than a-varying state enumeration.
CLASS_II_ROUND2_PRUNED_STATES_A_INDEPENDENCE_PROBE_BIN := $(BUILDDIR)/class_ii_round2_pruned_states_a_independence_probe
class_ii_round2_pruned_states_a_independence_probe: $(CLASS_II_ROUND2_PRUNED_STATES_A_INDEPENDENCE_PROBE_BIN)
	./$(CLASS_II_ROUND2_PRUNED_STATES_A_INDEPENDENCE_PROBE_BIN)
$(CLASS_II_ROUND2_PRUNED_STATES_A_INDEPENDENCE_PROBE_BIN): $(APPDIR)/class_ii_round2_pruned_states_a_independence_probe.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round2_edge_structure_a_independence_probe: sharper than
# the pruned-states probe above -- checks whether the entire
# forward-edge connectivity graph over the 195 raw states (not just
# the pruned node set) is a-independent.
CLASS_II_ROUND2_EDGE_STRUCTURE_A_INDEPENDENCE_PROBE_BIN := $(BUILDDIR)/class_ii_round2_edge_structure_a_independence_probe
class_ii_round2_edge_structure_a_independence_probe: $(CLASS_II_ROUND2_EDGE_STRUCTURE_A_INDEPENDENCE_PROBE_BIN)
	./$(CLASS_II_ROUND2_EDGE_STRUCTURE_A_INDEPENDENCE_PROBE_BIN)
$(CLASS_II_ROUND2_EDGE_STRUCTURE_A_INDEPENDENCE_PROBE_BIN): $(APPDIR)/class_ii_round2_edge_structure_a_independence_probe.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round2_edge_structure_far_a_check: stress test at a=50 (far
# outside the sampled cluster) for the finding above.
CLASS_II_ROUND2_EDGE_STRUCTURE_FAR_A_CHECK_BIN := $(BUILDDIR)/class_ii_round2_edge_structure_far_a_check
class_ii_round2_edge_structure_far_a_check: $(CLASS_II_ROUND2_EDGE_STRUCTURE_FAR_A_CHECK_BIN)
	./$(CLASS_II_ROUND2_EDGE_STRUCTURE_FAR_A_CHECK_BIN)
$(CLASS_II_ROUND2_EDGE_STRUCTURE_FAR_A_CHECK_BIN): $(APPDIR)/class_ii_round2_edge_structure_far_a_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round3_pruned_states_a_independence_probe: the Round 2
# a-independence probe applied to Round 3.
CLASS_II_ROUND3_PRUNED_STATES_A_INDEPENDENCE_PROBE_BIN := $(BUILDDIR)/class_ii_round3_pruned_states_a_independence_probe
class_ii_round3_pruned_states_a_independence_probe: $(CLASS_II_ROUND3_PRUNED_STATES_A_INDEPENDENCE_PROBE_BIN)
	./$(CLASS_II_ROUND3_PRUNED_STATES_A_INDEPENDENCE_PROBE_BIN)
$(CLASS_II_ROUND3_PRUNED_STATES_A_INDEPENDENCE_PROBE_BIN): $(APPDIR)/class_ii_round3_pruned_states_a_independence_probe.cpp \
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

# class_ii_round4_structure: the Round 2/3 treatment applied to
# Round 4, the last of the four base transitions.
CLASS_II_ROUND4_STRUCTURE_BIN := $(BUILDDIR)/class_ii_round4_structure
class_ii_round4_structure: $(CLASS_II_ROUND4_STRUCTURE_BIN)
	./$(CLASS_II_ROUND4_STRUCTURE_BIN)
$(CLASS_II_ROUND4_STRUCTURE_BIN): $(APPDIR)/class_ii_round4_structure.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round4_pruned_states_a_independence_probe: the Round 2/3
# a-independence probe applied to Round 4, completing the check
# across all four base rounds.
CLASS_II_ROUND4_PRUNED_STATES_A_INDEPENDENCE_PROBE_BIN := $(BUILDDIR)/class_ii_round4_pruned_states_a_independence_probe
class_ii_round4_pruned_states_a_independence_probe: $(CLASS_II_ROUND4_PRUNED_STATES_A_INDEPENDENCE_PROBE_BIN)
	./$(CLASS_II_ROUND4_PRUNED_STATES_A_INDEPENDENCE_PROBE_BIN)
$(CLASS_II_ROUND4_PRUNED_STATES_A_INDEPENDENCE_PROBE_BIN): $(APPDIR)/class_ii_round4_pruned_states_a_independence_probe.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round{3,4}_edge_structure_a_independence_probe: the Round 2
# edge-structure probe above, applied to Rounds 3/4.
CLASS_II_ROUND3_EDGE_STRUCTURE_A_INDEPENDENCE_PROBE_BIN := $(BUILDDIR)/class_ii_round3_edge_structure_a_independence_probe
class_ii_round3_edge_structure_a_independence_probe: $(CLASS_II_ROUND3_EDGE_STRUCTURE_A_INDEPENDENCE_PROBE_BIN)
	./$(CLASS_II_ROUND3_EDGE_STRUCTURE_A_INDEPENDENCE_PROBE_BIN)
$(CLASS_II_ROUND3_EDGE_STRUCTURE_A_INDEPENDENCE_PROBE_BIN): $(APPDIR)/class_ii_round3_edge_structure_a_independence_probe.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

CLASS_II_ROUND4_EDGE_STRUCTURE_A_INDEPENDENCE_PROBE_BIN := $(BUILDDIR)/class_ii_round4_edge_structure_a_independence_probe
class_ii_round4_edge_structure_a_independence_probe: $(CLASS_II_ROUND4_EDGE_STRUCTURE_A_INDEPENDENCE_PROBE_BIN)
	./$(CLASS_II_ROUND4_EDGE_STRUCTURE_A_INDEPENDENCE_PROBE_BIN)
$(CLASS_II_ROUND4_EDGE_STRUCTURE_A_INDEPENDENCE_PROBE_BIN): $(APPDIR)/class_ii_round4_edge_structure_a_independence_probe.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round4_edge_mechanism_probe: inspects the raw
# (p_k,q_k)-indexed candidate list for one representative state to
# find WHY the edge structure is a-independent (not just confirm that
# it is): x0'/x1' constant, x2' affine in q_k-p_k rather than in a.
CLASS_II_ROUND4_EDGE_MECHANISM_PROBE_BIN := $(BUILDDIR)/class_ii_round4_edge_mechanism_probe
class_ii_round4_edge_mechanism_probe: $(CLASS_II_ROUND4_EDGE_MECHANISM_PROBE_BIN)
	./$(CLASS_II_ROUND4_EDGE_MECHANISM_PROBE_BIN)
$(CLASS_II_ROUND4_EDGE_MECHANISM_PROBE_BIN): $(APPDIR)/class_ii_round4_edge_mechanism_probe.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round2_edge_mechanism_generality_check: tests whether the
# mechanism above generalizes across all 195 Round 2 raw states. It
# does not -- recorded plainly rather than left as an unchecked
# generalization.
CLASS_II_ROUND2_EDGE_MECHANISM_GENERALITY_CHECK_BIN := $(BUILDDIR)/class_ii_round2_edge_mechanism_generality_check
class_ii_round2_edge_mechanism_generality_check: $(CLASS_II_ROUND2_EDGE_MECHANISM_GENERALITY_CHECK_BIN)
	./$(CLASS_II_ROUND2_EDGE_MECHANISM_GENERALITY_CHECK_BIN)
$(CLASS_II_ROUND2_EDGE_MECHANISM_GENERALITY_CHECK_BIN): $(APPDIR)/class_ii_round2_edge_mechanism_generality_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round2_edge_mechanism_by_type_check: refines the check
# above by grouping surviving edges by (parent_letter_i,
# parent_letter_j) type first, matching Round 1's actual granularity.
CLASS_II_ROUND2_EDGE_MECHANISM_BY_TYPE_CHECK_BIN := $(BUILDDIR)/class_ii_round2_edge_mechanism_by_type_check
class_ii_round2_edge_mechanism_by_type_check: $(CLASS_II_ROUND2_EDGE_MECHANISM_BY_TYPE_CHECK_BIN)
	./$(CLASS_II_ROUND2_EDGE_MECHANISM_BY_TYPE_CHECK_BIN)
$(CLASS_II_ROUND2_EDGE_MECHANISM_BY_TYPE_CHECK_BIN): $(APPDIR)/class_ii_round2_edge_mechanism_by_type_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round2_edge_mechanism_general_slope_check: last refinement
# in this line of inquiry -- fits whatever slope the data shows rather
# than forcing slope 1. Real improvement, still not universal.
CLASS_II_ROUND2_EDGE_MECHANISM_GENERAL_SLOPE_CHECK_BIN := $(BUILDDIR)/class_ii_round2_edge_mechanism_general_slope_check
class_ii_round2_edge_mechanism_general_slope_check: $(CLASS_II_ROUND2_EDGE_MECHANISM_GENERAL_SLOPE_CHECK_BIN)
	./$(CLASS_II_ROUND2_EDGE_MECHANISM_GENERAL_SLOPE_CHECK_BIN)
$(CLASS_II_ROUND2_EDGE_MECHANISM_GENERAL_SLOPE_CHECK_BIN): $(APPDIR)/class_ii_round2_edge_mechanism_general_slope_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round2_edge_mechanism_exact: closes the mechanism line of
# inquiry -- groups by the FULL abelianization vector (not just
# occurrence length), giving 100% clean x0'/x1' constant, x2' affine
# with slope exactly 1, at every tested a. See the file header for
# the full derivation.
CLASS_II_ROUND2_EDGE_MECHANISM_EXACT_BIN := $(BUILDDIR)/class_ii_round2_edge_mechanism_exact
class_ii_round2_edge_mechanism_exact: $(CLASS_II_ROUND2_EDGE_MECHANISM_EXACT_BIN)
	./$(CLASS_II_ROUND2_EDGE_MECHANISM_EXACT_BIN)
$(CLASS_II_ROUND2_EDGE_MECHANISM_EXACT_BIN): $(APPDIR)/class_ii_round2_edge_mechanism_exact.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round4_coverage_threshold_check: confirms the "coverage"
# remaining-gap mechanism directly on the one concrete exception found
# earlier (Round 4's a=6 exceptional edge).
CLASS_II_ROUND4_COVERAGE_THRESHOLD_CHECK_BIN := $(BUILDDIR)/class_ii_round4_coverage_threshold_check
class_ii_round4_coverage_threshold_check: $(CLASS_II_ROUND4_COVERAGE_THRESHOLD_CHECK_BIN)
	./$(CLASS_II_ROUND4_COVERAGE_THRESHOLD_CHECK_BIN)
$(CLASS_II_ROUND4_COVERAGE_THRESHOLD_CHECK_BIN): $(APPDIR)/class_ii_round4_coverage_threshold_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round2_edge_mechanism_shape_count: gauges tractability of
# the remaining uniform-bound step by counting distinct group shapes.
CLASS_II_ROUND2_EDGE_MECHANISM_SHAPE_COUNT_BIN := $(BUILDDIR)/class_ii_round2_edge_mechanism_shape_count
class_ii_round2_edge_mechanism_shape_count: $(CLASS_II_ROUND2_EDGE_MECHANISM_SHAPE_COUNT_BIN)
	./$(CLASS_II_ROUND2_EDGE_MECHANISM_SHAPE_COUNT_BIN)
$(CLASS_II_ROUND2_EDGE_MECHANISM_SHAPE_COUNT_BIN): $(APPDIR)/class_ii_round2_edge_mechanism_shape_count.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round34_edge_mechanism_shape_count: extends the shape-count
# tractability check to Rounds 3/4.
CLASS_II_ROUND34_EDGE_MECHANISM_SHAPE_COUNT_BIN := $(BUILDDIR)/class_ii_round34_edge_mechanism_shape_count
class_ii_round34_edge_mechanism_shape_count: $(CLASS_II_ROUND34_EDGE_MECHANISM_SHAPE_COUNT_BIN)
	./$(CLASS_II_ROUND34_EDGE_MECHANISM_SHAPE_COUNT_BIN)
$(CLASS_II_ROUND34_EDGE_MECHANISM_SHAPE_COUNT_BIN): $(APPDIR)/class_ii_round34_edge_mechanism_shape_count.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round2_edge_mechanism_shape_range_check: corrects an
# overclaimed 22/22 shape split -- checks directly whether p_len/q_len
# are genuinely ranges or fixed per shape (true split: 20/20/4).
CLASS_II_ROUND2_EDGE_MECHANISM_SHAPE_RANGE_CHECK_BIN := $(BUILDDIR)/class_ii_round2_edge_mechanism_shape_range_check
class_ii_round2_edge_mechanism_shape_range_check: $(CLASS_II_ROUND2_EDGE_MECHANISM_SHAPE_RANGE_CHECK_BIN)
	./$(CLASS_II_ROUND2_EDGE_MECHANISM_SHAPE_RANGE_CHECK_BIN)
$(CLASS_II_ROUND2_EDGE_MECHANISM_SHAPE_RANGE_CHECK_BIN): $(APPDIR)/class_ii_round2_edge_mechanism_shape_range_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_round2_edge_mechanism_escaping_slopes: tests whether the
# 20 both-fixed shapes actually escape (Round-1-style) or are simply
# constant -- checked at a=7 vs a=40, zero slope in all 20.
CLASS_II_ROUND2_EDGE_MECHANISM_ESCAPING_SLOPES_BIN := $(BUILDDIR)/class_ii_round2_edge_mechanism_escaping_slopes
class_ii_round2_edge_mechanism_escaping_slopes: $(CLASS_II_ROUND2_EDGE_MECHANISM_ESCAPING_SLOPES_BIN)
	./$(CLASS_II_ROUND2_EDGE_MECHANISM_ESCAPING_SLOPES_BIN)
$(CLASS_II_ROUND2_EDGE_MECHANISM_ESCAPING_SLOPES_BIN): $(APPDIR)/class_ii_round2_edge_mechanism_escaping_slopes.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_shape_classification_rule: derives and checks a rule
# predicting which shapes range vs are fixed, from tau_a's word
# structure directly, against every one of the 44 shapes.
CLASS_II_SHAPE_CLASSIFICATION_RULE_BIN := $(BUILDDIR)/class_ii_shape_classification_rule
class_ii_shape_classification_rule: $(CLASS_II_SHAPE_CLASSIFICATION_RULE_BIN)
	./$(CLASS_II_SHAPE_CLASSIFICATION_RULE_BIN)
$(CLASS_II_SHAPE_CLASSIFICATION_RULE_BIN): $(APPDIR)/class_ii_shape_classification_rule.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_shape_state_consistency_check: corrects a methodological
# gap in the escaping-slopes check -- a shape does not uniquely
# determine a state, so shape-level constancy claims must be checked
# across ALL states sharing a shape, not one representative.
CLASS_II_SHAPE_STATE_CONSISTENCY_CHECK_BIN := $(BUILDDIR)/class_ii_shape_state_consistency_check
class_ii_shape_state_consistency_check: $(CLASS_II_SHAPE_STATE_CONSISTENCY_CHECK_BIN)
	./$(CLASS_II_SHAPE_STATE_CONSISTENCY_CHECK_BIN)
$(CLASS_II_SHAPE_STATE_CONSISTENCY_CHECK_BIN): $(APPDIR)/class_ii_shape_state_consistency_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_shape_per_state_constancy_check: resolves the correction
# above by checking the right question -- per-state destination
# constancy, not per-shape x2' constancy.
CLASS_II_SHAPE_PER_STATE_CONSTANCY_CHECK_BIN := $(BUILDDIR)/class_ii_shape_per_state_constancy_check
class_ii_shape_per_state_constancy_check: $(CLASS_II_SHAPE_PER_STATE_CONSTANCY_CHECK_BIN)
	./$(CLASS_II_SHAPE_PER_STATE_CONSTANCY_CHECK_BIN)
$(CLASS_II_SHAPE_PER_STATE_CONSTANCY_CHECK_BIN): $(APPDIR)/class_ii_shape_per_state_constancy_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_hybrid_per_state_constancy_check: applies the same
# per-state resolution to the 20-shape hybrid category.
CLASS_II_HYBRID_PER_STATE_CONSTANCY_CHECK_BIN := $(BUILDDIR)/class_ii_hybrid_per_state_constancy_check
class_ii_hybrid_per_state_constancy_check: $(CLASS_II_HYBRID_PER_STATE_CONSTANCY_CHECK_BIN)
	./$(CLASS_II_HYBRID_PER_STATE_CONSTANCY_CHECK_BIN)
$(CLASS_II_HYBRID_PER_STATE_CONSTANCY_CHECK_BIN): $(APPDIR)/class_ii_hybrid_per_state_constancy_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_hybrid_window_monotonicity_check: the key lemma for a
# genuine symbolic proof -- does each hybrid state's achievable window
# grow monotonically (nested) as a increases? Checked at a=7,12,20,30.
CLASS_II_HYBRID_WINDOW_MONOTONICITY_CHECK_BIN := $(BUILDDIR)/class_ii_hybrid_window_monotonicity_check
class_ii_hybrid_window_monotonicity_check: $(CLASS_II_HYBRID_WINDOW_MONOTONICITY_CHECK_BIN)
	./$(CLASS_II_HYBRID_WINDOW_MONOTONICITY_CHECK_BIN)
$(CLASS_II_HYBRID_WINDOW_MONOTONICITY_CHECK_BIN): $(APPDIR)/class_ii_hybrid_window_monotonicity_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_hybrid_window_slope_derivation: derives WHY the window
# grows monotonically -- exhaustive slope enumeration over every fixed
# occurrence type, not another numerical sample.
CLASS_II_HYBRID_WINDOW_SLOPE_DERIVATION_BIN := $(BUILDDIR)/class_ii_hybrid_window_slope_derivation
class_ii_hybrid_window_slope_derivation: $(CLASS_II_HYBRID_WINDOW_SLOPE_DERIVATION_BIN)
	./$(CLASS_II_HYBRID_WINDOW_SLOPE_DERIVATION_BIN)
$(CLASS_II_HYBRID_WINDOW_SLOPE_DERIVATION_BIN): $(APPDIR)/class_ii_hybrid_window_slope_derivation.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_both_fixed_rhs2_check: tests (and refutes) the natural
# guess for extending the hybrid proof to the both-fixed category.
CLASS_II_BOTH_FIXED_RHS2_CHECK_BIN := $(BUILDDIR)/class_ii_both_fixed_rhs2_check
class_ii_both_fixed_rhs2_check: $(CLASS_II_BOTH_FIXED_RHS2_CHECK_BIN)
	./$(CLASS_II_BOTH_FIXED_RHS2_CHECK_BIN)
$(CLASS_II_BOTH_FIXED_RHS2_CHECK_BIN): $(APPDIR)/class_ii_both_fixed_rhs2_check.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_both_fixed_corrected_condition: corrects the error in the
# rhs2 check above (rhs[0] can also carry an a-dependent term) --
# the real condition is rhs2 = slope_q - slope_p, exhaustively
# confirmed at a=7 and a=20, completing the both-fixed category's proof.
CLASS_II_BOTH_FIXED_CORRECTED_CONDITION_BIN := $(BUILDDIR)/class_ii_both_fixed_corrected_condition
class_ii_both_fixed_corrected_condition: $(CLASS_II_BOTH_FIXED_CORRECTED_CONDITION_BIN)
	./$(CLASS_II_BOTH_FIXED_CORRECTED_CONDITION_BIN)
$(CLASS_II_BOTH_FIXED_CORRECTED_CONDITION_BIN): $(APPDIR)/class_ii_both_fixed_corrected_condition.cpp \
		$(wildcard $(INCDIR)/ravel/*.hpp) | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

# class_ii_both_fixed_full_proof: closed-form proof (solving directly
# for the required a, not sampling) that no slope-nonzero both-fixed
# edge can ever be valid for any integer a>=7.
CLASS_II_BOTH_FIXED_FULL_PROOF_BIN := $(BUILDDIR)/class_ii_both_fixed_full_proof
class_ii_both_fixed_full_proof: $(CLASS_II_BOTH_FIXED_FULL_PROOF_BIN)
	./$(CLASS_II_BOTH_FIXED_FULL_PROOF_BIN)
$(CLASS_II_BOTH_FIXED_FULL_PROOF_BIN): $(APPDIR)/class_ii_both_fixed_full_proof.cpp \
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

# ====================================================================
# Universal n-bonacci proof campaign
# ====================================================================

RAVEL_PROOF_RUNNER_BIN := $(BUILDDIR)/ravel_proof_runner
RAVEL_PROOF_RUNNER_SRCS := $(APPDIR)/ravel_proof_runner.cpp $(SRCDIR)/proof/lua_strategy_loader.cpp $(SRCDIR)/proof/index_bijection_registry.cpp
RAVEL_PROOF_RUNNER_HDRS := $(INCDIR)/ravel/proof/lua_strategy_loader.hpp

ravel_proof_runner: $(RAVEL_PROOF_RUNNER_BIN)

$(RAVEL_PROOF_RUNNER_BIN): $(RAVEL_PROOF_RUNNER_SRCS) $(RAVEL_PROOF_RUNNER_HDRS) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(LUA_CFLAGS) $(INCLUDES) $(RAVEL_PROOF_RUNNER_SRCS) $(LUA_LIBS) $(MATH_LIB) -o $@

# One-button entry point. The runner writes a timestamped run package under
# out/shoot_the_moon/ and stops at the first required failed foundation gate,
# while preserving the complete alternate-course tree for every obligation.
shoot-the-moon:
	./scripts/shoot_the_moon.sh


PROOF_REFLECTION_TEST_BIN := $(BUILDDIR)/proof_reflection_test
$(PROOF_REFLECTION_TEST_BIN): $(TESTDIR)/proof_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
proof_reflection_test: $(PROOF_REFLECTION_TEST_BIN)
	./$(PROOF_REFLECTION_TEST_BIN)
.PHONY: proof_reflection_test

PROOF_CAMPAIGN_ENGINE_TEST_BIN := $(BUILDDIR)/proof_campaign_engine_test
$(PROOF_CAMPAIGN_ENGINE_TEST_BIN): $(TESTDIR)/proof_campaign_engine_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
proof_campaign_engine_test: $(PROOF_CAMPAIGN_ENGINE_TEST_BIN)
	./$(PROOF_CAMPAIGN_ENGINE_TEST_BIN)
.PHONY: proof_campaign_engine_test

# ==== LEAN Bridge renderer tests: each rebuilds its canonical
# lean/generated/*.lean file directly (no /tmp indirection), so
# `make <name>` is always the unambiguous way to regenerate and
# re-verify the C++ side of any of these findings.
TEST_BIN_CLASS_II_BACKWARD_LAYERS_REFLECTION_TEST := $(BUILDDIR)/class_ii_backward_layers_reflection_test
$(TEST_BIN_CLASS_II_BACKWARD_LAYERS_REFLECTION_TEST): $(TESTDIR)/class_ii_backward_layers_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_backward_layers_reflection_test: $(TEST_BIN_CLASS_II_BACKWARD_LAYERS_REFLECTION_TEST)
	./$(TEST_BIN_CLASS_II_BACKWARD_LAYERS_REFLECTION_TEST)
.PHONY: class_ii_backward_layers_reflection_test

TEST_BIN_CAYLEY_HAMILTON_CUBIC_REFLECTION_TEST := $(BUILDDIR)/cayley_hamilton_cubic_reflection_test
$(TEST_BIN_CAYLEY_HAMILTON_CUBIC_REFLECTION_TEST): $(TESTDIR)/cayley_hamilton_cubic_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
cayley_hamilton_cubic_reflection_test: $(TEST_BIN_CAYLEY_HAMILTON_CUBIC_REFLECTION_TEST)
	./$(TEST_BIN_CAYLEY_HAMILTON_CUBIC_REFLECTION_TEST)
.PHONY: cayley_hamilton_cubic_reflection_test

TEST_BIN_CLASS_II_SHELL_CITATION_TEST := $(BUILDDIR)/class_ii_shell_citation_test
$(TEST_BIN_CLASS_II_SHELL_CITATION_TEST): $(TESTDIR)/class_ii_shell_citation_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_shell_citation_test: $(TEST_BIN_CLASS_II_SHELL_CITATION_TEST)
	./$(TEST_BIN_CLASS_II_SHELL_CITATION_TEST)
.PHONY: class_ii_shell_citation_test

TEST_BIN_CLASS_II_PENULTIMATE_PAIR_REFLECTION_TEST := $(BUILDDIR)/class_ii_penultimate_pair_reflection_test
$(TEST_BIN_CLASS_II_PENULTIMATE_PAIR_REFLECTION_TEST): $(TESTDIR)/class_ii_penultimate_pair_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_penultimate_pair_reflection_test: $(TEST_BIN_CLASS_II_PENULTIMATE_PAIR_REFLECTION_TEST)
	./$(TEST_BIN_CLASS_II_PENULTIMATE_PAIR_REFLECTION_TEST)
.PHONY: class_ii_penultimate_pair_reflection_test

TEST_BIN_CLASS_II_BOTH_FIXED_AFFINE_REFLECTION_TEST := $(BUILDDIR)/class_ii_both_fixed_affine_reflection_test
$(TEST_BIN_CLASS_II_BOTH_FIXED_AFFINE_REFLECTION_TEST): $(TESTDIR)/class_ii_both_fixed_affine_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_both_fixed_affine_reflection_test: $(TEST_BIN_CLASS_II_BOTH_FIXED_AFFINE_REFLECTION_TEST)
	./$(TEST_BIN_CLASS_II_BOTH_FIXED_AFFINE_REFLECTION_TEST)
.PHONY: class_ii_both_fixed_affine_reflection_test

TEST_BIN_CLASS_II_SIX_VERTEX_GRADUATION_REFLECTION_TEST := $(BUILDDIR)/class_ii_six_vertex_graduation_reflection_test
$(TEST_BIN_CLASS_II_SIX_VERTEX_GRADUATION_REFLECTION_TEST): $(TESTDIR)/class_ii_six_vertex_graduation_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_six_vertex_graduation_reflection_test: $(TEST_BIN_CLASS_II_SIX_VERTEX_GRADUATION_REFLECTION_TEST)
	./$(TEST_BIN_CLASS_II_SIX_VERTEX_GRADUATION_REFLECTION_TEST)
.PHONY: class_ii_six_vertex_graduation_reflection_test

TEST_BIN_CLASS_II_NEIGHBOR_D_SUPPORT_REFLECTION_TEST := $(BUILDDIR)/class_ii_neighbor_d_support_reflection_test
$(TEST_BIN_CLASS_II_NEIGHBOR_D_SUPPORT_REFLECTION_TEST): $(TESTDIR)/class_ii_neighbor_d_support_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_neighbor_d_support_reflection_test: $(TEST_BIN_CLASS_II_NEIGHBOR_D_SUPPORT_REFLECTION_TEST)
	./$(TEST_BIN_CLASS_II_NEIGHBOR_D_SUPPORT_REFLECTION_TEST)
.PHONY: class_ii_neighbor_d_support_reflection_test

TEST_BIN_CONSTANT_LAST_LETTER_REFLECTION_TEST := $(BUILDDIR)/constant_last_letter_reflection_test
$(TEST_BIN_CONSTANT_LAST_LETTER_REFLECTION_TEST): $(TESTDIR)/constant_last_letter_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
constant_last_letter_reflection_test: $(TEST_BIN_CONSTANT_LAST_LETTER_REFLECTION_TEST)
	./$(TEST_BIN_CONSTANT_LAST_LETTER_REFLECTION_TEST)
.PHONY: constant_last_letter_reflection_test

TEST_BIN_CLASS_II_INTERIOR_TIP_REFLECTION_TEST := $(BUILDDIR)/class_ii_interior_tip_reflection_test
$(TEST_BIN_CLASS_II_INTERIOR_TIP_REFLECTION_TEST): $(TESTDIR)/class_ii_interior_tip_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_interior_tip_reflection_test: $(TEST_BIN_CLASS_II_INTERIOR_TIP_REFLECTION_TEST)
	./$(TEST_BIN_CLASS_II_INTERIOR_TIP_REFLECTION_TEST)
.PHONY: class_ii_interior_tip_reflection_test

TEST_BIN_CLASS_II_GLOBAL_ROUND_PHASE_REFLECTION_TEST := $(BUILDDIR)/class_ii_global_round_phase_reflection_test
$(TEST_BIN_CLASS_II_GLOBAL_ROUND_PHASE_REFLECTION_TEST): $(TESTDIR)/class_ii_global_round_phase_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_global_round_phase_reflection_test: $(TEST_BIN_CLASS_II_GLOBAL_ROUND_PHASE_REFLECTION_TEST)
	./$(TEST_BIN_CLASS_II_GLOBAL_ROUND_PHASE_REFLECTION_TEST)
.PHONY: class_ii_global_round_phase_reflection_test

TEST_BIN_BARGE_DIAMOND_FINDING37_TEST := $(BUILDDIR)/barge_diamond_finding37_test
$(TEST_BIN_BARGE_DIAMOND_FINDING37_TEST): $(TESTDIR)/barge_diamond_finding37_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
barge_diamond_finding37_test: $(TEST_BIN_BARGE_DIAMOND_FINDING37_TEST)
	./$(TEST_BIN_BARGE_DIAMOND_FINDING37_TEST)
.PHONY: barge_diamond_finding37_test

TEST_BIN_CLASS_II_D_CONT_IN_PRE_CONTACT_REFLECTION_TEST := $(BUILDDIR)/class_ii_d_cont_in_pre_contact_reflection_test
$(TEST_BIN_CLASS_II_D_CONT_IN_PRE_CONTACT_REFLECTION_TEST): $(TESTDIR)/class_ii_d_cont_in_pre_contact_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_d_cont_in_pre_contact_reflection_test: $(TEST_BIN_CLASS_II_D_CONT_IN_PRE_CONTACT_REFLECTION_TEST)
	./$(TEST_BIN_CLASS_II_D_CONT_IN_PRE_CONTACT_REFLECTION_TEST)
.PHONY: class_ii_d_cont_in_pre_contact_reflection_test

TEST_BIN_CLASS_II_FIXED_TABLES_REFLECTION_TEST := $(BUILDDIR)/class_ii_fixed_tables_reflection_test
$(TEST_BIN_CLASS_II_FIXED_TABLES_REFLECTION_TEST): $(TESTDIR)/class_ii_fixed_tables_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_fixed_tables_reflection_test: $(TEST_BIN_CLASS_II_FIXED_TABLES_REFLECTION_TEST)
	./$(TEST_BIN_CLASS_II_FIXED_TABLES_REFLECTION_TEST)
.PHONY: class_ii_fixed_tables_reflection_test

TEST_BIN_LAST_LETTER_ORBIT_REFLECTION_TEST := $(BUILDDIR)/last_letter_orbit_reflection_test
$(TEST_BIN_LAST_LETTER_ORBIT_REFLECTION_TEST): $(TESTDIR)/last_letter_orbit_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
last_letter_orbit_reflection_test: $(TEST_BIN_LAST_LETTER_ORBIT_REFLECTION_TEST)
	./$(TEST_BIN_LAST_LETTER_ORBIT_REFLECTION_TEST)
.PHONY: last_letter_orbit_reflection_test

TEST_BIN_DEPRESSED_CUBIC_NOT_PISOT_REFLECTION_TEST := $(BUILDDIR)/depressed_cubic_not_pisot_reflection_test
$(TEST_BIN_DEPRESSED_CUBIC_NOT_PISOT_REFLECTION_TEST): $(TESTDIR)/depressed_cubic_not_pisot_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
depressed_cubic_not_pisot_reflection_test: $(TEST_BIN_DEPRESSED_CUBIC_NOT_PISOT_REFLECTION_TEST)
	./$(TEST_BIN_DEPRESSED_CUBIC_NOT_PISOT_REFLECTION_TEST)
.PHONY: depressed_cubic_not_pisot_reflection_test

TEST_BIN_CLASS_II_TERMINAL_SHELL_REFLECTION_TEST := $(BUILDDIR)/class_ii_terminal_shell_reflection_test
$(TEST_BIN_CLASS_II_TERMINAL_SHELL_REFLECTION_TEST): $(TESTDIR)/class_ii_terminal_shell_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_terminal_shell_reflection_test: $(TEST_BIN_CLASS_II_TERMINAL_SHELL_REFLECTION_TEST)
	./$(TEST_BIN_CLASS_II_TERMINAL_SHELL_REFLECTION_TEST)
.PHONY: class_ii_terminal_shell_reflection_test

TEST_BIN_BARGE_DIAMOND_CERTIFICATE_TEST := $(BUILDDIR)/barge_diamond_certificate_test
$(TEST_BIN_BARGE_DIAMOND_CERTIFICATE_TEST): $(TESTDIR)/barge_diamond_certificate_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
barge_diamond_certificate_test: $(TEST_BIN_BARGE_DIAMOND_CERTIFICATE_TEST)
	./$(TEST_BIN_BARGE_DIAMOND_CERTIFICATE_TEST)
.PHONY: barge_diamond_certificate_test

TEST_BIN_CLASS_II_NEIGHBOR2_FIXED_REFLECTION_TEST := $(BUILDDIR)/class_ii_neighbor2_fixed_reflection_test
$(TEST_BIN_CLASS_II_NEIGHBOR2_FIXED_REFLECTION_TEST): $(TESTDIR)/class_ii_neighbor2_fixed_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_neighbor2_fixed_reflection_test: $(TEST_BIN_CLASS_II_NEIGHBOR2_FIXED_REFLECTION_TEST)
	./$(TEST_BIN_CLASS_II_NEIGHBOR2_FIXED_REFLECTION_TEST)
.PHONY: class_ii_neighbor2_fixed_reflection_test

TEST_BIN_CLASS_II_ROUND1_RAW27_REFLECTION_TEST := $(BUILDDIR)/class_ii_round1_raw27_reflection_test
$(TEST_BIN_CLASS_II_ROUND1_RAW27_REFLECTION_TEST): $(TESTDIR)/class_ii_round1_raw27_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_round1_raw27_reflection_test: $(TEST_BIN_CLASS_II_ROUND1_RAW27_REFLECTION_TEST)
	./$(TEST_BIN_CLASS_II_ROUND1_RAW27_REFLECTION_TEST)
.PHONY: class_ii_round1_raw27_reflection_test

TEST_BIN_BARGE_DIAMOND_WIDE_BATCH_TEST := $(BUILDDIR)/barge_diamond_wide_batch_test
$(TEST_BIN_BARGE_DIAMOND_WIDE_BATCH_TEST): $(TESTDIR)/barge_diamond_wide_batch_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
barge_diamond_wide_batch_test: $(TEST_BIN_BARGE_DIAMOND_WIDE_BATCH_TEST)
	./$(TEST_BIN_BARGE_DIAMOND_WIDE_BATCH_TEST)
.PHONY: barge_diamond_wide_batch_test

TEST_BIN_PERIOD_ROTATION_CERTIFICATE_TEST := $(BUILDDIR)/period_rotation_certificate_test
$(TEST_BIN_PERIOD_ROTATION_CERTIFICATE_TEST): $(TESTDIR)/period_rotation_certificate_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
period_rotation_certificate_test: $(TEST_BIN_PERIOD_ROTATION_CERTIFICATE_TEST)
	./$(TEST_BIN_PERIOD_ROTATION_CERTIFICATE_TEST)
.PHONY: period_rotation_certificate_test

TEST_BIN_CONSTANT_FIRST_LETTER_REFLECTION_TEST := $(BUILDDIR)/constant_first_letter_reflection_test
$(TEST_BIN_CONSTANT_FIRST_LETTER_REFLECTION_TEST): $(TESTDIR)/constant_first_letter_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
constant_first_letter_reflection_test: $(TEST_BIN_CONSTANT_FIRST_LETTER_REFLECTION_TEST)
	./$(TEST_BIN_CONSTANT_FIRST_LETTER_REFLECTION_TEST)
.PHONY: constant_first_letter_reflection_test

TEST_BIN_FIRST_LETTER_ORBIT_FINDING41_TEST := $(BUILDDIR)/first_letter_orbit_finding41_test
$(TEST_BIN_FIRST_LETTER_ORBIT_FINDING41_TEST): $(TESTDIR)/first_letter_orbit_finding41_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
first_letter_orbit_finding41_test: $(TEST_BIN_FIRST_LETTER_ORBIT_FINDING41_TEST)
	./$(TEST_BIN_FIRST_LETTER_ORBIT_FINDING41_TEST)
.PHONY: first_letter_orbit_finding41_test

TEST_BIN_FIRST_LETTER_ORBIT_REFLECTION_TEST := $(BUILDDIR)/first_letter_orbit_reflection_test
$(TEST_BIN_FIRST_LETTER_ORBIT_REFLECTION_TEST): $(TESTDIR)/first_letter_orbit_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
first_letter_orbit_reflection_test: $(TEST_BIN_FIRST_LETTER_ORBIT_REFLECTION_TEST)
	./$(TEST_BIN_FIRST_LETTER_ORBIT_REFLECTION_TEST)
.PHONY: first_letter_orbit_reflection_test

TEST_BIN_PISOT_ROOT_ORDERING_REFLECTION_TEST := $(BUILDDIR)/pisot_root_ordering_reflection_test
$(TEST_BIN_PISOT_ROOT_ORDERING_REFLECTION_TEST): $(TESTDIR)/pisot_root_ordering_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
pisot_root_ordering_reflection_test: $(TEST_BIN_PISOT_ROOT_ORDERING_REFLECTION_TEST)
	./$(TEST_BIN_PISOT_ROOT_ORDERING_REFLECTION_TEST)
.PHONY: pisot_root_ordering_reflection_test

TEST_BIN_STURM_CHAIN_REFLECTION_TEST := $(BUILDDIR)/sturm_chain_reflection_test
$(TEST_BIN_STURM_CHAIN_REFLECTION_TEST): $(TESTDIR)/sturm_chain_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
sturm_chain_reflection_test: $(TEST_BIN_STURM_CHAIN_REFLECTION_TEST)
	./$(TEST_BIN_STURM_CHAIN_REFLECTION_TEST)
.PHONY: sturm_chain_reflection_test

TEST_BIN_LEFTMOST_LOOP_REFLECTION_TEST := $(BUILDDIR)/leftmost_loop_reflection_test
$(TEST_BIN_LEFTMOST_LOOP_REFLECTION_TEST): $(TESTDIR)/leftmost_loop_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
leftmost_loop_reflection_test: $(TEST_BIN_LEFTMOST_LOOP_REFLECTION_TEST)
	./$(TEST_BIN_LEFTMOST_LOOP_REFLECTION_TEST)
.PHONY: leftmost_loop_reflection_test

TEST_BIN_ZERO_RUN_SAME_CHAIN_REFLECTION_TEST := $(BUILDDIR)/zero_run_same_chain_reflection_test
$(TEST_BIN_ZERO_RUN_SAME_CHAIN_REFLECTION_TEST): $(TESTDIR)/zero_run_same_chain_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
zero_run_same_chain_reflection_test: $(TEST_BIN_ZERO_RUN_SAME_CHAIN_REFLECTION_TEST)
	./$(TEST_BIN_ZERO_RUN_SAME_CHAIN_REFLECTION_TEST)
.PHONY: zero_run_same_chain_reflection_test

TEST_BIN_BRANCHING_WALK_COMPOSITION_TEST := $(BUILDDIR)/branching_walk_composition_test
$(TEST_BIN_BRANCHING_WALK_COMPOSITION_TEST): $(TESTDIR)/branching_walk_composition_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
branching_walk_composition_test: $(TEST_BIN_BRANCHING_WALK_COMPOSITION_TEST)
	./$(TEST_BIN_BRANCHING_WALK_COMPOSITION_TEST)
.PHONY: branching_walk_composition_test

TEST_BIN_COINCIDENCE_CLOSURE_PREFIX_TEST := $(BUILDDIR)/coincidence_closure_prefix_test
$(TEST_BIN_COINCIDENCE_CLOSURE_PREFIX_TEST): $(TESTDIR)/coincidence_closure_prefix_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
coincidence_closure_prefix_test: $(TEST_BIN_COINCIDENCE_CLOSURE_PREFIX_TEST)
	./$(TEST_BIN_COINCIDENCE_CLOSURE_PREFIX_TEST)
.PHONY: coincidence_closure_prefix_test

TEST_BIN_SINGLE_JUNCTION_COINCIDENCE_COMPOSITION_TEST := $(BUILDDIR)/single_junction_coincidence_composition_test
$(TEST_BIN_SINGLE_JUNCTION_COINCIDENCE_COMPOSITION_TEST): $(TESTDIR)/single_junction_coincidence_composition_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
single_junction_coincidence_composition_test: $(TEST_BIN_SINGLE_JUNCTION_COINCIDENCE_COMPOSITION_TEST)
	./$(TEST_BIN_SINGLE_JUNCTION_COINCIDENCE_COMPOSITION_TEST)
.PHONY: single_junction_coincidence_composition_test

TEST_BIN_SECOND_GENUINE_FOURTH_GENERATOR_PROPERTY_F_TEST := $(BUILDDIR)/second_genuine_fourth_generator_property_f_test
$(TEST_BIN_SECOND_GENUINE_FOURTH_GENERATOR_PROPERTY_F_TEST): $(TESTDIR)/second_genuine_fourth_generator_property_f_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
second_genuine_fourth_generator_property_f_test: $(TEST_BIN_SECOND_GENUINE_FOURTH_GENERATOR_PROPERTY_F_TEST)
	./$(TEST_BIN_SECOND_GENUINE_FOURTH_GENERATOR_PROPERTY_F_TEST)
.PHONY: second_genuine_fourth_generator_property_f_test

TEST_BIN_PROPERTY_F_FAMILY_AUTOPSY := $(BUILDDIR)/property_f_family_autopsy
$(TEST_BIN_PROPERTY_F_FAMILY_AUTOPSY): $(TESTDIR)/property_f_family_autopsy_test.cpp \
		$(INCDIR)/adelic/property_f_family_autopsy.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
property_f_family_autopsy: $(TEST_BIN_PROPERTY_F_FAMILY_AUTOPSY)
	./$(TEST_BIN_PROPERTY_F_FAMILY_AUTOPSY)

TEST_BIN_PROPERTY_F_TRANSPORT_CERTIFICATE := $(BUILDDIR)/property_f_transport_certificate_test
$(TEST_BIN_PROPERTY_F_TRANSPORT_CERTIFICATE): $(TESTDIR)/property_f_transport_certificate_test.cpp \
		$(INCDIR)/adelic/property_f_transport_certificate.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
property_f_transport_certificate_test: $(TEST_BIN_PROPERTY_F_TRANSPORT_CERTIFICATE)
	./$(TEST_BIN_PROPERTY_F_TRANSPORT_CERTIFICATE)

TEST_BIN_PROPERTY_F_CONTACT_TRANSPORT_BRIDGE := $(BUILDDIR)/property_f_contact_transport_bridge_test
$(TEST_BIN_PROPERTY_F_CONTACT_TRANSPORT_BRIDGE): $(TESTDIR)/property_f_contact_transport_bridge_test.cpp \
		$(INCDIR)/adelic/property_f_contact_transport_bridge.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
property_f_contact_transport_bridge_test: $(TEST_BIN_PROPERTY_F_CONTACT_TRANSPORT_BRIDGE)
	./$(TEST_BIN_PROPERTY_F_CONTACT_TRANSPORT_BRIDGE)

TEST_BIN_CANONICAL_NONUNIT_PROPERTY_F := $(BUILDDIR)/canonical_nonunit_property_f_test
$(TEST_BIN_CANONICAL_NONUNIT_PROPERTY_F): $(TESTDIR)/canonical_nonunit_property_f_test.cpp \
		$(INCDIR)/ravel/proof/canonical_nonunit_property_f.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
canonical_nonunit_property_f_test: $(TEST_BIN_CANONICAL_NONUNIT_PROPERTY_F)
	./$(TEST_BIN_CANONICAL_NONUNIT_PROPERTY_F)

TEST_BIN_NONUNIT_PROPERTY_F_THEOREM := $(BUILDDIR)/nonunit_property_f_theorem_test
$(TEST_BIN_NONUNIT_PROPERTY_F_THEOREM): $(TESTDIR)/nonunit_property_f_theorem_test.cpp \
		$(INCDIR)/ravel/proof/nonunit_property_f_theorem.hpp \
		$(INCDIR)/ravel/proof/monotone_coefficient_cone.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
nonunit_property_f_theorem_test: $(TEST_BIN_NONUNIT_PROPERTY_F_THEOREM)
	./$(TEST_BIN_NONUNIT_PROPERTY_F_THEOREM)

TEST_BIN_GENERALIZED_MULTINACCI_PROPERTY_F_TRANSPORT := $(BUILDDIR)/generalized_multinacci_property_f_transport_test
$(TEST_BIN_GENERALIZED_MULTINACCI_PROPERTY_F_TRANSPORT): $(TESTDIR)/generalized_multinacci_property_f_transport_test.cpp \
		$(INCDIR)/adelic/property_f_transport_certificate.hpp \
		$(INCDIR)/adelic/generalized_multinacci_affine_transport.hpp \
		$(INCDIR)/adelic/generalized_multinacci_block_transport.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
generalized_multinacci_property_f_transport_test: $(TEST_BIN_GENERALIZED_MULTINACCI_PROPERTY_F_TRANSPORT)
	./$(TEST_BIN_GENERALIZED_MULTINACCI_PROPERTY_F_TRANSPORT)

TEST_BIN_ADELIC_COCYCLE_EXTENSION := $(BUILDDIR)/adelic_cocycle_extension_test
$(TEST_BIN_ADELIC_COCYCLE_EXTENSION): $(TESTDIR)/adelic_cocycle_extension_test.cpp \
		$(INCDIR)/adelic/adelic_cocycle_extension.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
adelic_cocycle_extension_test: $(TEST_BIN_ADELIC_COCYCLE_EXTENSION)
	./$(TEST_BIN_ADELIC_COCYCLE_EXTENSION)

TEST_BIN_GENERALIZED_MULTINACCI_PROPERTY_F_TRANSPORT_EXTENDED := $(BUILDDIR)/generalized_multinacci_property_f_transport_extended_test
$(TEST_BIN_GENERALIZED_MULTINACCI_PROPERTY_F_TRANSPORT_EXTENDED): $(TESTDIR)/generalized_multinacci_property_f_transport_extended_test.cpp \
		$(INCDIR)/adelic/property_f_transport_certificate.hpp \
		$(INCDIR)/adelic/generalized_multinacci_affine_transport.hpp \
		$(INCDIR)/adelic/generalized_multinacci_block_transport.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
generalized_multinacci_property_f_transport_extended_test: $(TEST_BIN_GENERALIZED_MULTINACCI_PROPERTY_F_TRANSPORT_EXTENDED)
	./$(TEST_BIN_GENERALIZED_MULTINACCI_PROPERTY_F_TRANSPORT_EXTENDED)

TEST_BIN_GENERALIZED_MULTINACCI_GROWTH_PROFILE := $(BUILDDIR)/generalized_multinacci_growth_profile_test
$(TEST_BIN_GENERALIZED_MULTINACCI_GROWTH_PROFILE): $(TESTDIR)/generalized_multinacci_growth_profile_test.cpp \
		$(INCDIR)/adelic/generalized_multinacci_growth_profile.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
generalized_multinacci_growth_profile_test: $(TEST_BIN_GENERALIZED_MULTINACCI_GROWTH_PROFILE)
	./$(TEST_BIN_GENERALIZED_MULTINACCI_GROWTH_PROFILE)

TEST_BIN_GENERALIZED_MULTINACCI_BLOCK_AFFINE := $(BUILDDIR)/generalized_multinacci_block_affine_test
$(TEST_BIN_GENERALIZED_MULTINACCI_BLOCK_AFFINE): $(TESTDIR)/generalized_multinacci_block_affine_test.cpp \
		$(INCDIR)/adelic/generalized_multinacci_block_affine.hpp \
		$(INCDIR)/adelic/generalized_multinacci_block_transport.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
generalized_multinacci_block_affine_test: $(TEST_BIN_GENERALIZED_MULTINACCI_BLOCK_AFFINE)
	./$(TEST_BIN_GENERALIZED_MULTINACCI_BLOCK_AFFINE)

TEST_BIN_GENERALIZED_MULTINACCI_STRUCTURAL_SWEEP := $(BUILDDIR)/generalized_multinacci_structural_sweep_test
$(TEST_BIN_GENERALIZED_MULTINACCI_STRUCTURAL_SWEEP): $(TESTDIR)/generalized_multinacci_structural_sweep_test.cpp \
		$(INCDIR)/adelic/generalized_multinacci_block_transport.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
generalized_multinacci_structural_sweep_test: $(TEST_BIN_GENERALIZED_MULTINACCI_STRUCTURAL_SWEEP)
	./$(TEST_BIN_GENERALIZED_MULTINACCI_STRUCTURAL_SWEEP)

TEST_BIN_GENERALIZED_MULTINACCI_CROSS_DIMENSION := $(BUILDDIR)/generalized_multinacci_cross_dimension_test
$(TEST_BIN_GENERALIZED_MULTINACCI_CROSS_DIMENSION): $(TESTDIR)/generalized_multinacci_cross_dimension_test.cpp \
		$(INCDIR)/adelic/generalized_multinacci_cross_dimension.hpp \
		$(INCDIR)/adelic/generalized_multinacci_block_transport.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
generalized_multinacci_cross_dimension_test: $(TEST_BIN_GENERALIZED_MULTINACCI_CROSS_DIMENSION)
	./$(TEST_BIN_GENERALIZED_MULTINACCI_CROSS_DIMENSION)

TEST_BIN_GENERALIZED_MULTINACCI_CROSS_DIMENSION_PROPERTY_F := $(BUILDDIR)/generalized_multinacci_cross_dimension_property_f_test
$(TEST_BIN_GENERALIZED_MULTINACCI_CROSS_DIMENSION_PROPERTY_F): $(TESTDIR)/generalized_multinacci_cross_dimension_property_f_test.cpp \
		$(INCDIR)/adelic/generalized_multinacci_cross_dimension.hpp \
		$(INCDIR)/adelic/property_f_transport_certificate.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
generalized_multinacci_cross_dimension_property_f_test: $(TEST_BIN_GENERALIZED_MULTINACCI_CROSS_DIMENSION_PROPERTY_F)
	./$(TEST_BIN_GENERALIZED_MULTINACCI_CROSS_DIMENSION_PROPERTY_F)

TEST_BIN_GENERALIZED_MULTINACCI_FIBER_INTERACTION := $(BUILDDIR)/generalized_multinacci_fiber_interaction_test
$(TEST_BIN_GENERALIZED_MULTINACCI_FIBER_INTERACTION): $(TESTDIR)/generalized_multinacci_fiber_interaction_test.cpp \
		$(INCDIR)/adelic/generalized_multinacci_fiber_interaction.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
generalized_multinacci_fiber_interaction_test: $(TEST_BIN_GENERALIZED_MULTINACCI_FIBER_INTERACTION)
	./$(TEST_BIN_GENERALIZED_MULTINACCI_FIBER_INTERACTION)

PROPERTY_F_FAMILY_ARTIFACT_BIN := $(BUILDDIR)/property_f_family_artifact
property_f_family_artifact: $(PROPERTY_F_FAMILY_ARTIFACT_BIN)
	./$(PROPERTY_F_FAMILY_ARTIFACT_BIN) $(or $(OUTPUT),out/property_f_family.tsv) $(or $(NODE_BUDGET),100000) $(or $(COINCIDENCE_BUDGET),1000000)
$(PROPERTY_F_FAMILY_ARTIFACT_BIN): $(APPDIR)/property_f_family_artifact.cpp \
		$(INCDIR)/adelic/property_f_family_autopsy.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

TEST_BIN_CLASS_II_TERMINAL_SEXTET_REFLECTION_TEST := $(BUILDDIR)/class_ii_terminal_sextet_reflection_test
$(TEST_BIN_CLASS_II_TERMINAL_SEXTET_REFLECTION_TEST): $(TESTDIR)/class_ii_terminal_sextet_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
class_ii_terminal_sextet_reflection_test: $(TEST_BIN_CLASS_II_TERMINAL_SEXTET_REFLECTION_TEST)
	./$(TEST_BIN_CLASS_II_TERMINAL_SEXTET_REFLECTION_TEST)
.PHONY: class_ii_terminal_sextet_reflection_test

TEST_BIN_ADJACENT_SWAP_COUNT_REFLECTION_TEST := $(BUILDDIR)/adjacent_swap_count_reflection_test
$(TEST_BIN_ADJACENT_SWAP_COUNT_REFLECTION_TEST): $(TESTDIR)/adjacent_swap_count_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
adjacent_swap_count_reflection_test: $(TEST_BIN_ADJACENT_SWAP_COUNT_REFLECTION_TEST)
	./$(TEST_BIN_ADJACENT_SWAP_COUNT_REFLECTION_TEST)
.PHONY: adjacent_swap_count_reflection_test

TEST_BIN_FEEDER_CYCLE_CHARPOLY_REFLECTION_TEST := $(BUILDDIR)/feeder_cycle_charpoly_reflection_test
$(TEST_BIN_FEEDER_CYCLE_CHARPOLY_REFLECTION_TEST): $(TESTDIR)/feeder_cycle_charpoly_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
feeder_cycle_charpoly_reflection_test: $(TEST_BIN_FEEDER_CYCLE_CHARPOLY_REFLECTION_TEST)
	./$(TEST_BIN_FEEDER_CYCLE_CHARPOLY_REFLECTION_TEST)
.PHONY: feeder_cycle_charpoly_reflection_test

TEST_BIN_REGULAR_SHELL_CHARPOLY_REFLECTION_TEST := $(BUILDDIR)/regular_shell_charpoly_reflection_test
$(TEST_BIN_REGULAR_SHELL_CHARPOLY_REFLECTION_TEST): $(TESTDIR)/regular_shell_charpoly_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
regular_shell_charpoly_reflection_test: $(TEST_BIN_REGULAR_SHELL_CHARPOLY_REFLECTION_TEST)
	./$(TEST_BIN_REGULAR_SHELL_CHARPOLY_REFLECTION_TEST)
.PHONY: regular_shell_charpoly_reflection_test

TEST_BIN_STRICT_SHELL_PUMP_REFLECTION_TEST := $(BUILDDIR)/strict_shell_pump_reflection_test
$(TEST_BIN_STRICT_SHELL_PUMP_REFLECTION_TEST): $(TESTDIR)/strict_shell_pump_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
strict_shell_pump_reflection_test: $(TEST_BIN_STRICT_SHELL_PUMP_REFLECTION_TEST)
	./$(TEST_BIN_STRICT_SHELL_PUMP_REFLECTION_TEST)
.PHONY: strict_shell_pump_reflection_test

TEST_BIN_PREDICTED_CORE_SCC_EXHAUSTION_REFLECTION_TEST := $(BUILDDIR)/predicted_core_scc_exhaustion_reflection_test
$(TEST_BIN_PREDICTED_CORE_SCC_EXHAUSTION_REFLECTION_TEST): $(TESTDIR)/predicted_core_scc_exhaustion_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
predicted_core_scc_exhaustion_reflection_test: $(TEST_BIN_PREDICTED_CORE_SCC_EXHAUSTION_REFLECTION_TEST)
	./$(TEST_BIN_PREDICTED_CORE_SCC_EXHAUSTION_REFLECTION_TEST)
.PHONY: predicted_core_scc_exhaustion_reflection_test

TEST_BIN_COUPLED_WINNING_PREDICATE_REFLECTION_TEST := $(BUILDDIR)/coupled_winning_predicate_reflection_test
$(TEST_BIN_COUPLED_WINNING_PREDICATE_REFLECTION_TEST): $(TESTDIR)/coupled_winning_predicate_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
coupled_winning_predicate_reflection_test: $(TEST_BIN_COUPLED_WINNING_PREDICATE_REFLECTION_TEST)
	./$(TEST_BIN_COUPLED_WINNING_PREDICATE_REFLECTION_TEST)
.PHONY: coupled_winning_predicate_reflection_test

TEST_BIN_RADIAL_TRANSLATION_DEFECT_REFLECTION_TEST := $(BUILDDIR)/radial_translation_defect_reflection_test
$(TEST_BIN_RADIAL_TRANSLATION_DEFECT_REFLECTION_TEST): $(TESTDIR)/radial_translation_defect_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
radial_translation_defect_reflection_test: $(TEST_BIN_RADIAL_TRANSLATION_DEFECT_REFLECTION_TEST)
	./$(TEST_BIN_RADIAL_TRANSLATION_DEFECT_REFLECTION_TEST)
.PHONY: radial_translation_defect_reflection_test

TEST_BIN_DEFECT_SPLICE_STEP_REFLECTION_TEST := $(BUILDDIR)/defect_splice_step_reflection_test
$(TEST_BIN_DEFECT_SPLICE_STEP_REFLECTION_TEST): $(TESTDIR)/defect_splice_step_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
defect_splice_step_reflection_test: $(TEST_BIN_DEFECT_SPLICE_STEP_REFLECTION_TEST)
	./$(TEST_BIN_DEFECT_SPLICE_STEP_REFLECTION_TEST)
.PHONY: defect_splice_step_reflection_test

# universal_dominance_closed_relation_reflection_test retired 2026-08-08:
# it existed solely to emit lean/generated/universal_dominance_closed_relation_batch.lean
# for the shell-rank/carry-bound route, superseded by the canonical Q/R
# closure (docs/NBONACCI_UNIVERSAL_DOMINANCE_CLOSED_2026-08-08.md). Test
# moved to archive/2026-08-08_superseded_shell_rank_route/, Lean output to
# lean/archive/universal_dominance_closed_relation_batch.lean.

TEST_BIN_CONDITION_F_JOINT_DOMINANCE_REFLECTION_TEST := $(BUILDDIR)/condition_f_joint_dominance_reflection_test
$(TEST_BIN_CONDITION_F_JOINT_DOMINANCE_REFLECTION_TEST): $(TESTDIR)/condition_f_joint_dominance_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
condition_f_joint_dominance_reflection_test: $(TEST_BIN_CONDITION_F_JOINT_DOMINANCE_REFLECTION_TEST)
	./$(TEST_BIN_CONDITION_F_JOINT_DOMINANCE_REFLECTION_TEST)
.PHONY: condition_f_joint_dominance_reflection_test

TEST_BIN_FINITE_POSITIVE_GRAMMAR_MAJORANT_REFLECTION_TEST := $(BUILDDIR)/finite_positive_grammar_majorant_reflection_test
$(TEST_BIN_FINITE_POSITIVE_GRAMMAR_MAJORANT_REFLECTION_TEST): $(TESTDIR)/finite_positive_grammar_majorant_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
finite_positive_grammar_majorant_reflection_test: $(TEST_BIN_FINITE_POSITIVE_GRAMMAR_MAJORANT_REFLECTION_TEST)
	./$(TEST_BIN_FINITE_POSITIVE_GRAMMAR_MAJORANT_REFLECTION_TEST)
.PHONY: finite_positive_grammar_majorant_reflection_test

TEST_BIN_THIRD_SMALLEST_PISOT_PARRY_FACTORIZATION_REFLECTION_TEST := $(BUILDDIR)/third_smallest_pisot_parry_factorization_reflection_test
$(TEST_BIN_THIRD_SMALLEST_PISOT_PARRY_FACTORIZATION_REFLECTION_TEST): $(TESTDIR)/third_smallest_pisot_parry_factorization_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
third_smallest_pisot_parry_factorization_reflection_test: $(TEST_BIN_THIRD_SMALLEST_PISOT_PARRY_FACTORIZATION_REFLECTION_TEST)
	./$(TEST_BIN_THIRD_SMALLEST_PISOT_PARRY_FACTORIZATION_REFLECTION_TEST)
.PHONY: third_smallest_pisot_parry_factorization_reflection_test



GENERALIZED_CAMPAIGN_TEST_BIN := $(BUILDDIR)/generalized_campaign_test
$(GENERALIZED_CAMPAIGN_TEST_BIN): $(TESTDIR)/generalized_campaign_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
generalized_campaign_test: $(GENERALIZED_CAMPAIGN_TEST_BIN)
	./$(GENERALIZED_CAMPAIGN_TEST_BIN)
.PHONY: generalized_campaign_test


LEGACY_CAMPAIGN_BRIDGE_TEST_BIN := $(BUILDDIR)/legacy_campaign_bridge_test
$(LEGACY_CAMPAIGN_BRIDGE_TEST_BIN): $(TESTDIR)/legacy_campaign_bridge_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
legacy_campaign_bridge_test: $(LEGACY_CAMPAIGN_BRIDGE_TEST_BIN)
	./$(LEGACY_CAMPAIGN_BRIDGE_TEST_BIN)
.PHONY: legacy_campaign_bridge_test


MULTI_COMPARTMENT_CLOSURE_TEST_BIN := $(BUILDDIR)/multi_compartment_closure_test
$(MULTI_COMPARTMENT_CLOSURE_TEST_BIN): $(TESTDIR)/multi_compartment_closure_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
multi_compartment_closure_test: $(MULTI_COMPARTMENT_CLOSURE_TEST_BIN)
	./$(MULTI_COMPARTMENT_CLOSURE_TEST_BIN)
.PHONY: multi_compartment_closure_test

QMATRIX_TYPED_IR_TEST_BIN := $(BUILDDIR)/qmatrix_typed_ir_test
$(QMATRIX_TYPED_IR_TEST_BIN): $(TESTDIR)/qmatrix_typed_ir_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
qmatrix_typed_ir_test: $(QMATRIX_TYPED_IR_TEST_BIN)
	./$(QMATRIX_TYPED_IR_TEST_BIN)
.PHONY: qmatrix_typed_ir_test

SHARED_POLYNOMIAL_CLOSURE_TEST_BIN := $(BUILDDIR)/shared_polynomial_closure_test
$(SHARED_POLYNOMIAL_CLOSURE_TEST_BIN): $(TESTDIR)/shared_polynomial_closure_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
shared_polynomial_closure_test: $(SHARED_POLYNOMIAL_CLOSURE_TEST_BIN)
	./$(SHARED_POLYNOMIAL_CLOSURE_TEST_BIN)
.PHONY: shared_polynomial_closure_test

.PHONY: theorem_capability_index theorem_capability_machine_test ravel_truth_machine

theorem_capability_index:
	./tools/build_theorem_capability_index.py --root . --out config/theorem_capabilities.tsv

theorem_capability_machine_test: theorem_capability_index
	mkdir -p out
	$(CXX) $(CXXFLAGS) $(INCLUDES) tests/theorem_capability_machine_test.cpp -o out/theorem_capability_machine_test
	./out/theorem_capability_machine_test

ravel_truth_machine: theorem_capability_index
	mkdir -p out
	$(CXX) $(CXXFLAGS) $(INCLUDES) app/ravel_truth_machine.cpp -o out/ravel_truth_machine

.PHONY: typed_theorem_application_test ravel_truth_apply

typed_theorem_application_test: | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) tests/typed_theorem_application_test.cpp -o out/typed_theorem_application_test
	./out/typed_theorem_application_test

ravel_truth_apply: typed_theorem_application_test | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) app/ravel_truth_apply.cpp -o out/ravel_truth_apply
	./out/ravel_truth_apply --output=out/truth_machine_fast_power_reduction.lean

GRAPH_STRUCTURAL_CAMPAIGN_TEST_BIN := $(BUILDDIR)/graph_structural_campaign_test
$(GRAPH_STRUCTURAL_CAMPAIGN_TEST_BIN): $(TESTDIR)/graph_structural_campaign_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

graph_structural_campaign_test: $(GRAPH_STRUCTURAL_CAMPAIGN_TEST_BIN)
	./$(GRAPH_STRUCTURAL_CAMPAIGN_TEST_BIN)

.PHONY: graph_structural_campaign_test

GRAPH_CERTIFICATE_CONVERSION_TEST_BIN := $(BUILDDIR)/graph_certificate_conversion_test
$(GRAPH_CERTIFICATE_CONVERSION_TEST_BIN): $(TESTDIR)/graph_certificate_conversion_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

graph_certificate_conversion_test: $(GRAPH_CERTIFICATE_CONVERSION_TEST_BIN)
	./$(GRAPH_CERTIFICATE_CONVERSION_TEST_BIN)

.PHONY: graph_certificate_conversion_test

cycle_charpoly_campaign_test: out/cycle_charpoly_campaign_test
	./out/cycle_charpoly_campaign_test > out/cycle_charpoly_campaign_validation.lean

out/cycle_charpoly_campaign_test: tests/cycle_charpoly_campaign_test.cpp include/ravel/proof/cycle_charpoly_campaign.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Iinclude tests/cycle_charpoly_campaign_test.cpp -o $@


out/universal_dominance_campaign_test: tests/universal_dominance_campaign_test.cpp include/ravel/proof/universal_dominance_campaign.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude tests/universal_dominance_campaign_test.cpp -o $@

universal_dominance_campaign_test: out/universal_dominance_campaign_test
	./out/universal_dominance_campaign_test

out/universal_dominance_campaign: app/universal_dominance_campaign.cpp include/ravel/proof/universal_dominance_campaign.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude app/universal_dominance_campaign.cpp -o $@

universal_dominance_campaign: out/universal_dominance_campaign
	./out/universal_dominance_campaign


out/phase_rank_transport_test: tests/phase_rank_transport_test.cpp include/ravel/proof/phase_rank_transport.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

phase_rank_transport_test: out/phase_rank_transport_test
	./out/phase_rank_transport_test

out/radial_translation_defect_test: tests/radial_translation_defect_test.cpp include/ravel/proof/radial_translation_defect.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

radial_translation_defect_test: out/radial_translation_defect_test
	./out/radial_translation_defect_test


out/covering_translation_tube_test: tests/covering_translation_tube_test.cpp include/ravel/proof/covering_translation_tube.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

covering_translation_tube_test: out/covering_translation_tube_test
	./out/covering_translation_tube_test

out/defect_spliced_tube_test: tests/defect_spliced_tube_test.cpp include/ravel/proof/covering_translation_tube.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

defect_spliced_tube_test: out/defect_spliced_tube_test
	./out/defect_spliced_tube_test

out/uniform_radius_one_synthesis_test: tests/uniform_radius_one_synthesis_test.cpp include/ravel/proof/uniform_radius_one_synthesis.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

uniform_radius_one_synthesis_test: out/uniform_radius_one_synthesis_test
	./out/uniform_radius_one_synthesis_test


out/symbolic_radius_one_controller_test: tests/symbolic_radius_one_controller_test.cpp include/ravel/proof/symbolic_radius_one_controller.hpp include/ravel/proof/uniform_radius_one_synthesis.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

symbolic_radius_one_controller_test: out/symbolic_radius_one_controller_test
	./out/symbolic_radius_one_controller_test


out/coupled_winning_predicate_test: tests/coupled_winning_predicate_test.cpp include/ravel/proof/coupled_winning_predicate.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

coupled_winning_predicate_test: out/coupled_winning_predicate_test
	./out/coupled_winning_predicate_test


out/continuation_controller_family_test: tests/continuation_controller_family_test.cpp include/ravel/proof/continuation_controller_family.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

continuation_controller_family_test: out/continuation_controller_family_test
	./out/continuation_controller_family_test


out/universal_shell_pumping_test: tests/universal_shell_pumping_test.cpp include/ravel/proof/universal_shell_pumping.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

universal_shell_pumping_test: out/universal_shell_pumping_test
	./out/universal_shell_pumping_test


out/cyclic_continuation_controller_test: tests/cyclic_continuation_controller_test.cpp include/ravel/proof/cyclic_continuation_controller.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

cyclic_continuation_controller_test: out/cyclic_continuation_controller_test
	./out/cyclic_continuation_controller_test


out/strict_shell_pump_test: tests/strict_shell_pump_test.cpp include/ravel/proof/strict_shell_pump.hpp include/ravel/proof/cyclic_continuation_controller.hpp include/ravel/proof/covering_translation_tube.hpp include/ravel/proof/symbolic_radius_one_controller.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp $< -o $@

strict_shell_pump_test: out/strict_shell_pump_test
	./out/strict_shell_pump_test

out/predicted_core_scc_identification_test: tests/predicted_core_scc_identification_test.cpp include/ravel/proof/predicted_core_scc_identification.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp $< math/out/libmath.a -o $@

predicted_core_scc_identification_test: out/predicted_core_scc_identification_test
	./out/predicted_core_scc_identification_test

nbonacci_predicted_core_exhaustion_test: nbonacci_arithmetic_hull
	./out/nbonacci_arithmetic_hull --bound=1 --dump-core-exhaustion 3 4 5 6 7

strict_shell_pump_existence_test:
	$(CXX) $(CXXFLAGS) -Iinclude tests/strict_shell_pump_existence_test.cpp -o /tmp/strict_shell_pump_existence_test
	/tmp/strict_shell_pump_existence_test

out/first_return_completeness_prover: app/first_return_completeness_prover.cpp include/ravel/proof/symbolic_residual_induction.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude app/first_return_completeness_prover.cpp -o $@

first_return_completeness_prover: out/first_return_completeness_prover
	./out/first_return_completeness_prover 3 lean/generated/first_return_residual_induction.lean

out/symbolic_residual_induction_test: tests/symbolic_residual_induction_test.cpp include/ravel/proof/symbolic_residual_induction.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude tests/symbolic_residual_induction_test.cpp -o $@

symbolic_residual_induction_test: out/symbolic_residual_induction_test
	./out/symbolic_residual_induction_test

out/ravel-prove: app/ravel_prove.cpp include/ravel/proof/first_return_completeness_automation.hpp include/ravel/proof/symbolic_residual_induction.hpp include/ravel/proof/first_return_proof_strategist.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude app/ravel_prove.cpp -o $@

ravel-prove: out/ravel-prove
	./out/ravel-prove first-return-completeness 3 4 out/first_return_completeness.checkpoint

out/first_return_completeness_automation_test: tests/first_return_completeness_automation_test.cpp include/ravel/proof/first_return_completeness_automation.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude tests/first_return_completeness_automation_test.cpp -o $@

first_return_completeness_automation_test: out/first_return_completeness_automation_test
	./out/first_return_completeness_automation_test

out/first_return_joint_product_test: tests/first_return_joint_product_test.cpp include/ravel/proof/first_return_joint_product.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

first_return_joint_product_test: out/first_return_joint_product_test
	./out/first_return_joint_product_test

out/first_return_joint_role_quotient_test: tests/first_return_joint_role_quotient_test.cpp include/ravel/proof/first_return_joint_role_quotient.hpp include/ravel/proof/first_return_joint_product.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

first_return_joint_role_quotient_test: out/first_return_joint_role_quotient_test
	./out/first_return_joint_role_quotient_test

out/parametric_predicate_updates_test: tests/parametric_predicate_updates_test.cpp include/ravel/proof/parametric_predicate_updates.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude tests/parametric_predicate_updates_test.cpp -o $@

parametric_predicate_updates_test: out/parametric_predicate_updates_test
	./out/parametric_predicate_updates_test

out/residual_signature_transfer_test: tests/residual_signature_transfer_test.cpp include/ravel/proof/residual_signature_transfer.hpp include/ravel/proof/parametric_predicate_updates.hpp include/ravel/proof/first_return_joint_product.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude tests/residual_signature_transfer_test.cpp -o $@

residual_signature_transfer_test: out/residual_signature_transfer_test
	./out/residual_signature_transfer_test

out/symbolic_residual_formula_test: tests/symbolic_residual_formula_test.cpp include/ravel/proof/symbolic_residual_formula.hpp include/ravel/proof/first_return_joint_product.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude tests/symbolic_residual_formula_test.cpp -o $@

symbolic_residual_formula_test: out/symbolic_residual_formula_test
	./out/symbolic_residual_formula_test

out/realized_first_return_completeness_test: tests/realized_first_return_completeness_test.cpp include/ravel/proof/realized_first_return_completeness.hpp
	@mkdir -p out lean/generated
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@

realized_first_return_completeness_test: out/realized_first_return_completeness_test
	./out/realized_first_return_completeness_test

out/first_return_obligation_discharge_test: tests/first_return_obligation_discharge_test.cpp include/ravel/proof/first_return_obligation_discharge.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude tests/first_return_obligation_discharge_test.cpp -o $@

.PHONY: first_return_obligation_discharge_test
first_return_obligation_discharge_test: out/first_return_obligation_discharge_test
	./out/first_return_obligation_discharge_test

out/cyclic_splice_completion_test: tests/cyclic_splice_completion_test.cpp include/ravel/proof/cyclic_splice_completion.hpp math/out/libmath.a
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp $< math/out/libmath.a -o $@

.PHONY: cyclic_splice_completion_test
cyclic_splice_completion_test: out/cyclic_splice_completion_test
	./out/cyclic_splice_completion_test

cyclic_splice_compactness_test: math/out/libmath.a
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/cyclic_splice_compactness_test.cpp math/out/libmath.a -o out/cyclic_splice_compactness_test
	./out/cyclic_splice_compactness_test

periodic_word_controller_cycle_test:
	$(CXX) $(CXXFLAGS) -Iinclude tests/periodic_word_controller_cycle_test.cpp -o out/periodic_word_controller_cycle_test
	./out/periodic_word_controller_cycle_test

.PHONY: defect_corrected_radial_transport_test
defect_corrected_radial_transport_test:
	$(CXX) $(CXXFLAGS) -Iinclude tests/defect_corrected_radial_transport_test.cpp -o out/defect_corrected_radial_transport_test
	./out/defect_corrected_radial_transport_test

out/defect_corrected_shell_rank_test: tests/defect_corrected_shell_rank_test.cpp include/ravel/proof/defect_corrected_shell_rank.hpp include/ravel/proof/defect_corrected_radial_transport.hpp include/ravel/proof/phase_rank_transport.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Iinclude $< -o $@

defect_corrected_shell_rank_test: out/defect_corrected_shell_rank_test
	./out/defect_corrected_shell_rank_test

out/block_height_shell_rank_test: tests/block_height_shell_rank_test.cpp include/ravel/proof/block_height_shell_rank.hpp include/ravel/proof/defect_corrected_shell_rank.hpp include/ravel/proof/phase_rank_transport.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Iinclude $< -o $@

.PHONY: block_height_shell_rank_test
block_height_shell_rank_test: out/block_height_shell_rank_test
	./out/block_height_shell_rank_test

out/block_height_chamber_rank_test: tests/block_height_chamber_rank_test.cpp include/ravel/proof/block_height_shell_rank.hpp include/ravel/proof/generated_n4_block_height_chamber_rank.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Iinclude $< -o $@

.PHONY: block_height_chamber_rank_test
block_height_chamber_rank_test: out/block_height_chamber_rank_test
	./out/block_height_chamber_rank_test

out/sign_symmetric_chamber_rank_test: tests/sign_symmetric_chamber_rank_test.cpp include/ravel/proof/sign_symmetric_chamber_rank.hpp include/ravel/proof/generated_n4_block_height_chamber_rank.hpp include/ravel/proof/block_height_shell_rank.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Iinclude $< -o $@

.PHONY: sign_symmetric_chamber_rank_test
sign_symmetric_chamber_rank_test: out/sign_symmetric_chamber_rank_test
	./out/sign_symmetric_chamber_rank_test

out/sign_flux_chamber_fit: app/sign_flux_chamber_fit.cpp include/ravel/proof/block_height_shell_rank.hpp include/ravel/proof/sign_symmetric_chamber_rank.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Iinclude $< -o $@

sign_flux_chamber_fit: out/sign_flux_chamber_fit
	./out/sign_flux_chamber_fit

out/sign_flux_chamber_fit_triplet: app/sign_flux_chamber_fit_triplet.cpp include/ravel/proof/block_height_shell_rank.hpp include/ravel/proof/sign_symmetric_chamber_rank.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Iinclude $< -o $@

sign_flux_chamber_fit_triplet: out/sign_flux_chamber_fit_triplet
	./out/sign_flux_chamber_fit_triplet

out/boundary_flux_sparse_fit: app/boundary_flux_sparse_fit.cpp include/ravel/proof/block_height_shell_rank.hpp include/ravel/proof/sign_symmetric_chamber_rank.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

boundary_flux_sparse_fit: out/boundary_flux_sparse_fit
	./out/boundary_flux_sparse_fit

out/lazy_sparse_boundary_rank: app/lazy_sparse_boundary_rank.cpp include/ravel/proof/block_height_shell_rank.hpp include/ravel/proof/sign_symmetric_chamber_rank.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Iinclude $< -o $@

lazy_sparse_boundary_rank: out/lazy_sparse_boundary_rank
	./out/lazy_sparse_boundary_rank

out/reflective_boundary_grammar_test: tests/reflective_boundary_grammar_test.cpp include/ravel/proof/reflective_boundary_grammar.hpp include/ravel/proof/ordered_boundary_queue.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $(CPPFLAGS) -o $@ $<

reflective_boundary_grammar_test: out/reflective_boundary_grammar_test
	./out/reflective_boundary_grammar_test

out/derive_reflective_boundary_grammar: app/derive_reflective_boundary_grammar.cpp include/ravel/proof/reflective_boundary_grammar.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude $(CPPFLAGS) -o $@ $<

derive_reflective_boundary_grammar: out/derive_reflective_boundary_grammar
	./out/derive_reflective_boundary_grammar 20 lean/generated/reflective_boundary_grammar.lean

GRADED_CORE_DESCENT_TEST_BIN := $(BUILDDIR)/graded_core_descent_test
$(GRADED_CORE_DESCENT_TEST_BIN): $(TESTDIR)/graded_core_descent_test.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

graded_core_descent_test: $(GRADED_CORE_DESCENT_TEST_BIN)
	./$(GRADED_CORE_DESCENT_TEST_BIN)

out/reflective_collatz_weight_test: tests/reflective_collatz_weight_test.cpp math/out/libmath.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp $< math/out/libmath.a -o $@

reflective_collatz_weight_test: out/reflective_collatz_weight_test
	./out/reflective_collatz_weight_test

out/reflective_weight_grammar_test: tests/reflective_weight_grammar_test.cpp math/out/libmath.a
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include $< math/out/libmath.a $(LDFLAGS) $(LDLIBS) -o $@

reflective_weight_grammar_test: out/reflective_weight_grammar_test
	./out/reflective_weight_grammar_test

REFLECTIVE_WEIGHT_FAMILY_TEST_BIN := $(BUILDDIR)/reflective_weight_family_test
reflective_weight_family_test: $(REFLECTIVE_WEIGHT_FAMILY_TEST_BIN)
	./$(REFLECTIVE_WEIGHT_FAMILY_TEST_BIN)
$(REFLECTIVE_WEIGHT_FAMILY_TEST_BIN): \
		$(TESTDIR)/reflective_weight_family_test.cpp \
		$(INCDIR)/ravel/proof/reflective_weight_family.hpp \
		$(INCDIR)/ravel/proof/reflective_collatz_weight.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

PAIRED_MATRIX_DOMINANCE_TEST_BIN := $(BUILDDIR)/paired_matrix_dominance_test
paired_matrix_dominance_test: $(PAIRED_MATRIX_DOMINANCE_TEST_BIN)
	./$(PAIRED_MATRIX_DOMINANCE_TEST_BIN)

$(PAIRED_MATRIX_DOMINANCE_TEST_BIN): $(TESTDIR)/paired_matrix_dominance_test.cpp $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(LIBS) -o $@

PATH_COUNT_CONE_TEST_BIN := $(BUILDDIR)/path_count_cone_test
path_count_cone_test: $(PATH_COUNT_CONE_TEST_BIN)
	$(PATH_COUNT_CONE_TEST_BIN)
$(PATH_COUNT_CONE_TEST_BIN): $(TESTDIR)/path_count_cone_test.cpp $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

GRADED_TRANSFER_TEST_BIN := $(BUILDDIR)/graded_transfer_test
graded_transfer_test: $(GRADED_TRANSFER_TEST_BIN)
	@$(GRADED_TRANSFER_TEST_BIN)

$(GRADED_TRANSFER_TEST_BIN): $(TESTDIR)/graded_transfer_test.cpp $(MATH_LIB)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) $(LDLIBS) -o $@

PHASE_HANDOFF_RENEWAL_TEST_BIN := $(BUILDDIR)/phase_handoff_renewal_test
phase_handoff_renewal_test: $(PHASE_HANDOFF_RENEWAL_TEST_BIN)
	$(PHASE_HANDOFF_RENEWAL_TEST_BIN)

$(PHASE_HANDOFF_RENEWAL_TEST_BIN): $(TESTDIR)/phase_handoff_renewal_test.cpp $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(LIBS) -o $@

FACE_RELATIVE_SIGN_GRAMMAR_TEST_BIN := $(BUILDDIR)/face_relative_sign_grammar_test
face_relative_sign_grammar_test: $(FACE_RELATIVE_SIGN_GRAMMAR_TEST_BIN)
	@$(FACE_RELATIVE_SIGN_GRAMMAR_TEST_BIN)

$(FACE_RELATIVE_SIGN_GRAMMAR_TEST_BIN): $(TESTDIR)/face_relative_sign_grammar_test.cpp $(MATH_LIB)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

SIGNED_INTERVAL_BELLMAN_TEST_BIN := $(BUILDDIR)/signed_interval_bellman_test
signed_interval_bellman_test: $(SIGNED_INTERVAL_BELLMAN_TEST_BIN)
$(SIGNED_INTERVAL_BELLMAN_TEST_BIN): $(TESTDIR)/signed_interval_bellman_test.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(MATHDIR)/include -I$(MATHDIR)/include/mini-gmp $< $(MATHDIR)/src/mini-gmp.c $(MATHDIR)/src/mini-mpq.c -o $@

TWISTED_DIMENSION_EXTENSION_TEST_BIN := $(BUILDDIR)/twisted_dimension_extension_test
twisted_dimension_extension_test: $(TWISTED_DIMENSION_EXTENSION_TEST_BIN)
	$(TWISTED_DIMENSION_EXTENSION_TEST_BIN)
$(TWISTED_DIMENSION_EXTENSION_TEST_BIN): $(TESTDIR)/twisted_dimension_extension_test.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

DERIVE_TWISTED_DIMENSION_EXTENSION_BIN := $(BUILDDIR)/derive_twisted_dimension_extension
derive_twisted_dimension_extension: $(DERIVE_TWISTED_DIMENSION_EXTENSION_BIN)
	$(DERIVE_TWISTED_DIMENSION_EXTENSION_BIN) 10 lean/generated/twisted_dimension_extension.lean
$(DERIVE_TWISTED_DIMENSION_EXTENSION_BIN): app/derive_twisted_dimension_extension.cpp
	@mkdir -p $(BUILDDIR) lean/generated
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

TWISTED_BELLMAN_TRANSPORT_TEST_BIN := $(BUILDDIR)/twisted_bellman_transport_test
twisted_bellman_transport_test: $(TWISTED_BELLMAN_TRANSPORT_TEST_BIN)
	$(TWISTED_BELLMAN_TRANSPORT_TEST_BIN)
$(TWISTED_BELLMAN_TRANSPORT_TEST_BIN): $(TESTDIR)/twisted_bellman_transport_test.cpp $(MATH_LIB)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

DERIVE_TWISTED_BELLMAN_TRANSPORT_BIN := $(BUILDDIR)/derive_twisted_bellman_transport
derive_twisted_bellman_transport: $(DERIVE_TWISTED_BELLMAN_TRANSPORT_BIN)
	$(DERIVE_TWISTED_BELLMAN_TRANSPORT_BIN) lean/generated/twisted_bellman_transport.lean
$(DERIVE_TWISTED_BELLMAN_TRANSPORT_BIN): $(APPDIR)/derive_twisted_bellman_transport.cpp $(MATH_LIB)
	@mkdir -p $(BUILDDIR) lean/generated
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

ADJACENT_COMPETITOR_TRANSPORT_BIN := $(BUILDDIR)/adjacent_competitor_transport
adjacent_competitor_transport: $(ADJACENT_COMPETITOR_TRANSPORT_BIN)

$(ADJACENT_COMPETITOR_TRANSPORT_BIN): $(APPDIR)/adjacent_competitor_transport.cpp $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

CORONA_PROJECTION_TEST_BIN := $(BUILDDIR)/corona_projection_test
.PHONY: corona_projection_test
corona_projection_test: $(CORONA_PROJECTION_TEST_BIN)
	$(CORONA_PROJECTION_TEST_BIN)

$(CORONA_PROJECTION_TEST_BIN): $(TESTDIR)/corona_projection_test.cpp $(MATH_LIB)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

ADJACENT_TWISTED_RENEWAL_TEST_BIN := $(BUILDDIR)/adjacent_twisted_renewal_test
adjacent_twisted_renewal_test: $(ADJACENT_TWISTED_RENEWAL_TEST_BIN)
	$(ADJACENT_TWISTED_RENEWAL_TEST_BIN)

$(ADJACENT_TWISTED_RENEWAL_TEST_BIN): $(TESTDIR)/adjacent_twisted_renewal_test.cpp $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

RECURRENT_FAMILY_EXHAUSTION_TEST_BIN := $(BUILDDIR)/recurrent_family_exhaustion_test
.PHONY: recurrent_family_exhaustion_test
recurrent_family_exhaustion_test: $(RECURRENT_FAMILY_EXHAUSTION_TEST_BIN)
	$(RECURRENT_FAMILY_EXHAUSTION_TEST_BIN)

$(RECURRENT_FAMILY_EXHAUSTION_TEST_BIN): $(TESTDIR)/recurrent_family_exhaustion_test.cpp $(MATH_LIB)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

RECURRENT_FAMILY_EXHAUSTION_REAL_TEST_BIN := $(BUILDDIR)/recurrent_family_exhaustion_real_test
.PHONY: recurrent_family_exhaustion_real_test
recurrent_family_exhaustion_real_test: $(RECURRENT_FAMILY_EXHAUSTION_REAL_TEST_BIN)
	$(RECURRENT_FAMILY_EXHAUSTION_REAL_TEST_BIN)

$(RECURRENT_FAMILY_EXHAUSTION_REAL_TEST_BIN): $(TESTDIR)/recurrent_family_exhaustion_real_test.cpp $(MATH_LIB)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

out/corona_truth_n6.bin: $(ADJACENT_COMPETITOR_TRANSPORT_BIN)
	@mkdir -p out
	./$(ADJACENT_COMPETITOR_TRANSPORT_BIN) --legacy-corona --cache-only 6

TEST_BIN_RECURRENT_FAMILY_EXHAUSTION_REFLECTION_TEST := $(BUILDDIR)/recurrent_family_exhaustion_reflection_test
$(TEST_BIN_RECURRENT_FAMILY_EXHAUSTION_REFLECTION_TEST): $(TESTDIR)/recurrent_family_exhaustion_reflection_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@
recurrent_family_exhaustion_reflection_test: $(TEST_BIN_RECURRENT_FAMILY_EXHAUSTION_REFLECTION_TEST) out/corona_truth_n6.bin
	./$(TEST_BIN_RECURRENT_FAMILY_EXHAUSTION_REFLECTION_TEST)
.PHONY: recurrent_family_exhaustion_reflection_test

REJECTED_BOUNDARY_EXHAUSTION_TEST_BIN := $(BUILDDIR)/rejected_boundary_exhaustion_test
.PHONY: rejected_boundary_exhaustion_test
rejected_boundary_exhaustion_test: $(REJECTED_BOUNDARY_EXHAUSTION_TEST_BIN)
	$(REJECTED_BOUNDARY_EXHAUSTION_TEST_BIN)

$(REJECTED_BOUNDARY_EXHAUSTION_TEST_BIN): $(TESTDIR)/rejected_boundary_exhaustion_test.cpp $(MATH_LIB)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

out/predicted_core_symbolic_induction_test: tests/predicted_core_symbolic_induction_test.cpp include/ravel/proof/predicted_core_scc_identification.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp $< math/out/libmath.a -o $@

predicted_core_symbolic_induction_test: out/predicted_core_symbolic_induction_test
	./out/predicted_core_symbolic_induction_test

out/radial_linear_feature_factorization_test: tests/radial_linear_feature_factorization_test.cpp include/ravel/proof/radial_linear_feature_factorization.hpp include/ravel/proof/block_height_shell_rank.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Iinclude $< -o $@

.PHONY: radial_linear_feature_factorization_test
radial_linear_feature_factorization_test: out/radial_linear_feature_factorization_test
	./out/radial_linear_feature_factorization_test

out/segment_relation_splice_test: tests/segment_relation_splice_test.cpp include/ravel/proof/segment_relation_splice.hpp include/ravel/proof/cyclic_splice_completion.hpp
	@mkdir -p out
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Iinclude $< -o $@

.PHONY: segment_relation_splice_test
segment_relation_splice_test: out/segment_relation_splice_test
	./out/segment_relation_splice_test

out/stepped_face_residual_seriality_test: tests/stepped_face_residual_seriality_test.cpp include/ravel/proof/stepped_face_residual_seriality.hpp include/ravel/proof/first_return_joint_product.hpp
	$(CXX) $(CXXFLAGS) -Iinclude tests/stepped_face_residual_seriality_test.cpp -o $@

.PHONY: stepped_face_residual_seriality_test
stepped_face_residual_seriality_test: out/stepped_face_residual_seriality_test
	./out/stepped_face_residual_seriality_test

out/stepped_face_role_junction_test: tests/stepped_face_role_junction_test.cpp include/ravel/proof/stepped_face_role_junction.hpp include/ravel/proof/first_return_joint_product.hpp include/ravel/proof/cyclic_splice_compactness.hpp
	$(CXX) $(CXXFLAGS) -Iinclude tests/stepped_face_role_junction_test.cpp -o $@

.PHONY: stepped_face_role_junction_test
stepped_face_role_junction_test: out/stepped_face_role_junction_test
	./out/stepped_face_role_junction_test

out/maximum_shell_exclusion_test: tests/maximum_shell_exclusion_test.cpp include/ravel/proof/maximum_shell_exclusion.hpp
	$(CXX) $(CXXFLAGS) -Iinclude tests/maximum_shell_exclusion_test.cpp -o $@

.PHONY: maximum_shell_exclusion_test
maximum_shell_exclusion_test: out/maximum_shell_exclusion_test
	./out/maximum_shell_exclusion_test

out/parametric_radial_rank_test: tests/parametric_radial_rank_test.cpp include/ravel/proof/parametric_radial_rank.hpp
	$(CXX) $(CXXFLAGS) -Iinclude tests/parametric_radial_rank_test.cpp -o $@

.PHONY: parametric_radial_rank_test
parametric_radial_rank_test: out/parametric_radial_rank_test
	./out/parametric_radial_rank_test

out/parametric_block_height_rank_test: tests/parametric_block_height_rank_test.cpp include/ravel/proof/parametric_block_height_rank.hpp include/ravel/proof/parametric_radial_rank.hpp
	$(CXX) $(CXXFLAGS) -Iinclude tests/parametric_block_height_rank_test.cpp -o $@

.PHONY: parametric_block_height_rank_test
parametric_block_height_rank_test: out/parametric_block_height_rank_test
	./out/parametric_block_height_rank_test

out/parametric_maximum_shell_reduction_test: tests/parametric_maximum_shell_reduction_test.cpp include/ravel/proof/parametric_maximum_shell_reduction.hpp include/ravel/proof/shell_shadow_simulation.hpp include/ravel/proof/shell_shadow_rank_transport.hpp
	$(CXX) $(CXXFLAGS) -Iinclude tests/parametric_maximum_shell_reduction_test.cpp -o $@

.PHONY: parametric_maximum_shell_reduction_test
parametric_maximum_shell_reduction_test: out/parametric_maximum_shell_reduction_test
	./out/parametric_maximum_shell_reduction_test

out/shell_two_interior_factorization_test: tests/shell_two_interior_factorization_test.cpp include/ravel/proof/shell_two_interior_factorization.hpp
	$(CXX) $(CXXFLAGS) -Iinclude tests/shell_two_interior_factorization_test.cpp -o $@

.PHONY: shell_two_interior_factorization_test
shell_two_interior_factorization_test: out/shell_two_interior_factorization_test
	./out/shell_two_interior_factorization_test

BRANCHING_TERMINAL_REDUCTION_TEST_BIN := $(BUILDDIR)/branching_terminal_reduction_test
.PHONY: branching_terminal_reduction_test
branching_terminal_reduction_test: $(BRANCHING_TERMINAL_REDUCTION_TEST_BIN)
	$(BRANCHING_TERMINAL_REDUCTION_TEST_BIN)
$(BRANCHING_TERMINAL_REDUCTION_TEST_BIN): $(TESTDIR)/branching_terminal_reduction_test.cpp $(MATH_LIB)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) $(LDFLAGS) -o $@

PERMUTATION_SKEW_PRODUCT_TEST_BIN := $(BUILDDIR)/permutation_skew_product_test
.PHONY: permutation_skew_product_test
permutation_skew_product_test: $(PERMUTATION_SKEW_PRODUCT_TEST_BIN)
	$(PERMUTATION_SKEW_PRODUCT_TEST_BIN)
$(PERMUTATION_SKEW_PRODUCT_TEST_BIN): $(TESTDIR)/permutation_skew_product_test.cpp $(MATH_LIB)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) $< $(MATH_LIB) $(LDFLAGS) -o $@

UNIVERSAL_DOMINANCE_REDUCTION_TEST_BIN := $(BUILDDIR)/universal_dominance_reduction_test
.PHONY: universal_dominance_reduction_test
universal_dominance_reduction_test: $(UNIVERSAL_DOMINANCE_REDUCTION_TEST_BIN)
	$(UNIVERSAL_DOMINANCE_REDUCTION_TEST_BIN)
$(UNIVERSAL_DOMINANCE_REDUCTION_TEST_BIN): $(TESTDIR)/universal_dominance_reduction_test.cpp $(MATH_LIB)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) $(LDFLAGS) -o $@

PATH_INJECTIVE_SIMULATION_TEST_BIN := $(BUILDDIR)/path_injective_simulation_test
.PHONY: path_injective_simulation_test
path_injective_simulation_test: $(PATH_INJECTIVE_SIMULATION_TEST_BIN)
	$(PATH_INJECTIVE_SIMULATION_TEST_BIN)
$(PATH_INJECTIVE_SIMULATION_TEST_BIN): $(TESTDIR)/path_injective_simulation_test.cpp $(MATH_LIB)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) $< $(MATH_LIB) $(LDFLAGS) -o $@

FINITE_QUOTIENT_CORE_MAXIMALITY_TEST_BIN := $(BUILDDIR)/finite_quotient_core_maximality_test
.PHONY: finite_quotient_core_maximality_test
finite_quotient_core_maximality_test: $(FINITE_QUOTIENT_CORE_MAXIMALITY_TEST_BIN)
	./$(FINITE_QUOTIENT_CORE_MAXIMALITY_TEST_BIN)

$(FINITE_QUOTIENT_CORE_MAXIMALITY_TEST_BIN): $(TESTDIR)/finite_quotient_core_maximality_test.cpp $(MATH_LIB)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TESTDIR)/finite_quotient_core_maximality_test.cpp $(MATH_LIB) $(LDLIBS) -o $@

CONDITION_F_COMPONENT_MAXIMALITY_TEST_BIN := $(BUILDDIR)/condition_f_component_maximality_test
.PHONY: condition_f_component_maximality_test
condition_f_component_maximality_test: $(CONDITION_F_COMPONENT_MAXIMALITY_TEST_BIN)
	$(CONDITION_F_COMPONENT_MAXIMALITY_TEST_BIN)

$(CONDITION_F_COMPONENT_MAXIMALITY_TEST_BIN): $(TESTDIR)/condition_f_component_maximality_test.cpp $(MATH_LIB)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TESTDIR)/condition_f_component_maximality_test.cpp $(MATH_LIB) $(LDLIBS) -o $@

CONDITION_F_CYCLIC_VOLTAGE_FOURIER_TEST_BIN := $(BUILDDIR)/condition_f_cyclic_voltage_fourier_test
.PHONY: condition_f_cyclic_voltage_fourier_test
condition_f_cyclic_voltage_fourier_test: $(CONDITION_F_CYCLIC_VOLTAGE_FOURIER_TEST_BIN)
	$(CONDITION_F_CYCLIC_VOLTAGE_FOURIER_TEST_BIN)
$(CONDITION_F_CYCLIC_VOLTAGE_FOURIER_TEST_BIN): $(TESTDIR)/condition_f_cyclic_voltage_fourier_test.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) $< -o $@

CONDITION_F_PARENT_PAIR_RECURRENCE_TEST_BIN := $(BUILDDIR)/condition_f_parent_pair_recurrence_test
.PHONY: condition_f_parent_pair_recurrence_test
condition_f_parent_pair_recurrence_test: $(CONDITION_F_PARENT_PAIR_RECURRENCE_TEST_BIN)
	$(CONDITION_F_PARENT_PAIR_RECURRENCE_TEST_BIN)
$(CONDITION_F_PARENT_PAIR_RECURRENCE_TEST_BIN): $(TESTDIR)/condition_f_parent_pair_recurrence_test.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) $< -o $@

CONDITION_F_LOCAL_CHANNEL_RECURRENCE_TEST_BIN := $(BUILDDIR)/condition_f_local_channel_recurrence_test
.PHONY: condition_f_local_channel_recurrence_test
condition_f_local_channel_recurrence_test: $(CONDITION_F_LOCAL_CHANNEL_RECURRENCE_TEST_BIN)
	$(CONDITION_F_LOCAL_CHANNEL_RECURRENCE_TEST_BIN)
$(CONDITION_F_LOCAL_CHANNEL_RECURRENCE_TEST_BIN): $(TESTDIR)/condition_f_local_channel_recurrence_test.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) $< -o $@

CONDITION_F_PAIR_UNIVERSAL_REDUCTION_TEST_BIN := $(BUILDDIR)/condition_f_pair_universal_reduction_test
.PHONY: condition_f_pair_universal_reduction_test
condition_f_pair_universal_reduction_test: $(CONDITION_F_PAIR_UNIVERSAL_REDUCTION_TEST_BIN)
	$(CONDITION_F_PAIR_UNIVERSAL_REDUCTION_TEST_BIN)
$(CONDITION_F_PAIR_UNIVERSAL_REDUCTION_TEST_BIN): $(TESTDIR)/condition_f_pair_universal_reduction_test.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) $< -o $@

CONDITION_F_BOUNDARY_INTERFACE_TEST_BIN := $(BUILDDIR)/condition_f_boundary_interface_test
.PHONY: condition_f_boundary_interface_test
condition_f_boundary_interface_test: $(CONDITION_F_BOUNDARY_INTERFACE_TEST_BIN)
	$(CONDITION_F_BOUNDARY_INTERFACE_TEST_BIN)
$(CONDITION_F_BOUNDARY_INTERFACE_TEST_BIN): $(TESTDIR)/condition_f_boundary_interface_test.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) $< -o $@

MARKED_ATOM_CORE_EXTENSION_TEST_BIN := $(BUILDDIR)/marked_atom_core_extension_test
$(MARKED_ATOM_CORE_EXTENSION_TEST_BIN): $(TESTDIR)/marked_atom_core_extension_test.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LDFLAGS)
marked_atom_core_extension_test: $(MARKED_ATOM_CORE_EXTENSION_TEST_BIN)
	./$(MARKED_ATOM_CORE_EXTENSION_TEST_BIN)

CONDITION_F_PAIR_BOUNDARY_SUBSTITUTION_TEST_BIN := $(BUILDDIR)/condition_f_pair_boundary_substitution_test
.PHONY: condition_f_pair_boundary_substitution_test
condition_f_pair_boundary_substitution_test: $(CONDITION_F_PAIR_BOUNDARY_SUBSTITUTION_TEST_BIN)
	$(CONDITION_F_PAIR_BOUNDARY_SUBSTITUTION_TEST_BIN)
$(CONDITION_F_PAIR_BOUNDARY_SUBSTITUTION_TEST_BIN): $(TESTDIR)/condition_f_pair_boundary_substitution_test.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) $< -o $@

lean_condition_f_joint_qr_playground:
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/condition_f_joint_qr_playground.lean)

# Round 79: continued twist-dominance theorems.
.PHONY: twist_dominance_extensions_test
TWIST_DOMINANCE_EXTENSIONS_TEST_BIN := $(BUILDDIR)/twist_dominance_extensions_test
twist_dominance_extensions_test: $(TWIST_DOMINANCE_EXTENSIONS_TEST_BIN)
	$(TWIST_DOMINANCE_EXTENSIONS_TEST_BIN)
$(TWIST_DOMINANCE_EXTENSIONS_TEST_BIN): $(TESTDIR)/twist_dominance_extensions_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(MATH_LIB) -o $@

.PHONY: norm_weighted_qr_majorant_test
NORM_WEIGHTED_QR_MAJORANT_TEST_BIN := $(BUILDDIR)/norm_weighted_qr_majorant_test
norm_weighted_qr_majorant_test: $(NORM_WEIGHTED_QR_MAJORANT_TEST_BIN)
	$(NORM_WEIGHTED_QR_MAJORANT_TEST_BIN)

$(NORM_WEIGHTED_QR_MAJORANT_TEST_BIN): $(TESTDIR)/norm_weighted_qr_majorant_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LDFLAGS) $(LDLIBS)

.PHONY: lean_norm_weighted_qr_majorant
lean_norm_weighted_qr_majorant:
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/norm_weighted_qr_majorant.lean)

.PHONY: concrete_operator_qr_majorant_test
CONCRETE_OPERATOR_QR_MAJORANT_TEST_BIN := $(BUILDDIR)/concrete_operator_qr_majorant_test
concrete_operator_qr_majorant_test: $(CONCRETE_OPERATOR_QR_MAJORANT_TEST_BIN)
	$(CONCRETE_OPERATOR_QR_MAJORANT_TEST_BIN)

$(CONCRETE_OPERATOR_QR_MAJORANT_TEST_BIN): $(TESTDIR)/concrete_operator_qr_majorant_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Iinclude $< -o $@

.PHONY: lean_concrete_operator_qr_spectral_closure
lean_concrete_operator_qr_spectral_closure:
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/concrete_operator_qr_spectral_closure.lean)

# Round 84: arbitrary finite positive generator grammar.
.PHONY: finite_positive_grammar_majorant_test
FINITE_POSITIVE_GRAMMAR_MAJORANT_TEST_BIN := $(BUILDDIR)/finite_positive_grammar_majorant_test
finite_positive_grammar_majorant_test: $(FINITE_POSITIVE_GRAMMAR_MAJORANT_TEST_BIN)
	$(FINITE_POSITIVE_GRAMMAR_MAJORANT_TEST_BIN)
$(FINITE_POSITIVE_GRAMMAR_MAJORANT_TEST_BIN): $(TESTDIR)/finite_positive_grammar_majorant_test.cpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LDFLAGS) $(LDLIBS)

.PHONY: lean_finite_positive_grammar_majorant
lean_finite_positive_grammar_majorant:
	cd $(LEAN_ENV) && lake env lean $(abspath lean/generated/finite_positive_grammar_majorant.lean)

# Round 85: generalized multinacci unit family and first m=2 cases.
.PHONY: generalized_multinacci_family_test generalized_multinacci_unit_probe
GENERALIZED_MULTINACCI_FAMILY_TEST_BIN := $(BUILDDIR)/generalized_multinacci_family_test
GENERALIZED_MULTINACCI_UNIT_PROBE_BIN := $(BUILDDIR)/generalized_multinacci_unit_probe

generalized_multinacci_family_test: $(GENERALIZED_MULTINACCI_FAMILY_TEST_BIN)
	$(GENERALIZED_MULTINACCI_FAMILY_TEST_BIN)
$(GENERALIZED_MULTINACCI_FAMILY_TEST_BIN): $(TESTDIR)/generalized_multinacci_family_test.cpp $(INCDIR)/ravel/generalized_multinacci.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LDFLAGS) $(LDLIBS) $(MATH_LIB)

generalized_multinacci_unit_probe: $(GENERALIZED_MULTINACCI_UNIT_PROBE_BIN)
	$(GENERALIZED_MULTINACCI_UNIT_PROBE_BIN) 2 2 exact 60
$(GENERALIZED_MULTINACCI_UNIT_PROBE_BIN): $(APPDIR)/generalized_multinacci_unit_probe.cpp $(INCDIR)/ravel/generalized_multinacci.hpp | $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LDFLAGS) $(LDLIBS) $(MATH_LIB)

generalized_multinacci_prefix_phase_test: math/out/libmath.a
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/generalized_multinacci_prefix_phase_test.cpp -o out/generalized_multinacci_prefix_phase_test math/out/libmath.a

generalized_multinacci_boundary_word_lift_test: math/out/libmath.a
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/generalized_multinacci_boundary_word_lift_test.cpp -o out/generalized_multinacci_boundary_word_lift_test math/out/libmath.a

generalized_multinacci_phase_expanded_boundary_test: math/out/libmath.a
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/generalized_multinacci_phase_expanded_boundary_test.cpp -o out/generalized_multinacci_phase_expanded_boundary_test math/out/libmath.a

generalized_multinacci_admissible_subgrammar_test: math/out/libmath.a
	mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/generalized_multinacci_admissible_subgrammar_test.cpp -o out/generalized_multinacci_admissible_subgrammar_test math/out/libmath.a

generalized_multinacci_primitive_intertwiner_test: math/out/libmath.a
	mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/generalized_multinacci_primitive_intertwiner_test.cpp -o out/generalized_multinacci_primitive_intertwiner_test math/out/libmath.a

generalized_multinacci_general_m_test: math/out/libmath.a
	mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/generalized_multinacci_general_m_test.cpp -o out/generalized_multinacci_general_m_test math/out/libmath.a

generalized_multinacci_general_m_intertwiner_test: math/out/libmath.a
	mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/generalized_multinacci_general_m_intertwiner_test.cpp -o out/generalized_multinacci_general_m_intertwiner_test math/out/libmath.a

generalized_multinacci_signed_renewal_twist_test: math/out/libmath.a
	mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/generalized_multinacci_signed_renewal_twist_test.cpp -o out/generalized_multinacci_signed_renewal_twist_test math/out/libmath.a

monotone_profile_corridor_closure_test: math/out/libmath.a
	mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/monotone_profile_corridor_closure_test.cpp -o out/monotone_profile_corridor_closure_test math/out/libmath.a

plastic_three_generator_intertwiner_test: math/out/libmath.a
	mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/plastic_three_generator_intertwiner_test.cpp -o out/plastic_three_generator_intertwiner_test math/out/libmath.a

supergolden_three_generator_intertwiner_test: math/out/libmath.a
	mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/supergolden_three_generator_intertwiner_test.cpp -o out/supergolden_three_generator_intertwiner_test math/out/libmath.a

supergolden_qrs_test: math/out/libmath.a
	mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/supergolden_qrs_test.cpp -o out/supergolden_qrs_test math/out/libmath.a

supergolden_qrs_audit: supergolden_qrs_test
	cd $(CURDIR) && ./out/supergolden_qrs_test

theta5_contact_boundary_probe: math/out/libmath.a
	mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/theta5_contact_boundary_probe.cpp -o out/theta5_contact_boundary_probe math/out/libmath.a
	./out/theta5_contact_boundary_probe

shift_branch_three_generator_continuation_test: math/out/libmath.a
	mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/shift_branch_three_generator_continuation_test.cpp -o out/shift_branch_three_generator_continuation_test math/out/libmath.a

coefficient_profile_parity_obstruction_test: math/out/libmath.a
	mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/coefficient_profile_parity_obstruction_test.cpp -o out/coefficient_profile_parity_obstruction_test math/out/libmath.a

.PHONY: nonar_sigma02_long_probe
nonar_sigma02_long_probe: math/out/libmath.a
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp app/nonar_sigma02_long_probe.cpp -o out/nonar_sigma02_long_probe math/out/libmath.a

.PHONY: generalized_multinacci_symbolic_embedding_test
generalized_multinacci_symbolic_embedding_test: math/out/libmath.a
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/generalized_multinacci_symbolic_embedding_test.cpp -o out/generalized_multinacci_symbolic_embedding_test math/out/libmath.a
	./out/generalized_multinacci_symbolic_embedding_test

.PHONY: cyclotomic_obstruction_test
cyclotomic_obstruction_test: out/cyclotomic_obstruction_test
	./out/cyclotomic_obstruction_test

out/cyclotomic_obstruction_test: tests/cyclotomic_obstruction_test.cpp include/ravel/proof/cyclotomic_obstruction.hpp math/out/libmath.a
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ math/out/libmath.a

.PHONY: delayed_self_branch_three_generator_continuation_test
delayed_self_branch_three_generator_continuation_test: math/out/libmath.a
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/delayed_self_branch_three_generator_continuation_test.cpp -o out/delayed_self_branch_three_generator_continuation_test math/out/libmath.a
	./out/delayed_self_branch_three_generator_continuation_test

.PHONY: quartic_fourth_generator_audit_test
quartic_fourth_generator_audit_test: math/out/libmath.a
	@mkdir -p out
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp tests/quartic_fourth_generator_audit_test.cpp -o out/quartic_fourth_generator_audit_test math/out/libmath.a
	./out/quartic_fourth_generator_audit_test

.PHONY: first_genuine_fourth_generator_intertwiner_test
FIRST_GENUINE_FOURTH_GENERATOR_INTERTWINER_TEST_BIN := $(BUILDDIR)/first_genuine_fourth_generator_intertwiner_test
first_genuine_fourth_generator_intertwiner_test: $(FIRST_GENUINE_FOURTH_GENERATOR_INTERTWINER_TEST_BIN)
	./$(FIRST_GENUINE_FOURTH_GENERATOR_INTERTWINER_TEST_BIN)
$(FIRST_GENUINE_FOURTH_GENERATOR_INTERTWINER_TEST_BIN): \
		$(TESTDIR)/first_genuine_fourth_generator_intertwiner_test.cpp \
		$(INCDIR)/ravel/proof/first_genuine_fourth_generator_intertwiner.hpp \
		$(INCDIR)/ravel/proof/contact_boundary_generator_intertwiner.hpp \
		| $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp $< \
		-o $@ $(MATH_LIB)

.PHONY: second_genuine_fourth_generator_intertwiner_probe
SECOND_GENUINE_FOURTH_GENERATOR_INTERTWINER_PROBE_BIN := $(BUILDDIR)/second_genuine_fourth_generator_intertwiner_probe
second_genuine_fourth_generator_intertwiner_probe: $(SECOND_GENUINE_FOURTH_GENERATOR_INTERTWINER_PROBE_BIN)
	./$(SECOND_GENUINE_FOURTH_GENERATOR_INTERTWINER_PROBE_BIN)
$(SECOND_GENUINE_FOURTH_GENERATOR_INTERTWINER_PROBE_BIN): \
		$(TESTDIR)/second_genuine_fourth_generator_intertwiner_test.cpp \
		$(INCDIR)/ravel/proof/second_genuine_fourth_generator_intertwiner.hpp \
		$(INCDIR)/ravel/proof/contact_boundary_generator_intertwiner.hpp \
		| $(BUILDDIR) $(MATH_LIB)
	$(CXX) $(CXXFLAGS) -Iinclude -Imath/include -Imath/include/mini-gmp $< \
		-o $@ $(MATH_LIB)
