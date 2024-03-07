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

    global macConstLargeBufferSize
    macConstLargeBufferSize = 172
    global macConstSmallBufferSize
    macConstSmallBufferSize = 60
    
    res = Database.activateComponents(requiredComponents)
    
    # === Interfaces
    # MiWi Protocol Type
    global miwiProtocolType
    global checkProtocolTypeMesh
    global miwiProtocolConfig

    miwiProtocolConfig = miwicomp.createMenuSymbol("MIWI_PROTOCOL_CONFIG", None)
    miwiProtocolConfig.setLabel("MiWi Protocol Type Configuration")
    miwiProtocolConfig.setVisible(True)
    
    miwiProtocolType = miwicomp.createComboSymbol("MIWI_PROTOCOL_TYPE", miwiProtocolConfig, ["MIWI_MESH", "MIWI_PEER2PEER", "MIWI_STAR"])
    miwiProtocolType.setLabel("MiWi Protocol Type")
    miwiProtocolType.setDefaultValue("")
    miwiProtocolType.setVisible(True)
    print(miwicomp.getID())
    miwiProtocolType.setDefaultValue("MIWI_MESH")
    conditionAlwaysInclude = [True, None, []]
    checkProtocolTypeMesh    =  (miwiProtocolType.getValue() == 'MIWI_MESH')
    condProtocolMesh         =  [checkProtocolTypeMesh, setEnableMeshProtocol, ['MIWI_PROTOCOL']]

    # === Radio menu
    # execfile(Module.getPath() + "/config/miwi/drv_miwi_mesh_config.py")

