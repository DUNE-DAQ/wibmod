#include <WIB/WIB.hh>
#include <WIB/WIBException.hh>

std::vector<data_8b10b_t> WIB::ReadOutCDLinkSpyBuffer(){
  if(Read("FEMB_SPY.FIFO_EMPTY")){
    BUException::WIB_ERROR e;
    e.Append("CD Spy fifo is empty!");
    throw e;      
  }
  
  std::vector<data_8b10b_t> data;
  while(!Read("FEMB_SPY.FIFO_EMPTY")){
    uint32_t val = Read("FEMB_SPY.DATA");
    data.push_back( data_8b10b_t((val>>8)&0x1,uint8_t(val&0xff)));
  }
  return data;
}


std::vector<data_8b10b_t> WIB::ReadDAQLinkSpyBuffer(uint8_t iDAQLink,uint8_t trigger_mode){
  //TODO read DAQ link count
  std::string base("DAQ_LINK_");
  base.push_back(GetDAQLinkChar(iDAQLink));
  base.append(".SPY_BUFFER.");

  //Check if there is an active capture
  if(ReadWithRetry(base+"CAPTURING_DATA")){
    BUException::WIB_BUSY e;
    e.Append(base);
    e.Append(" is busy\n");
    throw e;
  }
  //The spy buffer isn't busy, so let's make sure the fifo is empty
  while(!ReadWithRetry(base+"EMPTY")){
    //Read out a workd from the fifo
    WriteWithRetry(base+"DATA",0x0);
  }
  
  //write trigger mode
  WriteWithRetry(base+"TRIGGER_MODE",trigger_mode & 0x1);
  
  //Start the capture
  Write(base+"START",0x1);

  //Wait for capture to finish
  while(ReadWithRetry(base+"CAPTURING_DATA")){
  }

  //Read out the data
  std::vector<data_8b10b_t> ret;
  
  while(!ReadWithRetry(base+"EMPTY")){
    //read out the k-chars
    uint32_t k_data = ReadWithRetry(base+"K_DATA");
    //read out the data
    uint32_t data = ReadWithRetry(base+"DATA");

    //    printf("0x%08X 0x%08X\n",k_data,data);

    for(size_t iWord = 0; iWord < 4;iWord++){
      ret.push_back( data_8b10b_t((k_data>>iWord)&0x1,
				  (data >>(iWord*8) &0xFF)));
    }   
    //mark word as read
    Write(base+"DATA",0x0);
  }
  return ret;
}
