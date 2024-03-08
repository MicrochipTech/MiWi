#if defined(PROTOCOL_P2P)
#include "driver/IEEE_802154_PHY/pal/inc/pal.h"
#include "stack_config.h"
#include "MiWi/MiWi_P2P/inc/miwi_config.h"
#if defined(ENABLE_NETWORK_FREEZER)
#include "MiWi/MiWi_P2P/Services/inc/wlPdsMemIds.h"
#include "MiWi/MiWi_P2P/Services/inc/wlPdsTypesConverter.h"
#endif
#ifdef ENABLE_SECURITY
#include "MiWi/MiWi_P2P/Services/SAL/inc/sal_generic.h"
#include "MiWi/MiWi_P2P/Services/SAL/inc/sal.h"
#include "MiWi/MiWi_P2P/Services/SAL/inc/sal_types.h"
#include "MiWi/MiWi_P2P/Services/STB/inc/stb_generic.h"
#include "MiWi/MiWi_P2P/Services/STB/inc/stb.h"
#endif
#include "MiWi/MiWi_P2P/Services/inc/miwi_tmr.h"
#include "MiWi/MiWi_P2P/inc/miwi_config_p2p.h"
#include "MiWi/MiWi_P2P/inc/miwi_app.h"
#include "MiWi/MiWi_P2P/inc/miwi_appconfig.h"
#include "MiWi/MiWi_P2P/inc/miwi_api.h"
#include "MiWi/MiWi_P2P/inc/miwi_init.h"
#include "MiWi/MiWi_P2P/inc/mimac.h"
#include "MiWi/MiWi_P2P/inc/miwi_p2p_star.h"
#include "MiWi/MiWi_P2P/inc/p2p_demo.h"
#if defined(OTAU_ENABLED)
#include "MiWi/MiWi_P2P/Services/otau/otau.h"
#include "MiWi/MiWi_P2P/Services/otau/otau_parser.h"
#include "MiWi/MiWi_P2P/Services/otau/circularBuffer.h"
#include "MiWi/MiWi_P2P/Services/otau/debug/otau_debug.h"
#include "MiWi/MiWi_P2P/Services/otau/notify/otau_notify.h"
#include "MiWi/MiWi_P2P/Services/otau/upgrade/otau_upgrade.h"
#ifdef OTAU_SERVER
#include "MiWi/MiWi_P2P/Services/otau/debug/server_debug.h"
#include "MiWi/MiWi_P2P/Services/otau/notify/server_notify.h"
#include "MiWi/MiWi_P2P/Services/otau/upgrade/server_upgrade.h"
#else
#include "MiWi/MiWi_P2P/Services/otau/debug/client_debug.h"
#include "MiWi/MiWi_P2P/Services/otau/notify/client_notify.h"
#include "MiWi/MiWi_P2P/Services/otau/upgrade/client_upgrade.h"
#endif
#endif
#if defined(LED_ENABLED)
#include "MiWi/MiWi_P2P/Services/inc/led.h"
#endif
#endif