# C++ course-handler interface to build next

The current runner validates and materializes the Lua campaign tree. The next source change should add a closed course-handler registry. Do not add arbitrary Lua callbacks.

Suggested C++ contract:

```cpp
struct CourseContext {
    const UniversalProofPlan& plan;
    const ProofObligation& obligation;
    const PivotCourse& course;
    std::filesystem::path run_dir;
};

enum class CourseOutcomeKind { proved, blocked, contradicted, infrastructure_error };

struct CourseOutcome {
    CourseOutcomeKind kind;
    std::string classification;
    std::vector<std::filesystem::path> artifacts;
    std::string next_course;
};

using CourseHandler = CourseOutcome (*)(const CourseContext&);
```

Registry keys must be checked-in course IDs. Each handler should have four phases:

1. construct a candidate symbolic certificate;
2. validate it independently in C++ and with finite mutation tests;
3. emit the smallest standalone Lean lemma;
4. invoke Lean and classify the first failure.

The first useful handlers are:

- `cm.reindex_library`: inventory adjoining Lean/Mathlib sources and emit a lemma map;
- `cm.custom_erase_index`: generate the reusable erase-index module;
- `rt.triangular_api`: emit the independent triangular determinant theorem;
- `qm.shared_erase_index`: reuse the generic transport for q-matrix reduction.

A handler must never report `proved` solely because finite regressions pass.
