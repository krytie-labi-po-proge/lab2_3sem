#include <iostream>
#include <string>

#include "BoolExpression.hpp"
#include "HashBenchmark.hpp"
#include "Huffman.hpp"
#include "LfuQueries.hpp"
#include "SetCli.hpp"
#include "SubarraySum.hpp"
#include "SubsetSplit.hpp"
#include "Subordinates.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Недостаточно аргументов\n";
        return 1;
    }
    std::string command = argv[1];
    if (command.rfind("--", 0) == 0) return set_cli::run_command(argc, argv);
    int subArgc = argc - 1;
    char** subArgv = argv + 1;

    if (command == "logic") return bool_expression::evaluate_stdin(subArgc, subArgv);
    if (command == "set") return set_cli::run_command(subArgc, subArgv);
    if (command == "partition") return subset_split::split_stdin(subArgc, subArgv);
    if (command == "subarrays") return subarray_sum::find_stdin(subArgc, subArgv);
    if (command == "hash-bench") return hash_benchmark::run_all(subArgc, subArgv);
    if (command == "subordinates") return subordinates::count_stdin(subArgc, subArgv);
    if (command == "lfu") return lfu_queries::process_stdin(subArgc, subArgv);
    if (command == "huffman") return huffman::process_stdin(subArgc, subArgv);

    std::cerr << "Неизвестная команда: " << argv[1] << '\n';
    return 1;
}