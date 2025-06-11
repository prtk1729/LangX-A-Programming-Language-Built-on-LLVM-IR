# Compile main and create the out.ll file
clang++ -o ada-llvm $(llvm-config --cxxflags --ldflags --system-libs --libs core) ada-llvm.cpp

# Run main and generate the out.ll
./ada-llvm

# Run the out.ll using interpreter
lli out.ll

# print result
echo $?
printf "\n"