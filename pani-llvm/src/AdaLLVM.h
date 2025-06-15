/* Parses the code and Emits IR */

#ifndef AdaLLVM_h // start of guard
#define AdaLLVM_h // if not defined

#include "string.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"

#include<iostream>

#include "./parser/AdaParser.h"
using syntax::AdaParser; // class AdaParser is encapsulated inside synatx namespace in Adaparser.h


class AdaLLVM {
public:
    // AdaLLVM() : parser = std::make_unique<AdaParser>() // b4 executing the consr body, we initialise unique_ptrs and const values in this init-list
    AdaLLVM() : parser(std::make_unique<AdaParser>()){ // init-list syntax: member(value) 
        moduleInit(); 
        setupExternFunctions();
    } // constructor inits the modules and `extern` functions

    void exec(const std::string& program){
        // takes this program as a const string

        // 1. Parses the code -> Generates the Abstract Syntax Tree
        auto ast = parser->parse(program); // We can uncomment this, given that we have the ptr to AdaParser obj
        // returns Exp* type

        // 2. Generates the AST nodes
        /* Imagine we have the ast, currently; Using the compiler
           Walk over the tree and generate the IR, pertaining to each method
           Can imagine .add, etc
        */ 
        // compile(/* ast */); // emits the AST into IR using IRBuilder
        compile(ast); // Emits the IR using the IRBuilder for the given ast

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

    /* Pointer to obj of AdaParser class */
    std::unique_ptr<AdaParser> parser; // AdaParser class inside AdaLLVM.h

    /* Poiunter pointing to Current Compiling Functio */
    llvm::Function* fn;


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

    /* Think in terms of creating a function -> 
        entrypoint(BB), 
        fn name and fn body 
        We can start updating from compile(ast)
    */

    // void compile(/* TODO: ast */){
    void compile( const Exp& ast ){
        // 1. Create the function main function [can be thoufght of as a block]
        /* define i32 @main  -> includes fnName, retType, argType, varargs*/
        // To create a fn, we need fnName and fnType describing the function
        fn = createFunction("main", 
                            llvm::FunctionType::get( /* llvm::Type* */ builder->getInt32Ty(),
                                                    /* isVarArg required? */ false )
                        ); 

        // 2. Create the function body -> DOne by CodeGen Part of the Viz
        /*
            define i32 @square( i32 %x ){
            entry: ; entry point added by the IRBuilder to point the compiler/interpreter where to start from
                %result = mul i32 %x, %x
                ret i32 result
            }
         */

        // ################################################################
        // // // Imagine for now, 42 is main's body
        //  llvm::Value* result = gen( /* ast */ ); // emits the IR and CodeGen part of pipeline

        //  // create the caster to cat "42" into i32 rewcall: ret i32 42
        // /*   Value *CreateIntCast(Value *V, Type *DestTy, bool isSigned,
        //     const Twine &Name = "") */
        //  auto i32Result = builder->CreateIntCast( result, /*  Value* */
        //                                           builder->getInt32Ty(), /* DestTy */
        //                                           true /* isIsgned? */ 
        //                                             );
                                                
        // // ret instruction
        // // ret i32 42
        // i32Result = builder->CreateRet(i32Result);
        // ################################################################

        // ############ printf checking #############
        gen( ast ); // no result required like 42, as we don't want to typecast
        // Just like cpp, llvm also treats string as a sequence of chars
        // each char is i8 i.e 8 bits or 1 byte
        // align 1 => recall: byte-aligned
        builder->CreateRet( builder->getInt32(0) );
        // ############ printf checking #############
    }

    llvm::Value* gen( const Exp& ast ){

        // handle root node of ast based on types and do this recursively
        switch(ast.type){
            /** 
             * simple numbers like 42
            */
            case ExpType::NUMBER:
                return builder->getInt32(ast.number);

            /**
             * String -> create global string as before
             */
            case ExpType::STRING:
                return builder->CreateGlobalStringPtr(ast.string);

            /**
             * Symbol
             */
            case ExpType::SYMBOL:
                return builder->getInt32(0); // we will handle this later

            /**
             * List: Example (printf "Value: %d" 42 )
             */
            case ExpType::LIST:
                // imagine (printf "Value: %d" 42) -> [ Exp(printf) Exp("Value: %d")  Exp(42) ]
                auto tag = ast.list[0]; // Imagine [ Exp(printf), type=ExpType::SYMBOL, string = "printf" ]

                // If tag is a symbol to handle printf (say) or any function structure
                if(tag.type == ExpType::SYMBOL){
                    // get the string i.e fnName
                    auto fnName = tag.string;

                    if( fnName == "printf" ){ // also called op == "printf"
                        // CreateCall logic

                        auto printfFn = module->getFunction(fnName);
                        // create args
                        std::vector<llvm::Value*> args{};
                        for(auto i=1; i<ast.list.size(); i++){
                            // iterate over [ Exp(printf) Exp("Value: %d")  Exp(42) ]
                            llvm::Value* val = gen(ast.list[i]); // recursive based on ExpType 
                            args.push_back(val);
                        }

                        // can CreateCall
                        builder->CreateCall(printfFn, args);
                    }
                }
        }    
        // Unreachable:
        return builder->getInt32(0); // temporarily return 0, if Unreachable
    }


    void setupExternFunctions(){
        /* Idea is: Creating prototype by adding to module; Just declaration NOT definition */
        /* These Extern Functions are telling the compiler, "don't worry, I am not
        defining the function here, rather just declaring"
        These are defined somewhere else (maybe in a diff file)
        Sc: When declaring the prototype `extern <functionSignature>` */

        // Just Insert in the module, this new func

        // int print( const char* format, ...) => Prototype of printf
        auto int8ptr = builder->getInt8Ty()->getPointerTo();
        llvm::FunctionType* printfType = llvm::FunctionType::get(
                                                                    /* ret type */ builder->getInt32Ty(),
                                                                    /* 1st arg type */ int8ptr,
                                                                    /* is varargs? */ true
                                                                );

        
        // add to module: Checks if already DNE, adds to module, later
        // fetch by: module->getFunction()
        module->getOrInsertFunction( "printf", printfType);
    }

    /* Checks if function is already present asdking the builder, if not creates the fn-prototype */
    llvm::Function* createFunction( const std::string &fnName, 
                                    llvm::FunctionType* fnType){
        
        /* Checks if fnName is present in the symbol table */
        fn = module->getFunction(fnName); // returns fn-pointer is presnet else nullptr
        
        if(fn == nullptr){
            fn = createFunctionProto(fnName, fnType);
        }

        /* create the entry and other basicBlock */
        createFunctionBlock(fn); // creates the funbction block and attaches to the parent block

        return fn;
    }

    llvm::Function* createFunctionProto( const std::string& fnName,
                                        llvm::FunctionType* fnType ){
    
        /* 1. create and show it to the outside world by ExternalLinkage */
        fn = llvm::Function::Create( 
                                    fnType, /*function type */
                                    llvm::Function::ExternalLinkage, /* Linkage Type */
                                    fnName, /* Name of the func */
                                    *module /* Module& */
         );

        // This is inside Verifier.h
        verifyFunction(*fn); // requires llvm/IR/Verifier.h

        return fn;
    }

    void createFunctionBlock(llvm::Function* fn){
        // create a basic block; allocates memory for this basic block
        /* Recall: 
            define i32 @main(){
            entry: // we are adding this part, this is a BB [a sequence of IR instructions]
            }
        */
        auto entry = createBB("entry", fn);

        // It's always imp to register any new block after irt gets allocated
        // so that the IRBuilder can emit the new block
        builder->SetInsertPoint(entry);
        return;
    }

    llvm::BasicBlock* createBB(const std::string blockName, llvm::Function* fn = nullptr){
        // NOTE:: fn is parent block ptr; NEdded so that we can appened this newly created block
        auto bb_ptr = llvm::BasicBlock::Create( *ctx, blockName, fn );
        return bb_ptr;
    }
};

/* End of guard for AdaLLVM header */
#endif