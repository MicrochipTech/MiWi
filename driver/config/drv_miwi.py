# coding: utf-8
##############################################################################
# Copyright (C) 2024 Microchip Technology Inc. and its subsidiaries.
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
###############################################################################

#-------------------------------------------------------------------------------
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ IEEE 802.15.4 MAC CONFIGURATIONS ~~~~~~~~~~~~~~~~
#-------------------------------------------------------------------------------

def instantiateComponent(miwicomp):
    print("MIWI driver component")
    configName = Variables.get("__CONFIGURATION_NAME")
    print(configName)
    # === Activate required components automatically
    global requiredComponents
    requiredComponents = [
        "IEEE_802154_PHY"
    ]

    res = Database.activateComponents(requiredComponents)
    
    # === Interfaces
    # MiWi Protocol Type

    global miwiProtocolConfig
    miwiProtocolConfig = miwicomp.createMenuSymbol("MIWI_PROTOCOL_CONFIG", None)
    miwiProtocolConfig.setLabel("MiWi Protocol Type Configuration")
    miwiProtocolConfig.setVisible(True)
    
    global miwiProtocolType
    miwiProtocolType = miwicomp.createComboSymbol("MIWI_PROTOCOL_TYPE", miwiProtocolConfig, ["MIWI_MESH", "MIWI_PEER2PEER", "MIWI_STAR"])
    miwiProtocolType.setLabel("MiWi Protocol Type")
    miwiProtocolType.setDefaultValue("")
    miwiProtocolType.setVisible(True)
    # print(miwicomp.getID())
    miwiProtocolType.setDefaultValue("MIWI_MESH")
    miwiProtocolType.setDependencies(setEnableMiWiProtocol, ["MIWI_PROTOCOL_TYPE"])

    global mimeshConfHeader
    mimeshConfHeader = miwicomp.createFileSymbol("MIWI_MESH_CONF_HEADER", None)
    mimeshConfHeader.setSourcePath("/driver/templates/miwi/miwi_mesh/miwi_config_mesh.h.ftl")
    mimeshConfHeader.setOutputName("miwi_config_mesh.h")
    mimeshConfHeader.setDestPath('MiWi/MiWi_Mesh/inc/')
    mimeshConfHeader.setProjectPath('config/default/MiWi/MiWi_Mesh/inc/')
    mimeshConfHeader.setType("HEADER")
    # mimeshConfHeader.setEnabled(False)
    mimeshConfHeader.setEnabled(True)
    mimeshConfHeader.setOverwrite(True)
    mimeshConfHeader.setMarkup(True)

    global mip2pConfHeader
    mip2pConfHeader = miwicomp.createFileSymbol("MIWI_P2P_CONF_HEADER", None)
    mip2pConfHeader.setSourcePath("/driver/templates/miwi/miwi_p2p/miwi_config_p2p.h.ftl")
    mip2pConfHeader.setOutputName("miwi_config_p2p.h")
    mip2pConfHeader.setDestPath('MiWi/MiWi_P2P/inc/')
    mip2pConfHeader.setProjectPath('config/default/MiWi/MiWi_P2P/inc/')
    mip2pConfHeader.setType("HEADER")
    mip2pConfHeader.setEnabled(False)

    global mistarConfHeader
    mistarConfHeader = miwicomp.createFileSymbol("MIWI_STAR_CONF_HEADER", None)
    mistarConfHeader.setSourcePath("/driver/templates/miwi/miwi_star/miwi_config_p2p.h.ftl")
    mistarConfHeader.setOutputName("miwi_config_p2p.h")
    mistarConfHeader.setDestPath('MiWi/MiWi_Star/inc/')
    mistarConfHeader.setProjectPath('config/default/MiWi/MiWi_Star/inc/')
    mistarConfHeader.setType("HEADER")
    mistarConfHeader.setEnabled(False)

    global SourceFileMesh
    SourceFileMesh = miwicomp.createFileSymbol(None, None)
    SourceFileMesh.setSourcePath('/driver/templates/miwi/miwi_mesh/app.c.ftl')
    SourceFileMesh.setOutputName('app.c')
    SourceFileMesh.setDestPath('../../')
    SourceFileMesh.setProjectPath('')
    SourceFileMesh.setType('SOURCE')
    SourceFileMesh.setOverwrite(True)
    SourceFileMesh.setEnabled(True)
    SourceFileMesh.setMarkup(True)
    
    global SourceFileP2P
    SourceFileP2P = miwicomp.createFileSymbol(None, None)
    SourceFileP2P.setSourcePath('/driver/templates/miwi/miwi_p2p/app.c.ftl')
    SourceFileP2P.setOutputName('app.c')
    SourceFileP2P.setDestPath('../../')
    SourceFileP2P.setProjectPath('')
    SourceFileP2P.setType('SOURCE')
    SourceFileP2P.setEnabled(False)

    global SourceFileStar
    SourceFileStar = miwicomp.createFileSymbol(None, None)
    SourceFileStar.setSourcePath('/driver/templates/miwi/miwi_star/app.c.ftl')
    SourceFileStar.setOutputName('app.c')
    SourceFileStar.setDestPath('../../')
    SourceFileStar.setProjectPath('')
    SourceFileStar.setType('SOURCE')
    SourceFileStar.setEnabled(False)

    global preprocessorCompilerMesh
    preprocessorCompilerMesh = miwicomp.createSettingSymbol("MIWIMESH_XC32_PREPROCESSOR", None)
    preprocessorCompilerMesh.setCategory("C32")
    preprocessorCompilerMesh.setKey("preprocessor-macros")
    # preprocessorCompilerMesh.setValue("PROTOCOL_MESH;PAN_COORDINATOR;")

    global preprocessorCompilerP2P
    preprocessorCompilerP2P = miwicomp.createSettingSymbol("MIWIP2P_XC32_PREPROCESSOR", None)
    preprocessorCompilerP2P.setCategory("C32")
    preprocessorCompilerP2P.setKey("preprocessor-macros")
    # preprocessorCompilerP2P.setValue("PROTOCOL_P2P;")

    global preprocessorCompilerStar
    preprocessorCompilerStar = miwicomp.createSettingSymbol("MIWISTAR_XC32_PREPROCESSOR", None)
    preprocessorCompilerStar.setCategory("C32")
    preprocessorCompilerStar.setKey("preprocessor-macros")
    # preprocessorCompilerStar.setValue("PROTOCOL_STAR;")

    global preprocessorCompilerP2PStar
    preprocessorCompilerP2PStar = miwicomp.createSettingSymbol("MIWIP2PSTAR_XC32_PREPROCESSOR", None)
    preprocessorCompilerP2PStar.setCategory("C32")
    preprocessorCompilerP2PStar.setKey("preprocessor-macros")
    # preprocessorCompilerP2PStar.setValue("ENABLE_QUEUE_CAPACITY;ENABLE_LARGE_BUFFER;PIC32CXBZ_SOC;PROTOCOL_STAR;ENABLE_NETWORK_FREEZER;")


    global checkProtocolTypeMesh
    global condProtocolMesh
    checkProtocolTypeMesh    =  (miwiProtocolType.getValue() == 'MIWI_MESH')
    condProtocolMesh         =  [checkProtocolTypeMesh, setEnableMiWiMeshProtocol, ['MIWI_PROTOCOL_TYPE']]
    conditionAlwaysInclude = [True, None, []]

    global checkProtocolTypeP2P
    global condProtocolP2P
    checkProtocolTypeP2P    =  ((miwiProtocolType.getValue() == 'MIWI_PEER2PEER'))
    condProtocolP2P         =  [checkProtocolTypeP2P, setEnableMiWiP2PProtocol, ['MIWI_PROTOCOL_TYPE']]
    
    global checkProtocolTypeStar
    global condProtocolStar
    checkProtocolTypeStar    =  (miwiProtocolType.getValue() == 'MIWI_STAR')
    condProtocolStar         =  [checkProtocolTypeStar, setEnableMiWiStarProtocol, ['MIWI_PROTOCOL_TYPE']]
    
    global checkProtocolTypeP2PStar
    global condProtocolP2PStar
    checkProtocolTypeP2PStar = ((miwiProtocolType.getValue() == 'MIWI_PEER2PEER') or (miwiProtocolType.getValue() == 'MIWI_STAR'))
    condProtocolP2PStar         =  [condProtocolStar or condProtocolP2P]

    global appMiWiMeshMenuSymbol
    appMiWiMeshMenuSymbol  = miwicomp.createMenuSymbol("MIWI_MESH_MENU_SYMBOL",None)
    appMiWiMeshMenuSymbol.setLabel("MiWi Mesh Configuration")
    appMiWiMeshMenuSymbol.setVisible(True)

    global appMiWiP2PMenuSymbol
    appMiWiP2PMenuSymbol  = miwicomp.createMenuSymbol("MIWI_P2P_MENU_SYMBOL",None)
    appMiWiP2PMenuSymbol.setLabel("MiWi P2P Configuration")
    appMiWiP2PMenuSymbol.setVisible(False)

    global appMiWiStarMenuSymbol
    appMiWiStarMenuSymbol  = miwicomp.createMenuSymbol("MIWI_STAR_MENU_SYMBOL",None)
    appMiWiStarMenuSymbol.setLabel("MiWi Star Configuration")
    appMiWiStarMenuSymbol.setVisible(False)

    global appMiWiP2PStarMenuSymbol
    appMiWiP2PStarMenuSymbol  = miwicomp.createMenuSymbol("MIWI_P2P_STAR_MENU_SYMBOL",None)
    appMiWiP2PStarMenuSymbol.setLabel("MiWi P2P Star Configuration")
    appMiWiP2PStarMenuSymbol.setVisible(False)
    # === Radio menu
    # execfile(Module.getPath() + "/config/miwi/drv_miwi_mesh_config.py")

# ========== Device type Configuration for MAC ====================
    global miwiDeviceConfig
    miwiDeviceConfig = miwicomp.createMenuSymbol("MIWI_DEVICE_CONFIG", appMiWiMeshMenuSymbol)
    miwiDeviceConfig.setLabel("MiWi Device Type Configuration")
    miwiDeviceConfig.setVisible(True)

    global miwiDeviceType
    miwiDeviceType = miwicomp.createKeyValueSetSymbol("MESH_DEVICE_TYPE", miwiDeviceConfig)
    miwiDeviceType.setLabel("MiWi Device Type")
    miwiDeviceType.addKey("PAN_COORDINATOR", "PAN_COORDINATOR", "PAN_COORDINATOR")
    miwiDeviceType.addKey("COORDINATOR", "COORDINATOR", "COORDINATOR")
    miwiDeviceType.addKey("ENDDEVICE", "ENDDEVICE", "ENDDEVICE")
    miwiDeviceType.setDefaultValue(0)
    miwiDeviceType.setOutputMode("Value")
    miwiDeviceType.setDisplayMode("Description")
    miwiDeviceType.setDescription("MiWi Protocol Device Type")
    miwiDeviceType.setDependencies(DeviceTypeConfiguration, ["MESH_DEVICE_TYPE"])
    print("miwi dev type")
    print(miwiDeviceType.getValue())

    global miwiEDType
    miwiEDType = miwicomp.createKeyValueSetSymbol("MIWI_MESH_END_DEVICE_TYPE", appMiWiMeshMenuSymbol)
    miwiEDType.setLabel("MiWi Mesh End Device Type")
    miwiEDType.addKey("SLEEPING_ENDDEVICE", "SLEEPING_ENDDEVICE", "SLEEPING_ENDDEVICE")
    miwiEDType.addKey("NONSLEEPING_ENDDEVICE", "NONSLEEPING_ENDDEVICE", "NONSLEEPING_ENDDEVICE")
    miwiEDType.setDefaultValue(0)
    miwiEDType.setOutputMode("Value")
    miwiEDType.setDisplayMode("Description")
    miwiEDType.setDescription("MiWi End Device Type")
    miwiEDType.setVisible(False)
    miwiEDType.setDependencies(edConfig, ["MIWI_MESH_END_DEVICE_TYPE"])
