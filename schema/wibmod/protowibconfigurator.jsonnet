local moo = import "moo.jsonnet";
local ns = "dunedaq.wibmod.protowibconfigurator";
local s = moo.oschema.schema(ns);

local types = {
    address : s.string("Address", doc="A WIB1 IP address"),
    
    setting : s.string("Setting", doc="An option string"),
    
    option : s.number("Option", "u4", doc="One of several indexed options"),

    value : s.number("Value", "u4", doc="A digitally variable value"),
    
    bool : s.number("Bool", "u4", doc="1/0 for enable/disabled or true/false"),
    
    phase : s.number("Phase", "u2", doc="A 16bit clock phase value"),
    
    phases : s.sequence("Phases", self.phase, doc="A list of 16bit clock phases"),

    femb_settings: s.record("FEMBSettings", [
    
        s.field("enabled", self.bool, 0,
                doc="True of FEMB should be configured and read out by WIB"),
        
        #crying shame that jsonnet doesn't support hex literals
        s.field("expected_femb_fw_version", self.value, 803,
                doc="This must match the FEMB firmware version"),
        s.field("enable_wib_fake_data", self.bool, 1, 
                doc="If true, enables WIB fake data mode, and ignores all data from this FEMB"), 
        s.field("enable_femb_fake_data", self.bool, 1, 
                doc="If true, enables FEMB fake data mode, else real data"), 
        s.field("fake_data_select", self.setting, "fake_word",
                doc="Select the fake data type: fake_word, fake_waveform, femb_channel_id, or counter_channel_id"),
        #crying shame that jsonnet doesn't support hex literals
        s.field("fake_word", self.value, 4075,
                doc="The fake data word for fake_word mode"),
        
        s.field("gain", self.option, 2, 
                doc="FE gain value select: 0 (4.7 mV/fC), 1 (7.8 mV/fC), 2 (14 mV/fC), 3 (25 mV/fC)" ), 
        s.field("shape", self.option, 2,
                doc="FE shaping time select: 0 (0.5 us), 1 (1 us), 2 (2 us), 3 (3 us)"),
        s.field("baseline_high", self.option, 2,
                doc="FE baseline select: 0 (200 mV), 1 (900 mV), 2 (200 mV for collection and 900 mV for induction channels)"),
        s.field("leak_high", self.option, 2,
                doc="1 for 500 pA FE leakage current, 0 for 100 pA (don't need to change)"),
        s.field("leak_10x", self.bool, 0,
                doc="1 for 10x FE leakage current, 0 for 1x"),
        s.field("ac_couple", self.bool, 0,
                doc="0 (DC coupling), 1 (AC coupling)"),
        s.field("buffer", self.bool, 0,
                doc="0 (bypass), 1 (use buffer)"),
        
        s.field("ext_clk", self.option, 1,
                doc="0 to use ADC ASIC internal clocking, 1 to use external FPGA clocking"),
        #crying shame that jsonnet doesn't support hex literals
        s.field("clk_phases", self.phases, [65535,65023,61423,49087,48639],
                doc="List of 32 bit ADC FIFO clock phases, will try each phase until header bits line up"),
                
        s.field("pulse_mode", self.option, 0,
                doc="Calibration pulser mode: 0 off, 1 internal, 2 FPGA external"),
        s.field("pulse_dac", self.value, 0,
                doc="Calibration pulser amplitude: 6 bits in internal mode, 5 bits in external"),
        
        s.field("start_frame_mode", self.bool, 1,
                doc="1 to make FPGA to WIB header frame as BU WIB expects"),
        s.field("start_frame_swap", self.bool, 1,
                doc="1 to swap bytes in header frame")
                
    ], doc="FEMB settings"),

    settings: s.record("WIBSettings", [
    
        #crying shame that jsonnet doesn't support hex literals
        s.field("expected_wib_fw_version", self.value, 403252737,
                doc="This must match the WIB firmware version"),
        s.field("expected_daq_mode", self.setting, "any",
                doc="This must match the WIB firmware version: any, RCE, FELIX"),
                
        #no idea what this does, copied straight from artDAQ
        s.field("use_wib_fake_data_counter", self.bool, 0,
                doc="if false, put a counters in the DAQ data frame, if true just transmit a raw counter"),
                
        s.field("local_clock", self.bool, 0,
                doc="if true, generate the clock on the WIB, if false, use timing system clock"),
        s.field("dts_source", self.option, 0,
                doc="where to get the timing system input from: 0 back plane, 1 front panel"),
        s.field("partition_number", self.option, 0,
                doc="which timing system partion or 'timing group' to use"),
                
        s.field("femb1", self.femb_settings, doc="Configuration for FEMB in slot 1"),
        s.field("femb2", self.femb_settings, doc="Configuration for FEMB in slot 2"),
        s.field("femb3", self.femb_settings, doc="Configuration for FEMB in slot 3"),
        s.field("femb4", self.femb_settings, doc="Configuration for FEMB in slot 4"),
        
        s.field("continue_on_femb_reg_read_error", self.bool, 0,
                doc="if true, continue on to the next FEMB if you can't seem to control an FEMB instead of raising exception"),
        s.field("continue_on_femb_spi_error", self.bool, 0,
                doc="if true, continue on to the next FEMB if you can't program the FE and ADC ASICs instead of raising exception"),
        s.field("continue_on_femb_sync_error", self.bool, 0,
                doc="if true, continue on to the next FEMB if you can't get the ADC-FPGA data path sync'd instead of raising exception"),
        s.field("continue_if_close_phases_dont_sync", self.bool, 1,
                doc="if true, continue on to the next FEMB if the list of clock phases don't sync instead of raising exception"),
                
        s.field("force_full_reset", self.bool, 0,
                doc="Use full reset over checked state"),
        s.field("dnd_wait_time", self.value, 10,
                doc="Time to wait after setting do-not-disturb (seconds)"),
        s.field("configuration_tries", self.value, 10,
                doc="Number of times to try configuring before giving up"),
        s.field("start_felix_links_at_run_start", self.bool, 0,
                doc="Start FELIX links in start transition, else during configure transition"),
        s.field("stop_felix_links_at_run_stop", self.bool, 0,
                doc="Stop FELIX links in stop transition, else don't stop")
                
    ], doc="ProtoWIB system settings (argument to settings)"),
    
    conf: s.record("WIBConf", [
    
        s.field("wib_addr", self.address, "192.168.121.1",
                doc="The network address for the WIB to interact with"),
                
        s.field("wib_table", self.setting, "WIB.adt",
                doc="FEMB register map file"),
        s.field("femb_table", self.setting, "PDUNE_FEMB_323.adt",
                doc="FEMB register map file"),
                
        s.field("settings", self.settings,
                doc="The initial settings applied without an explicit settings command")
                
    ], doc="ProtoWIB module settings (argument to conf)")

};

moo.oschema.sort_select(types, ns)
