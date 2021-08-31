The WIB DIM server uses the tags in the WIB address tables to decide what and how to send information to DIM.


Marking an address table entry for use in DIM slow control:
	Any entry marked with the flag "slowcontrol" will be read out by the WIB DIM server and sent on to DIM.
	If no other slow control tags are specified, the entry will be cast as a unsigned 32bit integer and sent to DIM. 
	
	If the tag "sc_conv" is also found in the flags of an entry, that flag's value will be use to convert the data to a more useful form. 
	For real numbers, the flag's value will start with "linear" and for ENUM text, it will start with "enum"

	For real numbers there are two additional elements in the flag's value.   The first is "scale=number" and the second is "offset=number".
	These numbers are floating point values (read with "%le") and are used to compute the output as follows:
	      (double(reg_value) * scale) + offset
	This is then sent to DIM as a double.

	For ENUM types, additional elements are the number to string mappings.  Strings must be less than 16 characters. 
	TODO: If no enum values are listed, try pulling the values from the an existing FORMAT tag.

Table examples:

#Convert voltage ADC value into volts. (From WIB_POWER_ENTRY.adt)
V                               0x0      0x00007FFF       r  description="Voltage: (LSB = 305.18μV) " Table="FEMB_PWR" Row="_3._4" Column="_2" Status="2" slowcontrol sc_conv="linear scale=305.18e-6"
2				

#Convert DAQ link count into RCE/FELIX string. (From WIB_SYSTEM.adt)
FW_TYPE                                 0x0      0x0F000000 r  description="compiled in DAQ link count" Table="FW" Row="_2" Column="" Status="1" Format="T 4 RCE,2 FELIX" slowcontrol sc_conv="enum 2=FELIX 4=RCE"
