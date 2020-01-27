#include "hl_sha256wrapper.h"
#include "hl_sha256.h"


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
std::string sha256wrapper::hashIt(void) {
    sha2_byte buff[SHA256_DIGEST_STRING_LENGTH];
    sha256->SHA256_End(&context, (char *) buff);

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
std::string sha256wrapper::convToString(unsigned char *data) {
    /*
     * we can just copy data to a string, because
     * the transforming to hash is already done
     * within the sha256 implementation
     */
    return std::string((const char *) data);
}

/**
 *  @brief 	This method adds the given data to the
 *  		current hash context
 *
 *  @param 	data The data to add to the current context
 *  @param 	len The length of the data to add
 */
void sha256wrapper::updateContext(unsigned char *data, unsigned int len) {
    this->sha256->SHA256_Update(&context, data, len);
}

/**
 *  @brief 	This method resets the current hash context.
 *  		In other words: It starts a new hash process.
 */
void sha256wrapper::resetContext(void) {
    sha256->SHA256_Init(&context);
}


/**
 * @brief 	This method should return the hash of the
 * 		test-string "The quick brown fox jumps over the lazy
 * 		dog"
 */
std::string sha256wrapper::getTestHash(void) {
    return "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592";
}

//----------------------------------------------------------------------
//public memberfunctions

/**
 *  @brief 	default constructor
 */
sha256wrapper::sha256wrapper() {
    this->sha256 = new SHA256();
}

/**
 *  @brief 	default destructor
 */
sha256wrapper::~sha256wrapper() {
    delete sha256;
}

//----------------------------------------------------------------------
//EOF
