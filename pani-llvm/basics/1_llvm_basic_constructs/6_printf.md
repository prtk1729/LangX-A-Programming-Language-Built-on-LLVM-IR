### Printing using passing a simple srting in `CodeGen`

![](../images/code_gen_printf.png)

> [!NOTE]
> - How the `builder->CreateGLobalStringPtr( my_string )` is called instead of the previous `42` i.e `builder->getInt32(42)`
> - In ther terminal, interseting things:-
>   - `@0` states the string is `global` and named `0`.
>   - [14 x i8] means `14 slots of i8 i.e 1 byte makes up this char-string`
>   - `align 1` : Means it is byte-aligned
>   - Alo, in ret: the type is i32 (This needs to be modified). WHy this happens? Inside `compile()`
>       - gen( ast ) -> Caster -> CreateRet() for `ret instr`
>       - Earlier, 42 was having i32 type, now the function body i.e string is already in correct type. We can simply return 0 for successful termination of program
>       - Just changed `builder->CreateRet( builder->getIntCast32(0) );` inside `compile()`
> ![](../images/type_cat_ret_for_str.png)