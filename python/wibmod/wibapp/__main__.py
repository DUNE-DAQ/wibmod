import json
import os
import rich.traceback
from rich.console import Console
from os.path import exists, join

# Add -h as default help option
CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'])

console = Console()

def generate_boot( wibapp: list, host) -> dict:
    '''
    Generates boot informations

    :param      wibapp:  The wib apps
    :type       wibapp:  list

    :param      host: host to run the wib control applications
    :type       host: string
    :returns:   { description_of_the_return_value }
    :rtype:     dict
    '''

    daq_app_specs = {
        'daq_application_ups' : {
            'comment': 'Application profile based on a full dbt runtime environment',
            'env': {
               'DBT_AREA_ROOT': 'getenv' 
            },
            'cmd': [
                'CMD_FAC=rest://localhost:${APP_PORT}',
                'INFO_SVC=file://info_${APP_ID}_${APP_PORT}.json',
                'cd ${DBT_AREA_ROOT}',
                'source dbt-setup-env.sh',
                'dbt-setup-runtime-environment',
                'cd ${APP_WD}',
                'daq_application --name ${APP_ID} -c ${CMD_FAC} -i ${INFO_SVC}'
            ]
        },
        'daq_application' : {
            'comment': 'Application profile using  PATH variables (lower start time)',
            'env':{
                'CET_PLUGIN_PATH': 'getenv',
                'DUNEDAQ_SHARE_PATH': 'getenv',
                'LD_LIBRARY_PATH': 'getenv',
                'PATH': 'getenv',
                'DUNEDAQ_ERS_DEBUG_LEVEL': '1',
                'WIBMOD_SHARE': 'getenv'
            },
            'cmd': [
                'CMD_FAC=rest://localhost:${APP_PORT}',
                'INFO_SVC=file://info_${APP_NAME}_${APP_PORT}.json',
                'cd ${APP_WD}',
                'daq_application --name ${APP_NAME} -c ${CMD_FAC} -i ${INFO_SVC}'
            ]
        }
    }


    boot = {
        'env': {
            'DUNEDAQ_ERS_VERBOSITY_LEVEL': 1
        },
        'apps': {
        },
        'hosts': {
            'host_wibapp': host
        },
        'response_listener': {
            'port': 56789
        },
        'exec': daq_app_specs
    }

    port_offset = 3380
    for appname in wibapp:
        boot['apps'][appname] = {
                   'exec': 'daq_application',
                   'host': 'host_wibapp',
                   'port': port_offset
                }
        port_offset = port_offset + 1

    console.log('Boot data')
    console.log(boot)
    return boot


import click

@click.command(context_settings=CONTEXT_SETTINGS)
@click.option('-w', '--wibserver', nargs=2, multiple=True) # e.g. -w TESTSTAND tcp://192.168.121.1:1234
@click.option('-p', '--protowib', nargs=2, multiple=True) # e.g. -p TESTSTAND 192.168.121.1
@click.option('--host_wib_sw', default='localhost')
@click.argument('json_dir', type=click.Path())
def cli(wibserver, protowib, host_wib_sw, json_dir):
    '''
      JSON_DIR: Json file output folder
    '''
    console.log('Loading wibapp config generator')
    from . import wibapp_gen

    
    print(wibserver,protowib)
    wibservers = {k:v for k,v in wibserver}
    protowibs = {k:v for k,v in protowib}
    

    console.log(f'Generating configs')
    cmd_data = [wibapp_gen.generate(k, v, 1) for k,v in protowibs.items()]
    cmd_data.extend([wibapp_gen.generate(k, v, 2) for k,v in wibservers.items()])

    console.log('wibapp cmd data:', cmd_data)

    if exists(json_dir):
        raise RuntimeError(f'Directory {json_dir} already exists')

    data_dir = join(json_dir, 'data')
    os.makedirs(data_dir)

    app_wibapp=[f"ctrl_{wibname}" for wibname in protowibs]
    app_wibapp.extend([f"ctrl_{wibname}" for wibname in wibservers])

    jf_wibapp = [join(data_dir, app_wibapp[idx]) for idx in range(len(app_wibapp))]

    cmd_set = ['init', 'conf', 'settings', 'start', 'pause', 'resume', 'stop', 'scrap']

    for app,data in zip(app_wibapp, cmd_data):
        console.log(f'Generating {app} command data json files')
        for c in cmd_set:
            with open(f'{join(data_dir, app)}_{c}.json', 'w') as f:
                json.dump(data[c].pod(), f, indent=4, sort_keys=True)


    console.log(f'Generating top-level command json files')
    #start_order = [app_wibapp]
    for c in cmd_set:
        with open(join(json_dir,f'{c}.json'), 'w') as f:
            cfg = {
                'apps': { app: f'data/{app}_{c}' for app in app_wibapp }
            }
            if c in ('resume', 'pause'):
               for wapp in app_wibapp:
                  del cfg['apps'][wapp]

            json.dump(cfg, f, indent=4, sort_keys=True)


    console.log(f'Generating boot json file')
    with open(join(json_dir,'boot.json'), 'w') as f:
        cfg = generate_boot(app_wibapp, host_wib_sw)
        json.dump(cfg, f, indent=4, sort_keys=True)
    console.log(f'MDAapp config generated in {json_dir}')

if __name__ == '__main__':
    try:
        cli(show_default=True, standalone_mode=True)
    except Exception as e:
        console.print_exception()