# ========== Device type Configuration for MAC ====================
    global miwiDeviceConfig
    miwiDeviceConfig = miwicomp.createMenuSymbol("MIWI_DEVICE_CONFIG", None)
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
    miwiEDType = miwicomp.createKeyValueSetSymbol("MIWI_MESH_END_DEVICE_TYPE", None)
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

    global miwiSecurityConfig
    miwiSecurityConfig = miwicomp.createMenuSymbol("MIWI_SECURITY_CONFIG", None)
    miwiSecurityConfig.setLabel("Security Configuration")
    miwiSecurityConfig.setVisible(True)

    global SecurityConfig
    SecurityConfig = miwicomp.createKeyValueSetSymbol("MIWI_SECURITY_OPTION", miwiSecurityConfig)
    SecurityConfig.setLabel("MIWI Security Config")
    SecurityConfig.addKey("DISABLED", "0", "DISABLED")
    SecurityConfig.addKey("ENABLED", "1", "ENABLED")
    SecurityConfig.setDefaultValue(0)
    SecurityConfig.setOutputMode("Value")
    SecurityConfig.setDisplayMode("Description")
    SecurityConfig.setDescription("MiWi Security option of the application")
    SecurityConfig.setDependencies(SecurityConfiguration,["MIWI_SECURITY_OPTION"])

    condSecurity = [False, SecurityFilesConfig, ['MIWI_SECURITY_OPTION']]
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

    meshConfigNumofCoordinators = miwicomp.createIntegerSymbol("NUM_OF_COORDINATORS", None)
    meshConfigNumofCoordinators.setLabel("Number of Coordinators")
    meshConfigNumofCoordinators.setDefaultValue(64)
    meshConfigNumofCoordinators.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigNumofCoordinators.setMin(10)
    meshConfigNumofCoordinators.setMax(199)
    meshConfigNumofCoordinators.setDependencies(checkMeshDevicePanCCoordV, ["NUM_OF_COORDINATORS"])

    meshConfigNumofNonSleepingEndevices = miwicomp.createIntegerSymbol("NUM_OF_NONSLEEPING_ENDDEVICES", None)
    meshConfigNumofNonSleepingEndevices.setLabel("Number of non-sleeping enddevices")
    meshConfigNumofNonSleepingEndevices.setDefaultValue(5)
    meshConfigNumofNonSleepingEndevices.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigNumofNonSleepingEndevices.setMin(1)
    meshConfigNumofNonSleepingEndevices.setMax(15)
    meshConfigNumofNonSleepingEndevices.setDependencies(checkMeshDevicePanCCoordV, ["NUM_OF_NONSLEEPING_ENDDEVICES"])

    meshConfigNumofSleepingEndevices = miwicomp.createIntegerSymbol("NUM_OF_SLEEPING_ENDDEVICES", None)
    meshConfigNumofSleepingEndevices.setLabel("Number of sleeping enddevices")
    meshConfigNumofSleepingEndevices.setDefaultValue(5)
    meshConfigNumofSleepingEndevices.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigNumofSleepingEndevices.setMin(1)
    meshConfigNumofSleepingEndevices.setMax(15)
    meshConfigNumofSleepingEndevices.setDependencies(checkMeshDevicePanCCoordV, ["NUM_OF_SLEEPING_ENDDEVICES"])

    meshConfigKeepAliveCoordinatorSendInterval = miwicomp.createIntegerSymbol("KEEP_ALIVE_COORDINATOR_SEND_INTERVAL", None)
    meshConfigKeepAliveCoordinatorSendInterval.setLabel("Keep Alive Coordinator Send Interval")
    meshConfigKeepAliveCoordinatorSendInterval.setDefaultValue(120)
    meshConfigKeepAliveCoordinatorSendInterval.setMin(10)
    meshConfigKeepAliveCoordinatorSendInterval.setMax(255)
    meshConfigKeepAliveCoordinatorSendInterval.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigKeepAliveCoordinatorSendInterval.setDependencies(checkMeshDevicePanCCoordV, ["KEEP_ALIVE_COORDINATOR_SEND_INTERVAL"])

    meshConfigKeepAliveRxOnEDSendInterval = miwicomp.createIntegerSymbol("KEEP_ALIVE_RXONENDDEVICE_SEND_INTERVAL", None)
    meshConfigKeepAliveRxOnEDSendInterval.setLabel("Keep Alive Rx On End device Send Interval")
    meshConfigKeepAliveRxOnEDSendInterval.setDefaultValue(120)
    meshConfigKeepAliveRxOnEDSendInterval.setMin(10)
    meshConfigKeepAliveRxOnEDSendInterval.setMax(255)
    meshConfigKeepAliveRxOnEDSendInterval.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigKeepAliveRxOnEDSendInterval.setDependencies(checkMeshDevicePanCCoordV, ["KEEP_ALIVE_RXONENDDEVICE_SEND_INTERVAL"])

    meshConfigDataRequestSendInterval = miwicomp.createIntegerSymbol("DATA_REQUEST_SEND_INTERVAL", None)
    meshConfigDataRequestSendInterval.setLabel("Data Request Send Interval")
    meshConfigDataRequestSendInterval.setDefaultValue(3)
    meshConfigDataRequestSendInterval.setMin(3)
    meshConfigDataRequestSendInterval.setMax(255)
    meshConfigDataRequestSendInterval.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 2))
    meshConfigDataRequestSendInterval.setDependencies(checkMeshDeviceEdV, ["DATA_REQUEST_SEND_INTERVAL"])

    meshConfigInDirectDatawaitInterval = miwicomp.createIntegerSymbol("INDIRECT_DATA_WAIT_INTERVAL", None)
    meshConfigInDirectDatawaitInterval.setLabel("Indirect Data Wait Interval")
    meshConfigInDirectDatawaitInterval.setDefaultValue(3)
    meshConfigInDirectDatawaitInterval.setMin(3)
    meshConfigInDirectDatawaitInterval.setMax(255)
    meshConfigInDirectDatawaitInterval.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigInDirectDatawaitInterval.setDependencies(checkMeshDevicePanCCoordV, ["INDIRECT_DATA_WAIT_INTERVAL"])

    meshConfigRoleUpgradeInterval = miwicomp.createIntegerSymbol("ROLE_UPGRADE_INTERVAL_IN_SEC", None)
    meshConfigRoleUpgradeInterval.setLabel("Role Upgrade Interval")
    meshConfigRoleUpgradeInterval.setDefaultValue(25)
    meshConfigRoleUpgradeInterval.setMin(5)
    meshConfigRoleUpgradeInterval.setMax(255)
    meshConfigRoleUpgradeInterval.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 0) or (miwiDeviceType.getValue() == 1))
    meshConfigRoleUpgradeInterval.setDependencies(checkMeshDevicePanCCoordV, ["ROLE_UPGRADE_INTERVAL_IN_SEC"])

    meshConfigRoleUpgradeInterval = miwicomp.createIntegerSymbol("CONNECTION_RESPONSE_WAIT_IN_SEC", None)
    meshConfigRoleUpgradeInterval.setLabel("Connection Response Interval")
    meshConfigRoleUpgradeInterval.setDefaultValue(25)
    meshConfigRoleUpgradeInterval.setMin(1)
    meshConfigRoleUpgradeInterval.setMax(255)
    meshConfigRoleUpgradeInterval.setVisible(checkProtocolTypeMesh and (miwiDeviceType.getValue() == 1) or (miwiDeviceType.getValue() == 2))
    meshConfigRoleUpgradeInterval.setDependencies(checkMeshDeviceCoordCEdV, ["MESH_DEVICE_TYPE"])
    
    meshConfigRouteUpdateInterval = miwicomp.createIntegerSymbol("ROUTE_UPDATE_INTERVAL", None)
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
    # meshEnableConsole.setDependencies(checkMeshDevicePancVisi, ["ENABLE_CONSOLE"])
    meshEnableConsole.setDependencies(isConsoleEnabled, ["ENABLE_CONSOLE"])

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
    includeStb = [
        ["MiWi/MiWi_Mesh/Services/STB/inc/stb.h", condSecurity],
        ["MiWi/MiWi_Mesh/Services/STB/inc/stb_generic.h", condSecurity],
    ]
    
    # === SAL services Header Files
    
    includeServices = [
        ["MiWi/MiWi_Mesh/Services/SAL/inc/sal.h", condSecurity],
        ["MiWi/MiWi_Mesh/Services/SAL/inc/sal_generic.h", condSecurity],
        ["MiWi/MiWi_Mesh/Services/SAL/inc/sal_types.h", condSecurity],
        ["MiWi/MiWi_Mesh/Services/inc/miwi_tmr.h",         condProtocolMesh],
        ["MiWi/MiWi_Mesh/Services/inc/wlPdsMemIds.h",                    condProtocolMesh],
        ["MiWi/MiWi_Mesh/Services/inc/wlPdsTypesConverter.h",            condProtocolMesh],
        ["MiWi/MiWi_Mesh/Services/inc/S_Nv.h",                           condProtocolMesh],   
        ["MiWi/MiWi_Mesh/Services/inc/led.h",                       condProtocolMesh],
    ]
    

    # === Import the header files
    
    for incFileEntry in includeStb:
        importIncFile(miwicomp, configName, incFileEntry)
    for incFileEntry in includeServices:
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
     
    srcStb = [
        ["MiWi/MiWi_Mesh/Services/STB/src/stb.c", condSecurity],
    ]
    
    # === SAL services Source Files

    srcServices = [
        ["MiWi/MiWi_Mesh/Services/SAL/src/sal.c", condSecurity]
    ]

    # === Import the source files
    for srcFileEntry in srcStb:
        importSrcFile(miwicomp, configName, srcFileEntry)
    for srcFileEntry in srcServices:
        importSrcFile(miwicomp, configName, srcFileEntry)
    for srcFileEntry in miwiMeshSrcFiles:
        importSrcFile(miwicomp, configName, srcFileEntry)
    for srcFileEntry in miwiMeshSerSrcFiles:
        importSrcFile(miwicomp, configName, srcFileEntry)

    # === Include path setting
    includePathsMac = [
        ["/MiWi/MiWi_Mesh/inc/", conditionAlwaysInclude],
        ["/MiWi/MiWi_Mesh/inc/Services/inc/", conditionAlwaysInclude]
    ]
    
    for incPathEntry in includePathsMac:
        setIncPath(miwicomp, configName, incPathEntry)

    includePathsMacSecurity = [
        ["/MiWi/MiWi_Mesh/Services/STB/inc/", condSecurity],
        ["/MiWi/MiWi_Mesh/Services/SAL/inc/", condSecurity]
    ]
    
    for incPathEntry in includePathsMacSecurity:
        setIncPath(miwicomp, configName, incPathEntry)
   

    # === Compiler macros
    global preprocessorCompiler
    preprocessorCompiler = miwicomp.createSettingSymbol("MIWIMESH_XC32_PREPROCESSOR", None)
    preprocessorCompiler.setCategory("C32")
    preprocessorCompiler.setKey("preprocessor-macros")
    preprocessorCompiler.setValue("ENABLE_QUEUE_CAPACITY;ENABLE_LARGE_BUFFER;CHIMERA_SOC;PROTOCOL_MESH;PAN_COORDINATOR;ENABLE_NETWORK_FREEZER;")

    # === File templates processing
    mimacConfHeader = miwicomp.createFileSymbol("MIWI_CONF_HEADER", None)
    mimacConfHeader.setSourcePath('/driver/templates/miwi/miwi_mesh/miwi_config.h.ftl')
    mimacConfHeader.setOutputName('miwi_config.h')
    mimacConfHeader.setDestPath('MiWi/MiWi_Mesh/inc/')
    mimacConfHeader.setProjectPath('config/default/MiWi/MiWi_Mesh/inc/')
    mimacConfHeader.setType('HEADER')
    mimacConfHeader.setOverwrite(True)
    mimacConfHeader.setEnabled(True)
    mimacConfHeader.setMarkup(True)
    
    mimeshConfHeader = miwicomp.createFileSymbol("MIWI_MESH_CONF_HEADER", None)
    mimeshConfHeader.setSourcePath("/driver/templates/miwi/miwi_mesh/miwi_config_mesh.h.ftl")
    mimeshConfHeader.setOutputName("miwi_config_mesh.h")
    mimeshConfHeader.setDestPath('MiWi/MiWi_Mesh/inc/')
    mimeshConfHeader.setProjectPath('config/default/MiWi/MiWi_Mesh/inc/')
    mimeshConfHeader.setType("HEADER")
    mimeshConfHeader.setOverwrite(True)
    mimeshConfHeader.setEnabled(True)
    mimeshConfHeader.setMarkup(True)

    SourceFile = miwicomp.createFileSymbol(None, None)
    SourceFile.setSourcePath('/driver/templates/miwi/miwi_mesh/app.c.ftl')
    SourceFile.setOutputName('app.c')
    SourceFile.setOverwrite(True)
    SourceFile.setDestPath('../../')
    SourceFile.setProjectPath('')
    SourceFile.setType('SOURCE')
    SourceFile.setEnabled(True)
    SourceFile.setMarkup(True)

    # idleTaskSourceFile = miwicomp.createFileSymbol(None, None)
    # idleTaskSourceFile.setSourcePath('/driver/templates/miwi/miwi_mesh/app_idle_task.c.ftl')
    # idleTaskSourceFile.setOutputName('app_idle_task.c')
    # idleTaskSourceFile.setOverwrite(True)
    # idleTaskSourceFile.setDestPath('../../')
    # idleTaskSourceFile.setProjectPath('')
    # idleTaskSourceFile.setType('SOURCE')
    # idleTaskSourceFile.setEnabled(True)
    # idleTaskSourceFile.setMarkup(True)

    mimacDefinitionsH = miwicomp.createFileSymbol('MIWIMESH_DEFINITIONS_H', None)
    mimacDefinitionsH.setType('STRING')
    mimacDefinitionsH.setOutputName('core.LIST_SYSTEM_DEFINITIONS_H_EXTERNS')
    mimacDefinitionsH.setSourcePath('driver/templates/miwi/miwi_mesh/definitions.h.ftl')
    mimacDefinitionsH.setMarkup(True)
    
    mimacTasksC = miwicomp.createFileSymbol('MIWIMESH_TASKS_C', None)
    mimacTasksC.setType('STRING')
    mimacTasksC.setOutputName('core.LIST_SYSTEM_TASKS_C_CALL_LIB_TASKS')
    mimacTasksC.setSourcePath('driver/templates/miwi/miwi_mesh/system_tasks.c.ftl')
    mimacTasksC.setMarkup(True)
    
    mimacTasksDefC = miwicomp.createFileSymbol('MIWIMESH_TASK_INITIALIZATION_C', None)
    mimacTasksDefC.setType('STRING')
    mimacTasksDefC.setOutputName('core.LIST_SYSTEM_RTOS_TASKS_C_DEFINITIONS')
    mimacTasksDefC.setSourcePath('driver/templates/miwi/miwi_mesh/system_tasks_def.c.ftl')
    mimacTasksDefC.setMarkup(True)
    
    mimacInitC = miwicomp.createFileSymbol('MIWIMESH_INITIALIZATION_C', None)
    mimacInitC.setType('STRING')
    mimacInitC.setOutputName('core.LIST_SYSTEM_INIT_C_APP_INITIALIZE_DATA')
    mimacInitC.setSourcePath('driver/templates/miwi/miwi_mesh/system_initialize_middleware.c.ftl')
    mimacInitC.setMarkup(True)
    
    mimacInitDataC = miwicomp.createFileSymbol('MIWIMESH_INITIALIZATION_DATA_C', None)
    mimacInitDataC.setType('STRING')
    mimacInitDataC.setOutputName('core.LIST_SYSTEM_INIT_C_LIBRARY_INITIALIZATION_DATA')
    mimacInitDataC.setSourcePath('driver/templates/miwi/miwi_mesh/system_initialize_data.c.ftl')
    mimacInitDataC.setMarkup(True)

    # === Treat warnings as errors
    mimacWarnAsErr = miwicomp.createSettingSymbol("MIWIMESH_GCC_WARN_ERROR", None)
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

