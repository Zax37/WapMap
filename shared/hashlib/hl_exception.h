#ifndef HL_EXCEPTION_H
#define HL_EXCEPTION_H

//----------------------------------------------------------------------
//STL
#include <string>

//----------------------------------------------------------------------

/**
 * definition of hashlib++ errornumbers
 */
typedef enum hlerrors {
    HL_NO_ERROR = 0,
    HL_FILE_READ_ERROR,
    HL_VERIFY_TEST_FAILED,
    HL_UNKNOWN_SEE_MSG
} hlerror;

//----------------------------------------------------------------------

/**
 *  @brief	This class represents a exception within the hashlib++
 *  		project
 */
class hlException {
private:

    /**
     * Error Number
     */
    hlerror iError;

    /**
     * Error message as string
     */
    std::string strMessge;


public:
    /**
     *  @brief 	constructor
     *  @param	er	Error number
     *  @param	m	Error message
     */
    hlException(hlerror er, std::string m) {
        this->iError = er;
        this->strMessge = m;
    }

    /**
     *  @brief 	constructor
     *  @param	m	Error Message
     */
    hlException(std::string m) {
        this->iError = HL_UNKNOWN_SEE_MSG;
        this->strMessge = m;
    }

    /**
     *  @brief 	returns the error message
     *  @return	the error message
     */
    std::string error_message(void) {
        return strMessge;
    }

    /**
     *  @brief 	returns the error number
     *  @return	the error number
     */
    hlerror error_number(void) {
        return iError;
    }
};

//----------------------------------------------------------------------
//end of include protection
#endif

//----------------------------------------------------------------------
//EOF
