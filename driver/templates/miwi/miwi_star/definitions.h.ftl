#if defined(PROTOCOL_STAR)
#include "driver/IEEE_802154_PHY/pal/inc/pal.h"
#include "stack_config.h"
#include "MiWi/MiWi_Star/inc/miwi_config.h"
#if defined(ENABLE_NETWORK_FREEZER)
#include "MiWi/MiWi_Star/Services/inc/wlPdsMemIds.h"
#include "MiWi/MiWi_Star/Services/inc/wlPdsTypesConverter.h"
#endif
#ifdef ENABLE_SECURITY
#include "MiWi/MiWi_Star/Services/SAL/inc/sal_generic.h"
#include "MiWi/MiWi_Star/Services/SAL/inc/sal.h"
#include "MiWi/MiWi_Star/Services/SAL/inc/sal_types.h"
#include "MiWi/MiWi_Star/Services/STB/inc/stb_generic.h"
#include "MiWi/MiWi_Star/Services/STB/inc/stb.h"
#endif
#include "MiWi/MiWi_Star/Services/inc/miwi_tmr.h"
#include "MiWi/MiWi_Star/inc/miwi_config_p2p.h"
#include "MiWi/MiWi_Star/inc/miwi_app.h"
#include "MiWi/MiWi_Star/inc/miwi_appconfig.h"
#include "MiWi/MiWi_Star/inc/miwi_api.h"
#include "MiWi/MiWi_Star/inc/miwi_init.h"
#include "MiWi/MiWi_Star/inc/mimac.h"
#include "MiWi/MiWi_Star/inc/miwi_p2p_star.h"
#include "MiWi/MiWi_Star/inc/star_demo.h"
#if defined(OTAU_ENABLED)
#include "MiWi/MiWi_Star/Services/otau/otau.h"
#include "MiWi/MiWi_Star/Services/otau/otau_parser.h"
#include "MiWi/MiWi_Star/Services/otau/circularBuffer.h"
#include "MiWi/MiWi_Star/Services/otau/debug/otau_debug.h"
#include "MiWi/MiWi_Star/Services/otau/notify/otau_notify.h"
#include "MiWi/MiWi_Star/Services/otau/upgrade/otau_upgrade.h"
#ifdef OTAU_SERVER
#include "MiWi/MiWi_Star/Services/otau/debug/server_debug.h"
#include "MiWi/MiWi_Star/Services/otau/notify/server_notify.h"
#include "MiWi/MiWi_Star/Services/otau/upgrade/server_upgrade.h"
#else
#include "MiWi/MiWi_Star/Services/otau/debug/client_debug.h"
#include "MiWi/MiWi_Star/Services/otau/notify/client_notify.h"
#include "MiWi/MiWi_Star/Services/otau/upgrade/client_upgrade.h"
#endif
#endif
#if defined(LED_ENABLED)
#include "MiWi/MiWi_Star/Services/inc/led.h"
#endif
#endif