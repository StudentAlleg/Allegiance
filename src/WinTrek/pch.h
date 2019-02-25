// pch.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#define __MODULE__ "WinTrek"

//
// C Headers
//

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

//
// Windows headers
//

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <commdlg.h>
#include <commctrl.h>

//
// STL headers
//

#pragma warning( disable : 4786)  

#include <algorithm>
#include <map>
#include <set>
#include <vector>

// Need to include zlib before any DirectX headers

#define ID_APP_ICON "winTrekIcon"
#include "zlib.h"

//
// DX headers
//

//#include <dplobby.h>
#include <dinput.h>

#include "guids.h"

// BT - STEAM
#include "steam_api.h"
//
// WinTrek headers
//

#include "engine.h"
#include "effect.h"
#include "utility.h"
#include "igc.h"
#include "zone.h"
#include "trekinput.h"
#include "resource.h"
#include "Zone.h"
#ifdef USEAUTH
#include "zauth.h"
#endif
#include "messagesAll.h"
#include "messages.h"
#include "messagesLC.h"
#include "..\Club\ClubMessages.h"
#include "clintlib.h"
#include "AutoDownload.h"

#include "soundengine.h"

#include "screen.h"
#include "wintrek.h"
#include "treki.h"
#include "indicator.h"
#include "radarimage.h"
#include "trekctrls.h"
#include "trekmdl.h"
#include "loadout.h"

#include "wintrekp.h"
#include "artwork.h"
#include "ZoneSquad.h"
#include "logon.h"
#include "passworddialog.h"
#include "regkey.h"
#include "treksound.h"
#include "GameTypes.h"

//
//#if defined __cplusplus_cli
//#define CALEETYPE __clrcall
//#else
//#define CALEETYPE __stdcall
//#endif
//#define __RELIABILITY_CONTRACT
//#define SECURITYCRITICAL_ATTRIBUTE
//#define ASSERT_UNMANAGED_CODE_ATTRIBUTE
//
//#if defined __cplusplus_cli
//#define CALLTYPE __clrcall 
//#elif defined _M_IX86
//#define CALLTYPE __thiscall
//#else
//#define CALLTYPE __stdcall
//#endif
//
//__RELIABILITY_CONTRACT
//void CALEETYPE __ArrayUnwind(
//	void*       ptr,                // Pointer to array to destruct
//	size_t      size,               // Size of each element (including padding)
//	int         count,              // Number of elements in the array
//	void(CALLTYPE *pDtor)(void*)    // The destructor to call
//);
//
//__RELIABILITY_CONTRACT
//inline void CALEETYPE __ehvec_ctor(
//	void*       ptr,                // Pointer to array to destruct
//	size_t      size,               // Size of each element (including padding)
//									//  int         count,              // Number of elements in the array
//	size_t      count,              // Number of elements in the array
//	void(CALLTYPE *pCtor)(void*),   // Constructor to call
//	void(CALLTYPE *pDtor)(void*)    // Destructor to call should exception be thrown
//) {
//	size_t i = 0;      // Count of elements constructed
//	int success = 0;
//
//	__try
//	{
//		// Construct the elements of the array
//		for (; i < count; i++)
//		{
//			(*pCtor)(ptr);
//			ptr = (char*)ptr + size;
//		}
//		success = 1;
//	}
//	__finally
//	{
//		if (!success)
//			__ArrayUnwind(ptr, size, (int)i, pDtor);
//	}
//}
//
//__RELIABILITY_CONTRACT
//SECURITYCRITICAL_ATTRIBUTE
//inline void CALEETYPE __ehvec_dtor(
//	void*       ptr,                // Pointer to array to destruct
//	UINT      size,               // Size of each element (including padding)
//									//  int         count,              // Number of elements in the array
//	UINT      count,              // Number of elements in the array
//	void(__thiscall *pDtor)(void*)    // The destructor to call
//) {
//	//_Analysis_assume_(count > 0);
//
//	int success = 0;
//
//	// Advance pointer past end of array
//	ptr = (char*)ptr + size * count;
//
//	__try
//	{
//		// Destruct elements
//		while (count-- > 0)
//		{
//			ptr = (char*)ptr - size;
//			(*pDtor)(ptr);
//		}
//		success = 1;
//	}
//	__finally
//	{
//		if (!success)
//			__ArrayUnwind(ptr, size, (int)count, pDtor);
//	}
//}