def finalizeComponent(ieee802154mac):
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
    else :
        secName = incFilePathTup[0]
        incFile = incFilePathTup[1]

    symName = incFile.replace(".", "_").upper()
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

    srcFilePrefix   = ""
    symName = srcFile.replace(".", "_").upper()
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
    incPathSym = component.createSettingSymbol("MIWI_MESH_INC_PATH" + incPath.replace(".", "_").replace("/", "_").upper(), None)
    print("../src/config/" + configName + incPath + ";")
    incPathSym.setValue("../src/config/" + configName + incPath + ";")
    incPathSym.setCategory("C32")
    incPathSym.setKey("extra-include-directories")
    incPathSym.setAppend(True, ";")
    incPathSym.setEnabled(isEnabled)
    incPathSym.setDependencies(callback, dependencies)
    
    
    incPathSymCpp = component.createSettingSymbol("MIWI_MESH_INC_PATH_CPP" + incPath.replace(".", "_").replace("/", "_").upper(), None)
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

# def ProtocolTypeConfiguration(symbol,event):
#     preprocessorMacro = preprocessorCompiler.getValue()
#     preprocessorMacro = preprocessorMacro + ";PROTOCOL_MESH"
#     preprocessorCompiler.setValue(preprocessorMacro)
#     preprocessorCompiler.setEnabled(True)  