# ========== Sleep Configuration =====================================

    global DeepSleepEnable
    DeepSleepEnable = miwicomp.createBooleanSymbol('DEEP_SLEEP_ENABLE', None)
    DeepSleepEnable.setLabel('Enable Deep Sleep')
    DeepSleepEnable.setDefaultValue(False)
    DeepSleepEnable.setDependencies(SleepConfiguration, ["DEEP_SLEEP_ENABLE"])

# ========== Security Configuration for MAC ===========================

    global miwiMeshSecurityConfig
    miwiMeshSecurityConfig = miwicomp.createMenuSymbol("MIWI_MESH_SECURITY_CONFIG", appMiWiMeshMenuSymbol)
    miwiMeshSecurityConfig.setLabel("Security Configuration")
    miwiMeshSecurityConfig.setVisible(True)

    global SecurityMeshConfig
    SecurityMeshConfig = miwicomp.createKeyValueSetSymbol("MIWI_MESH_SECURITY_OPTION", miwiMeshSecurityConfig)
    SecurityMeshConfig.setLabel("MIWI Security Config")
    SecurityMeshConfig.addKey("DISABLED", "0", "DISABLED")
    SecurityMeshConfig.addKey("ENABLED", "1", "ENABLED")
    SecurityMeshConfig.setDefaultValue(0)
    SecurityMeshConfig.setOutputMode("Value")
    SecurityMeshConfig.setDisplayMode("Description")
    SecurityMeshConfig.setDescription("MiWi Security option of the application")
    SecurityMeshConfig.setDependencies(SecurityMeshConfiguration,["MIWI_MESH_SECURITY_OPTION"])

    condMiWiMeshSecurity = [False, SecurityFilesMeshConfig, ['MIWI_MESH_SECURITY_OPTION']]

    global miwiP2PSecurityConfig
    miwiP2PSecurityConfig = miwicomp.createMenuSymbol("MIWI_P2P_SECURITY_CONFIG", appMiWiP2PMenuSymbol)
    miwiP2PSecurityConfig.setLabel("Security Configuration")
    miwiP2PSecurityConfig.setVisible(True)

    global SecurityP2PConfig
    SecurityP2PConfig = miwicomp.createKeyValueSetSymbol("MIWI_P2P_SECURITY_OPTION", miwiP2PSecurityConfig)
    SecurityP2PConfig.setLabel("MIWI Security Config")
    SecurityP2PConfig.addKey("DISABLED", "0", "DISABLED")
    SecurityP2PConfig.addKey("ENABLED", "1", "ENABLED")
    SecurityP2PConfig.setDefaultValue(0)
    SecurityP2PConfig.setOutputMode("Value")
    SecurityP2PConfig.setDisplayMode("Description")
    SecurityP2PConfig.setDescription("MiWi Security option of the application")
    SecurityP2PConfig.setDependencies(SecurityP2PStarConfiguration,["MIWI_P2P_SECURITY_OPTION"])

    condMiWiP2PSecurity = [False, SecurityFilesP2PStarConfig, ['MIWI_P2P_SECURITY_OPTION']]

    global miwiStarSecurityConfig
    miwiStarSecurityConfig = miwicomp.createMenuSymbol("MIWI_STAR_SECURITY_CONFIG", appMiWiStarMenuSymbol)
    miwiStarSecurityConfig.setLabel("Security Configuration")
    miwiStarSecurityConfig.setVisible(True)

    global SecurityStarConfig
    SecurityStarConfig = miwicomp.createKeyValueSetSymbol("MIWI_STAR_SECURITY_OPTION", miwiStarSecurityConfig)
    SecurityStarConfig.setLabel("MIWI Security Config")
    SecurityStarConfig.addKey("DISABLED", "0", "DISABLED")
    SecurityStarConfig.addKey("ENABLED", "1", "ENABLED")
    SecurityStarConfig.setDefaultValue(0)
    SecurityStarConfig.setOutputMode("Value")
    SecurityStarConfig.setDisplayMode("Description")
    SecurityStarConfig.setDescription("MiWi Security option of the application")
    SecurityStarConfig.setDependencies(SecurityP2PStarConfiguration,["MIWI_STAR_SECURITY_OPTION"])

    condMiWiStarSecurity = [False, SecurityFilesP2PStarConfig, ['MIWI_STAR_SECURITY_OPTION']]

    # Configuration selection to use channel or channel mask
    global appConfigUseChannel
    appConfigUseChannel = miwicomp.createComboSymbol("USE_CHANNEL", None,  ["YES", "NO"])
    appConfigUseChannel.setDefaultValue("YES")
    appConfigUseChannel.setLabel("Use Channel instead of Mask")

    # Channel 
    appConfigChannel = miwicomp.createIntegerSymbol("CHANNEL", None)
    appConfigChannel.setLabel("Channel")
    appConfigChannel.setDefaultValue(25)
    appConfigChannel.setMin(11)
    appConfigChannel.setMax(26)
    appConfigChannel.setVisible(appConfigUseChannel.getValue() == "YES")
    appConfigChannel.setDependencies(appChannelTypeCheckChannel, ["USE_CHANNEL"])

    # Channel Map
    appConfigChannelMap = miwicomp.createHexSymbol("CHANNEL_MAP", None)
    appConfigChannelMap.setLabel("Channel Map")
    appConfigChannelMap.setDefaultValue(0x2108800)
    appConfigChannelMap.setMin(0)
    appConfigChannelMap.setMax(0xFFFFFFFF)
    appConfigChannelMap.setVisible(appConfigUseChannel.getValue() == "NO")
    appConfigChannelMap.setDependencies(appChannelTypeCheckMask, ["USE_CHANNEL"])

    meshConfigNumofCoordinators = miwicomp.createIntegerSymbol("NUM_OF_COORDINATORS", appMiWiMeshMenuSymbol)
    meshConfigNumofCoordinators.setLabel("Number of Coordinators")
    meshConfigNumofCoordinators.setDefaultValue(64)
    meshConfigNumofCoordinators.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigNumofCoordinators.setMin(10)
    meshConfigNumofCoordinators.setMax(199)
    meshConfigNumofCoordinators.setDependencies(checkMeshDevicePanCCoordV, ["NUM_OF_COORDINATORS"])

    meshConfigNumofNonSleepingEndevices = miwicomp.createIntegerSymbol("NUM_OF_NONSLEEPING_ENDDEVICES", appMiWiMeshMenuSymbol)
    meshConfigNumofNonSleepingEndevices.setLabel("Number of non-sleeping enddevices")
    meshConfigNumofNonSleepingEndevices.setDefaultValue(5)
    meshConfigNumofNonSleepingEndevices.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigNumofNonSleepingEndevices.setMin(1)
    meshConfigNumofNonSleepingEndevices.setMax(15)
    meshConfigNumofNonSleepingEndevices.setDependencies(checkMeshDevicePanCCoordV, ["NUM_OF_NONSLEEPING_ENDDEVICES"])

    meshConfigNumofSleepingEndevices = miwicomp.createIntegerSymbol("NUM_OF_SLEEPING_ENDDEVICES", appMiWiMeshMenuSymbol)
    meshConfigNumofSleepingEndevices.setLabel("Number of sleeping enddevices")
    meshConfigNumofSleepingEndevices.setDefaultValue(5)
    meshConfigNumofSleepingEndevices.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigNumofSleepingEndevices.setMin(1)
    meshConfigNumofSleepingEndevices.setMax(15)
    meshConfigNumofSleepingEndevices.setDependencies(checkMeshDevicePanCCoordV, ["NUM_OF_SLEEPING_ENDDEVICES"])

    meshConfigKeepAliveCoordinatorSendInterval = miwicomp.createIntegerSymbol("KEEP_ALIVE_COORDINATOR_SEND_INTERVAL", appMiWiMeshMenuSymbol)
    meshConfigKeepAliveCoordinatorSendInterval.setLabel("Keep Alive Coordinator Send Interval")
    meshConfigKeepAliveCoordinatorSendInterval.setDefaultValue(120)
    meshConfigKeepAliveCoordinatorSendInterval.setMin(10)
    meshConfigKeepAliveCoordinatorSendInterval.setMax(255)
    meshConfigKeepAliveCoordinatorSendInterval.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigKeepAliveCoordinatorSendInterval.setDependencies(checkMeshDevicePanCCoordV, ["KEEP_ALIVE_COORDINATOR_SEND_INTERVAL"])

    meshConfigKeepAliveRxOnEDSendInterval = miwicomp.createIntegerSymbol("KEEP_ALIVE_RXONENDDEVICE_SEND_INTERVAL", appMiWiMeshMenuSymbol)
    meshConfigKeepAliveRxOnEDSendInterval.setLabel("Keep Alive Rx On End device Send Interval")
    meshConfigKeepAliveRxOnEDSendInterval.setDefaultValue(120)
    meshConfigKeepAliveRxOnEDSendInterval.setMin(10)
    meshConfigKeepAliveRxOnEDSendInterval.setMax(255)
    meshConfigKeepAliveRxOnEDSendInterval.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigKeepAliveRxOnEDSendInterval.setDependencies(checkMeshDevicePanCCoordV, ["KEEP_ALIVE_RXONENDDEVICE_SEND_INTERVAL"])

    meshConfigDataRequestSendInterval = miwicomp.createIntegerSymbol("DATA_REQUEST_SEND_INTERVAL", appMiWiMeshMenuSymbol)
    meshConfigDataRequestSendInterval.setLabel("Data Request Send Interval")
    meshConfigDataRequestSendInterval.setDefaultValue(3)
    meshConfigDataRequestSendInterval.setMin(3)
    meshConfigDataRequestSendInterval.setMax(255)
    meshConfigDataRequestSendInterval.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 2))
    meshConfigDataRequestSendInterval.setDependencies(checkMeshDeviceEdV, ["DATA_REQUEST_SEND_INTERVAL"])

    meshConfigInDirectDatawaitInterval = miwicomp.createIntegerSymbol("INDIRECT_DATA_WAIT_INTERVAL", appMiWiMeshMenuSymbol)
    meshConfigInDirectDatawaitInterval.setLabel("Indirect Data Wait Interval")
    meshConfigInDirectDatawaitInterval.setDefaultValue(3)
    meshConfigInDirectDatawaitInterval.setMin(3)
    meshConfigInDirectDatawaitInterval.setMax(255)
    meshConfigInDirectDatawaitInterval.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigInDirectDatawaitInterval.setDependencies(checkMeshDevicePanCCoordV, ["INDIRECT_DATA_WAIT_INTERVAL"])

    meshConfigRoleUpgradeInterval = miwicomp.createIntegerSymbol("ROLE_UPGRADE_INTERVAL_IN_SEC", appMiWiMeshMenuSymbol)
    meshConfigRoleUpgradeInterval.setLabel("Role Upgrade Interval")
    meshConfigRoleUpgradeInterval.setDefaultValue(25)
    meshConfigRoleUpgradeInterval.setMin(5)
    meshConfigRoleUpgradeInterval.setMax(255)
    meshConfigRoleUpgradeInterval.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigRoleUpgradeInterval.setDependencies(checkMeshDevicePanCCoordV, ["ROLE_UPGRADE_INTERVAL_IN_SEC"])

    meshConfigRoleUpgradeInterval = miwicomp.createIntegerSymbol("CONNECTION_RESPONSE_WAIT_IN_SEC", appMiWiMeshMenuSymbol)
    meshConfigRoleUpgradeInterval.setLabel("Connection Response Interval")
    meshConfigRoleUpgradeInterval.setDefaultValue(25)
    meshConfigRoleUpgradeInterval.setMin(1)
    meshConfigRoleUpgradeInterval.setMax(255)
    meshConfigRoleUpgradeInterval.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 1) or (miwiDeviceType.getValue() == 2))
    meshConfigRoleUpgradeInterval.setDependencies(checkMeshDeviceCoordCEdV, ["MESH_DEVICE_TYPE"])
    
    meshConfigRouteUpdateInterval = miwicomp.createIntegerSymbol("ROUTE_UPDATE_INTERVAL", appMiWiMeshMenuSymbol)
    meshConfigRouteUpdateInterval.setLabel("Route Update Interval")
    meshConfigRouteUpdateInterval.setDefaultValue(60)
    meshConfigRouteUpdateInterval.setMin(20)
    meshConfigRouteUpdateInterval.setMax(255)
    meshConfigRouteUpdateInterval.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigRouteUpdateInterval.setDependencies(checkMeshDevicePanCCoordV, ["ROUTE_UPDATE_INTERVAL"])
 
    global meshEnableConsole
    meshEnableConsole = miwicomp.createBooleanSymbol("ENABLE_CONSOLE", None)
    meshEnableConsole.setLabel("Enable Console")
    meshEnableConsole.setDefaultValue(False)
    meshEnableConsole.setVisible(True)
    meshEnableConsole.setDescription("Enable Console- check the box to enable")
    meshEnableConsole.setDependencies(isConsoleEnabled, ["ENABLE_CONSOLE"])

    global miwiEnableButtonPress
    miwiEnableButtonPress = miwicomp.createBooleanSymbol("ENABLE_BUTTON_PRESS", None)
    miwiEnableButtonPress.setLabel("Enable Button Press")
    miwiEnableButtonPress.setDefaultValue(False)
    miwiEnableButtonPress.setVisible(True)
    miwiEnableButtonPress.setDescription("Enable Button Press- check the box to enable")
    miwiEnableButtonPress.setDependencies(isEICEnabled, ["ENABLE_BUTTON_PRESS"])

    meshPanId = miwicomp.createHexSymbol("PANID", None)
    meshPanId.setLabel("PAN ID")
    meshPanId.setDefaultValue(0x1234)
    meshPanId.setMin(0x0001)
    meshPanId.setMax(0xFFFB)
    meshPanId.setVisible(True)

    meshAppBufSize = miwicomp.createIntegerSymbol("MIWIBUFFERSIZE", None)
    meshAppBufSize.setLabel("APP BUFFER SIZE")
    meshAppBufSize.setDefaultValue(140)
    meshAppBufSize.setMin(132)
    meshAppBufSize.setMax(200)
    meshAppBufSize.setVisible(True)

    meshEnableEdScan = miwicomp.createBooleanSymbol("ENABLE_EDSCAN", None)
    meshEnableEdScan.setLabel("ED Scan")
    meshEnableEdScan.setDefaultValue(True)
    meshEnableEdScan.setVisible(True)
    meshEnableEdScan.setDescription("Enable Energy Detection Scan- check the box to enable")
    meshEnableEdScan.setDependencies(isEDScanEnabled, ["ENABLE_EDSCAN"])

    meshEnableFreqAgi = miwicomp.createBooleanSymbol("ENABLE_FREQAGILITY", None)
    meshEnableFreqAgi.setLabel("Frequency Agility")
    meshEnableFreqAgi.setDefaultValue(False)
    meshEnableFreqAgi.setVisible(True)
    meshEnableFreqAgi.setDescription("Enable Frequency Agility- check the box to enable")
    meshEnableEdScan.setDependencies(isFreqAgilEnabled, ["ENABLE_FREQAGILITY"])

    meshEnableLed = miwicomp.createBooleanSymbol("ENABLE_LED", None)
    meshEnableLed.setLabel("Enable LED")
    meshEnableLed.setDefaultValue(False)
    meshEnableLed.setVisible(True)
    meshEnableLed.setDescription("Enable LED- check the box to enable")
    meshEnableLed.setDependencies(isLEDEnabled, ["ENABLE_LED"])

    starEnableLinkStatus = miwicomp.createBooleanSymbol("ENABLE_LINK_STATUS", appMiWiStarMenuSymbol)
    starEnableLinkStatus.setLabel("Enable Link Status")
    starEnableLinkStatus.setDefaultValue(False)
    starEnableLinkStatus.setVisible(True)
    starEnableLinkStatus.setDescription("Enable Link Status - check the box to enable")
    starEnableLinkStatus.setDependencies(isLinkStatusEnabled, ["ENABLE_LINK_STATUS"])

    p2pStarEnableIndirectMsg = miwicomp.createBooleanSymbol("ENABLE_INDIRECT_MESSAGE", appMiWiP2PStarMenuSymbol)
    p2pStarEnableIndirectMsg.setLabel("Enable Indirect Message")
    p2pStarEnableIndirectMsg.setDefaultValue(False)
    p2pStarEnableIndirectMsg.setVisible(True)
    p2pStarEnableIndirectMsg.setDescription("Enable Indirect Message - check the box to enable")
    p2pStarEnableIndirectMsg.setDependencies(isIndirectMsgEnabled, ["ENABLE_INDIRECT_MESSAGE"])

    p2pStarConnectionInterval = miwicomp.createIntegerSymbol("CONNECTION_INTERVAL", appMiWiP2PStarMenuSymbol)
    p2pStarConnectionInterval.setLabel("Connection Interval")
    p2pStarConnectionInterval.setDefaultValue(2)
    p2pStarConnectionInterval.setVisible(True)
    p2pStarConnectionInterval.setMin(0)
    p2pStarConnectionInterval.setMax(5)
    # p2pStarConnectionInterval.setDependencies(checkP2PStarDeviceV, ["CONNECTION_INTERVAL"])

    p2pStarConnectionRetryTimes = miwicomp.createIntegerSymbol("CONNECTION_RETRY_TIMES", appMiWiP2PStarMenuSymbol)
    p2pStarConnectionRetryTimes.setLabel("Connection Retry Times")
    p2pStarConnectionRetryTimes.setDefaultValue(3)
    p2pStarConnectionRetryTimes.setVisible(True)
    p2pStarConnectionRetryTimes.setMin(0)
    p2pStarConnectionRetryTimes.setMax(255)

    p2pStarConnectionSize = miwicomp.createIntegerSymbol("CONNECTION_SIZE", appMiWiP2PStarMenuSymbol)
    p2pStarConnectionSize.setLabel("Connection Size")
    p2pStarConnectionSize.setDefaultValue(5)
    p2pStarConnectionSize.setVisible(True)
    p2pStarConnectionSize.setMin(0)
    p2pStarConnectionSize.setMax(255)

    p2pStarActiveScanResSize = miwicomp.createIntegerSymbol("ACTIVE_SCAN_RESULT_SIZE", appMiWiP2PStarMenuSymbol)
    p2pStarActiveScanResSize.setLabel("Active Scan Result Size")
    p2pStarActiveScanResSize.setDefaultValue(4)
    p2pStarActiveScanResSize.setVisible(True)
    p2pStarActiveScanResSize.setMin(0)
    p2pStarActiveScanResSize.setMax(10)
    # p2pStarActiveScanResSize.setDependencies(checkP2PStarDeviceV, ["ACTIVE_SCAN_RESULT_SIZE"])

    p2pStarIndMsgSize = miwicomp.createIntegerSymbol("INDIRECT_MESSAGE_SIZE", appMiWiP2PStarMenuSymbol)
    p2pStarIndMsgSize.setLabel("Indirect Message Size")
    p2pStarIndMsgSize.setDefaultValue(2)
    p2pStarIndMsgSize.setVisible(True)
    p2pStarIndMsgSize.setMin(0)
    p2pStarIndMsgSize.setMax(5)
    # p2pStarIndMsgSize.setDependencies(checkP2PStarDeviceV, ["INDIRECT_MESSAGE_SIZE"])

    p2pStarRFDWakeInterval = miwicomp.createIntegerSymbol("RFD_WAKEUP_INTERVAL", appMiWiP2PStarMenuSymbol)
    p2pStarRFDWakeInterval.setLabel("RFD Wakeup Interval")
    p2pStarRFDWakeInterval.setDefaultValue(4)
    p2pStarRFDWakeInterval.setVisible(True)
    p2pStarRFDWakeInterval.setMin(0)
    p2pStarRFDWakeInterval.setMax(65535)
    # p2pStarRFDWakeInterval.setDependencies(checkP2PStarDeviceV, ["RFD_WAKEUP_INTERVAL"])

    # ==========================MiWi Mesh======================================= 
    # === MIWI Header Files

    miwiMeshIncFiles = [
        ['MiWi/MiWi_Mesh/inc/miwi_mesh.h',                 condProtocolMesh],
        ['MiWi/MiWi_Mesh/inc/miwi_mesh_commissioning.h',   condProtocolMesh],
        ['MiWi/MiWi_Mesh/inc/miwi_mesh_frame.h',           condProtocolMesh],
        ['MiWi/MiWi_Mesh/inc/miwi_mesh_join.h',            condProtocolMesh],
        ['MiWi/MiWi_Mesh/inc/miwi_mesh_routing.h',         condProtocolMesh],
        ['MiWi/MiWi_Mesh/inc/miwi_mesh_security.h',        condProtocolMesh], 
        ['MiWi/MiWi_Mesh/inc/miwi_init.h',                     condProtocolMesh],
        ['MiWi/MiWi_Mesh/inc/miwi_api.h',                     condProtocolMesh],  
        ['MiWi/MiWi_Mesh/inc/mimac.h',                   condProtocolMesh], 
        ['MiWi/MiWi_Mesh/inc/commands.h',                  condProtocolMesh],
        ['MiWi/MiWi_Mesh/inc/miwi_appconfig.h',             condProtocolMesh],
        ['MiWi/MiWi_Mesh/inc/miwi_app.h',                   condProtocolMesh],      
    ]

    # === STB Header Files
    includeMiWiMeshStb = [
        ["MiWi/MiWi_Mesh/Services/STB/inc/stb.h", condMiWiMeshSecurity],
        ["MiWi/MiWi_Mesh/Services/STB/inc/stb_generic.h", condMiWiMeshSecurity],
    ]
    
    # === SAL services Header Files
    
    includeMiWiMeshServices = [
        ["MiWi/MiWi_Mesh/Services/SAL/inc/sal.h", condMiWiMeshSecurity],
        ["MiWi/MiWi_Mesh/Services/SAL/inc/sal_generic.h", condMiWiMeshSecurity],
        ["MiWi/MiWi_Mesh/Services/SAL/inc/sal_types.h", condMiWiMeshSecurity],
        ["MiWi/MiWi_Mesh/Services/inc/miwi_tmr.h",         condProtocolMesh],
        ["MiWi/MiWi_Mesh/Services/inc/wlPdsMemIds.h",                    condProtocolMesh],
        ["MiWi/MiWi_Mesh/Services/inc/wlPdsTypesConverter.h",            condProtocolMesh],
        ["MiWi/MiWi_Mesh/Services/inc/S_Nv.h",                           condProtocolMesh],   
        ["MiWi/MiWi_Mesh/Services/inc/led.h",                       condProtocolMesh],
    ]
    

    # === Import the header files
    
    for incFileEntry in includeMiWiMeshServices:
        importIncFile(miwicomp, configName, incFileEntry)
    for incFileEntry in includeMiWiMeshStb:
        importIncFile(miwicomp, configName, incFileEntry)
    for incFileEntry in miwiMeshIncFiles:
        importIncFile(miwicomp, configName, incFileEntry)

    # # === Source files

    miwiMeshSrcFiles = [
        # General Files
        ['MiWi/MiWi_Mesh/src/miwi_mesh_app.c',                 condProtocolMesh],
        ['MiWi/MiWi_Mesh/src/miwi_mesh_pds.c',                 condProtocolMesh],
        # Core Files
        ['MiWi/MiWi_Mesh/src/miwi_mesh.c',                 condProtocolMesh],
        ['MiWi/MiWi_Mesh/src/miwi_mesh_commissioning.c',   condProtocolMesh],
        ['MiWi/MiWi_Mesh/src/miwi_mesh_frame.c',           condProtocolMesh],
        ['MiWi/MiWi_Mesh/src/miwi_mesh_join.c',            condProtocolMesh],
        ['MiWi/MiWi_Mesh/src/miwi_mesh_routing.c',         condProtocolMesh],
        ['MiWi/MiWi_Mesh/src/miwi_mesh_security.c',        condProtocolMesh],   
        ['MiWi/MiWi_Mesh/src/miwi_init.c',                     condProtocolMesh],
        ['MiWi/MiWi_Mesh/src/mimac_task.c',                    condProtocolMesh],  
        ['MiWi/MiWi_Mesh/src/mimac.c',                   condProtocolMesh],   
        ['MiWi/MiWi_Mesh/src/commands.c',                   condProtocolMesh],
        ['MiWi/MiWi_Mesh/src/miwi_app.c',                    condProtocolMesh], 
    ]
    
    miwiMeshSerSrcFiles = [  
        ['MiWi/MiWi_Mesh/Services/src/led.c',                       condProtocolMesh],     
        ['MiWi/MiWi_Mesh/Services/src/wlPdsTypesConverter.c',     condProtocolMesh],
    ]

    # === STB Source Files
     
    srcMiWiMeshStb = [
        ["MiWi/MiWi_Mesh/Services/STB/src/stb.c", condMiWiMeshSecurity],
    ]
    
    # === SAL services Source Files

    srcMiWiMeshServices = [
        ["MiWi/MiWi_Mesh/Services/SAL/src/sal.c", condMiWiMeshSecurity]
    ]

    # === Import the source files
    for srcFileEntry in srcMiWiMeshStb:
        importSrcFile(miwicomp, configName, srcFileEntry)
    for srcFileEntry in srcMiWiMeshServices:
        importSrcFile(miwicomp, configName, srcFileEntry)
    for srcFileEntry in miwiMeshSrcFiles:
        importSrcFile(miwicomp, configName, srcFileEntry)
    for srcFileEntry in miwiMeshSerSrcFiles:
        importSrcFile(miwicomp, configName, srcFileEntry)

    # === Include path setting
    includePathsMiWiMesh = [
        ["/MiWi/MiWi_Mesh/inc/", condProtocolMesh],
        ["/MiWi/MiWi_Mesh/inc/Services/inc/", condProtocolMesh]
    ]
    
    for incPathEntry in includePathsMiWiMesh:
        setIncPath(miwicomp, configName, incPathEntry)

    includePathsMiWiMeshSecurity = [
        ["/MiWi/MiWi_Mesh/Services/STB/inc/", condMiWiMeshSecurity],
        ["/MiWi/MiWi_Mesh/Services/SAL/inc/", condMiWiMeshSecurity]
    ]
    
    for incPathEntry in includePathsMiWiMeshSecurity:
        setIncPath(miwicomp, configName, incPathEntry)
   
    # ==========================MiWi P2P======================================= 
    # === MIWI Header Files

    miwiP2PIncFiles = [
        ['MiWi/MiWi_P2P/inc/p2p_demo.h',                    condProtocolP2P],   
        ['MiWi/MiWi_P2P/inc/demo_output.h',                    condProtocolP2P], 
        ['MiWi/MiWi_P2P/inc/miwi_p2p_star.h',             condProtocolP2P],
        ['MiWi/MiWi_P2P/inc/miwi_api.h',             condProtocolP2P],
        ['MiWi/MiWi_P2P/inc/miwi_app.h',             condProtocolP2P],
        ['MiWi/MiWi_P2P/inc/mimac.h',             condProtocolP2P],
        ['MiWi/MiWi_P2P/inc/miwi_init.h',             condProtocolP2P],
        ['MiWi/MiWi_P2P/inc/miwi_appconfig.h',             condProtocolP2P],
        ['MiWi/MiWi_P2P/inc/status_codes.h',             condProtocolP2P], 
    ]

    # === STB Header Files
    includeMiWiP2PStb = [
        ["MiWi/MiWi_P2P/Services/STB/inc/stb.h", condMiWiP2PSecurity],
        ["MiWi/MiWi_P2P/Services/STB/inc/stb_generic.h", condMiWiP2PSecurity],
    ]
    
    # === SAL services Header Files
    
    includeMiWiP2PServices = [
        ["MiWi/MiWi_P2P/Services/SAL/inc/sal.h", condMiWiP2PSecurity],
        ["MiWi/MiWi_P2P/Services/SAL/inc/sal_generic.h", condMiWiP2PSecurity],
        ["MiWi/MiWi_P2P/Services/SAL/inc/sal_types.h", condMiWiP2PSecurity],
        ["MiWi/MiWi_P2P/Services/inc/miwi_tmr.h",         condProtocolP2P],
        ["MiWi/MiWi_P2P/Services/inc/wlPdsMemIds.h",                    condProtocolP2P],
        ["MiWi/MiWi_P2P/Services/inc/wlPdsTypesConverter.h",            condProtocolP2P],
        ["MiWi/MiWi_P2P/Services/inc/S_Nv.h",                           condProtocolP2P],   
        ["MiWi/MiWi_P2P/Services/inc/led.h",                       condProtocolP2P],
    ]        

    # # === Source files

    miwiP2PSrcFiles = [
        # General Files
        # Core Files
        ['MiWi/MiWi_P2P/src/p2p_demo.c',                    condProtocolP2P],
        ['MiWi/MiWi_P2P/src/demo_output.c',                    condProtocolP2P],
        ['MiWi/MiWi_P2P/src/miwi_p2p_star.c',             condProtocolP2P],
        ['MiWi/MiWi_P2P/src/miwi_p2p_pds.c',             condProtocolP2P],
        ['MiWi/MiWi_P2P/src/miwi_app.c',             condProtocolP2P],
        ['MiWi/MiWi_P2P/src/miwi_init.c',             condProtocolP2P],
        ['MiWi/MiWi_P2P/src/mimac.c',             condProtocolP2P],
        ['MiWi/MiWi_P2P/src/mimac_task.c',             condProtocolP2P],
    ]

    miwiP2PSerSrcFiles = [  
        ['MiWi/MiWi_P2P/Services/src/led.c',                       condProtocolP2P],     
        ['MiWi/MiWi_P2P/Services/src/wlPdsTypesConverter.c',     condProtocolP2P],
    ]

    # === STB Source Files
     
    srcMiWiP2PStb = [
        ["MiWi/MiWi_P2P/Services/STB/src/stb.c", condMiWiP2PSecurity],
    ]
    
    # === SAL services Source Files

    srcMiWiP2PServices = [
        ["MiWi/MiWi_P2P/Services/SAL/src/sal.c", condMiWiP2PSecurity]
    ]

    # === Import the header files
    
    for incFileEntry in includeMiWiP2PServices:
        importIncFile(miwicomp, configName, incFileEntry)
    for incFileEntry in includeMiWiP2PStb:
        importIncFile(miwicomp, configName, incFileEntry)
    for incFileEntry in miwiP2PIncFiles:
        importIncFile(miwicomp, configName, incFileEntry)

    # === Import the source files
    for srcFileEntry in srcMiWiP2PStb:
        importSrcFile(miwicomp, configName, srcFileEntry)
    for srcFileEntry in srcMiWiP2PServices:
        importSrcFile(miwicomp, configName, srcFileEntry)
    for srcFileEntry in miwiP2PSrcFiles:
        importSrcFile(miwicomp, configName, srcFileEntry)
    for srcFileEntry in miwiP2PSerSrcFiles:
        importSrcFile(miwicomp, configName, srcFileEntry)

    # === Include path setting
    includePathsMiWiP2P = [
        ["/MiWi/MiWi_P2P/inc/", condProtocolP2P],
        ["/MiWi/MiWi_P2P/inc/Services/inc/", condProtocolP2P]
    ]
    
    for incPathEntry in includePathsMiWiP2P:
        setIncPath(miwicomp, configName, incPathEntry)

    includePathsMiWiP2PSecurity = [
        ["/MiWi/MiWi_P2P/Services/STB/inc/", condMiWiP2PSecurity],
        ["/MiWi/MiWi_P2P/Services/SAL/inc/", condMiWiP2PSecurity]
    ]
    
    for incPathEntry in includePathsMiWiP2PSecurity:
        setIncPath(miwicomp, configName, incPathEntry)

    # ==========================MiWi Star======================================= 
    # === MIWI Header Files

    miwiStarIncFiles = [
        ['MiWi/MiWi_Star/inc/star_demo.h',                    condProtocolStar],   
        ['MiWi/MiWi_Star/inc/demo_output.h',                    condProtocolStar], 
        ['MiWi/MiWi_Star/inc/miwi_p2p_star.h',             condProtocolStar],
        ['MiWi/MiWi_Star/inc/miwi_api.h',             condProtocolStar],
        ['MiWi/MiWi_Star/inc/miwi_app.h',             condProtocolStar],
        ['MiWi/MiWi_Star/inc/mimac.h',             condProtocolStar],
        ['MiWi/MiWi_Star/inc/miwi_init.h',             condProtocolStar],
        ['MiWi/MiWi_Star/inc/miwi_appconfig.h',             condProtocolStar],
        ['MiWi/MiWi_Star/inc/status_codes.h',             condProtocolStar], 
    ]

    # === STB Header Files
    includeMiWiStarStb = [
        ["MiWi/MiWi_Star/Services/STB/inc/stb.h", condMiWiStarSecurity],
        ["MiWi/MiWi_Star/Services/STB/inc/stb_generic.h", condMiWiStarSecurity],
    ]
    
    # === SAL services Header Files
    
    includeMiWiStarServices = [
        ["MiWi/MiWi_Star/Services/SAL/inc/sal.h", condMiWiStarSecurity],
        ["MiWi/MiWi_Star/Services/SAL/inc/sal_generic.h", condMiWiStarSecurity],
        ["MiWi/MiWi_Star/Services/SAL/inc/sal_types.h", condMiWiStarSecurity],
        ["MiWi/MiWi_Star/Services/inc/miwi_tmr.h",         condProtocolStar],
        ["MiWi/MiWi_Star/Services/inc/wlPdsMemIds.h",                    condProtocolStar],
        ["MiWi/MiWi_Star/Services/inc/wlPdsTypesConverter.h",            condProtocolStar],
        ["MiWi/MiWi_Star/Services/inc/S_Nv.h",                           condProtocolStar],   
        ["MiWi/MiWi_Star/Services/inc/led.h",                       condProtocolStar],
    ]        

    # # === Source files

    miwiStarSrcFiles = [
        # General Files
        # Core Files
        ['MiWi/MiWi_Star/src/star_demo.c',                    condProtocolStar],
        ['MiWi/MiWi_Star/src/demo_output.c',                    condProtocolStar],
        ['MiWi/MiWi_Star/src/miwi_p2p_star.c',             condProtocolStar],
        ['MiWi/MiWi_Star/src/miwi_p2p_pds.c',             condProtocolStar],
        ['MiWi/MiWi_Star/src/miwi_app.c',             condProtocolStar],
        ['MiWi/MiWi_Star/src/miwi_init.c',             condProtocolStar],
        ['MiWi/MiWi_Star/src/mimac.c',             condProtocolStar],
        ['MiWi/MiWi_Star/src/mimac_task.c',             condProtocolStar],
    ]

    miwiStarSerSrcFiles = [  
        ['MiWi/MiWi_Star/Services/src/led.c',                       condProtocolStar],     
        ['MiWi/MiWi_Star/Services/src/wlPdsTypesConverter.c',     condProtocolStar],
    ]

    # === STB Source Files
     
    srcMiWiStarStb = [
        ["MiWi/MiWi_Star/Services/STB/src/stb.c", condMiWiStarSecurity],
    ]
    
    # === SAL services Source Files

    srcMiWiStarServices = [
        ["MiWi/MiWi_Star/Services/SAL/src/sal.c", condMiWiStarSecurity]
    ]

    # === Import the header files
    
    for incFileEntry in includeMiWiStarServices:
        importIncFile(miwicomp, configName, incFileEntry)
    for incFileEntry in includeMiWiStarStb:
        importIncFile(miwicomp, configName, incFileEntry)
    for incFileEntry in miwiStarIncFiles:
        importIncFile(miwicomp, configName, incFileEntry)

    # === Import the source files
    for srcFileEntry in srcMiWiStarStb:
        importSrcFile(miwicomp, configName, srcFileEntry)
    for srcFileEntry in srcMiWiStarServices:
        importSrcFile(miwicomp, configName, srcFileEntry)
    for srcFileEntry in miwiStarSrcFiles:
        importSrcFile(miwicomp, configName, srcFileEntry)
    for srcFileEntry in miwiStarSerSrcFiles:
        importSrcFile(miwicomp, configName, srcFileEntry)

    # === Include path setting
    includePathsMiWiStar = [
        ["/MiWi/MiWi_Star/inc/", condProtocolStar],
        ["/MiWi/MiWi_Star/inc/Services/inc/", condProtocolStar]
    ]
    
    for incPathEntry in includePathsMiWiStar:
        setIncPath(miwicomp, configName, incPathEntry)

    includePathsMiWiStarSecurity = [
        ["/MiWi/MiWi_Star/Services/STB/inc/", condMiWiStarSecurity],
        ["/MiWi/MiWi_Star/Services/SAL/inc/", condMiWiStarSecurity]
    ]
    
    for incPathEntry in includePathsMiWiStarSecurity:
        setIncPath(miwicomp, configName, incPathEntry)

