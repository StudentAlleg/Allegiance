#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
//#include <dplobby.h>
#include <tchar.h>

// BT - STEAM
#include "steam_api.h"
//
// Headers for fed code.
//
#define __MODULE__  "clintlib"

#include "guids.h"
#include "zlib.h"
#include "utility.h"
#include "igc.h"
#include "Zone.h"
#include "messagesAll.h"
#include "messages.h"
#include "messagesLC.h"
#include "..\Club\ClubMessages.h"
#include "AutoDownload.h"

#include "AllegianceSecurity.h" // BT - STEAM						 
#ifdef USEAUTH
#include "zauth.h"
#endif
#include "clintlib.h"

#include "engine.h"