def DeviceTypeConfiguration(symbol,event):
    print(symbol)
    setDeviceType = event['value']
    miwiEDType.setVisible(False)
    miwiEDType.setValue(0)
    DeepSleepEnable.setVisible(False)
    DeepSleepEnable.setValue(False)
    if setDeviceType == 0:#PC
        preprocessorMacro = preprocessorCompiler.getValue()
        preprocessorMacro = preprocessorMacro + ";PAN_COORDINATOR"
        preprocessorCompiler.setValue(preprocessorMacro)
        preprocessorCompiler.setEnabled(True)
        preprocessorMacro = preprocessorCompiler.getValue() 
        preprocessorMacro = preprocessorMacro.replace(";COORDINATOR","")
        preprocessorCompiler.setValue(preprocessorMacro)
        preprocessorCompiler.setEnabled(True)
        preprocessorMacro = preprocessorCompiler.getValue() 
        preprocessorMacro = preprocessorMacro.replace(";ENDDEVICE","")
        preprocessorCompiler.setValue(preprocessorMacro)
        preprocessorCompiler.setEnabled(True)   
             
    if setDeviceType == 1:#C
        preprocessorMacro = preprocessorCompiler.getValue()
        preprocessorMacro = preprocessorMacro + ";COORDINATOR"
        preprocessorCompiler.setValue(preprocessorMacro)
        preprocessorCompiler.setEnabled(True)
        preprocessorMacro = preprocessorCompiler.getValue()
        preprocessorMacro = preprocessorMacro.replace(";PAN_COORDINATOR","")
        preprocessorCompiler.setValue(preprocessorMacro)
        preprocessorCompiler.setEnabled(True)
        preprocessorMacro = preprocessorCompiler.getValue()
        preprocessorMacro = preprocessorMacro.replace(";ENDDEVICE","")
        preprocessorCompiler.setValue(preprocessorMacro)
        preprocessorCompiler.setEnabled(True)  
    
    if setDeviceType == 2:#ED
        preprocessorMacro = preprocessorCompiler.getValue()
        preprocessorMacro = preprocessorMacro + ";ENDDEVICE"
        preprocessorCompiler.setValue(preprocessorMacro)
        preprocessorCompiler.setEnabled(True) 
        preprocessorMacro = preprocessorCompiler.getValue()
        preprocessorMacro = preprocessorMacro.replace(";PAN_COORDINATOR","")
        preprocessorCompiler.setValue(preprocessorMacro)
        preprocessorCompiler.setEnabled(True) 
        preprocessorMacro = preprocessorCompiler.getValue()
        preprocessorMacro = preprocessorMacro.replace(";COORDINATOR","")
        preprocessorCompiler.setValue(preprocessorMacro)
        preprocessorCompiler.setEnabled(True)  
        setDeviceType == 2
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