#======================================================================================        
    # === Compiler macros
    global preprocessorCompiler
    preprocessorCompiler = miwicomp.createSettingSymbol("MIWI_XC32_PREPROCESSOR", None)
    preprocessorCompiler.setCategory("C32")
    preprocessorCompiler.setKey("preprocessor-macros")
    preprocessorCompiler.setValue("ENABLE_QUEUE_CAPACITY;ENABLE_LARGE_BUFFER;PIC32CXBZ_SOC;ENABLE_NETWORK_FREEZER;WOLFSSL_IGNORE_FILE_WARN;HAVE_CONFIG_H;PROTOCOL_MESH;PAN_COORDINATOR;")

    # === File templates processing
    global mimacMeshConfHeader
    mimacMeshConfHeader = miwicomp.createFileSymbol("MIWI_CONF_MESHMAC_HEADER", None)
    mimacMeshConfHeader.setSourcePath('/driver/templates/miwi/miwi_mesh/miwi_config.h.ftl')
    mimacMeshConfHeader.setOutputName('miwi_config.h')
    mimacMeshConfHeader.setType('HEADER')
    mimacMeshConfHeader.setDestPath('MiWi/MiWi_Mesh/inc/')
    mimacMeshConfHeader.setProjectPath('config/default/MiWi/MiWi_Mesh/inc/')
    mimacMeshConfHeader.setOverwrite(True)
    mimacMeshConfHeader.setEnabled(True)
    mimacMeshConfHeader.setMarkup(True)

    global mimacP2PConfHeader
    mimacP2PConfHeader = miwicomp.createFileSymbol("MIWI_CONF_P2PMAC_HEADER", None)
    mimacP2PConfHeader.setSourcePath('/driver/templates/miwi/miwi_p2p/miwi_config.h.ftl')
    mimacP2PConfHeader.setOutputName('miwi_config.h')
    mimacP2PConfHeader.setType('HEADER')
    mimacP2PConfHeader.setDestPath('MiWi/MiWi_P2P/inc/')
    mimacP2PConfHeader.setProjectPath('config/default/MiWi/MiWi_P2P/inc/')
    mimacP2PConfHeader.setEnabled(False)

    global mimacStarConfHeader
    mimacStarConfHeader = miwicomp.createFileSymbol("MIWI_CONF_STARMAC_HEADER", None)
    mimacStarConfHeader.setSourcePath('/driver/templates/miwi/miwi_star/miwi_config.h.ftl')
    mimacStarConfHeader.setOutputName('miwi_config.h')
    mimacStarConfHeader.setType('HEADER')
    mimacStarConfHeader.setDestPath('MiWi/MiWi_Star/inc/')
    mimacStarConfHeader.setProjectPath('config/default/MiWi/MiWi_Star/inc/')
    mimacStarConfHeader.setEnabled(False)

    global mimacMeshDefinitionsH
    mimacMeshDefinitionsH = miwicomp.createFileSymbol('MIWIMESH_DEFINITIONS_H', None)
    mimacMeshDefinitionsH.setType('STRING')
    mimacMeshDefinitionsH.setOutputName('core.LIST_SYSTEM_DEFINITIONS_H_EXTERNS')
    mimacMeshDefinitionsH.setSourcePath('driver/templates/miwi/miwi_mesh/definitions.h.ftl')
    # mimacMeshDefinitionsH.setMarkup(True)
    mimacMeshDefinitionsH.setOverwrite(True)
    mimacMeshDefinitionsH.setEnabled(True)
    mimacMeshDefinitionsH.setMarkup(True)

    global mimacP2PDefinitionsH
    mimacP2PDefinitionsH = miwicomp.createFileSymbol('MIWIP2P_DEFINITIONS_H', None)
    mimacP2PDefinitionsH.setType('STRING')
    mimacP2PDefinitionsH.setOutputName('core.LIST_SYSTEM_DEFINITIONS_H_EXTERNS')
    mimacP2PDefinitionsH.setSourcePath('driver/templates/miwi/miwi_p2p/definitions.h.ftl')
    # mimacP2PDefinitionsH.setMarkup(True)
    mimacP2PDefinitionsH.setEnabled(False)

    global mimacStarDefinitionsH
    mimacStarDefinitionsH = miwicomp.createFileSymbol('MIWISTAR_DEFINITIONS_H', None)
    mimacStarDefinitionsH.setType('STRING')
    mimacStarDefinitionsH.setOutputName('core.LIST_SYSTEM_DEFINITIONS_H_EXTERNS')
    mimacStarDefinitionsH.setSourcePath('driver/templates/miwi/miwi_star/definitions.h.ftl')
    # mimacStarDefinitionsH.setMarkup(True)
    mimacStarDefinitionsH.setEnabled(False)
    
    mimacTasksC = miwicomp.createFileSymbol('MIWI_TASKS_C', None)
    mimacTasksC.setType('STRING')
    mimacTasksC.setOutputName('core.LIST_SYSTEM_TASKS_C_CALL_LIB_TASKS')
    mimacTasksC.setSourcePath('driver/templates/miwi/miwi_mesh/system_tasks.c.ftl')
    mimacTasksC.setMarkup(True)
    
    mimacTasksDefC = miwicomp.createFileSymbol('MIWI_TASK_INITIALIZATION_C', None)
    mimacTasksDefC.setType('STRING')
    mimacTasksDefC.setOutputName('core.LIST_SYSTEM_RTOS_TASKS_C_DEFINITIONS')
    mimacTasksDefC.setSourcePath('driver/templates/miwi/miwi_mesh/system_tasks_def.c.ftl')
    mimacTasksDefC.setMarkup(True)
    
    mimacInitC = miwicomp.createFileSymbol('MIWI_INITIALIZATION_C', None)
    mimacInitC.setType('STRING')
    mimacInitC.setOutputName('core.LIST_SYSTEM_INIT_C_APP_INITIALIZE_DATA')
    mimacInitC.setSourcePath('driver/templates/miwi/miwi_mesh/system_initialize_middleware.c.ftl')
    mimacInitC.setMarkup(True)
    
    mimacInitDataC = miwicomp.createFileSymbol('MIWI_INITIALIZATION_DATA_C', None)
    mimacInitDataC.setType('STRING')
    mimacInitDataC.setOutputName('core.LIST_SYSTEM_INIT_C_LIBRARY_INITIALIZATION_DATA')
    mimacInitDataC.setSourcePath('driver/templates/miwi/miwi_mesh/system_initialize_data.c.ftl')
    mimacInitDataC.setMarkup(True)

    # === Treat warnings as errors
    mimacWarnAsErr = miwicomp.createSettingSymbol("MIWI_GCC_WARN_ERROR", None)
    mimacWarnAsErr.setValue("false")
    mimacWarnAsErr.setCategory("C32")
    mimacWarnAsErr.setKey("make-warnings-into-errors")

    # === Set optimization level
    mimacOptLevel = miwicomp.createSettingSymbol("PET_LEVEL", None)
    mimacOptLevel.setValue("-O1")
    mimacOptLevel.setCategory("C32")
    mimacOptLevel.setKey("optimization-level")

    # === Library

