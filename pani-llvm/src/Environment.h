
/**
 * Environment.h
 */

// Need to make guard -> header guard
// Multiple files including this -> multiple definitions -> confuses compiler
/**
 * (A.h has included B.h) and (B.h has included A.h) -> circular inclusion
 * Use ifndef, define, endif
 *  - This will included defn of B.h when compiling A.h
 * - When we next compile B.h, it can see the defintions of A.h hence will skip the include
 * - At the end of the day, 
 * we include a file to get all the functions, classes, vars which is achieved even if we skip 
 * */ 

#ifndef Environment_h 
#define Environment_h

#include<sstream>
#include<memory>
#include<map>
#include "llvm/IR/Value.h"

#include "./Logger.h"

// inheriting std::enable_shared_from_this<Environment> -> will support creation of
// shared pointer to itself (i.e this class-obj)
class Environment : public std::enable_shared_from_this<Environment> {
public:
    Environment(std::map<std::string, llvm::Value*> record, 
                std::shared_ptr<Environment> parent) 
                : record_(record), parent_(parent) {} // Trap

    /**
     * define i.e setter
     * sets the varName and its value in this Environment
     */
    llvm::Value* define(const std::string &name, llvm::Value* val){
        // Every var will be called from gen( <with Env>)
            // For every new var we can define it. Reason: Env->define(name)
        record_[name] = val;
        return val;
    }

    /**
     * lookup i.e getter
     * Finds the "scope" of a given variable
     * "scope" can be thought of as shared_ptr<Environment> i.e ptr to the Environment it belongs to
     * lookup solves this by asking this question recursively and travsersing a chain of parents
     * It retuens the value from the env it finds it for first time
     */
    llvm::Value* lookup(const std::string &name){ 
        auto env = resolve(name); // ptr
        return env->record_[name]; // ptr wrapping this class containing this map DS with key name
    }

    /**
     * resolve
     */
    std::shared_ptr<Environment> resolve(const std::string &name){
        // if it finds it here for the first time
        // At every recurssion, we will be present in the cur Env
        // We can access members by shared_ptr as it inherits `std::enable_shared_from_this<Environment>`
        if( record_.count(name) != 0 ){
            // return this; // won't work but intend to do this
            return shared_from_this();
        }

        // If we have reahed topmost parent i.e whose parent is nullptr
        if( parent_ == nullptr )
        {
            // This var was in NO env -> cerr
            // write-and-exit concept
            DIE << "Couldn't Find Such a Variable with name: " << name;
        }

        // keep recursing
        return parent_->resolve(name); // update the Env i.e parent-Env and call its resolve method
    }

private:
    /**
     * parent: Which will be a shared ptr
     * As multiple Env can have the same parent-Env
     */
    std::shared_ptr<Environment> parent_; // inside memory.h

    /**
     * record_
     * - Stores all the varNames, fnNames etc and other SYYMBOLS
     */
    std::map<std::string, llvm::Value*> record_;
};

#endif