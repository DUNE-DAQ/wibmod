#ifndef __ADDRESSTABLEEXCEPTION_HH__
#define __ADDRESSTABLEEXCEPTION_HH__ 1

#include <BUException/ExceptionBase.hh>

namespace BUException{       
  //Exceptions for WIB
  ExceptionClassGenerator(BAD_FILE,"Bad File\n")
  ExceptionClassGenerator(INVALID_NAME,"Invalid name\n")
  ExceptionClassGenerator(BAD_TOKEN,"Bad Token\n")
  ExceptionClassGenerator(NULL_POINTER,"NULL pointer used\n")
  ExceptionClassGenerator(BAD_MODE,"Invalid mode for address table entry\n")
  ExceptionClassGenerator(NAME_COLLISION,"Collision in address table names\n")
  ExceptionClassGenerator(BAD_REGEX,"Invalid regex syntax\n")
  ExceptionClassGenerator(MAX_INCLUDE_FILE_DEPTH,"Included file depth exceeded maximum\n")
  ExceptionClassGenerator(BAD_BLOCK_WRITE,"Block write to registers with bad attributes\n")
}

#endif
