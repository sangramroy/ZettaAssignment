# TPC-H Query 5 in C++ (Single-threaded & Multi-threaded)

This project implements **TPC-H Query 5** in C++ with optional multithreading support for performance comparison.

It reads the input data generated using the [TPC-H dbgen tool](https://github.com/electrum/tpch-dbgen), executes the query logic, and writes the aggregated results to a specified output file.

------------------------------------
Dataset is generated using SF 2
------------------------------------

## 📁 Project Structure

```
ZettaProject/
├── tpch-dbgen/                      # TPCH data generator repo
│   ├── dbgen                        # Binary to generate data
│   └── tables/                      # Generated .tbl files (customer.tbl, orders.tbl, etc.)
│
├── tpch-query5/                     # This repository
│   ├── inc/
│   │   └── query5.hpp
│   ├── src/
│   │   ├── main.cpp
│   │   └── query5.cpp
│   ├── build/                       # Created after building (not tracked in Git)
│   │   └── tpch_query5.exe
│   ├── CMakeLists.txt
│
├── Results/
│   ├── Single_Thread/
│   │   ├── results1.txt
│   │   └── singlethreadscreenshot_runtime.jpg
│   └── Multi_Thread/
│       ├── results4.txt
│       └── resultmultithread_screenshot_runtime.jpg
```

---

## Build Instructions

Make sure CMake and a C++ compiler (with C++11 or higher support) are installed.

# Navigate to the source directory
cd tpch-query5

# Create build directory and compile
mkdir build
cd build
cmake ..
cmake --build .
```

After build, you'll find the executable at:
`tpch-query5/build/tpch_query5.exe`



## 🚀 How to Run

### 🔹 Single-threaded Execution

```bash
./tpch_query5.exe \
  --r_name ASIA \
  --start_date 1994-01-01 \
  --end_date 1995-01-01 \
  --threads 1 \
  --table_path /c/Users/Admin/Desktop/Whitefield/ZettaAssignment/ZettaProject/tpch-dbgen/tables \
  --result_path /c/Users/Admin/Desktop/Whitefield/ZettaAssignment/ZettaProject/Results/Single_Thread/results1.txt
```

### 🔹 Multi-threaded Execution

```bash
./tpch_query5.exe \
  --r_name ASIA \
  --start_date 1994-01-01 \
  --end_date 1995-01-01 \
  --threads 4 \
  --table_path /c/Users/Admin/Desktop/Whitefield/ZettaAssignment/ZettaProject/tpch-dbgen/tables \
  --result_path /c/Users/Admin/Desktop/Whitefield/ZettaAssignment/ZettaProject/Results/Multi_Thread/results4.txt
```



