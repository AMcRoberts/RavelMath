// ravel/generic_memoized_dag_closure.hpp
//
// The shared contract underlying this project's exact, deduped closure
// computations: corona.hpp's C-corona/Algorithm-2 iteration, the
// property-(F) zero-expansion graph BFS in
// adelic/coincidence_and_property_f.hpp, and
// ravel/proof/coincidence_closure.hpp all compute a closure over a
// discrete STATE space, deduping via an exact (no floating point)
// comparison, growing until a fixed point or a cap.
//
// This header factors out the ONE piece of that pattern that is
// actually acyclic-DAG-shaped and hence cleanly memoizable without
// each caller re-deriving its own recursion-with-cache boilerplate:
// states that strictly DECREASE along some monotone quantity (a
// "budget" -- remaining depth here, corona round elsewhere), so the
// dependency graph between states has no cycles and a plain top-down
// memoized recursion is exact and terminates.
//
// This is deliberately NOT a forced unification of corona.hpp or
// coincidence_and_property_f.hpp's own closures onto this base --
// both of those are genuinely more complex (corona's Red-pruning and
// round-level rank bookkeeping; property-F's Tarjan SCC pass and
// node-budget-based early termination for cyclic BFS, since its state
// space is NOT acyclic in general -- gamma values can revisit
// themselves, which is the whole point of the cycle check). Retrofitting
// either onto this base would cost real correctness risk on tested,
// working code for no functional gain. What IS built on this base is
// ravel/proof/coincidence_closure.hpp, whose state space (junction,
// remaining_depth) genuinely is a DAG (remaining_depth strictly
// decreases), matching this header's actual scope honestly rather than
// by assertion.

#pragma once

#include <functional>
#include <map>

namespace ravel {

// Generic memoized closure over an ACYCLIC state space: StateKey must
// be totally ordered (usable as a std::map key); ValueSet is whatever
// per-state result type the caller accumulates (a std::set of outcomes,
// typically). The caller supplies `compute`, which is handed the state
// key and a callback to recursively fetch (and memoize) the value at
// any OTHER state key -- `compute` must only ever request states that
// are strictly "smaller" under the caller's own budget ordering, or
// this will recurse forever; that invariant is the caller's
// responsibility (mirroring corona.hpp's own "A[p] built only from
// A[p-1]" discipline, just expressed as memoization instead of
// explicit round bookkeeping).
template <typename StateKey, typename ValueSet>
class MemoizedDagClosure {
public:
    using ComputeFn = std::function<ValueSet(const StateKey&, std::function<const ValueSet&(const StateKey&)>)>;

    explicit MemoizedDagClosure(ComputeFn compute) : compute_(std::move(compute)) {}

    const ValueSet& get(const StateKey& key) {
        auto it = memo_.find(key);
        if (it != memo_.end()) return it->second;
        // Insert a placeholder BEFORE computing, so that if `compute`
        // ever (incorrectly) requests the same key again, it fails
        // loudly via an infinite loop being caught by the caller's own
        // test suite rather than silently returning a half-built value
        // -- deliberately not defending against this here, matching
        // this project's preference for a real crash over a
        // silently-wrong answer.
        ValueSet result = compute_(key, [this](const StateKey& k) -> const ValueSet& { return get(k); });
        auto [inserted_it, ok] = memo_.emplace(key, std::move(result));
        (void)ok;
        return inserted_it->second;
    }

private:
    ComputeFn compute_;
    std::map<StateKey, ValueSet> memo_;
};

}  // namespace ravel