# end instantiateComponent

def finalizeComponent(miwicomp):
    pass
#end finalizeComponent

#######################################################################################
#################### Files and paths generation callbacks #############################
#######################################################################################  

def importIncFile(component, configName, incFileEntry, firmwarePath = None):
    incFilePath  = incFileEntry[0]
    isEnabled    = incFileEntry[1][0]
    callback     = incFileEntry[1][1]
    dependencies = incFileEntry[1][2]

    incFilePathTup = incFilePath.rsplit("/", 1)

    if len(incFilePathTup) == 1:
        secName = ""
        incFile = incFilePathTup[0]
        print("incFilePathTup[0]",incFilePathTup[0])
    else :
        secName = incFilePathTup[0]
        incFile = incFilePathTup[1]
        if "MiWi_Mesh" in incFilePathTup[0]:
            incFile1 = "MiWi_Mesh_" + incFilePathTup[1]
            # print("incFile1",incFile1)
        elif "MiWi_P2P" in incFilePathTup[0]:
            incFile1 = "MiWi_P2P_" + incFilePathTup[1]
            # print("incFile1",incFile1)
        else:
            incFile1 = "MiWi_Star_" + incFilePathTup[1]
            # print("incFile1",incFile1)
        

    # symName = incFile.replace(".", "_").upper()
    symName = incFile1.replace(".", "_").upper()
    secSName = secName + "/"
    secDName = secSName
    print("importIncFile: ", secDName)
    print("src path")
    print("driver/software/" + secSName + incFile)
    print("dest path")
    print(secDName + "")
    print("proj path")
    print("config/" + configName + "/"+ secDName + "")
    incFileSym = component.createFileSymbol(symName, None)
    incFileSym.setSourcePath("driver/software/" + secSName + incFile)
    incFileSym.setOutputName(incFile)
    incFileSym.setDestPath(secDName + "")
    incFileSym.setProjectPath("config/" + configName + "/"+ secDName + "")
    incFileSym.setType("HEADER")
    incFileSym.setOverwrite(True)
    incFileSym.setEnabled(isEnabled)

    if callback and dependencies:
        incFileSym.setDependencies(callback, dependencies)
