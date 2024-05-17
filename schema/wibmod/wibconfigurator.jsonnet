local moo = import "moo.jsonnet";
local ns = "dunedaq.wibmod.wibconfigurator";
local s = moo.oschema.schema(ns);

local types = {
    zmqaddr : s.string("ZMQAddress", doc="A ZeroMQ endpoint"),
    
    option : s.number("Option", "u4", doc="One of several indexed options"),

    value : s.number("Value", "u4", doc="A digitally variable value"),
    
    bool : s.boolean("Bool", doc="true/false"),

    list : s.sequence("List", self.value, doc="A list of digital values"),

    bool_list : s.sequence("BoolList", self.bool, doc="A list of boolean values"),

    coldadc_settings: s.record("ColdADCSettings", [
    	s.field("reg_0", self.value, 35, doc="Register 0: sdc_bypassed", optional=true),
	s.field("reg_4", self.value, 59, doc="Register 4: single-ended_input_mode", optional=true),
	s.field("reg_24", self.value, 223, doc="Register 24: vrefp", optional=true),
	s.field("reg_25", self.value, 51, doc="Register 25: vrefn", optional=true),
	s.field("reg_26", self.value, 137, doc="Register 26: vcmo", optional=true),	
	s.field("reg_27", self.value, 103, doc="Register 27: vcmi", optional=true),
	s.field("reg_29", self.value, 39, doc="Register 29: ibuff0_cmos", optional=true),
	s.field("reg_30", self.value, 39, doc="Register 30: ibuff1_cmos", optional=true)
    ], doc="Customized ColdADC register settings"),
	

    femb_settings: s.record("FEMBSettings", [
    
        s.field("enabled", self.bool, 1,
                doc="True of FEMB should be configured and read out by WIB"),
    
        s.field("test_cap", self.bool, false, 
                doc="Enable the test capacitor"), 
        s.field("gain", self.option, 0, 
                doc="Channel gain selector: 14, 25, 7.8, 4.7 mV/fC (0 - 3)" ), 
        s.field("peak_time", self.option, 3,
                doc="Channel peak time selector: 1.0, 0.5, 3, 2 us (0 - 3)"),
        s.field("baseline", self.option, 2,
                doc="Baseline selector: 0 (900 mV), 1 (200 mV), 2 (200 mV collection, 900 mV induction)"),
        s.field("pulse_dac", self.value, 0,
                doc="Pulser DAC setting [0-63]"),
        s.field("gain_match", self.bool, true,
                doc="Enable pulser DAC gain matching"),
                
        s.field("leak", self.option, 0,
                doc="Leak current selector: 0 (500 pA), 1 (100 pA)"),
        s.field("leak_10x", self.bool, false,
                doc="Multiply leak current by 10 if true"),
        s.field("ac_couple", self.bool, false,
                doc="false (DC coupling), true (AC coupling)"),
        s.field("buffering", self.option, 0,
                doc="0 (no buffer), 1 (se buffer), 2 (sedc buffer)"),
                
        s.field("strobe_skip", self.value, 255,
                doc="2MHz periods to skip after strobe (pulser period 0-255)"),
        s.field("strobe_delay", self.value, 255,
                doc="64MHz periods to skip after 2MHz edge for strobe (pulser offset 0-255)"),
        s.field("strobe_length", self.value, 255,
                doc="Length of strobe in 64MHz periods (pulser length 0-255)"),

	s.field("line_driver", self.list, [],
	 	doc="0 (Default), 1 (Short), 2 (25 m warm), 3 (35 m warm), 4 (25 m cold), 5 (35 m cold). Can submit up to 2 values for the two COLDATA."),

	s.field("pulse_channels", self.bool_list, [], doc="Array of up to 16 true/false values, for whether to send pulser to each corresponding channel per LArASIC.")
    ], doc="FEMB channel settings"),

    wib_pulser_settings: s.record("WIBPulserSettings", [

        s.field("enabled_0", self.bool, false,
	        doc="Enable WIB pulser for FEMB 0"),
        s.field("enabled_1", self.bool, false,
	        doc="Enable WIB pulser for FEMB 0"), 
        s.field("enabled_2", self.bool, false,
	        doc="Enable WIB pulser for FEMB 0"),
        s.field("enabled_3", self.bool, false,
	        doc="Enable WIB pulser for FEMB 0"),
	s.field("pulse_dac", self.value, 0,
		doc="WIB pulser DAC setting. 16 bits [0-65535]"),
	s.field("pulse_period", self.value, 2000,
		doc="WIB pulser period in ADC clock units (512 ns). 21 bits [0-2097151]"),
	s.field("pulse_phase", self.value, 0,
		doc="WIB pulser phase relative to digitization clock [0-31]"),
	s.field("pulse_duration", self.value, 255,
		doc="WIB pulser pulse duration in system clock units (16 ns). 27 bits [0-134217727]")
    ], doc="WIB Pulser settings"),

    settings: s.record("WIBSettings", [
  
        s.field("cold", self.bool, false,
                doc="True if the front end electronics are COLD (77k)"),
        s.field("pulser", self.bool, false,
                doc="True if the calibration pulser should be enabled"),
        s.field("detector_type", self.value, 0,
                doc="Detector type selector: WIB default (0), upper APA (1), lower APA (2), CRP (3)"),
        s.field("adc_test_pattern", self.bool, false,
                doc="True if the COLDADC test pattern should be enabled"),
                
        s.field("femb0", self.femb_settings, doc="Settings for FEMB in slot 0"),
        s.field("femb1", self.femb_settings, doc="Settings for FEMB in slot 1"),
        s.field("femb2", self.femb_settings, doc="Settings for FEMB in slot 2"),
        s.field("femb3", self.femb_settings, doc="Settings for FEMB in slot 3"),

	s.field("coldadc_settings", self.coldadc_settings, doc="Custom register settings for ColdADC", optional=true),

	s.field("wib_pulser", self.wib_pulser_settings, doc="Settings for WIB pulser")
    ], doc="WIB system settings (argument to settings)"),
    
    conf: s.record("WIBConf", [
    
        s.field("wib_addr", self.zmqaddr, "tcp://192.168.121.1:1234",
                doc="The ZeroMQ network address for the WIB to interact with"),
                
        s.field("settings", self.settings,
                doc="The initial settings applied without an explicit settings command"),
                
    ], doc="WIB module settings (argument to conf)")

};

moo.oschema.sort_select(types, ns)
