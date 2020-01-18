#ifndef SHA1WRAPPER_H
#define SHA1WRAPPER_H

//----------------------------------------------------------------------
//hashlib++ includes
#include "hl_hashwrapper.h"
#include "hl_sha1.h"

//----------------------------------------------------------------------

/**
 *  @brief 	This class represents the SHA1 wrapper-class
 *
 *  		You can use this class to easily create a sha1 hash.
 *  		Just create an instance of sha1wrapper and call the
 *  		inherited memberfunctions getHashFromString()
 *  		and getHashFromFile() to create a hash based on a
 *  		string or a file.
 *
 *  		Have a look at the following example:
 *
 *  @include 	sha1example.cpp
 *
 *  		sha1wrapper implements resetContext(), updateContext()
 *  		and hashIt() to create a hash.
 */
class sha1wrapper : public hashwrapper
{
	protected:
			/**
			 * SHA1 access
			 */
			SHA1 *sha1;

			/*
			 * SHA1 context
			 */
			HL_SHA1_CTX context;

			/**
			 *  @brief 	This method ends the hash process
			 *  		and returns the hash as string.
			 *
			 *  @return 	a hash as std::string
			 */
			virtual std::string hashIt(void);

			/**
			 *  @brief 	This internal member-function
			 *  		convertes the hash-data to a
			 *  		std::string (HEX).
			 *
			 *  @param 	data The hash-data to covert into HEX
			 *  @return	the converted data as std::string
			 */
			virtual std::string convToString(unsigned char *data);

			/**
			 *  @brief 	This method adds the given data to the
			 *  		current hash context
			 *
			 *  @param 	data The data to add to the current context
			 *  @param 	len The length of the data to add
			 */
			virtual void updateContext(unsigned char *data, unsigned int len);

			/**
			 *  @brief 	This method resets the current hash context.
			 *  		In other words: It starts a new hash process.
			 */
			virtual void resetContext(void);

			/**
			 * @brief 	This method should return the hash of the
			 * 		test-string "The quick brown fox jumps over the lazy
			 * 		dog"
			 */
			virtual std::string getTestHash(void);

	public:

			/**
			 *  @brief 	default constructor
			 */
			sha1wrapper();

			/**
			 *  @brief 	default destructor
			 */
			virtual ~sha1wrapper();
};

//----------------------------------------------------------------------
//end of include protection
#endif

//----------------------------------------------------------------------
//EOF
