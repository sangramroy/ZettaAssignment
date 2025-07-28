#include "query5.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <map>
#include <chrono>
#include <iomanip>

// Function to parse command line arguments
bool parseArgs(int argc, char* argv[], std::string& r_name, std::string& start_date, std::string& end_date, int& num_threads, std::string& table_path, std::string& result_path) {
    // TODO: Implement command line argument parsing
    // Example: --r_name ASIA --start_date 1994-01-01 --end_date 1995-01-01 --threads 4 --table_path /path/to/tables --result_path /path/to/results
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--r_name" && i + 1 < argc) {
            r_name = argv[++i];
        } else if (arg == "--start_date" && i + 1 < argc) {
            start_date = argv[++i];
        } else if (arg == "--end_date" && i + 1 < argc) {
            end_date = argv[++i];
        } else if (arg == "--threads" && i + 1 < argc) {
            num_threads = std::stoi(argv[++i]);
        } else if (arg == "--table_path" && i + 1 < argc) {
            table_path = argv[++i];
        } else if (arg == "--result_path" && i + 1 < argc) {
            result_path = argv[++i];
        } else {
            std::cerr << "Unknown or incomplete argument: " << arg << std::endl;
            return false;
        }
    }

    if (r_name.empty() || start_date.empty() || end_date.empty() ||
        table_path.empty() || result_path.empty() || num_threads <= 0) {
        std::cerr << "Missing required arguments." << std::endl;
        return false;
    }

    return true;
}


// Function to read TPCH data from the specified paths
bool readTPCHData(const std::string& table_path, std::vector<std::map<std::string, std::string>>& customer_data, std::vector<std::map<std::string, std::string>>& orders_data, std::vector<std::map<std::string, std::string>>& lineitem_data, std::vector<std::map<std::string, std::string>>& supplier_data, std::vector<std::map<std::string, std::string>>& nation_data, std::vector<std::map<std::string, std::string>>& region_data) {
    // TODO: Implement reading TPCH data from files
    auto readFile = [](const std::string& file_path,
                       const std::vector<std::string>& columns,
                       std::vector<std::map<std::string, std::string>>& data) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "Could not open file: " << file_path << std::endl;
            return false;
        }
        std::string line;
        while (std::getline(file, line)) {
            std::map<std::string, std::string> row;
            std::stringstream ss(line);
            std::string field;
            size_t idx = 0;
            while (std::getline(ss, field, '|') && idx < columns.size()) {
                row[columns[idx++]] = field;
            }
            data.push_back(row);
        }
        return true;
    };

    bool ok = true;
    ok &= readFile(table_path + "/customer.tbl",
                   {"C_CUSTKEY", "C_NAME", "C_ADDRESS", "C_NATIONKEY",
                    "C_PHONE", "C_ACCTBAL", "C_MKTSEGMENT", "C_COMMENT"},
                   customer_data);
    ok &= readFile(table_path + "/orders.tbl",
                   {"O_ORDERKEY", "O_CUSTKEY", "O_ORDERSTATUS",
                    "O_TOTALPRICE", "O_ORDERDATE", "O_ORDERPRIORITY",
                    "O_CLERK", "O_SHIPPRIORITY", "O_COMMENT"},
                   orders_data);
    ok &= readFile(table_path + "/lineitem.tbl",
                   {"L_ORDERKEY", "L_PARTKEY", "L_SUPPKEY",
                    "L_LINENUMBER", "L_QUANTITY", "L_EXTENDEDPRICE",
                    "L_DISCOUNT", "L_TAX", "L_RETURNFLAG",
                    "L_LINESTATUS", "L_SHIPDATE", "L_COMMITDATE",
                    "L_RECEIPTDATE", "L_SHIPINSTRUCT",
                    "L_SHIPMODE", "L_COMMENT"},
                   lineitem_data);
    ok &= readFile(table_path + "/supplier.tbl",
                   {"S_SUPPKEY", "S_NAME", "S_ADDRESS",
                    "S_NATIONKEY", "S_PHONE", "S_ACCTBAL", "S_COMMENT"},
                   supplier_data);
    ok &= readFile(table_path + "/nation.tbl",
                   {"N_NATIONKEY", "N_NAME", "N_REGIONKEY"},
                   nation_data);
    ok &= readFile(table_path + "/region.tbl",
                   {"R_REGIONKEY", "R_NAME", "R_COMMENT"},
                   region_data);

    std::cout << "Customer rows: " << customer_data.size() << std::endl;
    std::cout << "Orders rows: " << orders_data.size() << std::endl;
    std::cout << "Lineitem rows: " << lineitem_data.size() << std::endl;

    return ok;
}

