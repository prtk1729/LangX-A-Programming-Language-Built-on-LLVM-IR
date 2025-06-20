/* logger.h */

/**
 * write and exit concept
 * We want to support functionalities of a out-stream
 * That stores the Error Message as a string into buffer
 * Suports functionailities of a stream like cin , cout, cerr
 * And Exits/Terminates the Program due to an error
 */

#ifndef Logger_h
#define Logger_h

#include <sstream>
#include <iostream>

class ErrorLogMessage : public std::basic_ostringstream<char> {
public:
    ~ErrorLogMessage(){ 
        // before getting out-of-scope
        /* It Stores the msg and exits */
        // NOTE:: `DIE <<` has put the message n buffer
        // str() will fetch the contents of the Buffer and c_str() converts to c-readable string
        std::cerr << "Fatal Error: " << str().c_str();
        exit(EXIT_FAILURE);
    }
};


#define DIE ErrorLogMessage()
#endif