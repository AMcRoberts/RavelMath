#pragma once

#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace ravel::proof {

using RadialVector = std::vector<std::int64_t>;

inline std::int64_t signum(std::int64_t x) {
    return (x > 0) - (x < 0);
}

inline RadialVector nbonacci_linear_step(const RadialVector& x) {
    if (x.size() < 2)
        throw std::invalid_argument("nbonacci_linear_step: dimension must be >= 2");
    RadialVector y(x.size(), 0);
    for (std::size_t i = 0; i + 1 < x.size(); ++i)
        y[i] = x[i + 1];
    y.back() = x.front() -
        std::accumulate(x.begin() + 1, x.end(), std::int64_t{0});
    return y;
}

inline RadialVector radial_translation(const RadialVector& x,
                                       std::int64_t period) {
    if (period <= 0)
        throw std::invalid_argument("radial_translation: period must be positive");
    RadialVector t(x.size(), 0);
    for (std::size_t i = 0; i < x.size(); ++i)
        t[i] = period * signum(x[i]);
    return t;
}

struct RadialDefectProfile {
    std::vector<std::int64_t> signs;
    std::vector<std::int64_t> adjacent_flux;
    std::int64_t signed_imbalance = 0;
    std::int64_t terminal_flux = 0;
};

inline RadialDefectProfile radial_defect_profile(const RadialVector& x) {
    if (x.size() < 2)
        throw std::invalid_argument("radial_defect_profile: dimension must be >= 2");
    RadialDefectProfile p;
    p.signs.reserve(x.size());
    for (auto v : x) p.signs.push_back(signum(v));
    p.adjacent_flux.reserve(x.size() - 1);
    for (std::size_t i = 0; i + 1 < x.size(); ++i)
        p.adjacent_flux.push_back(p.signs[i + 1] - p.signs[i]);
    p.signed_imbalance = std::accumulate(
        p.signs.begin() + 1, p.signs.end(), std::int64_t{0});
    p.terminal_flux = p.signs.front() - p.signed_imbalance - p.signs.back();
    return p;
}

struct DefectCorrectedRadialCertificate {
    std::int64_t period = 0;
    RadialVector translation;
    RadialVector exact_defect;
    RadialVector formula_defect;
    RadialDefectProfile profile;
    bool exact = false;
};

inline DefectCorrectedRadialCertificate certify_defect_corrected_radial(
    const RadialVector& x, std::int64_t period) {
    DefectCorrectedRadialCertificate c;
    c.period = period;
    c.translation = radial_translation(x, period);
    const auto stepped = nbonacci_linear_step(c.translation);
    c.exact_defect.resize(x.size());
    for (std::size_t i = 0; i < x.size(); ++i)
        c.exact_defect[i] = stepped[i] - c.translation[i];
    c.profile = radial_defect_profile(x);
    c.formula_defect.reserve(x.size());
    for (auto flux : c.profile.adjacent_flux)
        c.formula_defect.push_back(period * flux);
    c.formula_defect.push_back(period * c.profile.terminal_flux);
    c.exact = c.exact_defect == c.formula_defect;
    return c;
}

} // namespace ravel::proof
