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
#include "Environment.h"

using syntax::AdaParser; // class AdaParser is encapsulated inside synatx namespace in Adaparser.h

// Generic Operation
/* ast and env will be found where this gets unpacked as it is by preprocessor
* do while creates a scope, cool cpp trick
* (+ x 21) -> (x + 21): opd1 is Exp(x), opd2: Exp(21) -> SInce, Exp() -> We pass through gen
*/
// / Generic binary operator
// Trap: preprocessor -> can't put xomments within
#define GENERIC_BINARY_OPERATION(Op, varName) \
    do{ \
        auto opd1 = gen(ast.list[1], env);  \
        auto opd2 = gen(ast.list[2], env);  \
        return builder->Op(opd1, opd2, varName); \
    }while(false)  

class AdaLLVM {
public:
    // AdaLLVM() : parser = std::make_unique<AdaParser>() // b4 executing the consr body, we initialise unique_ptrs and const values in this init-list
    AdaLLVM() : parser(std::make_unique<AdaParser>()){ // init-list syntax: member(value) 
        moduleInit(); 
        setupExternFunctions();
        setupGlobalVariables();
    } // constructor inits the modules and `extern` functions

    void exec(const std::string& program){
        // takes this program as a const string

        // 1. Parses the code -> Generates the Abstract Syntax Tree
        auto ast = parser->parse(program);
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
    /**
     * varsBuilder
     * To insert local vars at the beginning of an entryBlock of a function
     */
    std::unique_ptr< llvm::IRBuilder<> > varsBuilder;

    /* Pointer to obj of AdaParser class */
    std::unique_ptr<AdaParser> parser; // AdaParser class inside AdaLLVM.h

    /* Poiunter pointing to Current Compiling Functio */
    llvm::Function* fn;

    /* Shared Ptr to this root-Env */
    std::shared_ptr<Environment> GlobalEnv;


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

        varsBuilder = std::make_unique<llvm::IRBuilder<>> (*ctx);
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
                                                    /* isVarArg required? */ false ),
                            GlobalEnv     // add this "main" which is a fnName i.e SYMBOL inside this global-env
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

        // ################################################################
        // INIT GLOBAL
        std::string varName = "VERSION";
        llvm::Constant* init_value = builder->getInt32(42);
        createGlobalVariable(varName, init_value); // creates @VERSION = global i32 42, align 4

        // ################################################################


