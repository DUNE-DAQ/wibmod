
// Implementations of Item value conversions
//
// note that these are classes declared in a source file;
// ItemConversion::FromString is the one and only interface
//   for creating conversions


#include <WIB/ItemConversion.hh>

#include <stdio.h> // sscanf
#include <boost/foreach.hpp>

using boost::unordered_map;

namespace itemconv {

#define ARGS(...) __VA_ARGS__
#define EXPAND(x) x
#define STRIP(x) EXPAND( ARGS x )
#define DEFINE_CONVERSION_CLASS(name,body)                                    \
class itemconv_##name : public ItemConversion { STRIP(body) };
/*#define DEFINE_CONVERSION_CLASS_END                                           \
public:                                                                       \
  itemconv_##name (unordered_map<std::string, std::string> params) {          \
    init(params);                                                             \
  }                                                                           \
};*/
#include "conversions-impl.hh"
#undef DEFINE_CONVERSION_CLASS
#undef STRIP
#undef EXPAND
#undef STRIP

}

ItemConversion * ItemConversion::FromString(std::string convstring) {
  unordered_map<std::string, std::string> params;
  int i = 0;
  while (i < (int)convstring.size() && convstring[i]!=' ') i++;
  std::string fnname = std::string(convstring.c_str(), i);
  while (i < (int)convstring.size()) {
    while (i < (int)convstring.size() && convstring[i]==' ') i++;
    int pair_start = i;
    while (i < (int)convstring.size() && convstring[i]!=' ') i++;
    int pair_end = i;
    int j = pair_start;
    while (j < (int)convstring.size() && convstring[j]!='=') j++;
    std::string key =
      std::string(convstring.c_str(), pair_start, j-pair_start);
    std::string value =
      std::string(convstring.c_str(), j+1, pair_end-(j+1));
    params[key]=value;
  }


//  if (fnname==#name) return new itemconv::itemconv_##name(params);

#define DEFINE_CONVERSION_CLASS(name,body)                                   \
  if (fnname==#name) {                                                       \
    itemconv::itemconv_##name *c = new itemconv::itemconv_##name();          \
    c->init(params); return c; }
#include "conversions-impl.hh"
#undef DEFINE_CONVERSION_CLASS

  return NULL;
}

