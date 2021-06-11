#include <WIB/WIB.hh>
#include <WIB/WIBException.hh>

void WIB::ConfigWIBFakeData(bool enableFakeFEMB1, bool enableFakeFEMB2, 
                               bool enableFakeFEMB3, bool enableFakeFEMB4, 
                               bool counter) { // counter==true: counter instead of COLDATA frame, else samples in COLDATA frame

  if(DAQMode == FELIX){
    //Don't allow fake mode on only half of a FELIX link
    if ((enableFakeFEMB1 ^ enableFakeFEMB2) || (enableFakeFEMB3 ^ enableFakeFEMB4)) {
      BUException::WIB_FAKE_DATA_ON_HALF_FELIX_LINK e;
      throw e;
    }
  }

  //Setup the FEMBs/Links
  for(size_t iFEMB = 1; iFEMB <= FEMBCount; iFEMB++){
    for(size_t iCDA = 1; iCDA <= FEMBCDACount; iCDA++){
      SetFEMBFakeCOLDATAMode(iFEMB, iCDA, counter);
    }
  }
 
  for(size_t iStream = 1; iStream <= FEMBStreamCount; iStream++){
    SetFEMBStreamSource(1, iStream, !enableFakeFEMB1);
    SetFEMBStreamSource(2, iStream, !enableFakeFEMB2);
    SetFEMBStreamSource(3, iStream, !enableFakeFEMB3);
    SetFEMBStreamSource(4, iStream, !enableFakeFEMB4);
  }

  uint64_t enableWord1 = 0;
  uint64_t enableWord2 = 0;
  uint64_t enableWord3 = 0;
  uint64_t enableWord4 = 0;
  if (enableFakeFEMB1) enableWord1 = 0xF;
  if (enableFakeFEMB2) enableWord2 = 0xF;
  if (enableFakeFEMB3) enableWord3 = 0xF;
  if (enableFakeFEMB4) enableWord4 = 0xF;
  SourceFEMB(1,enableWord1);
  SourceFEMB(2,enableWord2);
  SourceFEMB(3,enableWord3);
  SourceFEMB(4,enableWord4);
}

uint8_t WIB::GetFEMBStreamSource(uint8_t iFEMB,uint8_t iStream){
  CheckFEMBStreamInRange(iStream);
  std::string base = "FEMB0.DAQ.FAKE_CD.RX_DATA_SOURCE";
  base[4] = GetFEMBChar(iFEMB);
  //Read the current settings
  uint32_t data = Read(base);
  return (data >> (iStream -1))&0x1;
}
void WIB::SetFEMBStreamSource(uint8_t iFEMB,uint8_t iStream,bool real){
  CheckFEMBStreamInRange(iStream);
  std::string base = "FEMB0.DAQ.FAKE_CD.RX_DATA_SOURCE";
  base[4] = GetFEMBChar(iFEMB);
  //Read the current settings
  uint32_t data = Read(base);
  //update the mask
  iStream--; // iStream is 1-4, but we want bits 0 to 3
  if(real){
    data &= ~(0x1<<iStream);
  }else{
    data |= 0x1<<iStream;
  }
  Write(base,data);
}

void WIB::SetFEMBFakeCOLDATAMode(uint8_t iFEMB,uint8_t iCD, bool mode){
  std::string base("FEMB0.DAQ.FAKE_CD.CD0.");
  base[4] = GetFEMBChar(iFEMB);
  base[20] = GetFEMBCDChar(iCD);
  
  //Set this COLDATA ASIC
  Write(base+"FAKE_MODE",uint32_t(mode));
}

uint8_t WIB::GetFEMBFakeCOLDATAMode(uint8_t iFEMB,uint8_t iCD){
  std::string base("FEMB0.DAQ.FAKE_CD.CD0.");
  base[4] = GetFEMBChar(iFEMB);
  base[20] = GetFEMBCDChar(iCD);
  
  return Read(base+"FAKE_MODE");
}
