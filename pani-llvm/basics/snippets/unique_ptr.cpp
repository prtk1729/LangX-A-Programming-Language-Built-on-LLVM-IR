#include<memory>
#include<iostream>

using namespace std;

/* Idea:-
    - Smart Pointer need:- Prevent memory leaks
        - Does this automatically
        - Raw pointers 
            - ClassName* p = new ClassName();
            - free(p); // Need to do this.... But, using mart ptr, this happens automatically

    - Need a single ptr pointing to that obj, not multiple pointers
    - Can't copy; Only move
        - ClassName *b = p; // should be avoided scenarios
*/

struct ClassName {
    void hello() { std::cout << "Hi\n"; }
};

int main(){
    // ClassName* a = new ClassName(); 
    // ClassName* b = a; // works
    // free(a);
    // free(b);


    // unique pointer
    auto a = std::make_unique<ClassName>(); // best way to create unique_pointer
    a->hello();    // a is now a ptr


    return 0;
}

