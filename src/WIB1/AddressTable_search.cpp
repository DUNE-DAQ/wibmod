#include <WIB/AddressTable.hh>
#include <fstream>
#include <WIB/AddressTableException.hh>
#include <boost/tokenizer.hpp> //tokenizer
#include <stdlib.h>  //strtoul & getenv
#include <boost/regex.hpp> //regex
#include <boost/algorithm/string/case_conv.hpp> //to_upper


std::vector<std::string> AddressTable::GetNames(){
  std::vector<std::string > names;
  for(std::map<std::string,Item*>::iterator it = nameItemMap.begin();
      it != nameItemMap.end();
      it++){
    names.push_back(it->first);
  }
  return names;
}

void ReplaceStringInPlace(std::string& subject, const std::string& search,
			  const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}
std::vector<std::string> AddressTable::GetNames(std::string const &regex){
  std::vector<std::string > names;
  //Fix regex
  std::string rx = regex;
  std::transform( rx.begin(), rx.end(), rx.begin(), ::toupper);  
  if( rx.size() > 6 && rx.substr(0,5) == "PERL:") {
    printf("Using PERL-style regex unchanged\n");
    rx = rx.substr( 5);
  } else {
    ReplaceStringInPlace( rx, ".", "#");
    ReplaceStringInPlace( rx, "*",".*");
    ReplaceStringInPlace( rx, "#","\\.");
  }
  
  //Create regex match
  boost::regex re;
  try{
    re = boost::regex(rx);
  }catch(std::exception &e){
    BUException::BAD_REGEX e2;
    e2.Append("In GetNames: (");
    e2.Append(rx.c_str());
    e2.Append(") ");
    e2.Append(regex.c_str());
    throw e2;
  }
  boost::cmatch match;
  for(std::map<std::string,Item*>::iterator it = nameItemMap.begin();
      it != nameItemMap.end();
      it++){
    if(regex_match(it->first.c_str(),match,re)){
      names.push_back(it->first);
    }
  }
  return names;
}


std::vector<std::string> AddressTable::GetAddresses(uint16_t lower,uint16_t upper){
  std::vector<std::string > names;
  //Get an iterator into our map of addresses to vectors of items that is the first entry that is not less than lower
  std::map<uint32_t,std::vector<Item*> >::iterator itAddress = addressItemMap.lower_bound(lower);
  for(;itAddress != addressItemMap.end();itAddress++){
    //loop over all following address keys

    if(itAddress->first < upper){
      //Address key is less than uppper, so add its entries to names
      std::vector<Item*> & items = itAddress->second;
      for(size_t iItem = 0; iItem < items.size();iItem++){
	names.push_back(items[iItem]->name);
      }
    }else{
      //address key is greater than or equal to upper, so stop
      break;
    }
  }
  return names;
}

std::vector<std::string> AddressTable::GetTables(std::string const &regex){
  
  //Fix regex
  std::string rx = regex;
  //  std::transform( rx.begin(), rx.end(), rx.begin(), ::toupper);  
  if( rx.size() > 6 && rx.substr(0,5) == "PERL:") {
    printf("Using PERL-style regex unchanged\n");
    rx = rx.substr( 5);
  } else {
    ReplaceStringInPlace( rx, ".", "#");
    ReplaceStringInPlace( rx, "*",".*");
    ReplaceStringInPlace( rx, "#","\\.");
  }
  
  //Create regex match
  boost::regex re;
  try{
    re = boost::regex(rx);
  }catch(std::exception &e){
    BUException::BAD_REGEX e2;
    e2.Append("In GetTables: (");
    e2.Append(rx.c_str());
    e2.Append(") ");
    e2.Append(regex.c_str());
    throw e2;
  }
  std::set<std::string> tableSearch;
  boost::cmatch match;
  for(std::map<std::string,Item*>::iterator it = nameItemMap.begin();
      it != nameItemMap.end();
      it++){
    //Check if this item has a table entry
    if(it->second->user.find("Table") != it->second->user.end()){
      std::string const & tableName = it->second->user.find("Table")->second;
      //check if this table isn't already in the set
      if(tableSearch.find(tableName) == tableSearch.end()){
	//Check if we should add it
	if(regex_match(tableName.c_str(),match,re)){
	  //Add this table to the set
	  tableSearch.insert(tableName);         
	}
      }
    }
  }
  std::vector<std::string > tables(tableSearch.begin(),tableSearch.end());
  return tables;
}

std::vector<const Item *> AddressTable::GetTagged (std::string const &tag) {
  std::vector<const Item *> matches;
  for(std::map<std::string,Item*>::iterator it = nameItemMap.begin();
      it != nameItemMap.end();
      it++){
    //Check if this item has the tag as an entry
    if(it->second->user.find(tag) != it->second->user.end()){
      matches.push_back(it->second);
    }
  }
  return matches;
}
