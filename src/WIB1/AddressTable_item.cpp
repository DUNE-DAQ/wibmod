#include <WIB/AddressTable.hh>
#include <fstream>
#include <WIB/AddressTableException.hh>
#include <boost/tokenizer.hpp> //tokenizer
#include <stdlib.h>  //strtoul & getenv
#include <boost/regex.hpp> //regex
#include <boost/algorithm/string/case_conv.hpp> //to_upper

static const char *SC_key_conv = "sc_conv";

Item const * AddressTable::GetItem(std::string const & registerName){
  std::map<std::string,Item *>::iterator itNameItem = nameItemMap.find(registerName);
  if(itNameItem == nameItemMap.end()){
    BUException::INVALID_NAME e;
    e.Append("Can't find item with name \"");
    e.Append(registerName.c_str());
    e.Append("\"");
    throw e;    
  }
  Item * item = itNameItem->second;
  return item;
}

void AddressTable::AddEntry(Item * item){
  //Check for null item
  if(item == NULL){
    BUException::NULL_POINTER e;
    e.Append("Null Item pointer passed to AddEntry\n");
    throw e;
  }
  //Check for invalid Mode
  if((item->mode & Item::WRITE) &&
     (item->mode & Item::ACTION)){    
    BUException::BAD_MODE e;
    e.Append("AddEntry called with both WRITE and ACTION modes\n");
    throw e;
  }
  
  //This function now owns the memory at item


  
  //Add the item to the address map and check that it doesn't conflict with 
  std::map<uint32_t,std::vector<Item*> >::iterator itAddressItem = addressItemMap.find(item->address);
  if(itAddressItem == addressItemMap.end()){
    //This is the first entry at this address.
    //Create the entry in addressItemMap and then push_back with our item.
    addressItemMap[item->address].push_back(item);
    //Update the iterator to the newly inserted item
    itAddressItem = addressItemMap.find(item->address);
  }else{
    std::vector<Item*> & addressItems = itAddressItem->second;
    //Check each entry for an overlap in masks.  This is only ok if one is a subset, via it's name, of the other
    for(size_t iItem = 0; iItem < addressItems.size();iItem++){
      if(addressItems[iItem]->mask & item->mask){
	//free for all now


	//These items overlap in mask
	//CHeck that one or the other contains the other starting at pos 0.
	//in other words, that they are the same up until the end of the shorter one
//	if( (addressItems[iItem]->name.find(item->name) != 0) &&
//	    (item->name.find(addressItems[iItem]->name) != 0) ) {
//	  BUException::BAD_MODE e;
//	  e.Append("Entry: ");
//	  e.Append(item->name.c_str());
//	  e.Append(" conflicts with entry ");
//	  e.Append(addressItems[iItem]->name.c_str());
//	  e.Append("\n");
//	  throw e;	
//	}
      }      
    }
    addressItems.push_back(item);
  }

  //Add the item to the list of addresses
  std::map<std::string,Item *>::iterator itNameItem = nameItemMap.find(item->name);
  if(itNameItem == nameItemMap.end()){
    //Add this entry and everything is good. 
    nameItemMap[item->name] = item;
  }else{
    //There was a collision in entry name, remote the newly added element and throw an exception
    
    //delete the addition to the vector
    std::vector<Item*> & addressItems = itAddressItem->second;
    for(size_t iItem = 0; iItem < addressItems.size();iItem++){
      if(addressItems[iItem] == item){
	//Found what we just added, removing
	addressItems.erase(addressItems.begin()+iItem);
      }
    }
    // throw exception about collission
    BUException::NAME_COLLISION e;
    e.Append("Item ");
    e.Append(item->name.c_str());
    e.Append(" already existed\n");

    //delete the item
    delete item;

    throw e;	    
  }

  // Create Slow Control ItemConversion for this item

  item->sc_conv = (ItemConversion*)0x0;

  if (item->user.count(SC_key_conv)) {
    std::string convstring = item->user.at(SC_key_conv);
    item->sc_conv = ItemConversion::FromString(convstring);
    if (!item->sc_conv) {
      fprintf(stderr, "Warning: Unknown item conversion \"%s\"\n",
        convstring.c_str());
      item->sc_conv = ItemConversion::FromString("pass");
    }
  } else {
    item->sc_conv = ItemConversion::FromString("pass");
  }

  //Everything is good, we've added it
}


