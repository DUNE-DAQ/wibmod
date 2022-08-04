# wibmod

WIB configuration and monitoring interface for DUNE's appfwk.

## Testing

A python utility is included to generate metadata to test control of multiple 
WIBs with [nanorc](https://github.com/DUNE-DAQ/nanorc). This metadata is a set
of JSON files that describe the DAQ applications (consisting of interconnected
[appfwk](https://github.com/DUNE-DAQ/appfwk) modules) and commands these 
applications (modules) can receive.

### Generate a WIB application

To build metadata for a single WIB(3) named `BLAND` with IP `192.168.1.4`:
```
wibconf_gen -w wib001 tcp://192.168.1.4:1234 wibapp
```
This will store the metadata in the folder `wibapp`. To include more WIBs,
include additional `-w [NAME] [ENDPOINT]` arguments. Where the `[NAME]` should
uniquely identify a WIB and the `[ENDPOINT]` is the ZMQ socket the WIB's 
[`wib_server`](https://github.com/DUNE-DAQ/dune-wib-firmware/tree/master/sw) is 
listening on. 

WIB1 are also supported with the `-p [NAME] [IP]` argument.
For all options: 
```
wibconf_gen -h
```

### Running with nanorc

The `wibapp` metadata can be launched using nanorc:
```
nanorc wibapp wibapp
```
The interactive run control can be used to send the `boot` command, to launch
the WIB application and initialize the WIB modules. 

Assuming the WIB(s) are powered and ready to receive configurations, send `conf`
to connect to the WIBs and load the initial settings. Sending additional 
settings to the WIBs requires sending a `settings` command, which `nanorc` does 
not currently support. A real WIB will now be streaming data out over its fibers
from all FEMBs.

To change settings sent to the WIBs, one can edit `wibapp/data/wib001_conf.json` 
by hand. In the future, some run control database will presumably build this
configuration information based on the desired detector state.

## TODOs

The `WIBConfigurator` module includes all the functionality of the `WIB2Reader`
developed for `artDAQ` except the ability to perform data readout using the WIB
spy buffer. This functionality is a simplest-working-solution to WIB control.
`ProtoWIBConfigurator` is an similarly an exact duplicate of the artDAQ 
functionality, and the `WIB` library developed by BU is included in this repo.

What follows is an (incomplete) list of potential improvements for a more
complete DAQ system.

### Power control

Slow controls will control FEMB power via an [OPC UA](https://documentation.unified-automation.com/uasdkcpp/1.7.4/html/index.html) 
server that has yet to be implemented. 

For ICEBERG and artDAQ, power control was a set of 
[external utilities](https://github.com/DUNE-DAQ/dune-wib-firmware/tree/master/sw)
which communicate directly with the `wib_server`. It is probably best to use
these utilities for now, and for debugging.

### Run configuration

Presumably the DAQ will want to change WIB/FEMB settings. `WIBConfigurator` 
implements the `settings` command, which exposes 
[all settings](schema/wibmod/wibconfigurator.jsonnet). 
Something in the DAQ needs to invoke this command with the desired arguments
when WIB settings need to be programmed.

### Calibration

`WIBConfigurator` may need a `calibrate` command to meet the calibration needs 
of the DAQ, whatever they end up being. In principle, DAQ can already turn the 
pulser on and off by sending `settings` commands with appropriate settings.

### Monitoring

`WIBConfigurator` should perhaps implement a `get_info` method to provide status
updates, if this is the route slow controls ends up going.

#### Sanity checking

`wib_server` has several version check commands (hardware and software) as well
as timing endpoint lock status, etc., which could be checked by `WIBConfigurator`
but is currently ignored. `ProtoWIBConfigurator` includes WIB and FEMB firmware
checks.

### Integration into larger DAQ system

The stand-alone WIB app is a good starting place and testing. The
run control can integratoe the WIB configuration and the overall DAQ configuration by starting nanorc with a specific top-configuration file. Example:
```
{
    "apparatus_id":"np04_coldbox",
    "np04_coldbox_wibs":"/nfs/sw/dunedaq/dunedaq-v3.1.0/configurations/np04_coldbox_wibs_2us",
    "np04_coldbox_felix_ctrl":"/nfs/sw/dunedaq/dunedaq-v3.1.0/configurations/np04_coldbox_flx_ctrl",
    "np04_coldbox_daq":"/nfs/sw/dunedaq/dunedaq-v3.1.0/configurations/np04_coldbox_daq_4ms"
}
```

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

In the DAQ framework it is possible to register commands with the states in which they can be successfully executed.
The DAQ application framework keeps track to the DAQ state (which is not the detector state!) and makes sanity checks at this level.
This is an extension to the wibmod code that may be considered.

That said, an argument could be made that `WIBConfigurator` should perhaps track
the last-programmed state and allow changes to this state, rather than requiring 
the entire WIB state be fully specified for each configuration. Such arguments
should be regarded with great caution, because `WIBConfigurator` is the wrong
place to track detector state, and will not preserve it across control software
restarts.
