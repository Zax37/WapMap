#ifndef SHA256_H
#define SHA256_H

//----------------------------------------------------------------------
//lenght defines
#define SHA256_BLOCK_LENGTH		64
#define SHA256_SHORT_BLOCK_LENGTH	(SHA256_BLOCK_LENGTH - 8)
#define SHA256_DIGEST_LENGTH		32
#define SHA256_DIGEST_STRING_LENGTH	(SHA256_DIGEST_LENGTH * 2 + 1)

//----------------------------------------------------------------------
//hl includes
#include "hl_types.h"

//----------------------------------------------------------------------
//typedefs

/**
 * Exactly 1 byte
 */
typedef hl_uint8  sha2_byte;

/**
 * Exactly 4 bytes
 */
typedef hl_uint32 sha2_word32;

/**
 * Exactly 8 bytes
 */
typedef hl_uint64 sha2_word64;

/**
 * @brief This struct represents a SHA256-hash context
 */
typedef struct HL_SHA256_CTX
{
	/**
	 * state
	 */
	hl_uint32		state[8];

	/**
	 * bitcount
	 */
	hl_uint64		bitcount;

	/**
	 * message buffer
	 */
	hl_uint8		buffer[SHA256_BLOCK_LENGTH];
} HL_SHA256_CTX;

//----------------------------------------------------------------------

/**
 *  @brief 	This class represents the implementation of
 *   		the sha256 algorithm.
 *
 *   		Basically the class provides three public member-functions
 *   		to create a hash:  SHA256_Init(), SHA256_Update() and SHA256_End().
 *   		If you want to create a hash based on a string or file quickly
 *   		you should use the sha256wrapper class instead of SHA256.
 */
class SHA256
{
	private:


		/**
		 *  @brief 	Finalize the sha256 operation
		 *  @param	digest The digest to finalize the operation with.
		 *  @param	context The context to finalize.
		 */
		void SHA256_Final(hl_uint8 digest[SHA256_DIGEST_LENGTH],
			          HL_SHA256_CTX* context);

		/**
		 *  @brief 	Internal data transformation
		 *  @param	context The context to use
		 *  @param	data The data to transform
		 */
		void SHA256_Transform(HL_SHA256_CTX* context,
			              const sha2_word32* data);

	public:

		/**
		 *  @brief 	Initialize the context
		 *  @param	context The context to init.
		 */
		void SHA256_Init(HL_SHA256_CTX *context);

		/**
		 *  @brief	Updates the context
		 *  @param	context The context to update.
		 *  @param	data The data for updating the context.
		 *  @param	len The length of the given data.
		 */
		void SHA256_Update(HL_SHA256_CTX* context,
			           const hl_uint8* data,
				   unsigned int len);

		/**
		 *  @brief 	Ends the sha256 operation and return the
		 *  		created hash in the given buffer.
		 *  @param	context The context to end.
		 *  @param	buffer This OUT-Parameter contains the created
		 *  		hash after ending the operation.
		 */
		char* SHA256_End(HL_SHA256_CTX* context,
			         char buffer[SHA256_DIGEST_STRING_LENGTH]);

};

//----------------------------------------------------------------------
//end of include protection
#endif


//----------------------------------------------------------------------
//EOF
