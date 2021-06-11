#ifndef __EXCEPTIONBASE_HPP__
#define __EXCEPTIONBASE_HPP__ 1

#include <exception> //for inheritance from std::exception
  
#include <string.h> //for memcpy, strlen.

#include <string> //std::string

#include <stdlib.h> // for malloc

#include <stdio.h>

//For SYS_gettid()
//#define _GNU_SOURCE          /* See feature_test_macros(7) */
#include <unistd.h>
#include <sys/syscall.h>    /* For SYS_xxx definitions */

//Backtrace code
#include <execinfo.h>

//abi demangling
#include <cxxabi.h>

namespace BUException{
        
  //Macro for derived exception classes
#define ExceptionClassGenerator( ClassName , ClassDescription )	\
  class ClassName : public BUException::exBase {		\
  public:							\
  ClassName() throw() {Init();}					\
  ClassName(const ClassName & rh) throw() : 			\
	BUException::exBase(rh) {Init();Copy(rh);}		\
  ClassName & operator=(const ClassName & rh) throw()		\
    {Init();Copy(rh);return *this;}				\
  ~ClassName() throw() {}					\
  const char * what() const throw() {return whatname;}		\
  private:							\
  void Init()							\
    {								\
      strcpy(whatname,ClassDescription);			\
    }								\
  char whatname[sizeof(ClassDescription)];			\
  };
    
    
  class exBase : public std::exception {
  public:
    //destructor
    virtual ~exBase() throw();
      
    //Returns a stack trace of where the exception happened
    const char * StackTrace() const throw();
      
    //Append info to the description message
    void Append(const char * buffer) throw();
    void Append(std::string str){Append(str.c_str());};
    const char * Description() const throw();
      
    //Return the what string
    virtual const char * what() const throw()  = 0;
      
      
    pid_t GetPID(){return PID;};
      
  protected:
    //Constructor is protected so only derived classes can call it
    exBase() throw() ;
    //Copy function for base class internals (called by derived)
    void Copy(const exBase & rh) throw();
    exBase(const exBase & rh) throw() {(void) rh;}; // have to cast to void to keep from unused param error
      
  private:
    //assignment operator is private and not implemented
    exBase & operator=(const exBase & rh) throw();
      
    //Stack trace code
    void GenerateStackTrace() throw();
    void AppendStackLine(const char * line) throw(); //adds a eol char if possible
      
    //Description internals
    size_t descriptionSize;
    size_t descriptionUsed;
    char * descriptionBuffer;
      
    //Stack trace internals
    size_t stackSize;
    size_t stackUsed;
    char * stackBuffer;
      
    pid_t PID;
  };
}




#endif
