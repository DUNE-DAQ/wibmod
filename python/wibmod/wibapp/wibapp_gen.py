# This module facilitates the generation of WIB modules within WIB apps

# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

# Load configuration types
import moo.otypes

moo.otypes.load_types('wibmod/wibconfigurator.jsonnet')
moo.otypes.load_types('wibmod/protowibconfigurator.jsonnet')

# Import new types
import dunedaq.wibmod.wibconfigurator as wib
import dunedaq.wibmod.protowibconfigurator as protowib


from daqconf.core.app import App, ModuleGraph
from daqconf.core.daqmodule import DAQModule

#===============================================================================
def get_wib_app(nickname, 
                endpoint, 
                version, gain, shaping_time, baseline, pulse_dac, pulser, buf,
                host="localhost"):
    '''
    Here an entire application consisting only of one (Proto)WIBConfigurator module is generated. 
    '''

    # Define modules

    modules = []

    if version == 1:
       modules += [DAQModule(name = nickname, 
                             plugin = 'ProtoWIBConfigurator',
                             conf = protowib.WIBConf(wib_addr = endpoint,
                                 settings = protowib.WIBSettings(
                                     femb1 = protowib.FEMBSettings(),
                                     femb2 = protowib.FEMBSettings(),
                                     femb3 = protowib.FEMBSettings(),
                                     femb4 = protowib.FEMBSettings()
                                     )
                                 )
                             )]
    else:
        modules += [DAQModule(name = nickname,
                             plugin = 'WIBConfigurator',
                             conf = wib.WIBConf(wib_addr = endpoint,
                                 settings = wib.WIBSettings(
                                     pulser = pulser,
                                     femb0 = wib.FEMBSettings(gain=gain, peak_time=shaping_time, baseline=baseline, pulse_dac=pulse_dac, buffering=buf, test_cap=pulser),
                                     femb1 = wib.FEMBSettings(gain=gain, peak_time=shaping_time, baseline=baseline, pulse_dac=pulse_dac, buffering=buf, test_cap=pulser),
                                     femb2 = wib.FEMBSettings(gain=gain, peak_time=shaping_time, baseline=baseline, pulse_dac=pulse_dac, buffering=buf, test_cap=pulser),
                                     femb3 = wib.FEMBSettings(gain=gain, peak_time=shaping_time, baseline=baseline, pulse_dac=pulse_dac, buffering=buf, test_cap=pulser)
                                     )
                                 )
                             )]

    mgraph = ModuleGraph(modules)
    wib_app = App(modulegraph=mgraph, host=host, name=nickname)

    return wib_app
