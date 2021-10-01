# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

from pprint import pprint
pprint(moo.io.default_load_path)
# Load configuration types
import moo.otypes
moo.otypes.load_types('rcif/cmd.jsonnet')
moo.otypes.load_types('appfwk/cmd.jsonnet')
moo.otypes.load_types('appfwk/app.jsonnet')

moo.otypes.load_types('wibmod/wibconfigurator.jsonnet')
from appfwk.utils import mspec, acmd
moo.otypes.load_types('wibmod/protowibconfigurator.jsonnet')

# Import new types
import dunedaq.cmdlib.cmd as basecmd
import dunedaq.rcif.cmd as rccmd
import dunedaq.appfwk.cmd as cmd
import dunedaq.appfwk.app as app
import dunedaq.wibmod.wibconfigurator as wibcfg
import dunedaq.wibmod.protowibconfigurator as protowibcfg


import json
import math

#===============================================================================
def generate(nickname, endpoint, version):
    '''
    Here an entire application consisting only of one (Proto)WIBConfigurator module is generated. 
    '''
    cmd_data = {}
    
    # Define queues
    
    #queue_bare_specs = [ ]
    #queue_specs = app.QueueSpecs(sorted(queue_bare_specs, key=lambda x: x.inst))

    # Define modules

    mod_specs = []
    if version == 1:
       mod_specs.extend([mspec(nickname, 'ProtoWIBConfigurator', [ ])])
    else:
       mod_specs.extend([mspec(nickname, 'WIBConfigurator', [ ])])
    
    # Define commands

    cmd_data['init'] = app.Init(queues=[ ], modules=mod_specs)

    if version == 1:
        cmd_data['conf'] = acmd([ 
            (nickname, protowibcfg.WIBConf(
                wib_addr = endpoint,
                settings = protowibcfg.WIBSettings( # Presumably a _real app_ would set non-default settings
                    femb1 = protowibcfg.FEMBSettings(),
                    femb2 = protowibcfg.FEMBSettings(),
                    femb3 = protowibcfg.FEMBSettings(),
                    femb4 = protowibcfg.FEMBSettings()
                ))
            )
        ])
    else:
        cmd_data['conf'] = acmd([ 
            (nickname, wibcfg.WIBConf(
                wib_addr = endpoint,
                settings = wibcfg.WIBSettings( # Presumably a _real app_ would set non-default settings
                    femb1 = wibcfg.FEMBSettings(),
                    femb2 = wibcfg.FEMBSettings(),
                    femb3 = wibcfg.FEMBSettings(),
                    femb4 = wibcfg.FEMBSettings()
                ))
            )
        ])

    if version == 1:
        cmd_data['settings'] = acmd([ 
            (nickname, protowibcfg.WIBConf(
                wib_addr = endpoint,
                settings = protowibcfg.WIBSettings( # Presumably a _real app_ would set non-default settings
                    femb1 = protowibcfg.FEMBSettings(),
                    femb2 = protowibcfg.FEMBSettings(),
                    femb3 = protowibcfg.FEMBSettings(),
                    femb4 = protowibcfg.FEMBSettings()
                ))
            )
        ])
    else:
        cmd_data['settings'] = acmd([
            (nickname, wibcfg.WIBConf(
                wib_addr = endpoint,
                settings = wibcfg.WIBSettings( # Presumably a _real app_ would set non-default settings
                    femb1 = wibcfg.FEMBSettings(),
                    femb2 = wibcfg.FEMBSettings(),
                    femb3 = wibcfg.FEMBSettings(),
                    femb4 = wibcfg.FEMBSettings()
                ))
            )
        ])
    
    startpars = rccmd.StartParams(run=1)
    cmd_data['start'] = acmd([
        (nickname, startpars)
    ])

    cmd_data['pause'] = acmd([(nickname,None)])
    cmd_data['resume'] = acmd([(nickname,None)])
    cmd_data['stop'] = acmd([(nickname,None)])
    cmd_data['scrap'] = acmd([(nickname,None)])

    return cmd_data
