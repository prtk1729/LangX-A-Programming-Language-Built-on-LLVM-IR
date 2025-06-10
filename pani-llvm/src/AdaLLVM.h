/* Parses the code and Emits IR */

#ifndef AdaLLVM_h // start of guard
#define AdaLLVM_h // if not defined

#include "string.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"




class AdaLLVM {
public:
    AdaLLVM() { moduleInit(); } // constructor inits the modules

    void exec(const std::string& program){
        // takes this program as a const string

        // 1. Parses the code -> Generates the Abstract Syntax Tree
        // auto ast = parser->parse(program); // Can imagine this for now

        // 2. Generates the AST
        /* Imagine we have the ast, currently; Using the compiler
           Walk over the tree and generate the IR, pertaining to each method
           Can imagine .add, etc
        */ 
        // compile(ast); // emits the AST into IR using IRBuilder

        // While executation let's print the output into the llvm outstream
        module->print( llvm::outs(), nullptr ); // raw_fd_ostream is outs(), AssemblyAnnotatorWriter is nullptr

        // 3. saves the genertaed IR to .ll file
        saveModuleToFile("./out.ll");
    }


private:
    /* Some imp. attributes */

    /* Recall The Context 
    - LLVMContext manages and owns the "global" data of LLVM's core
    infra like:-
        -  constant unique tables and types
    Container of other methods
    */
    std::unique_ptr< llvm::LLVMContext > ctx;

    /* Recall the Module */
    /* container of functions, methods and global vars etc */
    std::unique_ptr< llvm::Module > module;

    /* IRBuilder */
    /* It creates the LLVM instructions and puts them into the BasicBlock
     */
    std::unique_ptr< llvm::IRBuilder<> > builder;

private:
    void saveModuleToFile(const std::string& filename){
        // 1. Need error code to hold the error, if file can't be open or writtent o
        std::error_code errorCode;

        // 2. Using this, we can now write into a file (using file descriptor)
        // create a llvm's fd_ostream object that demands filename and errorCode
        llvm::raw_fd_ostream outLL(filename, errorCode); // using llvm's fd_ostream <fd stream>

        // 3. print using module pointer
        module->print(outLL, nullptr); // tries to wrirte into out.ll file and tracks the errors
    }   

    void moduleInit(){
        // 1. init the ctx
        // Create a memory of context in the heap and manage memory leaks by
        // wrapping it by a smart pointer
        ctx = std::make_unique<llvm::LLVMContext>();

        // 2 Using ctx init the module (create the module). Let's call it as AdaLLVM
        // create a blueprint called module and name it as "AdaLLVM"
        // Uses ctx pointer to overwrite this module
        module = std::make_unique<llvm::Module>("AdaLLVM", *ctx); // Trap: This also requires LLVMContext&

        // 3. Using ctx ptr create the builder
        // IRBuilder<>::IRBuilder(LLVMContext&) hence send a ptr
        builder = std::make_unique<llvm::IRBuilder<>>(*ctx);
    }
};

/* End of guard for AdaLLVM header */
#endif