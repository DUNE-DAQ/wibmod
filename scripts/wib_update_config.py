#!/usr/bin/env python3

import wibmod.conf_utils as wcu
import conffwk

import click

@click.command()
@click.argument('conf_db', nargs=1, type=click.Path(exists=True))
@click.option('--pulser', is_flag=True, help='Enable pulser (otherwise, disable)')
@click.option('--femb-mask', type=str,
              default='0xF', help='FEMB mask for changes (default=0xF, apply to all)')
@click.option('--pulser-dac','-p', default=20, help='Pulser DAC value (default=20)')
@click.option('--pulser-channel-mask', type=str,
              default='0xFFFF', help='Pulser channel mask (default=0xFFFF, all on)')
@click.option('--gain', '-g', default=2,
              help = 'Gain setting [14, 25, 7.8, 4.7 mV/fC]-->[0,1,2,3] (default=2, 7.8 mV/fC)')
@click.option('--peak-time', '-s', default=3,
              help = 'Peak time (shaping) setting [1, 0.5, 3, 2 us]-->[0,1,2,3] (default=3, 2 us)')

def wib_update_config(conf_db,pulser,femb_mask,pulser_dac,pulser_channel_mask,gain,peak_time):

    femb_mask = int(femb_mask,0)
    pulser_channel_mask = int(pulser_channel_mask,0)

    if(femb_mask!=0xF):
        raise click.BadParameter("FEMB mask not currently working for values != 0xF. Please rerun")

    #get the database
    db = conffwk.Configuration('oksconflibs:' + conf_db)

    #loop over all wib settings
    for wib_setting in db.get_dals('WIBSettings'):

        #do the pulser config
        if(pulser):
            wib_setting = wcu.enable_pulser_wib(wib_setting=wib_setting,
                                                pulse_dac=pulser_dac,
                                                femb_mask=femb_mask,
                                                channel_mask=pulser_channel_mask)
        else:
            wib_setting = wcu.disable_pulser_wib(wib_setting=wib_setting)

        #if gain or peak_time is set, change those too
        wib_setting = wcu.set_gain_peak_time_wib(wib_setting=wib_setting,
                                                 gain=gain,
                                                 peak_time=peak_time,
                                                 femb_mask=femb_mask)

        #update the wib setting. Recurse so FEMBSettings also get updated
        db.update_dal(wib_setting,recurse=True)

    db.commit()

if __name__ == '__main__':
    wib_update_config()