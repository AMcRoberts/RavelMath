# Lean theorem inventory — 2026-08-04

This inventory was regenerated while auditing the corona/Rauzy projection change. It lists every `theorem` and `lemma` declaration in the live `lean/` and `Ravel/` trees. It is an orientation index, not a claim that every file is kernel-checked; formal status remains governed by `THEOREM_STATUS.md`.

- Files containing theorem declarations: **52**
- Theorem/lemma declarations: **310**
- Files containing literal `sorry` or `admit`: **4**

## `Ravel/Matrix/EraseIndex.lean`

- L110: `theorem skip_injective`

## `Ravel/Polynomial/Normalization.lean`

- L11: `theorem one_add_X_mul_sum_range_pow`
- L27: `theorem neg_one_pow_twice`

## `lean/Ravel/Matrix/EraseIndex.lean`

- L61: `theorem skip_injective`

## `lean/bp_correction_determinant.lean`

Contains a literal `sorry` or `admit`; consult theorem status before use.

- L76: `theorem block_det_reduction`
- L117: `theorem main_reduction`
- L179: `theorem det_A_full_eq_det_P`

## `lean/class_ii_affine_shells.lean`

- L48: `theorem contactKind_card`
- L73: `theorem preContactKind_card`
- L100: `theorem dContKind_card`
- L114: `theorem dContNode_in_preContact`
- L136: `theorem preContactNode_backward_layer_partition`
- L205: `theorem preContactBackwardWitness_lattice_identity`
- L228: `theorem dContFaceCandidateKind_card`
- L276: `theorem acceptedFaceCandidate_is_dCont`
- L295: `theorem preContactNode_partition`
- L302: `theorem contactRedExcluded_disjoint`
- L312: `theorem shellKind_card`
- L376: `theorem shellHop_color_matches`
- L383: `theorem shellHop_from_explicit_signed_contact`
- L412: `theorem shellNode_propagates`
- L433: `theorem contactNode_in_restrictedH`
- L446: `theorem contactRedExcludedNode_in_restrictedH`
- L459: `theorem restrictedH_forces_bounded_x0`
- L505: `theorem dContFaceCandidate_window_iff`
- L526: `theorem shellNode_in_open_signed_strip`
- L542: `theorem class_ii_window_bounds`
- L604: `theorem class_ii_perron_window_bounds`
- L626: `theorem class_ii_perron_gap_lt_one`
- L656: `theorem class_ii_perron_gap_gt_half`
- L701: `theorem class_ii_perron_gap_gt_two_thirds`
- L746: `theorem class_ii_contactNode_valid`
- L774: `theorem class_ii_rawContact_x0_bounded`
- L809: `theorem class_ii_dCont_face_candidate_valid_iff`
- L838: `theorem class_ii_shell_endpoint_valid`
- L875: `theorem shellNode_injective_at_round`
- L884: `theorem shellNode_round_eq`
- L895: `theorem class_ii_affine_shell_range_infinite`

## `lean/class_ii_balanced_pivot.lean`

- L47: `theorem for`
- L68: `theorem balanced_pivot_at_one`
- L85: `theorem cubic_is_perfect_square_at_one`
- L94: `theorem ratios_equal_implies_cubic_perfect_square`
- L122: `theorem balanced_pivot_iff_a_eq_one`
- L214: `theorem deviation_identity`
- L239: `theorem deviation_abs_eq_a_minus_one`

## `lean/class_ii_global_round_partition.lean`

- L43: `theorem class_ii_global_round_domains_exhaustive`
- L53: `theorem class_ii_global_round_phase_spec`
- L65: `theorem class_ii_global_round_phase_unique`
- L79: `theorem class_ii_global_round_stitch`

## `lean/class_ii_neighbor2_extensions.lean`

