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


from appfwk.app import App, ModuleGraph
from appfwk.daqmodule import DAQModule
from appfwk.conf_utils import Direction, Connection

#===============================================================================
def get_wib_app(nickname, 
                endpoint, 
                version,
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
                                     femb0 = wib.FEMBSettings(),
                                     femb1 = wib.FEMBSettings(),
                                     femb2 = wib.FEMBSettings(),
                                     femb3 = wib.FEMBSettings()
                                     )
                                 )
                             )]

    mgraph = ModuleGraph(modules)
    wib_app = App(modulegraph=mgraph, host=host, name=nickname)

    return wib_app
