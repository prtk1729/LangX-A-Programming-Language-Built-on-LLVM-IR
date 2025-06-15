#!/bin/bash

# STEP 1: Compile the C++ file with LLVM flags and EXCEPTIONS ENABLED
clang++ -std=c++17 -fexceptions -o ada-llvm \
    $(llvm-config --cxxflags --ldflags --system-libs --libs core) \
    ada-llvm.cpp

# STEP 2: Run the compiled executable to generate 'out.ll'
./ada-llvm

# STEP 3: Execute the LLVM IR code using LLVM's interpreter
lli out.ll

# STEP 4: Print the return code of the LLVM program
echo "Program returned: $?"
