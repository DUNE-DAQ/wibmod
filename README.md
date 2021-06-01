# wibmod

WIB configuration and monitoring interface for DUNE's appfwk.

## Testing

A python utility is included to generate metadata to test control of multiple 
WIBs with [nanorc](https://github.com/DUNE-DAQ/nanorc). This metadata is a set
of JSON files that describe the DAQ applications (consisting of interconnected
[appfwk](https://github.com/DUNE-DAQ/appfwk) modules) and commands these 
applications (modules) can receive.

### Generate a WIB application

To build metadata for a single WIB named `BLAND` with IP `192.168.1.4`:
```
python -m wibmod.wibapp.toplevel wibapp -w BLAND tcp://192.168.1.4:1234
```
This will store the metadata in the folder `wibapp`. To include more WIBs,
include additional `-w [NAME] [ENDPOINT]` arguments. Where the `[NAME]` should
uniquely identify a WIB (internally, modules are generated with names prefixed
by `wib` e.g. `wibBLAND`) and the `[ENDPOINT]` is the ZMQ socket the WIB's 
[`wib_server`](https://github.com/DUNE-DAQ/dune-wib-firmware/tree/master/sw) is 
listening on. 

### Running with nanorc

The `wibapp` metadata can be launched using nanorc:
```
nanorc/nanorc.py wibapp
```
The interactive run control can be used to send the `boot` command, to launch
the WIB application. Follow this with `init` to start the WIB modules. 

Assuming the WIB(s) are powered and ready to receive configurations, send `conf`
to load the default configuration. In a pinch, one can edit `wibapp/data/wibapp_conf.json`
by hand to change the configuration that is sent. A real WIB will now be 
streaming data out over its fibers from all FEMBs.

NOTE: the `nanorc` does not allow `conf` to be sent multiple times. `quit` and 
start a fresh session to send another configuration.

## TODOs

The `WIBConfigurator` module includes all the functionality of the `WIB2Reader`
developed for `artDAQ` except the ability to perform data readout using the WIB
spy buffer. This functionality is a simplest-working-solution to WIB control.

What follows is an (incomplete) list of potential improvements for a more
complete DAQ system.

### Power control

How will DAQ (slow controls?) turn WIBs off and on? Could implement a `power`
command in `WIBConfigurator` to send `WIBPower` messages to the `wib_server`, if
this is how DAQ envisions this happening.

For ICEBERG and artDAQ, power control was a set of 
[external utilities](https://github.com/DUNE-DAQ/dune-wib-firmware/tree/master/sw)
which communicate directly with the `wib_server`.

### Run configuration

Presumably the DAQ will want to change WIB/FEMB settings. `WIBConfigurator` 
implements the `conf` command, which exposes 
[all settings](schema/wibmod/wibconfigurator.jsonnet). 
Will `conf` be called after a `stop`? If that's possible `WIBConfigurator` is 
good to go, otherwise additional commands will be needed.

### Calibration

`WIBConfigurator` may need a `calibrate` command to meet the calibration needs 
of the DAQ, whatever they end up being. In principle, DAQ can already turn the 
pulser on and off by sending `conf` commands with appropriate settings.

### Monitoring

`WIBConfigurator` should perhaps implement a `get_info` method to provide status
updates, if this is the route slow controls ends up going.

#### Sanity checking

`wib_server` has several version check commands (hardware and software) as well
as timing endpoint lock status, etc., which could be checked by `WIBConfigurator`
but is currently ignored. 

### Integration into larger DAQ system

The stand-alone WIB app is a good starting place and testing squite. Eventually
the run control or [minidaqapp](https://github.com/DUNE-DAQ/minidaqapp) may 
want to include `WIBConfigurator` modules to add WIB control to some larger 
system. 

## Stateful vs stateless

Currently, `WIBConfigurator` and `wib_server` are nominally stateless, meaning
they will attempt to program the settings they are passed and report success or
failure. (One exception to this is power state, which is tracked by `wib_server`.)

This has two important consequences:
* There is very little logic or parameter sanity checking, making the interface 
  and control code that much simpler.
* Whoever wants to configure the WIB (DUNE DAQ/SC/CCM) has to keep track of the
  intended state. Since these entities _should be_ tracking the detector state
  anyway, this seems fine.
  
That said, an argument could be made that `WIBConfigurator` should perhaps track
the last-programmed state and allow changes to this state, rather than requiring 
the entire WIB state be fully specified for each configuration. Such arguments
should be regarded with great caution, because `WIBConfigurator` is the wrong
place to track detector state, and will not preserve it across control software
restarts.
