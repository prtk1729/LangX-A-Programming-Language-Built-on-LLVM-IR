/* Executable 
- Takes the program, as input
- Inits the compiler
- Generates the IR
*/

#include<string>
#include "./src/AdaLLVM.h"

int main(){ 

    // Blocks Concept
        /* Create a global var called VERSION
        * 
        * Create Local Scope with begin op
        * Within this create a var called VERSION
        * print this local VERSION
        * 
        * Finally, outside this local scope print the global VERSION  
        */


    // Below will fail becoz, of multiple top-level S-expression
    // Soln: Make single top-level s-expression 
    // std::string program = R"(

    //     (var VERSION 42)

    //     (begin 
    //         (var VERSION 42)
    //         (printf "Version: %d\n\n" VERSION)
    //     )

    //     (printf "Version: %d\n\n" VERSION)
    // )";


    std::string program = R"(

    (begin
        (printf "Version: %d\n\n" VERSION)
    )

    )";

    // init the compiler
    AdaLLVM vm; 

    // Execute
    vm.exec(program);

    return 0;
}