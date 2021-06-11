#include <WIB/WIB.hh>
#include <WIB/WIBException.hh>
#include <WIB/BNL_UDP_Exception.hh>
#include <fstream>
#include <assert.h>
#include <algorithm>
#define FLASH_TIMEOUT 400

#define MIN_LINE_LENGTH 10 
#define MAX_LINE_LENGTH 266

static void ParseIHexLine(std::string line, std::vector <uint8_t> &data, uint32_t &upperAddr){
  if(line.empty()){         
    BUException::WIB_FLASH_IHEX_ERROR e;
    e.Append("line is empty");
    throw e;
  }else if(line[0] != ':'){         
    BUException::WIB_FLASH_IHEX_ERROR e;
    e.Append("BAD Line in file - does not start with ':'");
    throw e;
  }else{
    line.erase(0,1);
  }      
  if(line.size()<MIN_LINE_LENGTH){         
    BUException::WIB_FLASH_IHEX_ERROR e;
    e.Append("BAD Line in file - incomplete intel hex format\n");
    throw e;
  }else if(line.size()>MAX_LINE_LENGTH){
    BUException::WIB_FLASH_IHEX_ERROR e;         
    e.Append("BAD Line in file - intel hex format not supported, contains too many characters\n");
    throw e;
  }else if (line.size()&1){         
    BUException::WIB_FLASH_IHEX_ERROR e;
    e.Append("BAD Line in file - uneven number of characters\n");
    throw e;
  }else {
    char const *entry = line.c_str();
    uint32_t line_sum = 0, byte_count = 0, address = 0, record_type = 0;

    uint32_t byte = 0;
    for(uint32_t iSum = 0; iSum < line.size(); iSum+=2){
      sscanf(entry+iSum, "%2x", &byte);
      line_sum += byte;
    }  
    //the content of the line is cancelled out through addition to its
    //two's complement which is the checksum if all data is intact

    if(0xFF&line_sum){
      line_sum = 0xFF & line_sum;
      BUException::WIB_FLASH_IHEX_ERROR e;            
      e.Append("BAD Line in file - content inconsistent with checksum.\n");
      throw e;
    } else{
      line_sum = 0;
    }  

    sscanf(entry,"%2x %4x %2x ", &byte_count, &address, &record_type);
    if(4 == record_type){
      upperAddr = 0;
      sscanf(entry+8,"%4x", &upperAddr);
      upperAddr = upperAddr << 16;
    } else if(0 == record_type){
      address = upperAddr+address;
      if(data.size() < (address+byte_count)){
	data.resize(address+byte_count,0xFF);
      }  
      for(uint32_t iAddr = 0; iAddr < byte_count; iAddr++){
	uint32_t tempData = 0;
	sscanf(entry+(iAddr*2)+8,"%02x",&tempData);
	data[address+iAddr] = (uint8_t)tempData;
      }  

    }  
  }  
}  

 void LoadIHexFile(std::vector <uint8_t>&data, const char* file){
   std::string line;
   std::ifstream datafile(file);
   uint32_t upperAddr = 0;
   if(datafile.is_open()){
     while(getline(datafile, line)){
       ParseIHexLine(line, data, upperAddr);
     }
   } else{
     BUException::WIB_FLASH_IHEX_ERROR e;      
     e.Append("Unable to open file ");
     e.Append(file);
     throw e;
   }
 }

static std::vector<uint32_t> firmwareFromIntelHexFile(std::string const & iHexFileName){
  //Ihex parser loads intel hex into a vector of bytes.
  std::vector<uint8_t> rawData;
  LoadIHexFile(rawData,iHexFileName.c_str());

  //We now need to convert it to a uint32_t 
  //This has the issue of what to do if we are off by 32bit boundary.
  //For now we'll just die so we don't have to deal with the issue. 
  //Make sure this is called before we erase the flash :-p
  if((rawData.size()&0x3) != 0x0){
    BUException::WIB_FLASH_IHEX_ERROR e;      
    e.Append("Flash data is not a multiple of 32bit words");
    throw e;
  }else if(0 == rawData.size()){
    BUException::WIB_FLASH_IHEX_ERROR e;      
    e.Append("Flash data is empty");
    throw e;
  }
  
  //Create a std::vector<uint32_t> and fill it with rawData;
  std::vector<uint32_t> ret;
  for(size_t iRawData = 0; iRawData < rawData.size();iRawData+=4){
    //Build the 32bit word
    uint32_t tempData = (rawData[iRawData + 0] <<  0 |
			 rawData[iRawData + 1] <<  8 |
			 rawData[iRawData + 2] << 16 |
			 rawData[iRawData + 3] << 24 );
    ret.push_back(tempData);      
  }
  return ret;
}
 