- L37: `theorem neighbor2TerminalSextet_card`
- L47: `theorem neighbor2PenultimatePair_card`
- L54: `theorem neighbor2InteriorTip_injective`
- L60: `theorem neighbor2InteriorTips_infinite`
- L67: `theorem neighbor2InteriorTip_propagates`
- L75: `theorem neighbor2InteriorTip_in_open_strip`
- L132: `theorem neighbor2FixedNode_in_open_strip`
- L147: `theorem class_ii_neighbor2_perron_gap_bounds`
- L187: `theorem class_ii_neighbor2_perron_lt_two_a`
- L218: `theorem class_ii_neighbor2_perron_lt_three_halves_a`
- L249: `theorem class_ii_neighbor2_perron_gap_gt_two_thirds`
- L271: `theorem neighbor2FixedNode_perron_valid`
- L285: `theorem neighbor2FixedNode_cubic_valid`
- L303: `theorem neighbor2TerminalKind_card`
- L308: `theorem neighbor2FixedKind_card`
- L327: `theorem neighbor2TerminalNode_in_open_strip`
- L349: `theorem neighbor2PenultimateExtra_in_open_strip`
- L367: `theorem class_ii_neighbor2_terminal_margins`
- L461: `theorem class_ii_neighbor2_terminal_refined_margins`
- L550: `theorem class_ii_neighbor2_fixed_refined_margins`
- L676: `theorem neighbor2TerminalNode_cubic_valid`
- L702: `theorem neighbor2PenultimateExtra_cubic_valid`
- L726: `theorem neighbor2PenultimateTip_cubic_valid`
- L757: `theorem neighbor2InteriorTip_cubic_valid`
- L802: `theorem neighbor2SecondExtra_cubic_valid`
- L825: `theorem centerInterfaceSupportKind_card`
- L859: `theorem centerInterfaceSupportNode_in_open_strip`
- L874: `theorem centerInterfaceSupportNode_cubic_valid`
- L896: `theorem centerBaseRedExtraKind_card`
- L914: `theorem centerBaseRedExtraNode_in_open_strip`
- L927: `theorem centerBaseRedExtraNode_cubic_valid`

## `lean/class_ii_neighbor_d_support.lean`

- L69: `theorem affine_closed_form_identity`
- L75: `theorem affine_catalogue_affine`
- L102: `theorem neighbor0Edges_card`
- L115: `theorem neighbor0BoundarySource_eq`
- L120: `theorem neighbor0BoundaryTarget_eq`
- L125: `theorem neighbor0_d_nnz`
- L130: `theorem neighbor0_catalogue_affine`
- L138: `theorem neighbor0_boundary_layer_support`
- L173: `theorem neighbor1Edges_card`
- L183: `theorem neighbor1BoundarySource_eq`
- L188: `theorem neighbor1BoundaryTarget_eq`
- L193: `theorem neighbor1_d_nnz`
- L201: `theorem neighbor1_boundary_layer_support`
- L253: `theorem neighbor2Edges_card`
- L263: `theorem neighbor2BoundarySource_eq`
- L268: `theorem neighbor2BoundaryTarget_eq`
- L274: `theorem neighbor2_d_nnz`
- L282: `theorem neighbor2_boundary_layer_support`
- L304: `theorem all_neighbors_boundary_layer_support`

## `lean/class_ii_neighbor_dominance.lean`

- L40: `theorem neighbor0_eval_pred`
- L46: `theorem neighbor1_eval_self`
- L51: `theorem neighbor2_eval_self`
- L58: `theorem neighbor0_core_negative_at_pred`
- L80: `theorem neighbor1_core_negative_at_self`
- L88: `theorem neighbor2_core_negative_at_self`
- L112: `theorem neighbor0_shell_below_pred`
- L120: `theorem neighbor1_shell_below_pred`
- L128: `theorem neighbor2_shell_below_self`

## `lean/class_ii_round1_red_pruning.lean`

- L74: `theorem round1Raw27_length`
- L81: `theorem round1Raw27_x2_bound`
- L94: `theorem round1RawCandidateKind_card`
- L119: `theorem round1RawCandidateX2_outside_target_range`
- L137: `theorem round1_red_pruning_no_match`

## `lean/class_ii_round234_shape_closure.lean`

- L55: `theorem affine_no_solution_at_or_above_threshold`
- L99: `theorem occurrenceSlope_is_zero_or_one`
- L105: `theorem fixedOccurrenceKind_card`
- L117: `theorem both_fixed_slope_bounded`

## `lean/class_ii_six_vertex_graduation.lean`

