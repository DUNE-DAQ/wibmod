#include <WIB/BNL_UDP.hh>
#include <WIB/BNL_UDP_Exception.hh>
#include <sys/socket.h>
#include <string.h> //memset, strerror
#include <errno.h>
#include <string>
#include <fcntl.h> //fcntl

#include <sstream>
#include <iomanip>

#include <netdb.h>

#include <errno.h>
#include <algorithm> //STD::COUNT

#define WIB_WR_BASE_PORT 32000
#define WIB_RD_BASE_PORT 32001
#define WIB_RPLY_BASE_PORT 32002

#define WIB_PACKET_KEY 0xDEADBEEF
//#define WIB_REQUEST_PACKET_SIZE (4+2+4+2)
#define WIB_REQUEST_PACKET_TRAILER 0xFFFF

#define WIB_RPLY_PACKET_SIZE 12

#define TIMEOUT_SECONDS 2
#define TIMEOUT_MICROSECONDS 0
struct WIB_packet_t{
  uint32_t key;
  uint32_t reg_addr : 16;
  uint32_t data_MSW : 16;
  uint32_t data_LSW : 16;
  uint32_t trailer  : 16;
};

static std::string dump_packet(uint8_t * data, size_t size){
  //  printf("Err: %p %zu\n",data,size);
  std::stringstream ss;
  for(size_t iWord = 0; iWord < size;iWord++){
    ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << iWord;
    ss << ": 0x" << std::hex << std::setfill('0') << std::setw(2) << int(data[iWord]);
    iWord++;
    if(iWord < size){
      ss << std::hex << std::setfill('0') << std::setw(2) << int(data[iWord]);
    }
    ss << std::endl;
  }
  //  printf("%s",ss.str().c_str());
  return ss.str();
}

void BNL_UDP::FlushSocket(int sock){
  //turn on non-blocking
  fcntl(sock, F_SETFL, fcntl(sock, F_GETFL)| O_NONBLOCK);
  int ret;
  do{
    ret = recv(sock,buffer,buffer_size,0);      
  }while(ret != -1);
  fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) & (~O_NONBLOCK));
}
  
void BNL_UDP::Clear(){
  //close sockets
  if(readSocketFD != -1){
    close(readSocketFD);
    readSocketFD = -1;
  }
  if(writeSocketFD != -1){
    close(writeSocketFD);
    writeSocketFD = -1;
  }
  //Clear packet buffer
  if(buffer != NULL){
    delete [] buffer;
    buffer_size = 0;
  }  
  writeAck  = false;
  connected = false;

  //Reset send/recv addr structures
  memset(&readAddr,0,sizeof(readAddr));
  memset(&writeAddr,0,sizeof(writeAddr));
}

//static void printaddress(struct sockaddr_in const * addr){
//  printf("%u: %u.%u.%u.%u : %u\n",
//	 addr->sin_family,
//	 (addr->sin_addr.s_addr >> 0)&0xFF,
//	 (addr->sin_addr.s_addr >> 8)&0xFF,
//	 (addr->sin_addr.s_addr >> 16)&0xFF,
//	 (addr->sin_addr.s_addr >> 24)&0xFF,
//	 ntohs(addr->sin_port));
//}

