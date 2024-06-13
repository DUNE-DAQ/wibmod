// This is the configuration schema for timinglibs

local moo = import "moo.jsonnet";
local nc = moo.oschema.numeric_constraints;

local stypes = import "daqconf/types.jsonnet";
local types = moo.oschema.hier(stypes).dunedaq.daqconf.types;

local sboot = import "daqconf/bootgen.jsonnet";
local bootgen = moo.oschema.hier(sboot).dunedaq.daqconf.bootgen;

local ns = "dunedaq.wibmod.confgen";
local s = moo.oschema.schema(ns);

// A temporary schema construction context.
local cs = {
  shaping_time_selector: s.number('shaping_time', dtype='i4', constraints=nc(minimum=0, maximum=3),  doc='Channel peak time selector: 1.0, 0.5, 3, 2 us (0 - 3)'),
  baseline_selector:     s.number('baseline',     dtype='i4', constraints=nc(minimum=0, maximum=2),  doc='Baseline selector: 0 (900 mV), 1 (200 mV), 2 (200 mV collection, 900 mV induction)'),
  gain_selector:         s.number('gain',         dtype='i4', constraints=nc(minimum=0, maximum=3),  doc='Channel gain selector: 14, 25, 7.8, 4.7 mV/fC (0 - 3)'),
  pulse_dac_selector:    s.number('pulse_dac',    dtype='i4', constraints=nc(minimum=0, maximum=63), doc='Pulser DAC setting [0-63]'),
  buffering_selector:    s.number('buffering',    dtype='i4', constraints=nc(minimum=0, maximum=2),  doc='0 (no buffer), 1 (se buffer), 2 (sedc buffer)'),
  line_driver_type:      s.number('line_driver_type', dtype='i4', constraints=nc(minimum=0, maximum=5),  doc='0 (Default setting), 1 (Short cable), 2 (25 m warm), 3 (35 m warm), 4 (25 m cold), 5 (35 m cold)'),
  line_driver_selector:  s.sequence('line_driver',self.line_driver_type, doc='Array of up to 2 line driver settings'),
  pulse_channel_en:      s.boolean('pulse_channel_en', doc='True (pulse channel) or False (no pulse on the channel)'),
  pulse_channels_selector: s.sequence('pulse_channels', self.pulse_channel_en, doc='Array of 16 true/false values for whether to pulse each corresponding channel in each LArASIC'),
  register_setting:      s.number('register_setting', dtype='i4', constraints=nc(minimum=0, maximum=255),  doc='Custom setting for an ASIC register (0-255)'),
  coldadc_register_selector:  s.sequence('coldadc_register_selector',self.register_setting, doc='Array of 8 ColdADC register settings (registers 0, 4, 24, 25, 26, 27, 29, 30)'),
  detector_type_selector:s.number('detector_type',dtype='i4', constraints=nc(minimum=0, maximum=3),  doc='Detector type: 0 (use WIB default), 1 (upper APA), 2 (lower APA), 3 (CRP)'),
  wib_pulser_dac_selector:      s.number('wib_pulser_dac',     dtype='i4', constraints=nc(minimum=0, maximum=65535), doc='WIB pulser DAC setting [0-65535]'),
  wib_pulser_period_selector:   s.number('wib_pulser_period',  dtype='i4', constraints=nc(minimum=0, maximum=2097151), doc='WIB pulser period in 512 ns units [0-2097151]'),
  wib_pulser_phase_selector:    s.number('wib_pulser_phase',   dtype='i4', constraints=nc(minimum=0, maximum=31), doc='WIB pulser phase in 16 ns units [0-31]'),
  wib_pulser_duration_selector: s.number('wib_pulser_duration',dtype='i4', constraints=nc(minimum=0, maximum=134217727), doc='WIB pulser duration in 16 ns units [0-134217727]'),
 
  wib: s.record('wib', [
    s.field('name',    types.string, default="", doc='server name (?)'),
    s.field('address', types.string, default="", doc='server IP (?)'),
  ], doc='a WIB configuration'),

  wiblist: s.sequence('wibs', self.wib, doc='several WIBs'),

  wibmod: s.record('wibmod', [
    s.field('wibserver',    self.wiblist,               default=[],          doc='TESTSTAND tcp://192.168.121.1:1234'),
    s.field('protowib',     self.wiblist,               default=[],          doc='TESTSTAND 192.168.121.1'),
    s.field('host_wib',     types.host,                 default='localhost', doc='Host to run the WIB sw app on'),
    s.field('gain',         self.gain_selector,         default=0,           doc='Channel gain selector: 14, 25, 7.8, 4.7 mV/fC (0 - 3)'),
    s.field('shaping_time', self.shaping_time_selector, default=3,           doc='Channel peak time selector: 1.0, 0.5, 3, 2 us (0 - 3)'),
    s.field('baseline',     self.baseline_selector,     default=2,           doc='Baseline selector: 0 (900 mV), 1 (200 mV), 2 (200 mV collection, 900 mV induction)'),
    s.field('pulse_dac',    self.pulse_dac_selector,    default=0,           doc='Pulser DAC setting [0-63]'),
    s.field('pulser',       types.flag,                 default=false,       doc="Switch to enable pulser"),
    s.field('gain_match',   types.flag,                 default=true,        doc="Switch to enable gain matching for pulser amplitude"),
    s.field('buffering',    self.buffering_selector,    default=0,           doc='0 (no buffer), 1 (se buffer), 2 (sedc buffer)'),
    s.field('line_driver',  self.line_driver_selector,  default=[0,0],       doc='Array of up to 2 values. 0 (Default setting), 1 (Short cable), 2 (25 m warm), 3 (35 m warm), 4 (25 m cold), 5 (35 m cold)'),
    s.field('pulse_channels', self.pulse_channels_selector, default=[],      doc='Array of 16 true/false values, for whether each of those channels in a LArASIC should be pulsed'),
    s.field('coldadc_registers',  self.coldadc_register_selector,  default=[], doc='Array of up to 8 values to write to registers. These correspond to (in order) registers 0, 4, 24, 25, 26, 27, 29, 30 on the ColdADC.'),
    s.field('detector_type',self.detector_type_selector,default=0,           doc='Detector type: 0 (use WIB default), 1 (upper APA), 2 (lower APA), 3 (CRP)'),
    s.field('wib_pulser',   types.flag,                 default=false,       doc="Switch to enable WIB on-board pulser"),
    s.field('wib_pulser_dac',      self.wib_pulser_dac_selector,      default=0,    doc="WIB pulser 16-bit DAC [0-65535]"),
    s.field('wib_pulser_period',   self.wib_pulser_period_selector,   default=2000, doc="WIB pulser period in 512 ns units [0-2097151]"),
    s.field('wib_pulser_phase',    self.wib_pulser_phase_selector,    default=0,    doc="WIB pulser phase in 16 ns units [0-31]"),
    s.field('wib_pulser_duration', self.wib_pulser_duration_selector, default=255,  doc="WIB pulser duration in 16 ns units [0-134217727]"),
  ]),

  wibmod_gen: s.record('wibmod_gen', [
    s.field('boot',     bootgen.boot, default=bootgen.boot, doc='Boot parameters'),
    s.field('wibmod',   self.wibmod,  default=self.wibmod,  doc='WIB conf parameters'),
  ]),
};

// Output a topologically sorted array.
stypes + sboot + moo.oschema.sort_select(cs, ns)
