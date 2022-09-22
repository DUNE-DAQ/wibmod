// This is the configuration schema for timinglibs

local moo = import "moo.jsonnet";
local sdc = import "daqconf/confgen.jsonnet";
local nc = moo.oschema.numeric_constraints;
local daqconf = moo.oschema.hier(sdc).dunedaq.daqconf.confgen;

local ns = "dunedaq.wibmod.confgen";
local s = moo.oschema.schema(ns);

// A temporary schema construction context.
local cs = {
  shaping_time_selector: s.number('shaping_time', dtype='i4', constraints=nc(minimum=0, maximum=3),  doc='Channel peak time selector: 1.0, 0.5, 3, 2 us (0 - 3)'),
  baseline_selector:     s.number('baseline',     dtype='i4', constraints=nc(minimum=0, maximum=2),  doc='Baseline selector: 0 (900 mV), 1 (200 mV), 2 (200 mV collection, 900 mV induction)'),
  gain_selector:         s.number('gain',         dtype='i4', constraints=nc(minimum=0, maximum=3),  doc='Channel gain selector: 14, 25, 7.8, 4.7 mV/fC (0 - 3)'),
  pulse_dac_selector:    s.number('pulse_dac',    dtype='i4', constraints=nc(minimum=0, maximum=63), doc='Pulser DAC setting [0-63]'),
  buffering_selector:    s.number('buffering',    dtype='i4', constraints=nc(minimum=0, maximum=2),  doc='0 (no buffer), 1 (se buffer), 2 (sedc buffer)'),

  wib: s.record('wib', [
    s.field('name',    daqconf.Str, default="", doc='server name (?)'),
    s.field('address', daqconf.Str, default="", doc='server IP (?)'),
  ], doc='a WIB configuration'),

  wiblist: s.sequence('wibs', self.wib, doc='several WIBs'),

  wibmod: s.record('wibmod', [
    s.field('wibserver',    self.wiblist,               default=[],          doc='TESTSTAND tcp://192.168.121.1:1234'),
    s.field('protowib',     self.wiblist,               default=[],          doc='TESTSTAND 192.168.121.1'),
    s.field('host_wib',     daqconf.Host,               default='localhost', doc='Host to run the WIB sw app on'),
    s.field('gain',         self.gain_selector,         default=0,           doc='Channel gain selector: 14, 25, 7.8, 4.7 mV/fC (0 - 3)'),
    s.field('shaping_time', self.shaping_time_selector, default=3,           doc='Channel peak time selector: 1.0, 0.5, 3, 2 us (0 - 3)'),
    s.field('baseline',     self.baseline_selector,     default=2,           doc='Baseline selector: 0 (900 mV), 1 (200 mV), 2 (200 mV collection, 900 mV induction)'),
    s.field('pulse_dac',    self.pulse_dac_selector,    default=0,           doc='Pulser DAC setting [0-63]'),
    s.field('pulser',       daqconf.Flag,               default=false,       doc="Switch to enable pulser"),
    s.field('buffering',    self.buffering_selector,    default=0,           doc='0 (no buffer), 1 (se buffer), 2 (sedc buffer)'),
  ]),

  wibmod_gen: s.record('wibmod_gen', [
    s.field('boot',   daqconf.boot, default=daqconf.boot, doc='Boot parameters'),
    s.field('wibmod', self.wibmod,  default=self.wibmod,  doc='WIB conf parameters'),
  ]),
};

// Output a topologically sorted array.
sdc + moo.oschema.sort_select(cs, ns)