#ifndef HLTYPES_H
#define HLTYPES_H

//----------------------------------------------------------------------

/**
 * exactly 1 Byte
 */
typedef unsigned char 	hl_uint8;

/**
 * at least 2 Byte
 */
typedef unsigned short int 	hl_uint16;

/**
 * at least 4 Byte
 */
typedef unsigned int hl_uint32;

/**
* at least 8 Byte
*/
#ifdef __GNUC__
	typedef unsigned long long int	hl_uint64;
#elif __MINGW32__
	typedef unsigned long long int	hl_uint64;
#elif _MSC_VER
	typedef unsigned __int64 hl_uint64;
#else
	#error "Unsuppported compiler." \
               "Please use GCC,MINGW,MSVC " \
	       " or define hl_uint64 for your compiler in hl_types.h line 62"
#endif


//----------------------------------------------------------------------
//end of include protection
#endif

//----------------------------------------------------------------------
//EOF
