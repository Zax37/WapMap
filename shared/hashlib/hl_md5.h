#ifndef MD5_H
#define MD5_H

//----------------------------------------------------------------------
//STL includes
#include <string>

//----------------------------------------------------------------------
//hl includes
#include "hl_types.h"

//----------------------------------------------------------------------
//typedefs
typedef hl_uint8 *POINTER;

/**
 * @brief this struct represents a MD5-hash context.
 */
typedef struct {
    /** state (ABCD) */
    unsigned long int state[4];

    /** number of bits, modulo 2^64 (lsb first) */
    unsigned long int count[2];

    /** input buffer */
    unsigned char buffer[64];
} HL_MD5_CTX;

//----------------------------------------------------------------------

/**
 *  @brief 	This class represents the implementation of
 *   		the md5 message digest algorithm.
 *
 *   		Basically the class provides three public member-functions
 *   		to create a hash:  MD5Init(), MD5Update() and MD5Final().
 *   		If you want to create a hash based on a string or file quickly
 *   		you should use the md5wrapper class instead of MD5.
 */
class MD5 {

private:

    /**
     *  @brief 	Basic transformation. Transforms state based on block.
     *  @param	state	state to transform
     *  @param	block	block to transform
     */
    void MD5Transform(unsigned long int state[4], unsigned char block[64]);

    /**
     *  @brief 	Encodes input data
     *  @param	output Encoded data as OUT parameter
     *  @param	input Input data
     *  @param	len The length of the input assuming it is a
     *  		multiple of 4
     */
    void Encode(unsigned char *output,
                unsigned long int *input,
                unsigned int len);

    /**
     *  @brief 	Decodes input data into output
     *  @param	output Decoded data as OUT parameter
     *  @param	input Input data
     *  @param	len The length of the input assuming it is a
     *  		multiple of 4
     */
    void Decode(unsigned long int *output,
                unsigned char *input,
                unsigned int len);

    /**
     *  @brief 	internal memory management
     *  @param	output OUT parameter where POINTER is an unsigned
     *  		char*
     *  @param	input Data to copy where POINTER is a unsigned char*
     *  @param	len The length of the data
     */
    void MD5_memcpy(POINTER output, POINTER input, unsigned int len);

    /**
     *  @brief 	internal memory management
     *  @param 	output OUT parameter where POINTER is an unsigned
     *  		char*
     *  @param	value Value to fill the memory with
     *  @param	len The length of the data
     *
     */
    void MD5_memset(POINTER output, int value, unsigned int len);

public:

    /**
     *  @brief 	Initialization begins an operation,
     *  		writing a new context
     *  @param 	context	The HL_MD5_CTX context to initialize
     */
    void MD5Init(HL_MD5_CTX *context);

    /**
     *  @brief 	Block update operation. Continues an md5
     *  		message-digest operation, processing another
     *  		message block, and updating the context.
     *  @param	context The HL_MD5_CTX context to update
     *  @param	input The data to write into the context
     *  @param	inputLen The length of the input data
     */
    void MD5Update(HL_MD5_CTX *context,
                   unsigned char *input,
                   unsigned int inputLen);

    /**
     *  @brief 	Finalization ends the md5 message-digest
     *  		operation, writing the the message digest and
     *  		zeroizing the context.
     *  @param	digest This is an OUT parameter which contains
     *  		the created hash after the method returns
     *  @param	context The context to finalize
     */
    void MD5Final(unsigned char digest[16], HL_MD5_CTX *context);

    /**
     *  @brief 	default constructor
     */
    MD5() {};
};

//----------------------------------------------------------------------
//End of include protection
#endif

//----------------------------------------------------------------------
//EOF