void WIB::ReadFlash(std::string const & fileName,uint8_t update_percentage){
  bool print_updates = false;
  if(update_percentage < 100){
    print_updates = true;
  }
  size_t update_delta = (update_percentage * float(16*1024*1024/4))/100;
  size_t next_update = update_delta;

  FILE * outFile = fopen(fileName.c_str(),"w");
  if(outFile == NULL){
    BUException::WIB_BAD_ARGS e;
    e.Append("Failed to create: ");
    e.Append(fileName);
    throw e;
  }

  if(print_updates){
    fprintf(stderr,"   Reading flash\n");
    fprintf(stderr,"   [");
    for(size_t i = 0; i < 100.0/update_percentage;i++){
      fprintf(stderr,"=");
    }
    fprintf(stderr,"]\n   [");
  }
  //program flash in groups of 64 32bit words (256 bytes)
  uint32_t address = 0;
  
  uint32_t blockRegMapAddress = GetItem("FLASH.DATA00")->address;
  size_t blockSize = 64;
  //set block size
  WriteWithRetry("FLASH.BYTE_COUNT",255);

  for(size_t iWord = 0; iWord < 16*1024*1024/4;){
    FlashCheckBusy();

    //Set adddress
    WriteWithRetry("FLASH.ADDRESS",address);
    //Start read
    WriteWithRetry("FLASH.RUN_COMMAND",0x5);
    
    FlashCheckBusy();
    
    //Readout the data
    for(size_t iWordRead = 0;iWordRead < blockSize;iWordRead++){
      fprintf(outFile,"0x%06X 0x%08X\n",uint32_t(iWord),ReadWithRetry(blockRegMapAddress+iWordRead));
      iWord++;
    }

	//    iWord+= blockSize;
    address+=blockSize*4;
    
    if(print_updates && (iWord > next_update)){
      //      printf("   % 3f%% done\n",float(iWord)/float(flashData.size()));
      fprintf(stderr,"=");
      next_update += update_delta;
    }
  }
  if(print_updates){
    printf("]\n");
    printf("   done\n");
  }
  fclose(outFile);
}

void WIB::EraseFlash(bool print_updates){
  if(print_updates){
    fprintf(stderr,"   Erase flash\n");
  }
  WriteWithRetry("FLASH.RUN_COMMAND",0x7);
  size_t iTimeout = FLASH_TIMEOUT;
  while(ReadWithRetry("FLASH.BUSY") && (iTimeout != 0)){
    iTimeout--;
    usleep(100000);
  }
  if(iTimeout == 0){
    BUException::WIB_FLASH_TIMEOUT e;
      //throw an exception
      e.Append("Program (erase): FLASH.BUSY");
      throw e;
    //throw an exception
  }
}

void WIB::ProgramFlash(std::string const & fileName,uint8_t update_percentage){
  WriteWithRetry("SYSTEM.SLOW_CONTROL_DND",1);

  bool print_updates = false;
  if(update_percentage < 100){
    print_updates = true;
  }

  //Load data and validate
  if(print_updates){
    fprintf(stderr,"   Reading file: %s\n",fileName.c_str());
  }
  //  std::vector<uint32_t> flashData = firmwareFromDumpFile(fileName);
  std::vector<uint32_t> flashData = firmwareFromIntelHexFile(fileName);

  //erase flash  
  EraseFlash(print_updates);

  //Load data into flash.
  WriteFlash(flashData,update_percentage);


  //Validate flash
  CheckFlash(flashData,update_percentage);
  WriteWithRetry("SYSTEM.SLOW_CONTROL_DND",0);
}

