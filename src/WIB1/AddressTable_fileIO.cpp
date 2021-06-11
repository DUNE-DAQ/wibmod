#include <WIB/AddressTable.hh>
#include <fstream>
#include <WIB/AddressTableException.hh>
#include <boost/tokenizer.hpp> //tokenizer
#include <stdlib.h>  //strtoul & getenv
#include <boost/regex.hpp> //regex
#include <boost/algorithm/string/case_conv.hpp> //to_upper


#define MAX_FILE_LEVEL 10
#define WIB_ADDRESS_TABLE_PATH "WIB_ADDRESS_TABLE_PATH" 

void AddressTable::LoadFile(std::string const & fileName,
			    std::string const & prefix, uint16_t offset){

  std::ifstream inFile(fileName.c_str());
  if (!inFile.is_open()){
    std::string envBasedFileName = fileName;
    //Try to use address table path if it exists
    if(getenv(WIB_ADDRESS_TABLE_PATH) != NULL){      
      envBasedFileName=getenv(WIB_ADDRESS_TABLE_PATH);
      envBasedFileName+="/";
      envBasedFileName+=fileName;
      inFile.open(envBasedFileName.c_str());
    }    
    if (!inFile.is_open()){
      BUException::BAD_FILE e;
      e.Append("File not found: ");
      e.Append(envBasedFileName.c_str());
      throw e;        
    }
  }
  const size_t bufferSize = 1000;
  char buffer[bufferSize + 1];
  memset(buffer,0,bufferSize+1);
  uint32_t lineNumber = 1;
  while(! inFile.eof()){    
    inFile.getline(buffer,bufferSize);
    ProcessLine(std::string(buffer),lineNumber,prefix,offset);
    lineNumber++;
  }
}


