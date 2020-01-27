#ifndef HASHWRAPPER_H
#define HASHWRAPPER_H

//----------------------------------------------------------------------
//STL includes
#include <string>

//----------------------------------------------------------------------
//C includes
//#include <stdio.h>
#include <fstream>
#include <iostream>

//----------------------------------------------------------------------
//hashlib++ includes
#include "hl_exception.h"

//----------------------------------------------------------------------

/**
 *  @brief 	This class represents the baseclass for all subwrappers
 *
 *  hashwrapper is the abstract base class of all subwrappers like md5wrapper
 *  or sha1wrapper. This class implements two simple and easy to use
 *  memberfunctions to create a hash based on a string or a file.
 *  ( getHashFromString() and getHashFromFile() )
 *
 *  getHashFromString() calls resetContext(), updateContext() and hashIt()
 *  in this order. These three memberfunctions are pure virtual and have to
 *  be implemented by the subclasses.
 *
 *  getHashFromFile() calls resetContext() before reading the specified file
 *  in 1024 byte blocks which are forwarded to the hash context by calling
 *  updateContext(). Finaly hashIt() is called to return the hash.
 */
class hashwrapper {
private:
    const std::string teststring;

protected:

    /**
     *  @brief 	This method finalizes the hash process
     *  		and returns the hash as std::string
     *
     *  		This memberfunction is pure virtual and
     *  		has to be implemented by the subclass
     *
     *  @return 	the created hash as std::string
     */
    virtual std::string hashIt(void) = 0;

    /**
     *  @brief 	This internal member-function
     *  		convertes the hash-data to a
     *  		std::string (HEX)
     *
     *  		This memberfunction is pure virtual and
     *  		has to be implemented by the subclass
     *
     *  @param 	data The hash-data to covert into HEX
     *  @return	The converted data as std::string
     */
    virtual std::string convToString(unsigned char *data) = 0;

    /**
     *  @brief 	This method adds the given data to the
     *  		current hash context
     *
     *  		This memberfunction is pure virtual and
     *  		has to be implemented by the subclass
     *
     *  @param 	data The data to add to the current context
     *  @param 	len The length of the data to add
     */
    virtual void updateContext(unsigned char *data, unsigned int len) = 0;

    /**
     *  @brief 	This method resets the current hash context.
     *  		In other words: It starts a new hash process.
     *
     *  		This memberfunction is pure virtual and
     *  		has to be implemented by the subclass
     */
    virtual void resetContext(void) = 0;


    /**
     * @brief 	This method should return the hash of the
     * 		test-string "The quick brown fox jumps over the lazy
     * 		dog"
     */
    virtual std::string getTestHash(void) = 0;

public:

    /**
     * @brief Default Konstruktor
     */
    hashwrapper(void)
            : teststring("The quick brown fox jumps over the lazy dog") {
    }

    /**
     *  @brief 	Default destructor
     */
    virtual ~hashwrapper(void) {};

    /**
     * @brief Method for testing the concrete implementation
     */
    virtual void test(void) {
        std::string hash = this->getHashFromString(teststring);
        std::string verify = this->getTestHash();
        if (hash != verify) {
            throw hlException(HL_VERIFY_TEST_FAILED,
                              "hashlib test-error: \"" +
                              hash +
                              "\" is not \"" +
                              verify +
                              "\" as supposed to be.");
        }
    }

    /**
     *  @brief 	This method creates a hash based on the
     *  		given string
     *
     *  		This memberfunctions calls resetContext(),
     *  		updateContext() and hashIt() in this order.
     *  		These three memberfunctions are pure virtual and have to
     *  		be implemented by the subclasses.
     *
     *  @param 	text The text to create a hash from. This
     *  		parameter is forwarded to updateContext()
     *  @return 	the created hash as std::string
     */
    virtual std::string getHashFromString(std::string text) {
        /*
         * reset the context so that we can start
         * with a new hash process
         */
        resetContext();

        /*
         * we update the context with the given text
         */
        updateContext((unsigned char *) text.c_str(), text.length());

        /*
         * now we can close the hash process
         * and return the created hash
         */
        return this->hashIt();
    }

    virtual std::string getHashFromData(unsigned char *ptr, int len) {
        int caret = 0;
        while (caret < len) {
            int chunk = (caret + 1024 > len ? len - caret : 1024);
            updateContext(&ptr[caret], chunk);
            caret += 1024;
        }

        return hashIt();
    }

    /**
     *  @brief 	This method creates a hash from a given file
     *
     *  		First of all resetContext() is called before reading the
     *  		specified file	in 1024 byte blocks which are forwarded
     *  		to the hash context by calling updateContext().
     *  		Finaly hashIt() is called to return the hash.
     *  		These three memberfunctions are pure virtual and have to
     *  		be implemented by the subclasses.
     *
     *  @param 	filename The file to created a hash from
     *
     *  @return	The created hash of the file or "-1" in case
     *  		the file could not be opened
     *  @throw	Throws a hlException if the specified file could not
     *  		be opened.
     */
    virtual std::string getHashFromFile(std::string filename) {
        std::ifstream ifs;
        int len;
        unsigned char buffer[1024];

        /*
         * reset the current hash context
         */
        resetContext();

        /*
         * open the specified file
         */
        ifs.open(filename.c_str());
        if (!ifs.is_open()) {
            throw hlException(HL_FILE_READ_ERROR,
                              "Cannot read file \"" +
                              filename +
                              "\".");
        }

        /*
         * read the file in 1024b blocks and
         * update the context for every block
         */
        while ((len = ifs.readsome((char *) buffer, 1024))) {
            updateContext(buffer, len);
        }

        ifs.close();
        return (hashIt());
    }
};

//----------------------------------------------------------------------
//end of include protection
#endif

//----------------------------------------------------------------------
//EOF