#end importIncFile

def importSrcFile(component, configName, srcFileEntry, firmwarePath = None):
    srcFilePath  = srcFileEntry[0]
    isEnabled    = srcFileEntry[1][0]
    callback     = srcFileEntry[1][1]
    dependencies = srcFileEntry[1][2]

    srcFilePathTup = srcFilePath.rsplit("/", 1)

    if len(srcFilePathTup) == 1:
        secName = ""
        srcFile = srcFilePathTup[0]
    else:
        secName = srcFilePathTup[0]
        srcFile = srcFilePathTup[1]
        if "MiWi_Mesh" in srcFilePathTup[0]:
            srcFile1 = "MiWi_Mesh_" + srcFilePathTup[1]
            # print("incFile1",incFile1)
        elif "MiWi_P2P" in srcFilePathTup[0]:
            srcFile1 = "MiWi_P2P_" + srcFilePathTup[1]
            # print("incFile1",incFile1)
        else:
            srcFile1 = "MiWi_Star_" + srcFilePathTup[1]
            # print("incFile1",incFile1)
    srcFilePrefix   = ""
    # symName = srcFile.replace(".", "_").upper()
    symName = srcFile1.replace(".", "_").upper()
    secSName = secName + "/"
    secDName = secSName
    print("******", secDName)
    print("importsrccFile: ", secDName)
    print("src path")
    print("driver/software/" + secSName + srcFile)
    print("dest path")
    print("MiWi/MiWi_Mesh/" + secDName + "")
    print("proj path")
    print("config/" + configName + "/"+ secDName + "")
    
    srcFileSym = component.createFileSymbol(symName, None)
    srcFileSym.setSourcePath("driver/software/" + secSName + srcFile)
    srcFileSym.setOutputName(srcFile.rsplit("/", 1)[-1])
    srcFileSym.setDestPath(secDName + "")
    srcFileSym.setProjectPath("config/" + configName + "/"+ secDName + "")
    srcFileSym.setType("SOURCE")
    srcFileSym.setEnabled(isEnabled)

    if callback and dependencies:
        srcFileSym.setDependencies(callback, dependencies)