void WIB::WriteFlash(std::vector<uint32_t> flashData,uint8_t update_percentage){
  //Setup display if needed
  bool print_updates = false;
  if(update_percentage < 100){
    print_updates = true;
  }
  size_t update_delta = (update_percentage * float(flashData.size()))/100;
  size_t next_update = update_delta; 
  if(print_updates){
    fprintf(stderr,"   Programming flash\n");
    fprintf(stderr,"   [");
    for(size_t i = 0; i < 100.0/update_percentage;i++){
      fprintf(stderr,"=");
    }
    fprintf(stderr,"]\n   [");
  }


  //program flash in groups of 64 32bit words (256 bytes)
  uint32_t blockRegMapAddress = GetItem("FLASH.DATA00")->address;  //Address of first of 64 32bit words
  uint32_t flashAddress = 0; //Address in flash that we are writing to.

  for(size_t currentBlockStartIndex = 0; currentBlockStartIndex < flashData.size();){
    FlashCheckBusy();

    //Find size of this block to write (usually just 64 (256bytes)), but the last one might be smaller.
    size_t blockSize = std::min(size_t(64),flashData.size()-currentBlockStartIndex);
    //Set adddress
    WriteWithRetry("FLASH.ADDRESS",flashAddress);
    //set block size (in bytes and is 1 less than value; 0 means 1 byte, 255 means 256 bytes)
    WriteWithRetry("FLASH.BYTE_COUNT",(blockSize*sizeof(uint32_t))-1); 

    //Write block of data
    for(size_t iBlockWord = 0; iBlockWord < blockSize;iBlockWord++){
      //arg1: Address in WIB register map of this 32bit word
      //arg2: Data for this 32bit word reg map address in data vector
      WriteWithRetry(blockRegMapAddress               + iBlockWord,   
		     flashData[currentBlockStartIndex + iBlockWord]); 
    }
    //Do the block write
    WriteWithRetry("FLASH.RUN_COMMAND",0x1);
    currentBlockStartIndex += blockSize;
    flashAddress += blockSize*sizeof(uint32_t);
    
    //Update the screen if needed
    if(print_updates && (currentBlockStartIndex > next_update)){
      fprintf(stderr,"=");
      next_update += update_delta;
    }
  }
  //Update the screen if needed
  if(print_updates){
    fprintf(stderr,"]\n");
    fprintf(stderr,"   done\n");
  }

}

void WIB::CheckFlash(std::vector<uint32_t> flashData,uint8_t update_percentage){
  bool print_updates = false;
  if(update_percentage < 100){
    print_updates = true;
  }
  size_t update_delta = (update_percentage * float(16*1024*1024/4))/100;
  size_t next_update = update_delta;

  if(print_updates){
    fprintf(stderr,"   Checking flash\n");
    fprintf(stderr,"   [");
    for(size_t i = 0; i < 100.0/update_percentage;i++){
      fprintf(stderr,"=");
    }
    fprintf(stderr,"]\n   [");
  }
  //program flash in groups of 64 32bit words (256 bytes)
  uint32_t flashAddress = 0;  
  uint32_t blockRegMapAddress = GetItem("FLASH.DATA00")->address;

  for(size_t currentBlockStartIndex = 0; currentBlockStartIndex < 16*1024*1024/4;){
    FlashCheckBusy();

    //Find size of this block to write (usually just 64 (256bytes)), but the last one might be smaller.
    size_t blockSize = std::min(size_t(64),flashData.size()-currentBlockStartIndex);
    //Set adddress
    WriteWithRetry("FLASH.ADDRESS",flashAddress);
    //set block size (in bytes and is 1 less than value; 0 means 1 byte, 255 means 256 bytes)
    WriteWithRetry("FLASH.BYTE_COUNT",(blockSize*sizeof(uint32_t))-1); 
    //Start read
    WriteWithRetry("FLASH.RUN_COMMAND",0x5);

    FlashCheckBusy();

    //Check the data.
    for(size_t iBlockWord = 0;iBlockWord < blockSize;iBlockWord++){
      //flashData:     Address in WIB register map of this 32bit word
      //ReadWithRetry: Data from the flash for this word
      uint32_t dataRead;
      if((dataRead = ReadWithRetry(blockRegMapAddress + iBlockWord)) !=
	 flashData[currentBlockStartIndex + iBlockWord]){
	BUException::WIB_FLASH_ERROR e;	
	char errorbuffer[] = "Error on index 0xXXXXXXXX: 0xXXXXXXXX != 0xXXXXXXXX";
	snprintf(errorbuffer,
		 strlen(errorbuffer),
		 "Error on index 0x%08X: 0x%08X != 0x%08X",
		 flashAddress,
		 dataRead,
		 flashData[currentBlockStartIndex + iBlockWord]);
	e.Append(errorbuffer);
	throw e;
      }
    }
    currentBlockStartIndex += blockSize;
    flashAddress += blockSize*sizeof(uint32_t);
    
    if(print_updates && (currentBlockStartIndex > next_update)){
      fprintf(stderr,"=");
      next_update += update_delta;
    }
  }
  if(print_updates){
    printf("]\n");
    printf("   Check passed\n");
  }  
}


void WIB::FlashCheckBusy(){
  size_t iTimeout = FLASH_TIMEOUT;
  while(ReadWithRetry("FLASH.BUSY") && (iTimeout != 0)){
    iTimeout--;
    usleep(10000);
  }
  if(iTimeout == 0){
    BUException::WIB_FLASH_TIMEOUT e;
    //throw an exception
    e.Append("Read: FLASH.BUSY");
    throw e;
  }
}
