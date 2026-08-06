#include <iostream>
#include <map>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/proof/coefficient_profile_renewal_twist.hpp"
#include "ravel/proof/generalized_multinacci_boundary_word_lift.hpp"
#include "ravel/simple_parry_profile.hpp"

using namespace ravel;

struct ProbeResult {
    std::size_t boundary_states = 0;
    std::size_t edges = 0;
    std::map<std::string,std::size_t> words;
};

template <std::size_t D>
ProbeResult probe(const std::vector<std::size_t>& digits) {
    if (digits.size() != D) throw std::runtime_error("dimension mismatch");
    SubstitutionRule rule(simple_parry_profile_rule(digits));
    const double beta = simple_parry_profile_beta(digits);
    const auto subst = make_substitution<D>(rule, beta);
    const auto cands = search_D_cont<D>(subst, 2);
    std::vector<std::tuple<long long,std::vector<long long>,long long>> dcont;
    for (const auto& c : cands)
        dcont.emplace_back(c.i, std::vector<long long>(c.x.begin(),c.x.end()), c.j);
    const auto report = compute_contact_boundary<D>(rule,beta,0.0,dcont);
    std::size_t max_digit = 0;
    for (auto x : digits) max_digit = std::max(max_digit,x);
    const auto lift = ravel::proof::derive_generalized_multinacci_boundary_word_lift<D>(
        subst, report, max_digit);
    if (!lift.proved)
        throw std::runtime_error("boundary word lift failed: " + lift.obstruction);
    ProbeResult result;
    result.boundary_states = report.boundary_nodes.size();
    result.edges = lift.edges.size();
    result.words = lift.word_multiplicity;
    for (const auto& [word,count] : result.words) {
        if (word.empty() || (word != "Q" && word.find_first_not_of('R') != std::string::npos))
            throw std::runtime_error("new primitive word appeared: " + word);
        (void)count;
    }
    return result;
}

template <std::size_t D>
void check_dimension() {
    std::vector<std::size_t> nbonacci(D,1);
    std::vector<std::size_t> left = nbonacci;
    left[D-2] = 0; // smallest legal outward move: delete one internal zero-prefix.
    std::vector<std::size_t> middle(D,1);
    for (std::size_t i=0;i<(D-1)/2;++i) middle[i]=2;
    std::vector<std::size_t> perry(D,2);
    perry.back()=1;
    std::vector<std::size_t> right = perry;
    right[0]=3; // smallest monotone outward move beyond uniform m=2.

    const auto left_twist = ravel::proof::derive_coefficient_profile_renewal_twist(nbonacci,left);
    const auto right_twist = ravel::proof::derive_coefficient_profile_renewal_twist(perry,right);
    if (!left_twist.proved || left_twist.direction != ravel::proof::ProfileTwistDirection::phase_deletion)
        throw std::runtime_error("left twist classification failed");
    if (!right_twist.proved || right_twist.direction != ravel::proof::ProfileTwistDirection::phase_insertion)
        throw std::runtime_error("right twist classification failed");

    const auto L = probe<D>(left);
    const auto N = probe<D>(nbonacci);
    const auto M = probe<D>(middle);
    const auto P = probe<D>(perry);
    const auto R = probe<D>(right);
    auto show=[&](const char* name,const std::vector<std::size_t>& profile,const ProbeResult& x){
        std::cout<<"D="<<D<<" "<<name<<" profile=";
        for(auto d:profile) std::cout<<d;
        std::cout<<" states="<<x.boundary_states<<" edges="<<x.edges<<" words=";
        for(const auto& [w,c]:x.words) std::cout<<w<<":"<<c<<",";
        std::cout<<"\n";
    };
    show("left-outside",left,L);
    show("nbonacci",nbonacci,N);
    show("middle",middle,M);
    show("perry",perry,P);
    show("right-outside",right,R);
}

int main() {
    check_dimension<3>();
    check_dimension<4>();
    check_dimension<5>();
    std::cout << "coefficient profile renewal twist PASS\n";
}