void AddressTable::ProcessLine(std::string const & line,size_t lineNumber,
			       std::string const & prefix, uint16_t offset){
  //First, ignore commments
  std::string activeLine = line.substr(0,line.find('#'));
  

  //Tokenize the activeLine
  boost::char_separator<char> sep(" ");
  boost::tokenizer<boost::char_separator<char> > tokens(activeLine,sep);
  boost::tokenizer<boost::char_separator<char> >::iterator itToken = tokens.begin();
  
 
  //crate a new Item;
  Item * item = new Item;
  size_t iToken = 0;
  for(; itToken != tokens.end(); itToken++){
    switch (iToken){
    case 0:
      {// keep name out of everyone else's scope
	//Assign name
	
	std::string name(*itToken);		
	if(!prefix.empty()){
	  //we have a prefix to add on to the name
	  name = prefix+std::string(".")+name;
	}

	while( (name.size() > 0) && ('.' == name[name.size()-1])) {
	  //If the trailing entry is a dot (level marker) drop it as this entry just means the prefix
	  name.erase(name.size()-1);
	}


	if(name.size() == 0){
	  //We have an emtpy name, this is bad and we should throw
	  BUException::INVALID_NAME e;
	  e.Append("Empty name");
	  throw e;    	  
	}
	
	boost::to_upper(name);

	//Assign the name to this item
	item->name.assign(name);
	iToken++;
      }
      break;
    case 1:
      //Assign address and apply any offset
      //itToken means we don't have to check for size of string
      item->address = strtoul(itToken->c_str(),NULL,0) + offset;
      iToken++;
      break;
    case 2:
      
      //Check if this is an include line
      if(!isdigit((itToken->c_str()[0]))){
	//we have an include file, append all following tokens together to use as the filename. 
	std::string filename(*itToken);
	itToken++;
	while(itToken != tokens.end()){	  
	  filename+=" ";
	  filename+=*itToken;
	  itToken++;
	}
	fileLevel++;
	if(fileLevel <= MAX_FILE_LEVEL){
	  LoadFile(filename,item->name,item->address);		  
	}else{
	  BUException::MAX_INCLUDE_FILE_DEPTH e;
	  e.Append("File: ");
	  e.Append(filename);
	  e.Append(" at prefix ");
	  e.Append(item->name);
	  e.Append(" is too deep\n");
	  throw e;
	}
	fileLevel--;
	//Delete fake partial item
	delete item;
	//return to move on to the next line of the main file
	return;
      }

      //Assign mask
      item->mask = strtoul(itToken->c_str(),NULL,0);      
      //stolen from https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel
      //Find the bitshift offset
      {
	unsigned int v = item->mask;      // 32-bit word input to count zero bits on right
	unsigned int c = 32; // c will be the number of zero bits on the right
	v &= -signed(v);
	if (v) {c--;}
	if (v & 0x0000FFFF) {c -= 16;}
	if (v & 0x00FF00FF) {c -= 8;}
	if (v & 0x0F0F0F0F) {c -= 4;}
	if (v & 0x33333333) {c -= 2;}
	if (v & 0x55555555) {c -= 1;}
	item->offset = c;
      }

      iToken++;      
      break;
    case 3:
      //Assign mode
      item->mode = 0;

      //Read
      if(itToken->find('r') != std::string::npos){
	item->mode |= Item::READ;
      }else if (itToken->find('R') != std::string::npos){
	item->mode |= Item::READ;
      }

      //Write
      if(itToken->find('w') != std::string::npos){
	item->mode |= Item::WRITE;
      }else if (itToken->find('W') != std::string::npos){
	item->mode |= Item::WRITE;
      }

      //Action
      if(itToken->find('a') != std::string::npos){
	item->mode |= Item::ACTION;
      }else if (itToken->find('A') != std::string::npos){
	item->mode |= Item::ACTION;
      }

      iToken++;      
      break;
    default:
      //parse user arguments
      //repeated arguments will be over-written
      
      //Find if this is an argument or a flag, flags have no equal signs
      if(itToken->find('=') == std::string::npos){
	//Create an entry for this with no data
	item->user[*itToken];
	iToken++;
      }else{
	//Get the name of the user value
	size_t equalSignPos = itToken->find('=');
	//make sure there isn't a """ before the =
	if(itToken->find('"') != std::string::npos){
	  if(itToken->find('"') < equalSignPos){
	    BUException::BAD_TOKEN e;
	    e.Append("Malformed token : ");
	    e.Append(itToken->c_str());
	    e.Append(" on line ");	    
	    char numberBuffer[14] = "\0\0\0\0\0\0\0\0\0\0\0\0";
	    snprintf(numberBuffer,12,"%zu\n",lineNumber);
	    e.Append(numberBuffer);
	    e.Append("Bad line: ");
	    e.Append(activeLine);
	    e.Append("\n");
	    throw e;
	  }
	}
	//cache the name of the user field
	std::string name = itToken->substr(0,equalSignPos);
	//Parse the rest of the user value if there is more size
	if(itToken->size()-1 == equalSignPos){
	    BUException::BAD_TOKEN e;
	    e.Append("Malformed token : ");
	    e.Append(itToken->c_str());
	    e.Append(" on line ");	    
	    char numberBuffer[14] = "\0\0\0\0\0\0\0\0\0\0\0\0";
	    snprintf(numberBuffer,12,"%zu\n",lineNumber);
	    e.Append(numberBuffer);
	    throw e;	  
	}
	//star this user field's data (data after the '=' char)
	std::string val = itToken->substr(equalSignPos+1);
	if(val[0] != '"'){
	  //We have a simple entry that has no quotes
	  item->user[name] = val;
	  iToken++;
	}else{
	  //We have a quoted value
	  val.erase(0,1); //remove the quote

	  //Check if this is still a simple entry, but with quotes	  
	  if(val.find('"') != std::string::npos){
	    //We have another quote, remove it and everything after it
	    val = val.substr(0,val.find('"'));
	  }else{
	    //We have a complicated value
	    itToken++;
	    while(itToken != tokens.end()){
	      val.append(" ");
	      val.append(*itToken);
	      if((*itToken)[itToken->size() -1] == '"'){	      
		//stop now that we've reached the closing quote
		//remove it from the string
		val.erase(val.size()-1);
		break;
	      }
	      iToken++;
	      itToken++;
	    }
	  }
	  //convert "\n" to '\n'
	  while(val.find("\\n") != std::string::npos){
	    val.replace(val.find("\\n"),2,std::string("\n"));
	  }
	  item->user[name] = val;
	}
      }
      break;
    }
    if(itToken == tokens.end()){
      break;
    }
  }

  if(iToken >= 4){
    AddEntry(item);
  }else{
    delete item;
  }
}

