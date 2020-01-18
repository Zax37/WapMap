#ifndef SHA2ext_H
#define SHA2ext_H

//----------------------------------------------------------------------
//lenght defines
#define SHA384_BLOCK_LENGTH             128
#define SHA384_DIGEST_LENGTH            48
#define SHA384_DIGEST_STRING_LENGTH     (SHA384_DIGEST_LENGTH * 2 + 1)
#define SHA512_BLOCK_LENGTH             128
#define SHA512_DIGEST_LENGTH            64
#define SHA512_DIGEST_STRING_LENGTH     (SHA512_DIGEST_LENGTH * 2 + 1)
#define SHA512_SHORT_BLOCK_LENGTH	(SHA512_BLOCK_LENGTH - 16)

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
 * @brief This struct represents a SHA512-hash context
 */
typedef struct HL_SHA512_CTX
{
	hl_uint64       state[8];
	hl_uint64       bitcount[2];
	hl_uint8        buffer[SHA512_BLOCK_LENGTH];
} HL_SHA512_CTX;


/**
 * @brief This struct represents a SHA384-hash context
 */
typedef HL_SHA512_CTX HL_SHA_384_CTX;

//----------------------------------------------------------------------

/**
 *  @brief 	This class represents the implementation of
 *   		the SHA384 and SHA512 algorithm.
 *
 *   		Basically the class provides six public member-functions
 *   		to create a hash:  SHA384_Init(), SHA384_Update(), SHA384_End(),
 *		SHA512_Init(), SHA512_Update() and SHA512_End().
 *   		If you want to create a hash based on a string or file quickly
 *   		you should use the sha384wrapper or sha512wrapper classes.
 */
class SHA2ext
{
	private:

		/**
		 *  @brief 	Finalize the sha384 operation
		 *  @param	digest The digest to finalize the operation with.
		 *  @param	context The context to finalize.
		 */
		void SHA384_Final(hl_uint8 digest[SHA384_DIGEST_LENGTH],
			          HL_SHA_384_CTX* context);

		/**
		 *  @brief 	Finalize the sha512 operation
		 *  @param	digest The digest to finalize the operation with.
		 *  @param	context The context to finalize.
		 */
		void SHA512_Final(hl_uint8 digest[SHA512_DIGEST_LENGTH],
			       	  HL_SHA512_CTX* context);

		/**
		 *  @brief 	Internal method
		 *
		 *  		used by SHA512 and SHA384
		 *  @author	Benjamin Grüdelbach
		 *  @param	context The context of the operation
		 */
		void SHA512_Last(HL_SHA512_CTX* context);

		/**
		 *  @brief 	Internal data transformation
		 *  @param	context The context to use
		 *  @param	data The data to transform
		 */
		void SHA512_Transform(HL_SHA512_CTX* context,
			              const sha2_word64* data);


	public:

		/**
		 *  @brief 	Initialize the SHA384 context
		 *  @param	context The context to init.
		 */
		void SHA384_Init(HL_SHA_384_CTX* context);

		/**
		 *  @brief 	Initialize the SHA512 context
		 *  @param	context The context to init.
		 */
		void SHA512_Init(HL_SHA512_CTX* context);

		/**
		 *  @brief	Updates the SHA512 context
		 *  @param	context The context to update.
		 *  @param	data The data for updating the context.
		 *  @param	len The length of the given data.
		 */
		void SHA384_Update(HL_SHA_384_CTX* context,
			           const hl_uint8* data,
				   unsigned int len);

		/**
		 *  @brief	Updates the SHA284 context
		 *  @param	context The context to update.
		 *  @param	data The data for updating the context.
		 *  @param	len The length of the given data.
		 */
		void SHA512_Update(HL_SHA512_CTX* context,
			           const hl_uint8* data,
				   unsigned int len);

		/**
		 *  @brief 	Ends the SHA384 operation and return the
		 *  		created hash in the given buffer.
		 *  @param	context The context to end.
		 *  @param	buffer This OUT-Parameter contains the created
		 *  		hash after ending the operation.
		 */
		char* SHA384_End(HL_SHA_384_CTX* context,
			       	 char buffer[SHA384_DIGEST_STRING_LENGTH]);

		/**
		 *  @brief 	Ends the SHA512 operation and return the
		 *  		created hash in the given buffer.
		 *  @param	context The context to end.
		 *  @param	buffer This OUT-Parameter contains the created
		 *  		hash after ending the operation.
		 */
		char* SHA512_End(HL_SHA512_CTX* context,
			       	 char buffer[SHA512_DIGEST_STRING_LENGTH]);

};


//----------------------------------------------------------------------
//end of include protection
#endif

//----------------------------------------------------------------------
//EOF
