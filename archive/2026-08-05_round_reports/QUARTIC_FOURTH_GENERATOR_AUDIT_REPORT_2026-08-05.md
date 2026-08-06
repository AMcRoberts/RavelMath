# Quartic fourth-generator audit

The quartic Pisot substitution

- 0 -> 1
- 1 -> 2
- 2 -> 3
- 3 -> 03

has two exact parent prefixes: epsilon and 0. Retaining the ordered prefix pair gives four nonempty positive matrices:

- E_ee: 286 boundary edges
- E_e0: 158 boundary edges
- E_0e: 67 boundary edges
- E_00: 45 boundary edges

All four matrices separately satisfy the ordered-parent-role comparison inequality.

However, the signed prefix-defect observable sends

- E_ee and E_00 to defect 0,
- E_e0 to +e0,
- E_0e to -e0.

Thus the four-colour presentation factors through the exact three-generator quotient

G0 = E_ee + E_00,  G+ = E_e0,  G- = E_0e.

The split between E_ee and E_00 is a kernel refinement of the defect map. It is useful for provenance and controller refinements but is not a fourth independent transport observable. The same raw four-way split already occurs for plastic, so counting it would retroactively make plastic a four-matrix system and defeat the established irreducibility criterion.

Conclusion: the quartic branch remains genuinely Q/R/S, not Q/R/S/T.
