
// Implementations of Item value conversions
//
// note that these are classes declared in a source file;
// ItemConversion::FromString is the one and only interface
//   for creating conversions


DEFINE_CONVERSION_CLASS(
  pass,
(
public:
  void init (unordered_map<std::string, std::string> params) {
    (void) params; // to make compiler not complain about unused arguments
    data_size = sizeof(uint32_t);
    data_description = "i";
  }
  void Convert(uint32_t src, void *dest) {
    uint32_t *val = (uint32_t *)dest;
    *val = src;
  }
))

DEFINE_CONVERSION_CLASS(
  linear,
(
  double scale;
  double offset;
public:
  void init (unordered_map<std::string, std::string> params) {
    data_size = sizeof(double);
    data_description = "d";
    std::string scale_str = params["scale"];
    std::string offset_str = params["offset"];
    sscanf(scale_str.c_str(), "%le", &scale);
    sscanf(offset_str.c_str(), "%le", &offset);
  };
  void Convert(uint32_t src, void *dest) {
    double *val = (double *)dest;
    *val = ((double)src) * scale + offset;
  }
))

DEFINE_CONVERSION_CLASS(
  enum,
(
  unordered_map<uint32_t, std::string> names;
public:
  void init (unordered_map<std::string, std::string> params) {
    size_t length = 16; // enough for "???? 0x00000000\0"
    BOOST_FOREACH( STRIP((std::pair<std::string, std::string>)) i, params ) {
      long int number = 0;
      std::string key = i.first;
      std::string value = i.second;
      if (1 != sscanf(key.c_str(), "%li", &number)) {
        // error: unrecognized number
        continue;
      }
      names[(uint32_t)number] = value;
      size_t newlength = value.size();
      length = (newlength > length ? newlength : length);
    }
    data_size = length;
    const size_t descrlen = 16;
    char descr[descrlen];
    snprintf(descr, descrlen, "c:%zu;", length);
    data_description = std::string(descr);
  };
  void Convert(uint32_t src, void *dest) {
    char *val = (char *)dest;
    if (names.count(src)) {
      std::string name = names[src];
      // strncpy pads to end of buffer with null byte
      strncpy(val, name.c_str(), data_size);
    } else {
      // due to snprintf limitation we can't write to the last byte
      //   thus the formatted string uses 15 out of 16 characters
      int i = snprintf(val, data_size, "???? 0x%08X", src);
      // pad to end of buffer with null byte
      while (i < (int)data_size) {val[i] = '\0'; i++;}
    }
  }
))