- L78: `theorem promotedNodes_length`
- L85: `theorem promotedNodes_nodup`
- L92: `theorem promoted_disjoint_transferred`
- L103: `theorem promotedNodes_distinct_at_4`
- L106: `theorem promotedNodes_distinct_at_5`
- L109: `theorem promotedNodes_distinct_at_6`
- L112: `theorem promotedNodes_distinct_at_7`
- L115: `theorem promotedNodes_distinct_at_8`
- L118: `theorem promotedNodes_distinct_at_9`
- L121: `theorem promotedNodes_distinct_at_10`
- L124: `theorem promotedNodes_distinct_at_20`
- L127: `theorem promotedNodes_distinct_at_100`
- L136: `theorem promoted_disjoint_transferred_at_4`
- L140: `theorem promoted_disjoint_transferred_at_5`
- L144: `theorem promoted_disjoint_transferred_at_6`
- L148: `theorem promoted_disjoint_transferred_at_7`
- L152: `theorem promoted_disjoint_transferred_at_8`
- L156: `theorem promoted_disjoint_transferred_at_9`
- L160: `theorem promoted_disjoint_transferred_at_10`
- L164: `theorem promoted_disjoint_transferred_at_20`
- L168: `theorem promoted_disjoint_transferred_at_100`
- L178: `theorem six_vertex_graduation_total_at_4`
- L182: `theorem six_vertex_graduation_total_at_5`
- L186: `theorem six_vertex_graduation_total_at_10`
- L190: `theorem six_vertex_graduation_total_at_100`
- L206: `theorem six_vertex_graduation_closed_form`
- L249: `theorem six_vertex_catalogue_shape_universal`

## `lean/class_ii_terminal_shells.lean`

- L95: `theorem shellNode_injective_at_round`
- L103: `theorem shellNode_right_le_one`
- L110: `theorem shellNode_right_one_implies_left_zero`
- L163: `theorem terminalCrossColours_distinct`
- L172: `theorem terminalCrossColour1_not_eq_extreme00`
- L181: `theorem terminalCrossColour2_not_eq_extreme11`
- L189: `theorem terminalCrossColour1_not_eq_extreme11`
- L197: `theorem terminalCrossColour2_not_eq_extreme00`
- L205: `theorem terminalCrossColours_not_eq_interior_extremes`
- L217: `theorem terminalCrossColour1_injective`
- L225: `theorem terminalCrossColour2_injective`
- L234: `theorem terminalCrossColours_cross_injective`
- L248: `theorem terminalCrossColour1_not_in_interior_shell`
- L264: `theorem terminalCrossColour2_not_in_interior_shell`
- L281: `theorem terminalCrossColour1_range_infinite`
- L295: `theorem terminalCrossColour2_range_infinite`
- L327: `theorem interiorShellKindList_length`
- L330: `theorem interiorShellKindList_nodup`
- L343: `theorem interiorShellFinset_card`
- L377: `theorem terminalShellList_length`
- L383: `theorem terminalShellList_nodup`
- L412: `theorem terminalShellFinset_card`

## `lean/continuation_controller_family_algebra.lean`

- L18: `theorem mem_controllerPre_iff`
- L39: `theorem residualController_nil`
- L47: `theorem residualController_cons`
- L60: `theorem residualController_sound`

## `lean/coupled_automaton_characterization.lean`

- L16: `theorem synthesized_winning_predicate_sound`

## `lean/covering_translation_tube.lean`

- L3: `theorem cover_tube_coordinate_bound`
- L7: `theorem affine_path_transport`

## `lean/cycle_charpoly_campaign_validation.lean`

- L16: `theorem cyclePolynomial_monic_succ`
- L22: `theorem cyclePolynomial_ne_zero`
- L36: `theorem canonicalCycleCore_charpoly`
- L53: `theorem canonicalCycleCore_companion_entries`

## `lean/cyclic_controller_pumping.lean`

- L26: `theorem cyclic_affine_transport_closes`
- L88: `theorem cyclicStrictShellPump_of_witness`
- L108: `theorem cyclicStrictShellPump_of_complete_family`

## `lean/defect_corrected_radial_transport.lean`

- L15: `theorem nbonacci_radial_defect_adjacent`
- L26: `theorem nbonacci_radial_defect_terminal`
- L39: `theorem nbonacci_radial_defect_profile_complete`

## `lean/defect_spliced_covering_tube.lean`

- L8: `theorem defect_splice_step`
- L25: `theorem defect_spliced_affine_path_transport`

## `lean/family_of_families_adjacent_swap_counts.lean`

- L13: `theorem classII_adjacent_swap_count`
- L22: `theorem nbonacci_adjacent_swap_count`
- L27: `theorem nbonacci_adjacent_swap_count_closed_form`

## `lean/free_involution_perron_core.lean`

Contains a literal `sorry` or `admit`; consult theorem status before use.

- L66: `theorem perron_eigenvector_is_phi_invariant`
- L119: `theorem quotient_perron_eigenequation`