void BNL_UDP::Setup(std::string const & address,uint16_t port_offset){
  //Reset the network structures
  Clear();

  //Allocate the recv buffer
  ResizeBuffer();

  //Check port_offset range
  if(port_offset > 128){
    BUException::BNL_UDP_PORT_OUT_OF_RANGE e;
    throw e;    
  }

  //Set the ports for this device (FEMBs are iFEMB*0x10 above the base)
  readPort  = WIB_RD_BASE_PORT   + port_offset;
  writePort = WIB_WR_BASE_PORT   + port_offset;
  //  replyPort = WIB_RPLY_BASE_PORT + port_offset;

  remoteAddress = address;
  //Get the sockaddr for the address
  struct addrinfo * res;
  if(getaddrinfo(address.c_str(),NULL,NULL,&res)){
    //Check if we have just one "." character and is less than 5 characters
    if(address.size() <= 5 && 1 == std::count(address.begin(),address.end(),'.')){
      std::string strCrate = address.substr(0,address.find('.'));
      std::string strSlot  = address.substr(address.find('.')+1);
      if(strCrate.size() != 0 && strSlot.size() != 0){
	uint8_t crate = strtoul(strCrate.c_str(),NULL,0);
	uint8_t slot  = strtoul(strSlot.c_str(), NULL,0);
	if( (((crate > 0) && (crate < 7)) || 0xF == crate) &&
	    (((slot > 0) && (slot < 7)) || 0xF == slot)){
	  remoteAddress = std::string("192.168.");
	  //Add the crate part of the address (200 + crate number)
	  if(crate == 0xF){
	    remoteAddress += "200.";
	  }else{
	    //generate the crate number which is 200 + crate number
	    remoteAddress += "20";
	    remoteAddress += ('0' + crate);
	    remoteAddress += '.';
	  }
	  if(slot == 0xF){
	    remoteAddress += "50";
	  }else{
	    //crate last IP octet that is slot number
	    remoteAddress += ('0' + slot);
	  }
	}
      }
    }
    //try a second time assumin gthis is a crate.slot address, fail if this still doesn't work
    if(getaddrinfo(address.c_str(),NULL,NULL,&res)){
      BUException::BAD_REMOTE_IP e;
      e.Append("Addr: ");
      e.Append(address.c_str());
      e.Append(" could not be resolved.\n");
      throw e;
    }
  }
  //  readAddr = *((struct sockaddr_in *) res->ai_addr);
  //  printaddress(&readAddr);

  //Generate the sockets for read and write
  if((readSocketFD = socket(AF_INET,SOCK_DGRAM,0)) < 0){
    BUException::BAD_SOCKET e;
    e.Append("read socket\n");    
    throw e;
  }      
  if((writeSocketFD = socket(AF_INET,SOCK_DGRAM,0)) < 0){
    BUException::BAD_SOCKET e;
    e.Append("write socket\n");
    throw e;
  }      
  //Set a timeout for the recv socket so we don't hang on a reply
  struct timeval tv; tv.tv_sec=TIMEOUT_SECONDS; tv.tv_usec=TIMEOUT_MICROSECONDS;
  setsockopt(readSocketFD, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
  setsockopt(writeSocketFD, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

  //connect the read socket
  readAddr = *((struct sockaddr_in *) res->ai_addr);
  readAddr.sin_port = htons(readPort); 
  if(connect(readSocketFD,(struct sockaddr *) &readAddr,sizeof(readAddr)) < 0){
    BUException::CONNECTION_FAILED e;
    e.Append("read socket connect\n");
    e.Append(strerror(errno));
    throw e;
  }
  //connect the write socket
  writeAddr = *((struct sockaddr_in *) res->ai_addr);
  writeAddr.sin_port = htons(writePort); 
  if(connect(writeSocketFD,(struct sockaddr *) &writeAddr,sizeof(writeAddr)) < 0){
    BUException::CONNECTION_FAILED e;
    e.Append("write socket connect\n");
    e.Append(strerror(errno));
    throw e;
  }

  //Allocate the receive buffer to default size
  ResizeBuffer();
}

void BNL_UDP::WriteWithRetry(uint16_t address, uint32_t value, uint8_t retry_count){
  while(retry_count > 1){
    try{
      //Do the write
      Write(address,value);
      usleep(10);
      //if everything goes well, return
      return;
    }catch(BUException::BAD_REPLY &e){
      //eat the exception
    }
    total_retry_count++;
    retry_count--;
    usleep(10);
  }
  //Last chance we don't catch the exception and let it fall down the stack
  //Do the write
  Write(address,value);
  usleep(10);
}
void BNL_UDP::Write(uint16_t address, uint32_t value){

  //Flush this socket
  FlushSocket(writeSocketFD);

  //Build the packet to send
  //build the send packet
  WIB_packet_t packet;
  packet.key = htonl(WIB_PACKET_KEY);
  packet.reg_addr = htons(address);
  packet.data_MSW = htons(uint16_t((value >> 16) & 0xFFFF));
  packet.data_LSW = htons(uint16_t((value >>  0) & 0xFFFF));
  packet.trailer = htons(WIB_REQUEST_PACKET_TRAILER);

  //send the packet
  ssize_t send_size = sizeof(packet);
  ssize_t sent_size = 0;
  if( send_size != (sent_size = send(writeSocketFD,
				     &packet,send_size,0))){
    //bad send
    BUException::SEND_FAILED e;
    if(sent_size == -1){
      e.Append("BNL_UDP::Write(uint16_t,uint32_t)\n");
      e.Append("Errnum: ");
      e.Append(strerror(errno));
    } 
    throw e;
  }

  //If configured, capture confirmation packet
  if(writeAck ){
    ssize_t reply_size = recv(writeSocketFD,
			      buffer,buffer_size,0);

    if(-1 == reply_size){
      BUException::BAD_REPLY e;
      std::stringstream ss;
      e.Append("BNL_UDP::Write(uint16_t,uint32_t)\n");
      ss << "Errnum(" << errno << "): " << strerror(errno) << "\n";
      e.Append(ss.str().c_str());
      e.Append(dump_packet((uint8_t*) &packet,send_size).c_str());
      throw e;
    }else if( reply_size < WIB_RPLY_PACKET_SIZE){
      BUException::BAD_REPLY e;
      std::stringstream ss;
      ss << "Bad Size: " << reply_size << "\n";
      e.Append("BNL_UDP::Write(uint16_t,uint32_t)\n");
      e.Append(ss.str().c_str());
      e.Append(dump_packet(buffer,reply_size).c_str());
      throw e;
    }
    uint16_t reply_address =  uint16_t(buffer[0] << 8 | buffer[1]);
    if( reply_address != address){
      BUException::BAD_REPLY e;
      std::stringstream ss;
      ss << "Bad address: " << uint32_t(address) << " != " << uint32_t(reply_address) << "\n";
      e.Append("BNL_UDP::Write(uint16_t,uint32_t)\n");
      e.Append(ss.str().c_str());
      e.Append(dump_packet(buffer,reply_size).c_str());
      throw e;    
    }    
  }  
}
void BNL_UDP::Write(uint16_t address, std::vector<uint32_t> const & values){
  Write(address,values.data(),values.size());
}
void BNL_UDP::Write(uint16_t address, uint32_t const * values, size_t word_count){
  for(size_t iWrite = 0; iWrite < word_count;iWrite++){
    WriteWithRetry(address,values[iWrite]);
    address++;
  }

////////  //Flush the socket
////////  FlushSocket(writeSocketFD);
////////
////////  //Compute the size of this multi-write packet
////////  // values.size() - 1 gets the number of address and MSW/LSW groups that aren't already in the packet
////////  size_t packetSize = sizeof(WIB_packet_t) + (word_count-1)*6; 
////////  //resize the buffer if needed  
////////  ResizeBuffer(packetSize);
////////  
////////  //Set the packet key
////////  (*((uint32_t*) buffer)) = htonl(WIB_PACKET_KEY);
////////  //Create a pointer to 16bit words that points to the parts of buffer that are after the first 32bit word (WIB key)
////////  uint16_t * packet = (uint16_t*) (buffer + sizeof(uint32_t));
////////  for(size_t iWord = 0; iWord < word_count;iWord++){
////////    //Set the word address
////////    packet[0] = htons(address);
////////    //Set the MS 16bit part of the 32bit word
////////    packet[1] = htons(uint16_t((values[iWord] >> 16) & 0xFFFF));
////////    //Set the LS 16bit part of the 32bit word
////////    packet[2] = htons(uint16_t((values[iWord] >>  0) & 0xFFFF));
////////    //move the write address forward 1 word
////////    address++;
////////    //move the packet pointer forward to the next word block
////////    packet+=3;
////////  }
////////  //Set the packet trailer
////////  (*packet) = htons(WIB_REQUEST_PACKET_TRAILER);
////////
////////  //send the packet
////////  ssize_t send_size = packetSize;
////////  ssize_t sent_size = 0;
////////  if( send_size != (sent_size = send(writeSocketFD,
////////				     buffer,send_size,0))){
////////    //bad send
////////    BUException::SEND_FAILED e;
////////    if(sent_size == -1){
////////      e.Append("BNL_UDP::Write(uint16_t,uint32_t*,size_t)\n");
////////      e.Append("Errnum: ");
////////      e.Append(strerror(errno));
////////    } 
////////    throw e;
////////  }
////////  //If configured, capture confirmation packet
////////  if(writeAck ){
////////    ssize_t reply_size = recv(writeSocketFD,
////////			      buffer,buffer_size,0);
////////
////////
////////    //    writeAddr = readAddr;
////////    //    writeAddr.sin_port = htons(replyPort);
////////
////////    //    sockaddr_len = sizeof(writeAddr);
////////    //    ssize_t reply_size = recvfrom(recvSocketFD,
//////////    recvfrom(recvSocketFD,
//////////	     buffer,buffer_size,0,
//////////	     (struct sockaddr*)&writeAddr,&sockaddr_len);    
////////    if(-1 == reply_size){
////////      BUException::BAD_REPLY e;
////////      e.Append("BNL_UDP::Write(uint16_t,uint32_t*,size_t)\n");
////////      e.Append(strerror(errno));
////////      throw e;
////////    }else if( reply_size < WIB_RPLY_PACKET_SIZE){
////////      BUException::BAD_REPLY e;
////////      std::stringstream ss;
////////      ss << "Bad Size: " << reply_size << "\n";
////////      e.Append("BNL_UDP::Write(uint16_t,uint32_t*,size_t)\n");
////////      e.Append(ss.str().c_str());
////////      e.Append(dump_packet(buffer,reply_size).c_str());
////////      throw e;
////////    }
//////////    uint16_t reply_address =  uint16_t(buffer[0] << 8 | buffer[1]);
//////////    if( reply_address != address){
//////////      BUException::BAD_REPLY e;
//////////      std::stringstream ss;
//////////      ss << "Bad address: " << address << " != " << reply_address << "\n";
//////////      e.Append(ss.str().c_str());
//////////      throw e;    
//////////    }    
////////  }  
}

uint32_t BNL_UDP::ReadWithRetry(uint16_t address,uint8_t retry_count){
  uint32_t val;
  while(retry_count > 1){
    try{
      //Do the write
      val = Read(address);
      usleep(10);
      //if everything goes well, return
      return val;
    }catch(BUException::BAD_REPLY &e){
      //eat the exception
    }
    usleep(10);
    total_retry_count++;
    retry_count--;
  }
  //Last chance we don't catch the exception and let it fall down the stack
  val = Read(address);  
  usleep(10);
  return val;
}
uint32_t BNL_UDP::Read(uint16_t address){
  //Flush the socket
  FlushSocket(readSocketFD);

  //build the send packet
  WIB_packet_t packet;
  packet.key = htonl(WIB_PACKET_KEY);
  packet.reg_addr = htons(address);
  packet.data_MSW = packet.data_LSW = 0;
  packet.trailer = htons(WIB_REQUEST_PACKET_TRAILER);

  //send the packet
  ssize_t send_size = sizeof(packet);
  ssize_t sent_size = 0;
  if( send_size != (sent_size = send(readSocketFD,
				     &packet,send_size,0))){
    //bad send
    BUException::SEND_FAILED e;
    if(sent_size == -1){
      e.Append("BNL_UDP::Read(uint16_t)\n");
      e.Append("Errnum: ");
      e.Append(strerror(errno));
    } 
    throw e;
  }

  //Get the reply packet with the register data in it.   
  ssize_t reply_size = recv(readSocketFD,
			    buffer,buffer_size,0);
			    
  if(ssize_t(-1) == reply_size){
    BUException::BAD_REPLY e;
    std::stringstream ss;
    e.Append("BNL_UDP::Read(uint16_t)\n");
    ss << "Errnum(" << errno << "): " << strerror(errno) << "\n";
    e.Append(ss.str().c_str());
    e.Append(dump_packet((uint8_t *)&packet,send_size).c_str());
    throw e;
  }else if( reply_size < WIB_RPLY_PACKET_SIZE){
    BUException::BAD_REPLY e;
    std::stringstream ss;
    ss << "Bad Size: " << reply_size << "\n";
    e.Append("BNL_UDP::Read(uint16_t)\n");
    e.Append(ss.str().c_str());
    e.Append(dump_packet(buffer,reply_size).c_str());
    throw e;
  }
  uint16_t reply_address =  uint16_t(buffer[0] << 8 | buffer[1]);
  if( reply_address != address){
    BUException::BAD_REPLY e;
    std::stringstream ss;
    ss << "Bad address: " << uint32_t(address) << " != " << uint32_t(reply_address) << "\n";
    e.Append("BNL_UDP::Read(uint16_t)\n");
    e.Append(ss.str().c_str());
    e.Append(dump_packet(buffer,reply_size).c_str());
    throw e;    
  }
  

    //  }
  uint32_t ret = ( (uint32_t(buffer[2]) << 24) | 
		   (uint32_t(buffer[3]) << 16) | 
		   (uint32_t(buffer[4]) <<  8) | 
		   (uint32_t(buffer[5]) <<  0));
  return ret;
}


BNL_UDP::~BNL_UDP(){
  Clear();
}


void BNL_UDP::ResizeBuffer(size_t size){
  //CHeck if the requested size is larger than the already allocated size
  //  printf("before %p %zu %zd\n",buffer,buffer_size,buffer_size);
  if(buffer_size < size){
    //We need to re-allocate
    if(buffer != NULL){
      delete [] buffer;
    }
    buffer = new uint8_t[size];
    buffer_size = size;
  }
  //  printf("after %p %zu %zd\n",buffer,buffer_size,buffer_size);
}
