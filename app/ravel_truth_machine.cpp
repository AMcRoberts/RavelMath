#include <iostream>
#include <string>
#include "ravel/proof/theorem_capability_machine.hpp"

int main(int argc, char** argv) {
    std::string index_path = "config/theorem_capabilities.tsv";
    std::string question;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg.rfind("--index=", 0) == 0) index_path = arg.substr(8);
        else if (arg.rfind("--question=", 0) == 0) question = arg.substr(11);
        else if (!question.empty()) question += " " + arg;
        else question = arg;
    }
    if (question.empty()) {
        std::getline(std::cin, question);
    }
    if (question.empty()) {
        std::cerr << "usage: ravel_truth_machine --question='question' [--index=path]\n";
        return 2;
    }
    try {
        auto index = ravel::proof::truth::CapabilityIndex::load_tsv(index_path);
        ravel::proof::truth::TruthMachine machine(std::move(index));
        std::cout << "indexed theorem capabilities: " << machine.capability_count() << "\n\n";
        std::cout << machine.ask(question).render();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "truth machine failure: " << ex.what() << "\n";
        return 1;
    }
}