## `lean/free_involution_perron_existence_draft.lean`

Contains a literal `sorry` or `admit`; consult theorem status before use.

- L76: `theorem perron_eigenvector_is_phi_invariant`
- L117: `theorem quotient_perron_eigenequation`
- L199: `theorem perron_unique_of_irreducible`
- L411: `theorem almost`
- L452: `theorem perron_exists_of_irreducible`
- L661: `theorem quotient_perron_eigenequation_complete`

## `lean/generated/adjacent_competitor_macro_profile.lean`

- L9: `theorem macroProfile_eq_relabel`
- L24: `theorem profileEval_relabel`

## `lean/generated/cyclic_splice_compactness.lean`

- L30: `theorem strict_shell_pump_of_cyclic_lap`
- L42: `theorem recurrent_radius_le_one_of_cyclic_lap`

## `lean/generated/cyclic_splice_completion.lean`

- L12: `lemma chooseNext_spec`
- L22: `lemma orbit_step`
- L32: `theorem finite_serial_relation_has_repeated_orbit`
- L70: `theorem strict_shell_pump_of_serial_lap`
- L85: `theorem recurrent_radius_le_one_of_serial_lap`

## `lean/generated/first_return_obligations_discharged.lean`

- L32: `theorem eval_deriveWord_iff_path`
- L81: `theorem realized_language_complete_of_replayed_segment`
- L99: `theorem accepted_path_affine_transport_of_replayed_segment`
- L130: `theorem strict_shell_pump_of_cyclic_splice`
- L144: `theorem recurrent_radius_le_one_of_cyclic_splice`

## `lean/generated/first_return_residual_induction.lean`

- L2437: `theorem residualIndex_nil`
- L2438: `theorem residualIndex_cons`
- L2442: `theorem every_word_has_exact_residual`

## `lean/generated/generic_residual_formula.lean`

- L34: `theorem eval_pre_iff`
- L43: `theorem eval_deriveWord_iff_path`

## `lean/generated/multi_compartment_closure_validation.lean`

- L5: `theorem classII_site_swap_count`
- L11: `theorem graph_matrix_scalar_agreement`
- L13: `theorem tribonacci_charpoly_agreement`

## `lean/generated/nbonacci_universal_n.kernel_checked.lean`

- L43: `lemma rMatrix_lowerTriangular`
- L55: `lemma rMatrix_det`
- L61: `lemma qMatrix_minor_eq_qMatrix`
- L83: `lemma qResidualCore_eq_rMatrix`
- L109: `lemma qResidualMinor_det`
- L178: `lemma qMatrix_det_recurrence`
- L271: `lemma qMatrix_det_closed_form_base`
- L276: `lemma qMatrix_det_closed_form_succ`
- L282: `lemma qMatrix_det_closed_form`
- L289: `lemma nbonacciCharacteristic_minor_q`
- L304: `lemma nbonacciCharacteristic_minor_r`
- L315: `lemma nbonacci_characteristic_split`
- L398: `lemma nbonacci_characteristic_polynomial`
- L405: `lemma nbonacci_universal_n`

## `lean/generated/realized_first_return_completeness.lean`

- L32: `theorem eval_deriveWord_iff_path`
- L100: `theorem strictShellPump_of_realized_language`
- L117: `theorem iterate_strict_shell_lift`
- L136: `theorem recurrent_radius_le_one_of_realized_language`

## `lean/generated/reflective_boundary_grammar.lean`

- L17: `theorem shiftAppend_cons`
- L22: `theorem getLast_shiftAppend`
- L27: `theorem head_shiftAppend`

## `lean/generated/shared_polynomial_closure_validation.lean`

- L5: `theorem matrix_spectral_polynomial_normal_form`
- L11: `theorem close_matrix_geometric_recurrence`
- L29: `theorem close_spectral_geometric_recurrence`

## `lean/generated/sign_flux_rank_playground_2026-08-04.lean`

- L21: `theorem strict_of_positive_gain`

## `lean/generated/twisted_bellman_transport.lean`

- L15: `theorem bellmanUpper_transport`
- L31: `theorem strict_renewal_transports`

## `lean/generated/twisted_dimension_extension.lean`

- L8: `theorem twisted_extension_transport`
- L22: `theorem xor_path_substitution`

## `lean/graph_cycle_charpoly_factor_validation.lean`

- L10: `theorem feeder_cycle_charpoly_factor`
- L27: `theorem feeder_cycle_charpoly_closed`

