#include "hl_sha1wrapper.h"
#include "hl_sha1.h"
#include "hl_types.h"

//----------------------------------------------------------------------
//STL includes
#include <sstream>

//----------------------------------------------------------------------
//private memberfunctions

/**
 *  @brief 	This method ends the hash process
 *  		and returns the hash as string.
 *
 *  @return 	a hash as std::string
 */
std::string sha1wrapper::hashIt(void)
{
	hl_uint8 Message_Digest[20];
	sha1->SHA1Result(&context, Message_Digest);

	return convToString(Message_Digest);
}

/**
 *  @brief 	This internal member-function
 *  		convertes the hash-data to a
 *  		std::string (HEX).
 *
 *  @param 	data The hash-data to covert into HEX
 *  @return	the converted data as std::string
 */
std::string sha1wrapper::convToString(unsigned char *data)
{
	std::ostringstream os;
	for (int i = 0; i < 20; ++i)
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
 *  		current hash context
 *
 *  @param 	data The data to add to the current context
 *  @param 	len The length of the data to add
 */
void sha1wrapper::updateContext(unsigned char *data, unsigned int len)
{
	sha1->SHA1Input(&context, data, len);
}

/**
 *  @brief 	This method resets the current hash context.
 *  		In other words: It starts a new hash process.
 */
void sha1wrapper::resetContext(void)
{
	sha1->SHA1Reset(&context);
}

/**
 * @brief 	This method should return the hash of the
 * 		test-string "The quick brown fox jumps over the lazy
 * 		dog"
 */
std::string sha1wrapper::getTestHash(void)
{
	return "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12";
}

//----------------------------------------------------------------------
//public memberfunctions

/**
 *  @brief 	default constructor
 */
sha1wrapper::sha1wrapper()
{
	this->sha1 = new SHA1();
}

/**
 *  @brief 	default destructor
 */
sha1wrapper::~sha1wrapper()
{
	delete this->sha1;
}

//----------------------------------------------------------------------
//EOF
