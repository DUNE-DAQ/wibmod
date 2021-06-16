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
from appfwk.utils import mspec
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
        WIBSERVERS: dict = {},
        PROTOWIBS: dict = {},
):
    '''
    Here an entire application consisting only of (Proto)WIBConfigurator modules 
    is generated. 
    
    The WIBSERVERS dict should map nicknames to zmq endpoints for each WIB2 
    desired in the application. WIBConfigurator modules will be generated with 
    names derived by appending 'wib' to the nicknames. 
    
    The PROTOWIBS should similarly map nicknames to UDP IP addresses of the WIB1
    desired in the applicaiton. ProtoWIBConfigurator modules will be generated
    with named derived by appending 'protowib' to the nicknames.
    '''
    cmd_data = {}
    
    # Define queues
    
    queue_bare_specs = [
    ]
    queue_specs = app.QueueSpecs(sorted(queue_bare_specs, key=lambda x: x.inst))

    # Define modules

    mod_specs = [
        mspec(f'wib{nickname}', 'WIBConfigurator', [ ])
        for nickname in WIBSERVERS
    ] + [
        mspec(f'protowib{nickname}', 'ProtoWIBConfigurator', [ ])
        for nickname in PROTOWIBS
    ]
    
    # Define commands

    cmd_data['init'] = app.Init(queues=queue_specs, modules=mod_specs)

    cmd_data['conf'] = acmd([
        (f'wib{nickname}', wibcfg.WIBConf(
          wib_addr = endpoint
        ))
        for nickname,endpoint in WIBSERVERS.items()
    ]+[
        (f'protowib{nickname}', wibcfg.WIBConf(
          wib_addr = ip
        ))
        for nickname,ip in PROTOWIBS.items()
    ])

    cmd_data['settings'] = acmd([
        (f'wib{nickname}', wibcfg.WIBSettings(
          femb0 = wibcfg.FEMBSettings(),
          femb1 = wibcfg.FEMBSettings(),
          femb2 = wibcfg.FEMBSettings(),
          femb3 = wibcfg.FEMBSettings()
        ))
        for nickname in WIBSERVERS
    ]+[
        (f'protowib{nickname}', protowibcfg.WIBSettings(
          femb1 = protowibcfg.FEMBSettings(),
          femb2 = protowibcfg.FEMBSettings(),
          femb3 = protowibcfg.FEMBSettings(),
          femb4 = protowibcfg.FEMBSettings()
        ))
        for nickname in PROTOWIBS
    ])
    
    startpars = rccmd.StartParams(run=1)
    cmd_data['start'] = acmd([
        (f'wib{nickname}', startpars)
        for nickname in WIBSERVERS
    ]+[
        (f'protowib{nickname}', startpars)
        for nickname in PROTOWIBS
    ])

    cmd_data['stop'] = acmd([
        (f'wib{nickname}', None)
        for nickname in WIBSERVERS
    ]+[
        (f'protowib{nickname}', None)
        for nickname in PROTOWIBS
    ])
    
    cmd_data['pause'] = acmd([
    ])
    
    cmd_data['resume'] = acmd([
    ])

    cmd_data['scrap'] = acmd([
        (f'wib{nickname}', None)
        for nickname in WIBSERVERS
    ]+[
        (f'protowib{nickname}', None)
        for nickname in PROTOWIBS
    ])

    return cmd_data