## `lean/graph_cycle_core_seam_validation.lean`

- L10: `theorem cycleWithFeederOutdegrees_length`
- L14: `theorem cycleWithFeeder_edge_count`
- L20: `theorem edge_count_of_cycle_core_and_feeder`

## `lean/nbonacci_margin_catalogue.lean`

- L50: `theorem nbonacciCoreMember_mirror`
- L73: `theorem nbonacciPairNodeCount_mirror`
- L85: `theorem nbonacciCoreStrata_numerator_identity`
- L100: `theorem nbonacciDistanceFromPreviousShadow_le_three`
- L119: `theorem nbonacciDistanceToPreviousShadow_le_two`
- L150: `theorem carryUpdate_sum`
- L158: `theorem carryUpdate_sum_digit_bound`
- L164: `theorem carryUpdate_take_prefix`
- L173: `theorem blockDefect_of_sum_update`
- L184: `theorem nbonacciGeomSum_succ`
- L188: `theorem nbonacci_mul_geomSum`
- L206: `theorem nbonacci_block_identity_scalar`
- L223: `theorem nbonacci_block_identity_matrix`
- L233: `theorem nbonacci_geomSum_of_charpoly`
- L260: `theorem nbonacci_block_identity_of_charpoly`
- L390: `theorem nbonacciBlockForcing_coord_bound`
- L415: `theorem nbonacciBlockForcing_l1_bound`
- L434: `theorem no_strict_rank_finite`
- L454: `theorem no_strict_rank_relation_closed`
- L472: `theorem no_strict_rank_closed_subset`

## `lean/perron_column_difference.lean`

- L23: `theorem perron_column_difference`
- L36: `theorem perron_three_coordinate_identity`
- L46: `theorem classII_column_difference`
- L56: `theorem tribonacci_column_difference`
- L67: `theorem companion_perron_gap_positive`
- L80: `theorem companion_perron_gap_negative`
- L97: `theorem nbonacci_gap_step`
- L107: `theorem positive_gap_of_positive_scaled_gap`
- L116: `theorem tetrabonacci_first_gap`
- L127: `theorem tetrabonacci_gap_order`
- L155: `theorem pentanacci_gap_order`
- L179: `theorem hexanacci_gap_order`

## `lean/predicted_core_scc_exhaustion.lean`

- L8: `theorem exact_scc_of_stronglyConnected_noReturnAfterExit`

## `lean/predicted_core_scc_identification.lean`

- L7: `theorem stronglyConnected_of_shadow`
- L23: `theorem predictedCore_stronglyConnected_induction`
- L36: `theorem core_is_exact_scc`

## `lean/rMatrix_proof_attempt.lean`

- L17: `lemma rMatrix_lowerTriangular`
- L27: `theorem rMatrix_det`

## `lean/radial_translation_defect.lean`

- L10: `theorem affine_block_translation`
- L20: `theorem affine_block_same_translation_defect`
- L34: `theorem nbonacci_block_translation_defect`

## `lean/return_contact_lift.lean`

- L70: `theorem lifted_edge_projects`
- L80: `theorem lifted_left_occurrence_commutes`
- L96: `theorem lifted_right_occurrence_commutes`

## `lean/symbolic_radius_one_controller.lean`

- L8: `theorem adjusted_digit_exists_iff_tail_inequality`
- L31: `theorem winning_certificate_sound`

## `lean/uniform_radius_one_synthesis.lean`

- L16: `theorem forcedStep_add_translation`
- L32: `theorem interior_transport_radius_one`
- L47: `theorem boundary_alignment_radius_one`
- L63: `theorem uniform_radius_one_transport_correct`

## `lean/universal_dominance_phase_rank_transport.lean`

- L8: `theorem strict_rank_of_phase_offset`
- L27: `theorem shell_empty_of_phase_rank_transport`

## `lean/universal_dominance_shell_return_validation.lean`

- L8: `theorem no_strict_rank_relation_closed`
- L28: `theorem shell_empty_of_strict_first_return_rank`
- L61: `theorem no_nonempty_shell_with_strict_first_return_rank`

## `lean/universal_shell_pumping_proof.lean`

Contains a literal `sorry` or `admit`; consult theorem status before use.

- L7: `theorem iterate_strict_shell_lift`
- L32: `theorem no_outer_recurrence_of_bounded_strict_lift`
- L58: `theorem universal_carry_bound_from_controller_family`

