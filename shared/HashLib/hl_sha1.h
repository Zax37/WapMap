#ifndef SHA1_H
#define SHA1_H

//----------------------------------------------------------------------
//hl includes
#include "hl_types.h"

//----------------------------------------------------------------------
//enums

#ifndef _SHA_enum_
#define _SHA_enum_
enum
{
    shaSuccess = 0,
    shaNull,            /* Null pointer parameter */
    shaInputTooLong,    /* input data too long */
    shaStateError       /* called Input after Result */
};
#endif

//----------------------------------------------------------------------
//defines
#define SHA1HashSize 20

//----------------------------------------------------------------------
//structs

/**
 * @brief this struct represents a SHA1-hash context.
 */
typedef struct HL_SHA1_CTX
{
	/** Message Digest */
	hl_uint32 Intermediate_Hash[SHA1HashSize/4];

	/** Message length in bits */
	hl_uint32 Length_Low;

	/** Message length in bits */
	hl_uint32 Length_High;

	/** Index into message block array */
	hl_uint16 Message_Block_Index;

	/** 512-bit message blocks */
	hl_uint8 Message_Block[64];

	/** Is the digest computed? */
	int Computed;

	/** Is the message digest corrupted? */
	int Corrupted;

} HL_SHA1_CTX;

//----------------------------------------------------------------------
//class definition

/**
 *  @brief 	This class represents the implementation of
 *   		the sha1 algorithm.
 *
 *   		Basically the class provides three public member-functions
 *   		to create a hash:  SHA1Reset(), SHA1Input() and SHA1Result().
 *   		If you want to create a hash based on a string or file quickly
 *   		you should use the sha1wrapper class instead of SHA1.
 */
class SHA1
{
	private:

			/**
			 *  @brief 	Internal method to padd the message
			 *
			 *      	According to the standard, the message must
			 *      	be padded to an even 512 bits. The first
			 *      	padding bit must be a '1'.  The last 64	bits
			 *      	represent the length of the original message.
			 *      	All bits in between should be 0.
			 *      	This function will pad the message according
			 *      	to those rules by filling the Message_Block array
			 *      	accordingly.  It will also call the
			 *      	ProcessMessageBlock function provided appropriately.
			 *      	When it returns, it can be assumed that the message
			 *      	digest has been computed.
			 *
			 *  @param	context The context to padd
			 *
			 */
			void SHA1PadMessage(HL_SHA1_CTX *context);

			/**
			 *  @brief      This member-function will process the next 512 bits of the
			 *  		message stored in the Message_Block array.
			 *
			 *      	Many of the variable names in this code, especially the
			 *      	single character names, were used because those were the
			 *      	names used in the publication.
			 *
			 *  @param	context The context to process
			 */
			void SHA1ProcessMessageBlock(HL_SHA1_CTX *context);

	public:

		/**
		 *  @brief 	Resets the sha1 context and starts a new
		 *  		hashprocess
		 *  @param	context The context to reset
		 *  @return	0 on succes an error number otherwise
		 */
		int SHA1Reset(  HL_SHA1_CTX *context);

		/**
		 *  @brief 	Data input.
		 *
		 *  		This memberfunction add data to the specified
		 *  		context.
		 *
		 *  @param	context The context to add data to
		 *  @param	message_array The data to add
		 *  @param	length The length of the data to add
		 */
		int SHA1Input(  HL_SHA1_CTX   *context,
				const hl_uint8 *message_array,
				unsigned int  length);

		/**
		 *  @brief 	This ends the sha operation, zeroizing the context
		 *  		and returning the computed hash.
		 *
		 *  @param	context The context to get the hash from
		 *  @param	Message_Digest This is an OUT parameter which
		 *  		contains the hash after the menberfunction returns
		 *  @return	0 on succes, an error-code otherwise
		 */
		int SHA1Result( HL_SHA1_CTX *context,
				hl_uint8     Message_Digest[SHA1HashSize]);
};

//----------------------------------------------------------------------
//end of include protection
#endif

//----------------------------------------------------------------------
//EOF
