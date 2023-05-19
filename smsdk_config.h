#pragma once

#define SMEXT_CONF_NAME			    "[L4D2] Improved Survivor Bots"
#define SMEXT_CONF_DESCRIPTION	    "Many bug fixes and behavioral tweaks for survivor bots"
#define SMEXT_CONF_VERSION		    "1.8.0"
#define SMEXT_CONF_AUTHOR		    "Justin \"Sir Jay\" Chellah"
#define SMEXT_CONF_URL			    "https://justin-chellah.com"
#define SMEXT_CONF_LOGTAG		    "L4D2-ISB"
#define SMEXT_CONF_LICENSE		    "MIT"
#define SMEXT_CONF_DATESTRING	    __DATE__
#define SMEXT_CONF_GAMEDATA_FILE	"improved_survivor_bots"

#define SMEXT_LINK(name) 			SDKExtension *g_pExtensionIface = name;

#define SMEXT_CONF_METAMOD

#define SMEXT_ENABLE_GAMECONF
#define SMEXT_ENABLE_GAMEHELPERS
#define SMEXT_ENABLE_PLAYERHELPERS