#end importSrcFile

def setIncPath(component, configName, incPathEntry):
    incPath      = incPathEntry[0]
    isEnabled    = incPathEntry[1][0]
    callback     = incPathEntry[1][1]
    dependencies = incPathEntry[1][2]

    setIncFilePathTup = incPathEntry[0].rsplit("/", 1)
    if "MiWi_Mesh" in setIncFilePathTup[0]:
        setIncPath = "MIWI_MESH_INC_PATH"
        setIncPathCpp = "MIWI_MESH_INC_PATH_CPP"
    elif "MiWi_P2P" in setIncFilePathTup[0]:
        setIncPath = "MIWI_P2P_INC_PATH"
        setIncPathCpp = "MIWI_P2P_INC_PATH_CPP"
    else:
        setIncPath = "MIWI_STAR_INC_PATH"
        setIncPathCpp = "MIWI_STAR_INC_PATH_CPP"

    incPathSym = component.createSettingSymbol(setIncPath + incPath.replace(".", "_").replace("/", "_").upper(), None)
    print("../src/config/" + configName + incPath + ";")
    incPathSym.setValue("../src/config/" + configName + incPath + ";")
    incPathSym.setCategory("C32")
    incPathSym.setKey("extra-include-directories")
    incPathSym.setAppend(True, ";")
    incPathSym.setEnabled(isEnabled)
    incPathSym.setDependencies(callback, dependencies)
    
    
    incPathSymCpp = component.createSettingSymbol(setIncPathCpp + incPath.replace(".", "_").replace("/", "_").upper(), None)
    print("../src/config/" + configName + incPath + ";")
    incPathSymCpp.setValue("../src/config/" + configName + incPath + ";")
    incPathSymCpp.setCategory("C32CPP")
    incPathSymCpp.setKey("extra-include-directories")
    incPathSymCpp.setAppend(True, ";")
    incPathSymCpp.setEnabled(isEnabled)
    incPathSymCpp.setDependencies(callback, dependencies)
#end setIncPath

#######################################################################################
#################### DEPENDENCY CALLBACKS ###################################
#######################################################################################  


#---------------------------------------------------------------------------------------
#~~~~~~~~~~~~~~~ Device type configuration FFD/RFD CALLBACK ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# This callback handles for adding Preprocessor macro, handles indirect buffers for FFD
# and Deep sleep macro configuration based on device type seletion
#---------------------------------------------------------------------------------------
def edConfig(symbol,event):
    setEdDeviceType = event['value']
    DeepSleepEnable.setVisible(False)
    DeepSleepEnable.setValue(False)
    if(setEdDeviceType == 0):
        DeepSleepEnable.setVisible(True)
        DeepSleepEnable.setValue(False)

def DeviceTypeConfiguration(symbol,event):
    print(symbol)
    setDeviceType = event['value']
    miwiEDType.setVisible(False)
    miwiEDType.setValue(0)
    DeepSleepEnable.setVisible(False)
    DeepSleepEnable.setValue(False)
    if setDeviceType == 0:#PC
        preprocessorMacropc = preprocessorCompiler.getValue()
        preprocessorMacropc = preprocessorMacropc + ";PAN_COORDINATOR"
        preprocessorCompiler.setValue(preprocessorMacropc)
        preprocessorCompiler.setEnabled(True)
        preprocessorMacropc = preprocessorCompiler.getValue() 
        preprocessorMacropc = preprocessorMacropc.replace(";COORDINATOR","")
        preprocessorCompiler.setValue(preprocessorMacropc)
        preprocessorCompiler.setEnabled(True)
        preprocessorMacropc = preprocessorCompiler.getValue() 
        preprocessorMacropc = preprocessorMacropc.replace(";ENDDEVICE","")
        preprocessorCompiler.setValue(preprocessorMacropc)
        preprocessorCompiler.setEnabled(True)   
             
    if setDeviceType == 1:#C
        preprocessorMacroc = preprocessorCompiler.getValue()
        preprocessorMacroc = preprocessorMacroc + ";COORDINATOR"
        preprocessorCompiler.setValue(preprocessorMacroc)
        preprocessorCompiler.setEnabled(True)
        preprocessorMacroc = preprocessorCompiler.getValue()
        preprocessorMacroc = preprocessorMacroc.replace(";PAN_COORDINATOR","")
        preprocessorCompiler.setValue(preprocessorMacroc)
        preprocessorCompiler.setEnabled(True)
        preprocessorMacroc = preprocessorCompiler.getValue()
        preprocessorMacroc = preprocessorMacroc.replace(";ENDDEVICE","")
        preprocessorCompiler.setValue(preprocessorMacroc)
        preprocessorCompiler.setEnabled(True)  
    
    if setDeviceType == 2:#ED
        preprocessorMacroed = preprocessorCompiler.getValue()
        preprocessorMacroed = preprocessorMacroed + ";ENDDEVICE"
        preprocessorCompiler.setValue(preprocessorMacroed)
        preprocessorCompiler.setEnabled(True) 
        preprocessorMacroed = preprocessorCompiler.getValue()
        preprocessorMacroed = preprocessorMacroed.replace(";PAN_COORDINATOR","")
        preprocessorCompiler.setValue(preprocessorMacroed)
        preprocessorCompiler.setEnabled(True) 
        preprocessorMacroed = preprocessorCompiler.getValue()
        preprocessorMacroed = preprocessorMacroed.replace(";COORDINATOR","")
        preprocessorCompiler.setValue(preprocessorMacroed)
        preprocessorCompiler.setEnabled(True)  
        miwiEDType.setVisible(True)
        miwiEDType.setValue(0)