// Function to execute TPCH Query 5 using multithreading
bool executeQuery5(
    const std::string& r_name,
    const std::string& start_date,
    const std::string& end_date,
    int num_threads,
    const std::vector<std::map<std::string, std::string>>& customer_data,
    const std::vector<std::map<std::string, std::string>>& orders_data,
    const std::vector<std::map<std::string, std::string>>& lineitem_data,
    const std::vector<std::map<std::string, std::string>>& supplier_data,
    const std::vector<std::map<std::string, std::string>>& nation_data,
    const std::vector<std::map<std::string, std::string>>& region_data,
    std::map<std::string, double>& results
) {
    std::mutex results_mutex;

    // Pre-join lookup maps
    std::unordered_map<std::string, std::map<std::string, std::string>> supplier_map;
    for (const auto& s : supplier_data) {
        supplier_map[s.at("S_SUPPKEY")] = s;
    }

    std::unordered_map<std::string, std::map<std::string, std::string>> nation_map;
    for (const auto& n : nation_data) {
        nation_map[n.at("N_NATIONKEY")] = n;
    }

    std::unordered_map<std::string, std::map<std::string, std::string>> region_map;
    for (const auto& r : region_data) {
        region_map[r.at("R_REGIONKEY")] = r;
    }

    size_t total = lineitem_data.size();
    size_t chunk_size = (total + num_threads - 1) / num_threads;

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = std::min(total, start + chunk_size);
        if (start >= end) continue;

        threads.emplace_back([&, start, end]() {
            std::map<std::string, double> local_results;

            for (size_t idx = start; idx < end; ++idx) {
                const auto& line = lineitem_data[idx];

                auto it_ship = line.find("L_SHIPDATE");
                if (it_ship == line.end()) continue;
                std::string ship_date = it_ship->second;
                if (ship_date < start_date || ship_date > end_date) continue;

                auto it_supp = line.find("L_SUPPKEY");
                if (it_supp == line.end()) continue;
                std::string suppkey = it_supp->second;

                auto supplier_it = supplier_map.find(suppkey);
                if (supplier_it == supplier_map.end()) continue;

                std::string nation_key = supplier_it->second.at("S_NATIONKEY");
                auto nation_it = nation_map.find(nation_key);
                if (nation_it == nation_map.end()) continue;

                std::string region_key = nation_it->second.at("N_REGIONKEY");
                auto region_it = region_map.find(region_key);
                if (region_it == region_map.end()) continue;

                if (region_it->second.at("R_NAME") != r_name) continue;

                std::string nation_name = nation_it->second.at("N_NAME");

                try {
                    double extended_price = std::stod(line.at("L_EXTENDEDPRICE"));
                    double discount = std::stod(line.at("L_DISCOUNT"));
                    double revenue = extended_price * (1.0 - discount);
                    local_results[nation_name] += revenue;
                } catch (...) {
                    continue;
                }
            }

            std::lock_guard<std::mutex> lock(results_mutex);
            for (const auto& [nation, revenue] : local_results) {
                results[nation] += revenue;
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    return true;
}


// Function to output results to the specified path
bool outputResults(const std::string& result_path, const std::map<std::string, double>& results) {
    // TODO: Implement outputting results to a file
    std::ofstream out(result_path);
    if (!out.is_open()) {
        std::cerr << "Could not open result file: " << result_path << std::endl;
        return false;
    }

    for (const auto& [nation, revenue] : results) {
        out << nation << "|" << revenue << std::endl;
    }

    out.close();
    return true;
}