        // ############ printf checking #############
        gen( ast, GlobalEnv ); // no result required like 42, as we don't want to typecast
        // Just like cpp, llvm also treats string as a sequence of chars
        // each char is i8 i.e 8 bits or 1 byte
        // align 1 => recall: byte-aligned
        builder->CreateRet( builder->getInt32(0) );
        // ############ printf checking #############
    }

    llvm::Value* gen( const Exp& ast, std::shared_ptr<Environment> env ){

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
                /**
                 * Boolean
                 * R "(
                 *      (printf "Value: %d\n\n" true) 
                 *  )"
                 */
                if( (ast.string == "true") or (ast.string == "false") ){
                    auto value = ((ast.string == "true") ? true : false);
                    return builder->getInt1(value);
                }
                else{
                    /**
                     * Global Vars
                     */
                    // (var VERSION 42)
                    // assume we have created the global var named VERSION
                    // stored inside module
                    // ast.string will contain the name
                    auto varName = ast.string;
                    auto value = env->lookup(varName);

                    /* Using the following logic, we can know if this symbol is a GlobalVar or AllocaInst i.e local */
                    // once we get the value, we ask if this is of type GlobalVariable
                    auto globalVar = llvm::dyn_cast<llvm::GlobalVariable>(value); // if value is of type GlobaVariable* get the add, else nullptr
                    // SImilarly, if localVar is of type AllocaInstr -> local
                    auto localVar = llvm::dyn_cast<llvm::AllocaInst> (value); // nullptr if not stack-var
                    
                    if( globalVar ){ // 1. if global
                        // not nullptr
                        // Emit LLVM's Load Instr
                        return builder->CreateLoad(
                                            globalVar->getInitializer()->getType(), /* expects type */
                                            globalVar, /* expects llvm::Value* */
                                            varName.c_str() /* Twine& Name = "" */
                                        );
                    }

                    else{ // 2. if local
                        return builder->CreateLoad( localVar->getAllocatedType(), /* llvm::AllocaInst* */ 
                                                localVar, 
                                                varName.c_str() );
                    }
                    // // module->getNamedGlobal(varName); // returns the llvm::GlobalVariable*
                }
            /**
             * List: Example (printf "Value: %d" 42 )
             */
            case ExpType::LIST:
                // imagine (printf "Value: %d" 42) -> [ Exp(printf) Exp("Value: %d")  Exp(42) ]
                auto tag = ast.list[0]; // Imagine [ Exp(printf), type=ExpType::SYMBOL, string = "printf" ]

                // If tag is a symbol to handle printf (say) or any function structure or global/local var
                if(tag.type == ExpType::SYMBOL){
                    // get the string i.e fnName or "var"
                    auto op = tag.string;

                    /* =========== BINARY OPERATORS ==================== */
                    if( op == "+" ){
                        // creates the IR: add, y the builder and returns the Value*
                        GENERIC_BINARY_OPERATION(CreateAdd, "tempadd");
                    }
                    else if( op == "-" ){
                        GENERIC_BINARY_OPERATION(CreateSub, "tempsub");
                    }
                    else if( op == "*" ){
                        GENERIC_BINARY_OPERATION(CreateMul, "tempmul");
                    }
                    else if( op == "/" ){ 
                        GENERIC_BINARY_OPERATION(CreateSDiv, "tempdiv");
                    }

                    // cmp operators
                    else if(op == "=="){
                        GENERIC_BINARY_OPERATION(CreateICmpEQ, "tempcmp");
                    }
                    else if(op == ">="){
                        GENERIC_BINARY_OPERATION(CreateICmpUGE, "tempcmp");
                    }
                    else if(op == "<="){
                        GENERIC_BINARY_OPERATION(CreateICmpULE, "tempcmp");
                    }
                    else if(op == ">"){
                        GENERIC_BINARY_OPERATION(CreateICmpUGT, "tempcmp");
                    }
                    else if(op == "<"){
                        GENERIC_BINARY_OPERATION(CreateICmpULT, "tempcmp");
                    }
                    else if(op == "!="){
                        GENERIC_BINARY_OPERATION(CreateICmpNE, "tempcmp");
                    }
                    /* =========== BINARY OPERATORS ==================== */


                    /*========== Control Flow =========================== */
                    else if( op == "if" ){
                        /* Idea

                            - R string
                                (begin
                                        (var x 40)
                                        (if (>= x 40)
                                            1
                                            3
                                        )
                                        (printf "%d " x)
                                    )
                            - <if> <cond> <then> <else>
                            - Compile the condition and generate IR for that
                            - We need to create the basicBlocks
                                - ifBlock, elseBlock, ifEndBlock 
                        */

                        // Compile the <cond>: (>= x 40)
                        // The following compilation emits:- if (>= x 40)
                            // %cond = icmp i1 sge i32 40
                        auto cond = gen(ast.list[1], env);

                        // create blocks
                        // creates label "then", "else", 
                        auto thenBlock = createBB("then", fn);
                        auto elseBlock = createBB("else"); // Let's not attach right-away to parent
                        auto ifEndBlock = createBB("ifend"); // Wont attach to parent. As we wont be emitting code here, right-away
                        
                        // Conditional Branch
                        // create cond-branching instruction i.e control-flow split inst
                        // split control-flow: if <cond> <then> <else> : This control flow is created
                        // br i1 %cond, label thenBlock, label elseBlock    -> This is emitted
                        builder->CreateCondBr(cond, thenBlock, elseBlock); 

                        // Then branch
                        // Bring pen(Builder) to location where we want to emit
                        builder->SetInsertPoint(thenBlock); // here, I want to wmit IR
                        // resolve and compile the IR, of then i.e thenRes
                        // (if <cond> <then>) -> list[2]
                        // emits IR: then here is return 1 and goto ifEndBlock
                        // br 
                        auto thenRes = gen(ast.list[2], env); // compile: <then>
                        // uncond goto ifend with the ret value
                        // br label %ifend
                        builder->CreateBr(ifEndBlock);
                        // restore then-block for phi instruction
                        thenBlock = builder->GetInsertBlock();

                        // Else Branch
                        // We need to now, attach it to the basic block list of fn
                            // Why? Needed for phi-node to recognise this and also to restore elseBlock
                        fn->getBasicBlockList().push_back(elseBlock); // attaches elseBlock to parent's func-ptr
                        // Bring IRBuilder to the state: <ElseBlock, IR-instr>
                        builder->SetInsertPoint(elseBlock);
                        // Compile: <else> -> ( if <cond> <then> <else> )
                        auto elseRes = gen(ast.list[3], env);
                        // create uncond br to ifend
                        builder->CreateBr(ifEndBlock);
                        // restore for phi instruction, 
                        // after we have added the IR within a block, there might be a chance we have changed the address
                        elseBlock = builder->GetInsertBlock();

                        // ifEnd Branch: Where we merge the control-flow
                        // attach to parent
                        fn->getBasicBlockList().push_back(ifEndBlock);
                        // Needs to know based on from it came from, which value to return
                        // Needs phi instruction for control-flow merge instr
                        builder->SetInsertPoint(ifEndBlock);
                        // create PHI node
                        auto phi = builder->CreatePHI( builder->getInt32Ty(), /* llvm::Type* */ 
                                            2, 
                                            "tmpif"
                                          );
                        // merge them, we need to have the thenBlock and elseBlock
                        // pushed to BasicBlocksList() prior to adding the incoming of phi node
                            // This we have already donw, when sending the parent-fn createBB()
                        phi->addIncoming(thenRes, thenBlock); // llvm::Value*, llvm::BasicBlock*
                        phi->addIncoming(elseRes, elseBlock); // llvm::Value*, llvm::BasicBlock*
                        
                        // Above 3 will create:
                            // ifend:
                               // phi i32 [1, %thenBlock], [3, %elseBlock]
                        return phi;
                    }

                    // else if( op == "while"){

                    // }


                    /* (while <cond> <body> ) */
                    else if( op == "while" ){
                        // Here we are in label entry: i.e We are in entry-block
                        // From entryBlock, w/o any cond we move to condBlock

                        // entry to condBlock, as we want to emit/compile <cond> inside condBlock
                        /* 
                        * entry:
                        * br label %condBlock
                        */
                        auto condBlock = createBB("cond", fn);
                        builder->CreateBr(condBlock); // br label %condBlock

                        // =========== condBlock ====================
                        // Inside condBlock, we compile <cond>
                        // <cond> here: (> x 0)
                        // Based on this we goto bodyBlock or loopEndBlock
                        // Since, we need to emit this conditional Branch IR, we need to create these 2 blocks
                        // cond -> T -> body
                        // cond -> F -> loopend ; We need to creat this condbranch
                        auto bodyBlock = createBB("body");
                        auto loopEndBlock = createBB("loopend");

                        // compile <cond> : (> x 0) 
                        // still in entryBlock -> SetInsertPoint to condBlock
                        builder->SetInsertPoint(condBlock); // Trap
                        // %cmp = icmp i1 ugt i32 %x, 0
                        auto condRes = gen(ast.list[1], env);

                        // conditional br
                        // br i1 %cmp, label %body, label %loopend
                        builder->CreateCondBr(condRes, bodyBlock, loopEndBlock);
                        // =========== condBlock ====================



                        // =========== bodyBlock ====================
                        // register the parent in parent-fn's BasicBlockList
                        fn->getBasicBlockList().push_back(bodyBlock); 
                        // Currently inside condBlock => IRBuilder is a state-m/c
                        builder->SetInsertPoint(bodyBlock); 
                        // compile <body>
                        auto bodyRes = gen(ast.list[2], env); // printf and x -= 1 etc
                        // This is how we createt the looping structure
                            // already codBlock -> bodyBlock in CreateCondBr()
                        // Here, since body was successfully executed, we loopBack to condBlock
                            // Inside condBlock, we can run the cond-branch instr
                        builder->CreateBr(condBlock); // br label %cond from bodyBlock
                        // =========== bodyBlock ====================
                        
                        // =========== loopEndBlock ====================
                        // register the parent in parent-fn's BasicBlockList
                        fn->getBasicBlockList().push_back(loopEndBlock); 
                        // Currently inside bodyBlock => IRBuilder is a state-m/c
                        builder->SetInsertPoint(loopEndBlock); 
                        // We dont loop back, rather the next sequence of IR
                        // =========== loopEndBlock ====================

                        return builder->getInt32(0);
                    }
                    /*========== Control Flow =========================== */


                    else if( op == "var" ){
                        /**
                         * var declarartion in s-expression
                         * - (var x 42) 
                         * - (var (x number) 42 )
                         * - (var (x string) "Hello" )
                         * - Difference between global and local vars
                         *  - Can be know from the init_value's type i.e is it:-
                         *      - llvm::dyn< llvm::GlobalVariable > (value) OR
                         *      - llvm::dyn< llvm::AllocInst > (value)
                         *          - Every local var is stored in stack memory
                         *          - The ptr to it is saved and accessed using load and store of this ptr
                         *          - Hence, one way to distinguih is asking if it is AllocInst type?
                         */

                         // Trap: ast.list[1] will have Exp("VERSION"), we need to get the string
                        auto varNameDecl = ast.list[1]; // (var x 42) or (var (x number) 42) -> x or (x number)
                        auto varName = extractVarName(varNameDecl); // varName = x

                        // we need to extract the type to create the alloca instr
                        auto varType = extractVarType(varNameDecl); // llvm::Type* 

                        // Now, we have both the name and type, we can create teh bindings
                        /**
                         * First we need to insert this local var at the entryBlock of the fn it belongs to 
                         * We have a general builder that emits usual load, store, call instr
                         *  - The primary builder follows the control flow
                         * - But, the local vars, should be inserted as first instr at the entryBlock. Why?
                         *     - So, that compiler knows beforehand, how much mem to allocate in stack space for these
                         *     - Hence, the need for another builder -> varsBuilder
                         *  - We also, need to attach this local var to its env
                         */     
                        llvm::Value* varBinding = allocVars(varName, varType, env); // binds

                        // Set value
                        auto init_value = gen(ast.list[2], env); // ast.list[2] has Exp(42), which is expected for gen

                        /* store ir syntax:  store <type> <value>, <type> * <ptrName> */
                        // emits IR: store i32 42, i32* %x; Here %x is varBinding of alloca which is ptr
                        // store i32 42, i32* %x; write value "42" to the mem locn pointed by pointer %x
                        return builder->CreateStore( init_value, varBinding );
                    }

                    else if(op == "set"){
                        /* (set x 100) */
                        auto varName = ast.list[1].string;

                        // value: Needed to create Store Inst
                        auto value = gen(ast.list[2], env);

                        // Need Value* of this name
                        auto varBinding = env->lookup(varName); // returns llvm::Value* searches recursively record_[varName]

                        // emits IR instruction
                        // store i32 100, i32* %x, align 4
                        builder->CreateStore(value, varBinding);

                        return value; // expects llvm::Value*
                    }

                    else if(op == "begin"){
                        /* Here, we have to make a new Env due to this begin scope */
                        std::map<std::string, llvm::Value*> record;
                        std::shared_ptr<Environment> beginEnv = std::make_shared<Environment>(
                                                                                            record, 
                                                                                            env);

                        // (begin <expression>)
                        llvm::Value* retBlock; // parse each block and return the last block
                        for(int i=1; i<ast.list.size(); i++){
                            retBlock = gen(ast.list[i], beginEnv); //last update will be retBlock
                        }

                        return retBlock;
                    }

                    else if( op == "printf" ){ // also called op == "printf"
                        // CreateCall logic
                        
                        auto printfFn = module->getFunction(op);
                        // create args
                        std::vector<llvm::Value*> args{};
                        for(auto i=1; i<ast.list.size(); i++){
                            // iterate over [ Exp(printf) Exp("Value: %d")  Exp(42) ]
                            llvm::Value* val = gen(ast.list[i], env); // recursive based on ExpType 
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

    /**
     * extractVarName 
     * (var x 42) -> extracts x
     * (var (x number) 42 ) -> extracts x
     */
    std::string extractVarName(Exp& exp){
        // exp can be of type LIST or ow -> if list then [0]
        if( exp.type == ExpType::LIST ){
            return exp.list[0].string; // Exp( [Exp(x) Exp(number)] )
        }

        // ow usual
        return exp.string; // Exp(x)
    }

    /**
     * (var (x number) 42) -> type is number -> i32
     * (var (x string) "Hello") -> type is string -> i8*
     * (var x 42) -> default i32
     */
    llvm::Type* extractVarType(Exp& exp){
        if( exp.type == ExpType::LIST ){
            return getStringType(exp.list[1].string);
        }
        return builder->getInt32Ty();
    }

    /**
     * "number" -> i32
     * "string" -> i8*
     * default: i32
     */
    llvm::Type* getStringType(const std::string& name_){
        if (name_ == "number")
            return builder->getInt32Ty();

        else if(name_ == "string"){
            // "string" which is char* 
            return builder->getInt8Ty()->getPointerTo();
        }
        // default
        return builder->getInt32Ty();
    }

    llvm::Value* allocVars(const std::string& varName, 
                            llvm::Type* varType, 
                            std::shared_ptr<Environment> env){
        
        // varsBuilder to use so that this alloca instr is created at EntryBlock
        varsBuilder->SetInsertPoint(&fn->getEntryBlock()); // points to start of the entryBlock of cur function

                            
        // %x = alloca i32; if 0
        // %x = alloca i32, i32 5; if 5 i.e 5 cts blocks allocated in stack each of type i32
        // setInsert... by varsBuilder makes it point to entry block then the following code is emiited there
        /*
        entry:
            %x = alloca i32
        */
        auto varPtr = varsBuilder->CreateAlloca(varType, /* llvm::Type* */
                                    0,     /* single scalar value, had it been arr of 5, here 5 */
                                    varName); // pointing here, it creates this alloca instr here
        // This builder emits code: %x = alloca i32; 

        // bind with thi env
        env->define(varName, varPtr); // varPtr is expected as above returns i32* i.e llvm::Value*
        return varPtr;
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

    /**
     * sets up all the global vars
     */
    void setupGlobalVariables(){
        // first lets create a dummy variable
        std::map<std::string, llvm::Value*> globalObj;
        globalObj["VERSION"] = builder->getInt32(42);

        // We create a globalRecord and add all these variables
        // finally using this record create a class and wrap inside a share_ptr which will be 
        // globalPtr and add all the SYMBOLS to module by calling createGlobalVar

        std::map<std::string, llvm::Value*> globalRecord;
        for(auto &p: globalObj){    
            globalRecord[p.first] = createGlobalVariable( p.first, // varName
                                            (llvm::Constant*)p.second // init_value
                                     );
        }
        // finally setup this global-shared-ptr of this root-Env
        GlobalEnv = std::make_shared<Environment>( globalRecord, nullptr); // 2nd guy is parent of this Env which is null
        // learning: 
            // shared_ptr p = std::make_shared< className >( constructor_arg1, constructor_arg2 )
    }

    llvm::GlobalVariable* createGlobalVariable( const std::string& varName, 
                                                llvm::Constant* init_value) {
        // using the module setOrInsert
        module->getOrInsertGlobal(varName, init_value->getType() );
        // fetch once registered
        llvm::GlobalVariable* varFn = module->getNamedGlobal(varName); 
        // set some properties of global: isItConstant, setInitializer, setAlignment
        varFn->setAlignment(llvm::MaybeAlign(4)); // Make thgis variable 4B aligned as int
        varFn->setConstant(false); // multable i.e can be changed
        varFn->setInitializer(init_value);
        return varFn;
    }

    /* Checks if function is already present asdking the builder, if not creates the fn-prototype */
    llvm::Function* createFunction( const std::string &fnName, 
                                    llvm::FunctionType* fnType,
                                    std::shared_ptr<Environment> env){
        
        /* Checks if fnName is present in the symbol table */
        fn = module->getFunction(fnName); // returns fn-pointer is presnet else nullptr
        
        if(fn == nullptr){
            fn = createFunctionProto(fnName, fnType, env);
        }

        /* create the entry and other basicBlock */
        createFunctionBlock(fn); // creates the funbction block and attaches to the parent block

        return fn;
    }

    llvm::Function* createFunctionProto( const std::string& fnName,
                                        llvm::FunctionType* fnType,
                                        std::shared_ptr<Environment> env ){
    
        /* 1. create and show it to the outside world by ExternalLinkage */
        fn = llvm::Function::Create( 
                                    fnType, /*function type */
                                    llvm::Function::ExternalLinkage, /* Linkage Type */
                                    fnName, /* Name of the func */
                                    *module /* Module& */
         );

        env->define(fnName, fn);
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










