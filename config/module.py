# coding: utf-8
##############################################################################
# Copyright (C) 2023 Microchip Technology Inc. and its subsidiaries.
#
# Subject to your compliance with these terms, you may use Microchip software
# and any derivatives exclusively with Microchip products. It is your
# responsibility to comply with third party license terms applicable to your
# use of third party software (including open source software) that may
# accompany Microchip software.
#
# THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
# EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
# WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
# PARTICULAR PURPOSE.
#
# IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
# INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
# WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
# BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
# FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
# ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
# THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
##############################################################################
def loadModule():
    print('Load Module: Harmony Wireless MiWi (Applications and Protocol Libraries)')

    supportedDevices = {'PIC32CX1012BZ25048',
                          'PIC32CX1012BZ25032',
                          'PIC32CX1012BZ24032',
                          'WBZ451',
                          'WBZ451H',
                          'WBZ450',
                          'WBZ351',
                          'ATSAML21J18B',
                          'ATSAMR21G18A',
                          'ATSAMD20J18A',
                          'ATSAMD21J18A',
                          'ATSAMR30G18A',
                          'ATSAMR30E18A',
                          } 
                          
    processor = Variables.get('__PROCESSOR') 
    
    print('processor={}'.format(processor))
    
    if (processor in supportedDevices):  
        # MiWi Protocol Type
        miwicomp  = Module.CreateComponent('microchipwireless', 'MiWi', 'Wireless/Drivers/', 'driver/config/drv_miwi.py')
        miwicomp.setDisplayType('MiWi Protocol Stack')
        miwicomp.addDependency('ieee802154phyDependency', 'IEEE 802.15.4 PHY', 'IEEE 802.15.4 PHY', True, True)
        miwicomp.addDependency('HarmonyCoreDependency', 'Core Service', 'Core', True, True)
        miwicomp.addDependency('SysTimeDependency', 'SYS_TIME', 'SYS_TIME', True, True)
        miwicomp.addDependency('FreeRtosDependency', 'RTOS', 'RTOS', True, True)
        miwicomp.addDependency('DeviceSupportDependency', 'Device_Support', 'Device_Support', True, True)