# def ConsoleConfiguration(symbol,event):
#     Consoleoption = event['value']
#     if Consoleoption == 1:#Enabled   
#         Database.activateComponents(["sys_console"]) 
                  
#     if Consoleoption == 0:#Disabled
#         Database.deactivateComponents(["sys_console"])
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

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')
    symbol.setValue((symbol.getValue()))

def checkMeshDevicePanc(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_MESH") and (getDevice == 1)):
        symbol.setEnabled(True)
        symbol.setVisible(True)
    else:
        symbol.setEnabled(False)
        symbol.setVisible(False)

def checkMeshDeviceCoord(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_MESH") and (getDevice == 2)):
        symbol.setEnabled(True)
    else:
        symbol.setEnabled(False)

def checkMeshDeviceEd(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')
    if ((protocol == "MIWI_MESH") and (getDevice == 3)):
        symbol.setEnabled(True)
    else:
        symbol.setEnabled(False)

def checkMeshDevicePancV(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_MESH") and (getDevice == 1)):
        symbol.setVisible(True)
    else:
        symbol.setVisible(False)

def checkMeshDeviceCoordV(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_MESH") and (getDevice == 2)):
        symbol.setVisible(True)
    else:
        symbol.setVisible(False)

def checkMeshDevicePanCCoordV(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_MESH") and ((getDevice == 0) or (getDevice == 1))):
        symbol.setVisible(True)
    else:
        symbol.setVisible(False)

def checkMeshDeviceCoordCEdV(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL')
    getDevice  =  component.getSymbolValue('MESH_DEVICE_TYPE')

    if ((protocol == "MIWI_MESH") and ((getDevice == 1) or (getDevice == 2))):
        symbol.setVisible(True)
    else:
        symbol.setVisible(False)

def checkMeshDeviceEdV(symbol, event):
    component = symbol.getComponent()

    protocol  =  component.getSymbolValue('MIWI_PROTOCOL')
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
    else:
        symbol.setEnabled(False) 

def isFreqAgilEnabled(symbol, event):
    component = symbol.getComponent()
    enableFreqAgility  =  component.getSymbolValue('ENABLE_FREQAGILITY')
    if ((enableFreqAgility == True)):
        symbol.setEnabled(True)
    else:
        symbol.setEnabled(False) 
#####################################################################################################
################################### APP FILES ################################################
#####################################################################################################
def setEnableMeshProtocol(symbol, event):
    component = symbol.getComponent()

    setDevice  =  component.getSymbolValue('MIWI_PROTOCOL')

    if ((setDevice == "MIWI_MESH")):
        symbol.setEnabled(True)
        symbol.setDisplayType('MiWi Mesh')
    else:
        symbol.setEnabled(False)
        symbol.setDisplayType('MiWi')
        

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