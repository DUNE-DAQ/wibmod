#ifndef __BNL_UDP_EXCEPTION_HH__
#define __BNL_UDP_EXCEPTION_HH__ 1

#include <BUException/ExceptionBase.hh>

namespace BUException{       
  //Exceptions for WIB
  ExceptionClassGenerator(BNL_UDP_PORT_OUT_OF_RANGE,"WIB Port offset too large\n")
  ExceptionClassGenerator(BAD_REMOTE_IP,"Invalid remote IP address\n")
  ExceptionClassGenerator(BAD_SOCKET,"Socket creation failed\n")
  ExceptionClassGenerator(SEND_FAILED,"Failed to send WIB packet\n")
  ExceptionClassGenerator(CONNECTION_FAILED,"Connect failed\n")
  ExceptionClassGenerator(BAD_REPLY,"Bad WIB reply packet\n")
}




#endif
