#include "hl_sha384wrapper.h"

//----------------------------------------------------------------------
//STL includes
#include <string>

//----------------------------------------------------------------------
//private memberfunctions

/**
 *  @brief 	This method ends the hash process
 *  		and returns the hash as string.
 *
 *  @return 	a hash as std::string
 */
std::string sha384wrapper::hashIt(void)
{
	sha2_byte buff[SHA384_DIGEST_STRING_LENGTH];
	sha384->SHA384_End(&context,(char*)buff);
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
std::string sha384wrapper::convToString(unsigned char *data)
{
	/*
	 * we can just copy data to a string, because
	 * the transforming to hash is already done
	 * within the sha384 implementation
	 */
	return std::string((const char*)data);
}

/**
 *  @brief 	This method adds the given data to the
 *  		current hash context
 *
 *  @param 	data The data to add to the current context
 *  @param 	len The length of the data to add
 */
void sha384wrapper::updateContext(unsigned char *data, unsigned int len)
{
	this->sha384->SHA384_Update(&context,data,len);
}

/**
 *  @brief 	This method resets the current hash context.
 *  		In other words: It starts a new hash process.
 */
void sha384wrapper::resetContext(void)
{
	sha384->SHA384_Init(&context);
}

/**
 * @brief 	This method should return the hash of the
 * 		test-string "The quick brown fox jumps over the lazy
 * 		dog"
 */
std::string sha384wrapper::getTestHash(void)
{
	return "ca737f1014a48f4c0b6dd43cb177b0afd9e5169367544c494011e3317dbf9a509cb1e5dc1e85a941bbee3d7f2afbc9b1";
}

//----------------------------------------------------------------------
//public memberfunctions

/**
 *  @brief 	default constructor
 */
sha384wrapper::sha384wrapper()
{
	this->sha384 = new SHA2ext();
}

/**
 *  @brief 	default destructor
 */
sha384wrapper::~sha384wrapper()
{
	delete sha384;
}
