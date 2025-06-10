/* Executable 
- Takes the program, as input
- Inits the compiler
- Generates the IR
*/

#include<string>
#include "./src/AdaLLVM.h"

int main(){ 

    const std::string program = R"( 42 
    )";

    // init the compiler
    AdaLLVM vm; 

    // Execute
    vm.exec(program);

    return 0;
}