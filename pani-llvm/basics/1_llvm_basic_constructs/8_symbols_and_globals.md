#### Symbols

- Inside gen(), for `Symbols`
```cpp
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
```
- We ask the `ast.string`, for if "true" or "false"
- Accordingly, set the value using `builder->getInt1(...)`
- Int1 as `Intx` here x is num_bits to store variable, bool is 1 bit
- The raw-string here to test is present in the multi-line comment
![](../images/symbols_bool.png)
- Here, in the result, we canm see LLVM understands `true and false` constants and prints `i1 true`


#### Global Variables
- Since R"" for global checking is `(var VERSION 42)`, this will go as a list
- So, we update inside `list`
```cpp
if(tag.type == ExpType::SYMBOL){
                    // get the string i.e fnName or "var"
                    auto op = tag.string;

                    if( op == "var" ){
                        /**
                         * Variables
                         * 1. Local (TODO)
                         * 2. Global
                         *      - R" ( (var VERSION 42) ) " 
                         *      - var: tag
                         *      - VERSION: name of global var i.e varName
                         *      - 42:  init_value
                         */
                        std::string varName = ast.list[1].string; // Trap: ast.list[1] will have Exp("VERSION"), we need to get the string
                        // Initialiser
                        llvm::Value* init_value = gen(ast.list[2]); // ast.list[2] has Exp(42), which is expected for gen
                        // sets properties and of this global variable, just like createFunction
                        llvm::GlobalVariable* g = createGlobalVariable(varName, 
                                                                        (llvm::Constant*)init_value);
                        return g;
                    }
```

##### createGlobalVariable()
```cpp
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
```


##### Final O/p
![](../images/global_var.png)
> [!NOTE]
> - @VERSION means `global tag`
> - ALso, it says it is `global` 
> - It is i32 type with value 42
> - Has align 4 i.e `varFn->setAlignment(llvm::MaybeAlign(4))`