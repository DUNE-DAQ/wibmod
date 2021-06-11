#ifndef __BNL_UDP_HH__
#define __BNL_UDP_HH__

#include <string>
#include <vector>

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/ip.h>


#define WIB_RESPONSE_PACKET_BUFFER_SIZE 4048

class BNL_UDP {
public:
  BNL_UDP():readSocketFD(-1),writeSocketFD(-1),buffer_size(0),buffer(NULL),total_retry_count(0) {Clear();};
  ~BNL_UDP();

  void Setup(std::string const & address, uint16_t port_offset = 0); 
  bool Ready(){return connected;};

  void SetWriteAck(bool val){writeAck=val;};
  bool GetWriteAck(){return writeAck;};

  uint32_t ReadWithRetry(uint16_t address,uint8_t retry_count=10);
  uint32_t Read(uint16_t address);
  void WriteWithRetry(uint16_t address, uint32_t value, uint8_t retry_count=10);
  void Write(uint16_t address,uint32_t value);
  void Write(uint16_t address,std::vector<uint32_t> const & values);
  void Write(uint16_t address,uint32_t const * values, size_t word_count);

  std::string GetAddress(){return remoteAddress;};

  uint64_t GetRetryCount(){return total_retry_count;};

private:  
  // Prevent copying of BNL_UDP objects
  BNL_UDP( const BNL_UDP& other) ; // prevents construction-copy
  BNL_UDP& operator=( const BNL_UDP&) ; // prevents copying

  void FlushSocket(int sock);

  //functions
  void Clear();
  void Reset();
  void ResizeBuffer(size_t size  = WIB_RESPONSE_PACKET_BUFFER_SIZE);
  
  bool writeAck;
  
  //Network addresses
  std::string remoteAddress;  
  int16_t readPort;
  int16_t writePort;
  int16_t replyPort;

  //Network sockets and sockaddrs
  bool connected;
  int readSocketFD;
  int writeSocketFD;
  struct sockaddr_in readAddr;
  struct sockaddr_in writeAddr;

  //Packet buffer
  size_t buffer_size;
  uint8_t *buffer;
  uint64_t total_retry_count;
};
#endif
