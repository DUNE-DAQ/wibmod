import json
import os
import rich.traceback
from rich.console import Console
from os.path import exists, join

# Add -h as default help option
CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'])

console = Console()

def generate_boot( wibapp_spec: dict) -> dict:
    '''
    Generates boot informations

    :param      wibapp_spec:  The wibapp specifications
    :type       wibapp_spec:  dict

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
                'DUNEDAQ_ERS_DEBUG_LEVEL': '1'
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
            wibapp_spec['name']: {
                'exec': 'daq_application',
                'host': 'host_wibapp',
                'port': wibapp_spec['port']
            }
        },
        'hosts': {
            'host_wibapp': wibapp_spec['host']
        },
        'response_listener': {
            'port': 56789
        },
        'exec': daq_app_specs
    }

    console.log('Boot data')
    console.log(boot)
    return boot


import click

@click.command(context_settings=CONTEXT_SETTINGS)
@click.option('-w', '--wibserver', nargs=2, multiple=True, default='LOCAL tcp://localhost:1234')
@click.argument('json_dir', type=click.Path())
def cli(wibserver, json_dir):
    '''
      JSON_DIR: Json file output folder
    '''
    console.log('Loading wibapp config generator')
    from . import wibapp_gen
    
    wibservers = {k:v for k,v in wibserver}
    
    console.log(f'Generating configs')
    cmd_data = wibapp_gen.generate(
        WIBSERVERS=wibservers
    )

    console.log('wibapp cmd data:', cmd_data)

    if exists(json_dir):
        raise RuntimeError(f'Directory {json_dir} already exists')

    data_dir = join(json_dir, 'data')
    os.makedirs(data_dir)

    app_wibapp='wibapp'

    jf_wibapp = join(data_dir, app_wibapp)

    cmd_set = ['init', 'conf', 'start', 'stop', 'pause', 'resume', 'scrap']
    for app,data in ((app_wibapp, cmd_data),):
        console.log(f'Generating {app} command data json files')
        for c in cmd_set:
            with open(f'{join(data_dir, app)}_{c}.json', 'w') as f:
                json.dump(data[c].pod(), f, indent=4, sort_keys=True)


    console.log(f'Generating top-level command json files')
    start_order = [app_wibapp]
    for c in cmd_set:
        with open(join(json_dir,f'{c}.json'), 'w') as f:
            cfg = {
                'apps': { app: f'data/{app}_{c}' for app in (app_wibapp, ) }
            }
            if c == 'start':
                cfg['order'] = start_order
            elif c == 'stop':
                cfg['order'] = start_order[::-1]

            json.dump(cfg, f, indent=4, sort_keys=True)


    console.log(f'Generating boot json file')
    with open(join(json_dir,'boot.json'), 'w') as f:
        cfg = generate_boot(
            wibapp_spec = {
                'name': app_wibapp,
                'host': 'localhost',
                'port': 3333
            },
        )
        json.dump(cfg, f, indent=4, sort_keys=True)
    console.log(f'MDAapp config generated in {json_dir}')

if __name__ == '__main__':
    try:
        cli(show_default=True, standalone_mode=True)
    except Exception as e:
        console.print_exception()
