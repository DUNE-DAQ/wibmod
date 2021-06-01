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

# Import new types
import dunedaq.cmdlib.cmd as basecmd
import dunedaq.rcif.cmd as rccmd
import dunedaq.appfwk.cmd as cmd
import dunedaq.appfwk.app as app
import dunedaq.wibmod.wibconfigurator as wibcfg

from appfwk.utils import mcmd, mrccmd, mspec

import json
import math
from pprint import pprint


#===============================================================================
def acmd(mods: list) -> cmd.CmdObj:
    ''' 
    Helper function to create appfwk's Commands addressed to modules.
        
    :param      cmdid:  The coommand id
    :type       cmdid:  str
    :param      mods:   List of module name/data structures 
    :type       mods:   list
    
    :returns:   A constructed Command object
    :rtype:     dunedaq.appfwk.cmd.Command
    '''
    return cmd.CmdObj(
        modules=cmd.AddressedCmds(
            cmd.AddressedCmd(match=m, data=o)
            for m,o in mods
        )
    )

#===============================================================================
def generate(
        WIBSERVERS: dict = {'LOCAL':'tcp://localhost:1234'},
):
    '''
    Here an entire application consisting only of WIBConfigurator modules is 
    generated. The WIBSERVERS dict should map nicknames to hostnames for each
    WIB desired in the application. WIBConfigurator modules will be generated
    with names derived by appending 'wib' to the nicknames. 
    '''
    cmd_data = {}
    
    # Definequeues
    
    queue_bare_specs = [
    ]
    queue_specs = app.QueueSpecs(sorted(queue_bare_specs, key=lambda x: x.inst))

    # Define modules

    mod_specs = [
        mspec(f'wib{nickname}', 'WIBConfigurator', [ ])
        for nickname in WIBSERVERS
    ]
    
    # Define commands

    cmd_data['init'] = app.Init(queues=queue_specs, modules=mod_specs)

    cmd_data['conf'] = acmd([
        (f'wib{nickname}', wibcfg.WIBConf(
          wib_addr = hostname,
          femb0 = wibcfg.FEMBConf(),
          femb1 = wibcfg.FEMBConf(),
          femb2 = wibcfg.FEMBConf(),
          femb3 = wibcfg.FEMBConf()
        ))
        for nickname,hostname in WIBSERVERS.items()
    ])

    cmd_data['start'] = acmd([
        (f'wib{nickname}', None) #FIXME
        for nickname in WIBSERVERS
    ])

    cmd_data['stop'] = acmd([
        (f'wib{nickname}', None)
        for nickname in WIBSERVERS
    ])
    
    cmd_data['pause'] = acmd([
    ])
    
    cmd_data['resume'] = acmd([
    ])

    cmd_data['scrap'] = acmd([
        ('', None)
    ])

    return cmd_data