#-----------------------------------------------------------------------------------------------------
#~~~~~~~~~~~~~~~ Sleep configuration CALLBACK ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# This callbacks handles for Deep sleep macro configuration and RTC support from wireless_pic32cxbz_wbz
#-----------------------------------------------------------------------------------------------------
       
def SleepConfiguration(symbol,event):
    sleepEnable = event['value']
    HandleSleep(sleepEnable)
    
def HandleSleep(sleepEnable):
    if sleepEnable == True:
        preprocessorSleepMacro = preprocessorCompiler.getValue()
        preprocessorSleepMacro = preprocessorSleepMacro + ";ENABLE_SLEEP_FEATURE"
        preprocessorCompiler.setValue(preprocessorSleepMacro)
        preprocessorCompiler.setEnabled(True)
        Database.setSymbolValue("pic32cx_bz2_devsupport", "ENABLE_DEEP_SLEEP", True)
        Database.setSymbolValue("pic32cx_bz2_devsupport", "SYSTEM_ENABLE_PMUMODE_SETTING", True)
        
    if sleepEnable == False:
        preprocessorSleepMacro = preprocessorCompiler.getValue()
        preprocessorSleepMacro = preprocessorSleepMacro.replace(";ENABLE_SLEEP_FEATURE","")
        preprocessorCompiler.setValue(preprocessorSleepMacro)
        preprocessorCompiler.setEnabled(True) 
        Database.setSymbolValue("pic32cx_bz2_devsupport", "ENABLE_DEEP_SLEEP", False)
        Database.setSymbolValue("pic32cx_bz2_devsupport", "SYSTEM_ENABLE_PMUMODE_SETTING", False) 
     
#-----------------------------------------------------------------------------------------------------
#~~~~~~~~~~~~~~~ Security Configuration CALLBACK ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# This callbacks handles for adding Security preprocessor macros, activating wolfCrypt library module
# and set AES_CLOCK_ENABLE based on security option
#-----------------------------------------------------------------------------------------------------
def SecurityFilesConfig(symbol,event):
    Securityoption = event['value']
    if Securityoption == 1:#Enabled
        symbol.setEnabled(True)        
    if Securityoption == 0:#Disabled
        symbol.setEnabled(False) 

def SecurityConfiguration(symbol,event):
    Securityoption = event['value']
    if Securityoption == 1:#Enabled
        symbol.setEnabled(True)  
        preprocessorSecurity = preprocessorCompiler.getValue()
        preprocessorSecurity = preprocessorSecurity + ";MESH_SECURITY;STB_ON_SAL"
        preprocessorCompiler.setValue(preprocessorSecurity)
        preprocessorCompiler.setEnabled(True)      
        Database.activateComponents(['lib_wolfcrypt']) 
        Database.setSymbolValue("core", "AES_CLOCK_ENABLE", True)
                  
    if Securityoption == 0:#Disabled
        symbol.setEnabled(False) 
        preprocessorSecurity = preprocessorCompiler.getValue()
        preprocessorSecurity = preprocessorSecurity.replace(";MESH_SECURITY;STB_ON_SAL","")
        preprocessorCompiler.setValue(preprocessorSecurity)
        preprocessorCompiler.setEnabled(True)
        Database.deactivateComponents(['lib_wolfcrypt'])
        Database.setSymbolValue("core", "AES_CLOCK_ENABLE", False)

def SecurityFilesMeshConfig(symbol,event):
    Securityoption = event['value']
    if Securityoption == 1:#Enabled
        symbol.setEnabled(True)        
    if Securityoption == 0:#Disabled
        symbol.setEnabled(False) 

def SecurityMeshConfiguration(symbol,event):
    Securityoption = event['value']
    if Securityoption == 1:#Enabled
        symbol.setEnabled(True)  
        preprocessorSecurityMesh = preprocessorCompiler.getValue()
        preprocessorSecurityMesh = preprocessorSecurityMesh + ";MESH_SECURITY;STB_ON_SAL"
        preprocessorCompiler.setValue(preprocessorSecurityMesh)
        preprocessorCompiler.setEnabled(True)      
        Database.activateComponents(['lib_wolfcrypt']) 
        Database.setSymbolValue("core", "AES_CLOCK_ENABLE", True)
                  
    if Securityoption == 0:#Disabled
        symbol.setEnabled(False) 
        preprocessorSecurityMesh = preprocessorCompiler.getValue()
        preprocessorSecurityMesh = preprocessorSecurityMesh.replace(";MESH_SECURITY;STB_ON_SAL","")
        preprocessorCompiler.setValue(preprocessorSecurityMesh)
        preprocessorCompiler.setEnabled(True)
        Database.deactivateComponents(['lib_wolfcrypt'])
        Database.setSymbolValue("core", "AES_CLOCK_ENABLE", False)

def SecurityFilesP2PStarConfig(symbol,event):
    Securityoption = event['value']
    if Securityoption == 1:#Enabled
        symbol.setEnabled(True)        
    if Securityoption == 0:#Disabled
        symbol.setEnabled(False) 

def SecurityP2PStarConfiguration(symbol,event):
    Securityoption = event['value']
    if Securityoption == 1:#Enabled
        symbol.setEnabled(True)  
        preprocessorSecurityp2pstar = preprocessorCompiler.getValue()
        preprocessorSecurityp2pstar = preprocessorSecurityp2pstar + ";ENABLE_SECURITY;STB_ON_SAL"
        preprocessorCompiler.setValue(preprocessorSecurityp2pstar)
        preprocessorCompiler.setEnabled(True)      
        Database.activateComponents(['lib_wolfcrypt']) 
        Database.setSymbolValue("core", "AES_CLOCK_ENABLE", True)
                  
    if Securityoption == 0:#Disabled
        symbol.setEnabled(False) 
        preprocessorSecurityp2pstar = preprocessorCompiler.getValue()
        preprocessorSecurityp2pstar = preprocessorSecurityp2pstar.replace(";ENABLE_SECURITY;STB_ON_SAL","")
        preprocessorCompiler.setValue(preprocessorSecurityp2pstar)
        preprocessorCompiler.setEnabled(True)
        Database.deactivateComponents(['lib_wolfcrypt'])
        Database.setSymbolValue("core", "AES_CLOCK_ENABLE", False)
#-----------------------------------------------------------------------------------------------------
#~~~~~~~~~~~~~~~ Security Files Config CALLBACK ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# This callbacks handles for adding Security files(SAL & STB) based on security option
#-----------------------------------------------------------------------------------------------------     

def SecurityFilesConfig(symbol,event):
    Securityoption = event['value']
    if Securityoption == 1:#Enabled
        symbol.setEnabled(True)        
    if Securityoption == 0:#Disabled
        symbol.setEnabled(False) 

#-----------------------------------------------------------------------------------------------------
def checkMeshDevicePancVisi(symbol, event):
    print(symbol.getValue())
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')
    symbol.setValue((symbol.getValue()))

