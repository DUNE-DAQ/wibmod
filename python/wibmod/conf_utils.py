#function to enable calibration pulser for a given FEMBSettings configuration
def enable_pulser_femb(femb_setting,pulse_dac=15,channel_mask=0xFFFF):
    femb_setting.test_cap = True
    femb_setting.pulse_dac = pulse_dac
    femb_setting.pulse_channels = [ (channel_mask >> i_ch) & 1 == 1 for i_ch in range(16) ]
    return femb_setting

#function to disable calibration pulser for a given FEMBSettings configuration
def disable_pulser_femb(femb_setting):
    femb_setting.test_cap = False
    femb_setting.pulse_dac = 0
    femb_setting.pulse_channels = [ False ] * 16
    return femb_setting


#function to set the gain and peak time for a given FEMBSettings configuration
def set_gain_peak_time_femb(femb_setting,gain=2,peak_time=3):
    femb_setting.gain = gain
    femb_setting.peak_time = peak_time
    return femb_setting


# function to enable calibration pulser for a given WIBSettings configuration
def enable_pulser_wib(wib_setting,pulse_dac=15,femb_mask=0xF,channel_mask=0xFFFF):
    wib_setting.pulser = True

    for i_femb in range(4):
        femb_setting = getattr(wib_setting,f'femb{i_femb}')

        if (femb_mask >> i_femb) & 1 == 1:
            femb_setting = enable_pulser_femb(femb_setting=femb_setting,
                                              pulse_dac=pulse_dac,
                                              channel_mask=channel_mask)
        else:
            femb_setting = disable_pulser_femb(femb_setting=femb_setting)

        setattr(wib_setting,f'femb{i_femb}',femb_setting)

    return wib_setting


#function to disable calibration pulser across all FEMBs for a given WIBSettings configuration
def disable_pulser_wib(wib_setting):
    wib_setting.pulser = False

    for i_femb in range(4):
        femb_setting = getattr(wib_setting,f'femb{i_femb}')
        femb_setting = disable_pulser_femb(femb_setting=femb_setting)
        setattr(wib_setting,f'femb{i_femb}',femb_setting)

    return wib_setting


#function to set the gain and peak time for FEMBs in a WIBSettings configuration
def set_gain_peak_time_wib(wib_setting,gain=2,peak_time=3,femb_mask=0xF):

    for i_femb in range(4):
        if (femb_mask >> i_femb) & 1 == 0: continue
        femb_setting = getattr(wib_setting,f'femb{i_femb}')
        femb_setting = set_gain_peak_time_femb(femb_setting=femb_setting,
                                               gain=gain,
                                               peak_time=peak_time)
        setattr(wib_setting,f'femb{i_femb}',femb_setting)

    return wib_setting
