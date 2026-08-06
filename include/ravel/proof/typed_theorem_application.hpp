#pragma once

#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ravel::proof::truth {

struct TypedHypothesis {
    std::string key;
    std::string lean_name;
    std::string lean_type;
};

struct ApplicationRequest {
    std::string rule_id;
    std::map<std::string, TypedHypothesis> hypotheses;
};

struct ApplicationResult {
    bool closed = false;
    std::vector<std::string> missing;
    std::string lean_source;
};

class TypedTheoremApplicator {
public:
    ApplicationResult apply(const ApplicationRequest& request) const {
        if (request.rule_id == "charpoly-to-fast-power-reduction")
            return apply_charpoly_power(request);
        ApplicationResult result;
        result.missing.push_back("No typed theorem-application backend is registered for " + request.rule_id);
        return result;
    }

private:
    static ApplicationResult apply_charpoly_power(const ApplicationRequest& request) {
        ApplicationResult result;
        const auto it = request.hypotheses.find("charpoly_identity");
        if (it == request.hypotheses.end()) {
            result.missing.push_back("charpoly_identity : A.charpoly = nbonacciCharpoly n");
            return result;
        }
        const auto& h = it->second;
        if (h.lean_name.empty() || h.lean_type.empty())
            throw std::runtime_error("typed hypothesis requires a Lean name and type");

        std::ostringstream out;
        out << "import Mathlib.Tactic\n\n";
        out << "open Matrix Polynomial\n\n";
        out << "def nbonacciGeomSum {R : Type} [Ring R] (a : R) : ℕ → R\n";
        out << "  | 0 => 0\n";
        out << "  | k + 1 => nbonacciGeomSum a k + a ^ (k + 1)\n\n";
        out << "theorem nbonacciGeomSum_succ {R : Type} [Ring R] (a : R) (n : ℕ) :\n";
        out << "    nbonacciGeomSum a (n + 1) = nbonacciGeomSum a n + a ^ (n + 1) := by\n";
        out << "  rfl\n\n";
        out << "theorem nbonacci_mul_geomSum {R : Type} [Ring R] (a : R) (n : ℕ) :\n";
        out << "    a * nbonacciGeomSum a n = nbonacciGeomSum a (n + 1) - a := by\n";
        out << "  induction n with\n";
        out << "  | zero => simp [nbonacciGeomSum]\n";
        out << "  | succ n ih =>\n";
        out << "      calc\n";
        out << "        a * nbonacciGeomSum a (n + 1) =\n";
        out << "            a * (nbonacciGeomSum a n + a ^ (n + 1)) := by rw [nbonacciGeomSum_succ]\n";
        out << "        _ = (nbonacciGeomSum a (n + 1) - a) + a * a ^ (n + 1) := by rw [mul_add, ih]\n";
        out << "        _ = nbonacciGeomSum a (n + 1 + 1) - a := by\n";
        out << "              rw [nbonacciGeomSum_succ a (n + 1), ← pow_succ' a (n + 1)]\n";
        out << "              noncomm_ring\n";
        out << "        _ = nbonacciGeomSum a (n + 2) - a := by congr 2\n\n";
        out << "theorem nbonacci_block_identity_scalar {R : Type} [Ring R]\n";
        out << "    (a : R) (n : ℕ) (hsum : nbonacciGeomSum a n = 1) :\n";
        out << "    a ^ (n + 1) = 2 * a - 1 := by\n";
        out << "  have hshift : nbonacciGeomSum a (n + 1) = 2 * a := by\n";
        out << "    calc\n";
        out << "      nbonacciGeomSum a (n + 1) = a * nbonacciGeomSum a n + a := by\n";
        out << "        rw [nbonacci_mul_geomSum]\n";
        out << "        noncomm_ring\n";
        out << "      _ = a * 1 + a := by rw [hsum]\n";
        out << "      _ = 2 * a := by noncomm_ring\n";
        out << "  calc\n";
        out << "    a ^ (n + 1) = nbonacciGeomSum a (n + 1) - nbonacciGeomSum a n := by\n";
        out << "      rw [nbonacciGeomSum_succ]\n";
        out << "      noncomm_ring\n";
        out << "    _ = 2 * a - 1 := by rw [hshift, hsum]\n\n";
        out << "noncomputable def nbonacciCharpoly (n : ℕ) : Polynomial ℤ :=\n";
        out << "  (Finset.sum (Finset.range n) (fun k => Polynomial.X ^ (k + 1))) - Polynomial.C 1\n\n";
        out << "theorem nbonacci_geomSum_of_charpoly {ι : Type} [Fintype ι]\n";
        out << "    [DecidableEq ι] (A : Matrix ι ι ℤ) (n : ℕ)\n";
        out << "    (hchar : A.charpoly = nbonacciCharpoly n) :\n";
        out << "    nbonacciGeomSum A n = 1 := by\n";
        out << "  have hc := Matrix.aeval_self_charpoly A\n";
        out << "  rw [hchar, Polynomial.aeval_def] at hc\n";
        out << "  simp [nbonacciCharpoly] at hc\n";
        out << "  change (Polynomial.eval₂AddMonoidHom (algebraMap ℤ (Matrix ι ι ℤ)) A)\n";
        out << "      (Finset.sum (Finset.range n) (fun k => Polynomial.X ^ (k + 1))) - 1 = 0 at hc\n";
        out << "  rw [map_sum] at hc\n";
        out << "  simp [Polynomial.eval₂AddMonoidHom_apply] at hc\n";
        out << "  have hsum_all : ∀ q : ℕ, nbonacciGeomSum A q =\n";
        out << "      Finset.sum (Finset.range q) (fun k => A ^ (k + 1)) := by\n";
        out << "    intro q\n";
        out << "    induction q with\n";
        out << "    | zero => simp [nbonacciGeomSum]\n";
        out << "    | succ q ih => rw [nbonacciGeomSum, Finset.sum_range_succ, ih]\n";
        out << "  rw [← hsum_all n] at hc\n";
        out << "  exact sub_eq_zero.mp hc\n\n";
        out << "theorem truth_machine_fast_power_reduction {ι : Type} [Fintype ι]\n";
        out << "    [DecidableEq ι] (A : Matrix ι ι ℤ) (n : ℕ)\n";
        out << "    (" << h.lean_name << " : " << h.lean_type << ") :\n";
        out << "    A ^ (n + 1) = 2 * A - 1 := by\n";
        out << "  exact nbonacci_block_identity_scalar A n\n";
        out << "    (nbonacci_geomSum_of_charpoly A n " << h.lean_name << ")\n";

        result.closed = true;
        result.lean_source = out.str();
        return result;
    }
};

} // namespace ravel::proof::truth