def checkMeshDevicePanc(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_MESH") and (getDevice == 1)):
        symbol.setEnabled(True)
        symbol.setVisible(True)
    else:
        symbol.setEnabled(False)
        symbol.setVisible(False)

def checkMeshDeviceCoord(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_MESH") and (getDevice == 2)):
        symbol.setEnabled(True)
    else:
        symbol.setEnabled(False)

def checkMeshDeviceEd(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')
    if ((protocol == "MIWI_MESH") and (getDevice == 3)):
        symbol.setEnabled(True)
    else:
        symbol.setEnabled(False)

def checkMeshDevicePancV(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_MESH") and (getDevice == 1)):
        symbol.setVisible(True)
    else:
        symbol.setVisible(False)

def checkMeshDeviceCoordV(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_MESH") and (getDevice == 2)):
        symbol.setVisible(True)
    else:
        symbol.setVisible(False)

def checkMeshDevicePanCCoordV(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_MESH") and ((getDevice == 0) or (getDevice == 1))):
        symbol.setVisible(True)
    else:
        symbol.setVisible(False)

def checkMeshDeviceCoordCEdV(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_MESH") and ((getDevice == 1) or (getDevice == 2))):
        symbol.setVisible(True)
    else:
        symbol.setVisible(False)

def checkMeshDeviceEdV(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_MESH") and (getDevice == 2)):
        symbol.setVisible(True)
    else:
        symbol.setVisible(False)


def appChannelTypeCheckChannel(symbol, event):
    if (appConfigUseChannel.getValue() == "YES"):
        symbol.setVisible(True)
    else:
        symbol.setVisible(False)

def appChannelTypeCheckMask(symbol, event):
    if (appConfigUseChannel.getValue() == "NO"):
        symbol.setVisible(True)
    else:
        symbol.setVisible(False)


def isConsoleEnabled(symbol, event):
    component = symbol.getComponent()
    enableConsole  =  component.getSymbolValue('ENABLE_CONSOLE')
    if ((enableConsole == True)):
        symbol.setEnabled(True)
        symbol.setValue(True)
        Database.activateComponents(['sys_console']) 
    else:
        symbol.setEnabled(False) 
        symbol.setValue(False)
        Database.deactivateComponents(['sys_console']) 

def isEICEnabled(symbol, event):
    component = symbol.getComponent()
    enableConsole  =  component.getSymbolValue('ENABLE_BUTTON_PRESS')
    if ((enableConsole == True)):
        symbol.setEnabled(True)
        symbol.setValue(True)
        Database.activateComponents(['eic']) 
    else:
        symbol.setEnabled(False) 
        symbol.setValue(False)
        Database.deactivateComponents(['eic']) 

def isEDScanEnabled(symbol, event):
    component = symbol.getComponent()
    enableEDScan  =  component.getSymbolValue('ENABLE_EDSCAN')
    if ((enableEDScan == True)):
        symbol.setEnabled(True)
    else:
        symbol.setEnabled(False) 

def isLEDEnabled(symbol, event):
    component = symbol.getComponent()
    enableLED  =  component.getSymbolValue('ENABLE_LED')
    if ((enableLED == True)):
        symbol.setEnabled(True)
        # Database.activateComponents(['bsp']) 
    else:
        symbol.setEnabled(False)
        # Database.deactivateComponents(['bsp']) 

def isFreqAgilEnabled(symbol, event):
    component = symbol.getComponent()
    enableFreqAgility  =  component.getSymbolValue('ENABLE_FREQAGILITY')
    if ((enableFreqAgility == True)):
        symbol.setEnabled(True)
    else:
        symbol.setEnabled(False) 

def isIndirectMsgEnabled(symbol, event):
    component = symbol.getComponent()
    enableIndirectMsg  =  component.getSymbolValue('ENABLE_INDIRECT_MESSAGE')
    if ((enableIndirectMsg == True)):
        symbol.setEnabled(True)
    else:
        symbol.setEnabled(False) 

def isLinkStatusEnabled(symbol, event):
    component = symbol.getComponent()
    enableLinkStatus  =  component.getSymbolValue('ENABLE_LINK_STATUS')
    if ((enableLinkStatus == True)):
        symbol.setEnabled(True)
    else:
        symbol.setEnabled(False) 


#####################################################################################################
################################### APP FILES ################################################
#####################################################################################################
def setEnableMiWiProtocol(symbol, event):
    component = symbol.getComponent()
    setDevice  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')
    print("set device",setDevice)
    if ((setDevice == "MIWI_MESH")):
        print("unique")
        symbol.setEnabled(True)
        appMiWiMeshMenuSymbol.setVisible(True)
        appMiWiP2PMenuSymbol.setVisible(False)
        appMiWiStarMenuSymbol.setVisible(False)
        appMiWiP2PStarMenuSymbol.setVisible(False)
        preprocessorMacroMesh = preprocessorCompiler.getValue()
        preprocessorMacroMesh = preprocessorMacroMesh + ";PROTOCOL_MESH"
        preprocessorCompiler.setValue(preprocessorMacroMesh)
        preprocessorCompiler.setEnabled(True)
    elif((setDevice == "MIWI_PEER2PEER")):
        symbol.setEnabled(True)
        appMiWiP2PMenuSymbol.setVisible(True)
        appMiWiMeshMenuSymbol.setVisible(False)
        appMiWiStarMenuSymbol.setVisible(False)
        appMiWiP2PStarMenuSymbol.setVisible(True)
        DeepSleepEnable.setVisible(True)
        DeepSleepEnable.setValue(False)
        preprocessorMacroP2P = preprocessorCompiler.getValue()
        preprocessorMacroP2P = preprocessorMacroP2P + ";PROTOCOL_P2P"
        preprocessorCompiler.setValue(preprocessorMacroP2P)
        preprocessorCompiler.setEnabled(True)
    elif((setDevice == "MIWI_STAR")):
        symbol.setEnabled(True)
        appMiWiStarMenuSymbol.setVisible(True)
        appMiWiMeshMenuSymbol.setVisible(False)
        appMiWiP2PMenuSymbol.setVisible(False)
        appMiWiP2PStarMenuSymbol.setVisible(True)
        DeepSleepEnable.setVisible(True)
        DeepSleepEnable.setValue(False)
        preprocessorMacroStar = preprocessorCompiler.getValue()
        preprocessorMacroStar = preprocessorMacroStar + ";PROTOCOL_STAR"
        preprocessorCompiler.setValue(preprocessorMacroStar)
        preprocessorCompiler.setEnabled(True)
    else:
        pass


def checkMiWiConfigEnable(symbol, event):
        symbol.setEnabled(True)

def checkP2PStarDeviceV(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')
    # getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_PEER2PEER") or (protocol == "MIWI_STAR")):
        symbol.setVisible(True)
    else:
        symbol.setVisible(False)

def setEnableMiWiMeshProtocol(symbol, event):
    component = symbol.getComponent()

    setDevice  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')
    print("setdevice mesh",setDevice)
    if ((setDevice == "MIWI_MESH")):
        print("setdevice mesh if")
        symbol.setEnabled(True)
        # symbol.setDisplayType('MiWi Mesh')
        mimeshConfHeader.setEnabled(True)
        mimeshConfHeader.setOverwrite(True)
        mimeshConfHeader.setMarkup(True)
        mimacMeshConfHeader.setOverwrite(True)
        mimacMeshConfHeader.setEnabled(True)
        mimacMeshConfHeader.setMarkup(True)
        SourceFileMesh.setOverwrite(True)
        SourceFileMesh.setEnabled(True)
        SourceFileMesh.setMarkup(True)
        mimacMeshDefinitionsH.setOverwrite(True)
        mimacMeshDefinitionsH.setEnabled(True)
        mimacMeshDefinitionsH.setMarkup(True)
        print("endding")
    else:
        print("setdevice mesh else")
        symbol.setEnabled(False)
        # symbol.setDisplayType('MiWi')
        mimeshConfHeader.setOverwrite(False)
        mimeshConfHeader.setEnabled(False)
        mimeshConfHeader.setMarkup(False)
        mimacMeshConfHeader.setOverwrite(False)
        mimacMeshConfHeader.setEnabled(False)
        mimacMeshConfHeader.setMarkup(False)
        DeepSleepEnable.setVisible(True)
        DeepSleepEnable.setValue(False)
        SourceFileMesh.setOverwrite(False)
        SourceFileMesh.setEnabled(False)
        SourceFileMesh.setMarkup(False)
        mimacMeshDefinitionsH.setOverwrite(False)
        mimacMeshDefinitionsH.setEnabled(False)
        mimacMeshDefinitionsH.setMarkup(False)
        preprocessorMacroMesh1 = preprocessorCompiler.getValue()
        preprocessorMacroMesh1 = preprocessorMacroMesh1.replace(";PROTOCOL_MESH","")
        preprocessorCompiler.setValue(preprocessorMacroMesh1)
        preprocessorCompiler.setEnabled(True)
        preprocessorMacroMesh2 = preprocessorCompiler.getValue()
        preprocessorMacroMesh2 = preprocessorMacroMesh2.replace(";PAN_COORDINATOR","")
        preprocessorCompiler.setValue(preprocessorMacroMesh2)
        preprocessorCompiler.setEnabled(True)
        print("endding1")

        
def setEnableMiWiP2PProtocol(symbol, event):
    component = symbol.getComponent()

    setDevice  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')

    if ((setDevice == "MIWI_PEER2PEER")):
        symbol.setEnabled(True)
        # symbol.setDisplayType('MiWi P2P')
        mip2pConfHeader.setEnabled(True)
        mip2pConfHeader.setOverwrite(True)
        mip2pConfHeader.setMarkup(True)
        mimacP2PConfHeader.setOverwrite(True)
        mimacP2PConfHeader.setEnabled(True)
        mimacP2PConfHeader.setMarkup(True)
        SourceFileP2P.setOverwrite(True)
        SourceFileP2P.setEnabled(True)
        SourceFileP2P.setMarkup(True)
        mimacP2PDefinitionsH.setOverwrite(True)
        mimacP2PDefinitionsH.setEnabled(True)
        mimacP2PDefinitionsH.setMarkup(True)
    else:
        symbol.setEnabled(False)
        # symbol.setDisplayType('MiWi')
        mip2pConfHeader.setEnabled(False)
        mip2pConfHeader.setOverwrite(False)
        mip2pConfHeader.setMarkup(False)
        mimacP2PConfHeader.setOverwrite(False)
        mimacP2PConfHeader.setEnabled(False)
        mimacP2PConfHeader.setMarkup(False)
        SourceFileP2P.setOverwrite(False)
        SourceFileP2P.setEnabled(False)
        SourceFileP2P.setMarkup(False)
        preprocessorMacroP2P1 = preprocessorCompiler.getValue() 
        preprocessorMacroP2P1 = preprocessorMacroP2P1.replace(";PROTOCOL_P2P","")
        preprocessorCompiler.setValue(preprocessorMacroP2P1)
        preprocessorCompiler.setEnabled(True)
        mimacP2PDefinitionsH.setOverwrite(True)
        mimacP2PDefinitionsH.setEnabled(True)
        mimacP2PDefinitionsH.setMarkup(True)

def setEnableMiWiStarProtocol(symbol, event):
    component = symbol.getComponent()

    setDevice  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')

    if ((setDevice == "MIWI_STAR")):
        symbol.setEnabled(True)
        # symbol.setDisplayType('MiWi Star')
        mistarConfHeader.setEnabled(True)
        mistarConfHeader.setOverwrite(True)
        mistarConfHeader.setMarkup(True)
        mimacStarConfHeader.setOverwrite(True)
        mimacStarConfHeader.setEnabled(True)
        mimacStarConfHeader.setMarkup(True)
        SourceFileStar.setOverwrite(True)
        SourceFileStar.setEnabled(True)
        SourceFileStar.setMarkup(True)
        mimacStarDefinitionsH.setOverwrite(True)
        mimacStarDefinitionsH.setEnabled(True)
        mimacStarDefinitionsH.setMarkup(True)
        print("endingstar")
    else:
        symbol.setEnabled(False)
        # symbol.setDisplayType('MiWi')
        mistarConfHeader.setEnabled(False)
        mistarConfHeader.setOverwrite(False)
        mistarConfHeader.setMarkup(False)
        mimacStarConfHeader.setOverwrite(False)
        mimacStarConfHeader.setEnabled(False)
        mimacStarConfHeader.setMarkup(False)
        SourceFileStar.setOverwrite(False)
        SourceFileStar.setEnabled(False)
        SourceFileStar.setMarkup(False)
        preprocessorMacroStar1 = preprocessorCompiler.getValue() 
        preprocessorMacroStar1 = preprocessorMacroStar1.replace(";PROTOCOL_STAR","")
        preprocessorCompiler.setValue(preprocessorMacroStar1)
        preprocessorCompiler.setEnabled(True)
        mimacStarDefinitionsH.setOverwrite(True)
        mimacStarDefinitionsH.setEnabled(True)
        mimacStarDefinitionsH.setMarkup(True)
        print("endingstar1")

def setEnableMiWiP2PStarProtocol(symbol, event):
    component = symbol.getComponent()

    setDevice  =  component.getSymbolValue('MIWI_PROTOCOL_TYPE')

    if ((setDevice == "MIWI_PEER2PEER") or (setDevice == "MIWI_STAR")):
        symbol.setEnabled(True)
        # symbol.setDisplayType('MiWi P2P Star')
    else:
        symbol.setEnabled(False)
        # symbol.setDisplayType('MiWi')
#end dependency callbacks

def onAttachmentConnected(source, target):
    localComponent = source["component"]
    remoteComponent = target["component"]
    remoteID = remoteComponent.getID()
    connectID = source["id"]
    targetID = target["id"]
    if  connectID == 'ieee802154phyDependency':   
          remoteComponent.getSymbolByID("CREATE_PHY_RTOS_TASK").setValue(True)
          remoteComponent.getSymbolByID("CREATE_PHY_RTOS_TASK").setReadOnly(True)  

    remoteComponent = Database.getComponentByID("trng")
    if (remoteComponent):
          print('Printing TRNG remoteComponent Value')
          remoteComponent.getSymbolByID("trngEnableInterrupt").setReadOnly(True)
          remoteComponent.getSymbolByID("trngEnableEvent").setReadOnly(True)
          remoteComponent.getSymbolByID("TRNG_STANDBY").setReadOnly(True)         

#end onAttachmentConnected  
    
def onAttachmentDisconnected(source, target):

    localComponent = source["component"]
    remoteComponent = target["component"]
    remoteID = remoteComponent.getID()
    connectID = source["id"]
    targetID = target["id"]

    if  connectID == 'ieee802154phyDependency':   
          remoteComponent.getSymbolByID("CREATE_PHY_RTOS_TASK").setValue(True)
          remoteComponent.getSymbolByID("CREATE_PHY_RTOS_TASK").setReadOnly(False)
          
#end onAttachmentDisconnected 
          
def destroyComponent(miwicomp):
    Database.deactivateComponents(requiredComponents)
    
#end destroyComponent 