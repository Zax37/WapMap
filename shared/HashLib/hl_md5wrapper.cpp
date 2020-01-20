#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

//----------------------------------------------------------------------
//hashlib++ includes
#include "hl_md5wrapper.h"

//----------------------------------------------------------------------
//private member functions

/**
 *  @brief 	This method ends the hash process
 *  		and returns the hash as string.
 *
 *  @return 	the hash as std::string
 */
std::string md5wrapper::hashIt(void)
{
	//create the hash
	unsigned char buff[16] = "";
	md5->MD5Final((unsigned char*)buff, &ctx);

	//converte the hash to a string and return it
	return convToString(buff);
}

/**
 *  @brief 	This internal member-function
 *  		convertes the hash-data to a
 *  		std::string (HEX).
 *
 *  @param 	data The hash-data to covert into HEX
 *  @return	the converted data as std::string
 */
std::string md5wrapper::convToString(unsigned char *data)
{
	/*
	 * using a ostringstream to convert the hash in a
	 * hex string
	 */
	std::ostringstream os;
	for (int i = 0; i < 16; ++i)
	{
		/*
		 * set the width to 2
		 */
		os.width(2);

		/*
		 * fill with 0
		 */
		os.fill('0');

		/*
		 * conv to hex
		 */
		os << std::hex << static_cast<unsigned int>(data[i]);
	}

	/*
	 * return as std::string
	 */
	return os.str();
}

/**
 *  @brief 	This method adds the given data to the
 *  		current hash context.
 *
 *  @param 	data The data to add to the current context
 *  @param 	len The length of the data to add
 */
void md5wrapper::updateContext(unsigned char *data, unsigned int len)
{
	//update
	md5->MD5Update(&ctx, data, len);
}

/**
 *  @brief 	This method resets the current hash context.
 *  		In other words: It starts a new hash process.
 */
void md5wrapper::resetContext(void)
{
	//init md5
	md5->MD5Init(&ctx);
}

/**
 * @brief 	This method should return the hash of the
 * 		test-string "The quick brown fox jumps over the lazy
 * 		dog"
 */
std::string md5wrapper::getTestHash(void)
{
	return "9e107d9d372bb6826bd81d3542a419d6";
}

//----------------------------------------------------------------------
//public member functions

/**
 *  @brief 	default constructor
 */
md5wrapper::md5wrapper()
{
	md5 = new MD5();
}

/**
 *  @brief 	default destructor
 */
md5wrapper::~md5wrapper()
{
	delete md5;
}

//----------------------------------------------------------------------
//EOF
