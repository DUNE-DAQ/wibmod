local moo = import "moo.jsonnet";
local ns = "dunedaq.wibmod.wibconfigurator";
local s = moo.oschema.schema(ns);

local types = {
    zmqaddr : s.string("ZMQAddress", doc="A ZeroMQ endpoint"),
    
    option : s.number("Option", "u4", doc="One of several indexed options"),

    value : s.number("Value", "u4", doc="A digitally variable value"),
    
    bool : s.boolean("Bool", doc="true/false"),

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
                doc="Length of strobe in 64MHz periods (pulser length 0-255)")
                
    ], doc="FEMB channel settings"),

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
        s.field("femb3", self.femb_settings, doc="Settings for FEMB in slot 3")
        
    ], doc="WIB system settings (argument to settings)"),
    
    conf: s.record("WIBConf", [
    
        s.field("wib_addr", self.zmqaddr, "tcp://192.168.121.1:1234",
                doc="The ZeroMQ network address for the WIB to interact with"),
                
        s.field("settings", self.settings,
                doc="The initial settings applied without an explicit settings command"),
                
    ], doc="WIB module settings (argument to conf)")

};

moo.oschema.sort_select(types, ns)
