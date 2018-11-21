//--------------------------------------------------------------------------------------
// File: AntiAlias.cpp
//
// Multisampling attempts to reduce aliasing by mimicking a higher resolution display; 
// multiple sample points are used to determine each pixel's color. This sample shows 
// how the various multisampling techniques supported by your video card affect the 
// scene's rendering. Although multisampling effectively combats aliasing, under 
// particular situations it can introduce visual artifacts of it's own. As illustrated 
// by the sample, centroid sampling seeks to eliminate one common type of multisampling 
// artifact. Support for centroid sampling is supported under Pixel Shader 2.0 in the 
// latest version of the DirectX runtime.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "dxstdafx.h"
#include "resource.h"

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 


//--------------------------------------------------------------------------------------
// Custom vertex
//--------------------------------------------------------------------------------------
struct Vertex
{
    D3DXVECTOR3 Position;
    D3DCOLOR    Diffuse;
    D3DXVECTOR2 TexCoord;
};

D3DVERTEXELEMENT9 VertexElements[] =
{
    { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
    { 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
    D3DDECL_END()
};



//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
const int GUI_WIDTH = 305;

char                         g_strActiveTechnique[ MAX_PATH+1 ] = {0};
D3DXMATRIXA16                g_mRotation;

IDirect3DVertexDeclaration9* g_pVertexDecl = NULL;
IDirect3DVertexBuffer9*      g_pVBTriangles = NULL;

ID3DXMesh*                   g_pMeshSphereHigh = NULL;
ID3DXMesh*                   g_pMeshSphereLow = NULL;

ID3DXMesh*                   g_pMeshQuadHigh = NULL;
ID3DXMesh*                   g_pMeshQuadLow = NULL;


ID3DXFont*                   g_pFont = NULL;         // Font for drawing text
ID3DXSprite*                 g_pTextSprite = NULL;   // Sprite for batching draw text calls
ID3DXEffect*                 g_pEffect = NULL;       // D3DX effect interface
CDXUTDialog                  g_HUD;                  // dialog for standard controls

CDXUTDialog                  g_DialogLabels;         // Labels within the current scene

bool                         g_bCentroid = false;

IDirect3DTexture9*           g_pTriangleTexture = NULL;
IDirect3DTexture9*           g_pCheckerTexture = NULL;

DXUTDeviceSettings           g_D3DDeviceSettings;



//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_MULTISAMPLE_TYPE    4
#define IDC_MULTISAMPLE_QUALITY 5
#define IDC_CENTROID            6 
#define IDC_FILTERGROUP         7
#define IDC_FILTER_POINT        8
#define IDC_FILTER_LINEAR       9
#define IDC_FILTER_ANISOTROPIC  10
#define IDC_SCENE               11
#define IDC_ROTATIONGROUP       12
#define IDC_ROTATION_RPM        13
#define IDC_RPM                 14
#define IDC_ROTATION_DEGREES    15
#define IDC_DEGREES             16
#define IDC_LABEL0              17
#define IDC_LABEL1              18



//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool    CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed );
void    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc );
void    CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime );
void    CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing );
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown  );
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl );
void    CALLBACK OnLostDevice();
void    CALLBACK OnDestroyDevice();

void RenderSceneTriangles( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime );
void RenderSceneQuads( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime );
void RenderSceneSpheres( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime );

void    InitApp();
HRESULT FillVertexBuffer();
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh );
void    RenderText();

HRESULT InitializeUI();
HRESULT OnMultisampleTypeChanged();
HRESULT OnMultisampleQualityChanged();
void AddMultisampleType( D3DMULTISAMPLE_TYPE type );
D3DMULTISAMPLE_TYPE GetSelectedMultisampleType();
void AddMultisampleQuality( DWORD dwQuality );
DWORD GetSelectedMultisampleQuality();
WCHAR* DXUTMultisampleTypeToString(D3DMULTISAMPLE_TYPE MultiSampleType);
CD3DEnumDeviceSettingsCombo* GetCurrentDeviceSettingsCombo();



//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    // Set the callback functions. These functions allow the sample framework to notify
    // the application about device changes, user input, and windows messages.  The 
    // callbacks are optional so you need only set callbacks for events you're interested 
    // in. However, if you don't handle the device reset/lost callbacks then the sample 
    // framework won't be able to reset your device since the application must first 
    // release all device resources before resetting.  Likewise, if you don't handle the 
    // device created/destroyed callbacks then the sample framework won't be able to 
    // recreate your device resources.
    DXUTSetCallbackDeviceCreated( OnCreateDevice );
    DXUTSetCallbackDeviceReset( OnResetDevice );
    DXUTSetCallbackDeviceLost( OnLostDevice );
    DXUTSetCallbackDeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameRender( OnFrameRender );
    DXUTSetCallbackFrameMove( OnFrameMove );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    InitApp();

    // Initialize the sample framework and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( L"AntiAlias" );
    DXUTCreateDevice( D3DADAPTER_DEFAULT, true, 640, 480, IsDeviceAcceptable, ModifyDeviceSettings );

    // Pass control to the sample framework for handling the message pump and 
    // dispatching render calls. The sample framework will call your FrameMove 
    // and FrameRender callback when there is idle time between handling window messages.
    DXUTMainLoop();

    // Perform any application-level cleanup here. Direct3D device resources are released within the
    // appropriate callback functions and therefore don't require any cleanup code here.

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    // Title font for comboboxes
    g_HUD.SetFont( 1, L"Arial", 14, FW_BOLD );
    CDXUTElement* pElement = g_HUD.GetDefaultElement( DXUT_CONTROL_STATIC, 0 );
    pElement->iFont = 1;
    pElement->dwTextFormat = DT_LEFT | DT_BOTTOM;

    // Scene label font
    g_DialogLabels.SetFont( 1, L"Arial", 16, FW_BOLD );
    pElement = g_DialogLabels.GetDefaultElement( DXUT_CONTROL_STATIC, 0 );
    pElement->iFont = 1;
    pElement->FontColor.Init( D3DCOLOR_RGBA( 200, 200, 200, 255 ) );
    pElement->dwTextFormat = DT_LEFT | DT_BOTTOM;

    // Initialize dialogs
    int iX = 25, iY = 10;
    g_HUD.SetCallback( OnGUIEvent );  
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", iX + 135, iY, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", iX + 135, iY += 24, 125, 22 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", iX + 135, iY += 24, 125, 22 );
    
    iY += 10;
    g_HUD.AddStatic( -1, L"Scene", iX, iY += 24, 260, 22 );
    g_HUD.AddComboBox( IDC_SCENE, iX, iY += 24, 260, 22 );
    g_HUD.GetComboBox( IDC_SCENE )->AddItem( L"Triangles", NULL );
    g_HUD.GetComboBox( IDC_SCENE )->AddItem( L"Quads", NULL );
    g_HUD.GetComboBox( IDC_SCENE )->AddItem( L"Spheres", NULL );
    
    iY += 10;
    g_HUD.AddStatic( -1, L"Multisample Type", iX, iY += 24, 260, 22 );
    g_HUD.AddComboBox( IDC_MULTISAMPLE_TYPE, iX, iY += 24, 260, 22 );
    g_HUD.AddStatic( -1, L"Multisample Quality", iX, iY += 24, 260, 22 );
    g_HUD.AddComboBox( IDC_MULTISAMPLE_QUALITY, iX, iY += 24, 260, 22 );
    
    iY += 10;
    g_HUD.AddStatic( -1, L"Texture Filtering", iX, iY += 24, 260, 22 );
    
    iY += 10;
    g_HUD.AddCheckBox( IDC_CENTROID, L"Centroid sampling", iX+150, iY + 20, 260, 18, false );

    g_HUD.AddRadioButton( IDC_FILTER_POINT, IDC_FILTERGROUP, L"Point", iX+10, iY += 20, 130, 18 );
    g_HUD.AddRadioButton( IDC_FILTER_LINEAR, IDC_FILTERGROUP, L"Linear", iX+10, iY += 20, 130, 18, true );
    g_HUD.AddRadioButton( IDC_FILTER_ANISOTROPIC, IDC_FILTERGROUP, L"Anisotropic", iX+10, iY += 20, 130, 18 );
    
    iY += 10;
    g_HUD.AddStatic( -1, L"Mesh Rotation", iX, iY += 24, 260, 22 );
    
    g_HUD.AddEditBox( IDC_RPM, L"4.0", iX+86, iY + 32, 50, 30 );
    g_HUD.AddRadioButton( IDC_ROTATION_RPM, IDC_ROTATIONGROUP, L"Rotating at                   rpm", iX+10, iY += 38, 300, 18, true );
    g_HUD.AddEditBox( IDC_DEGREES, L"90.0", iX+74, iY + 36, 50, 30 );
    g_HUD.AddRadioButton( IDC_ROTATION_DEGREES, IDC_ROTATIONGROUP, L"Fixed at                   degrees", iX+10, iY += 42, 300, 18 );

    // Add labels
    g_DialogLabels.AddStatic( IDC_LABEL0, L"", 0, 0, 200, 25 );
    g_DialogLabels.AddStatic( IDC_LABEL1, L"", 0, 0, 200, 25 );
    
    
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed )
{
    // No fallback defined by this app, so reject any device that 
    // doesn't support at least ps2.0
    if( pCaps->PixelShaderVersion < D3DPS_VERSION(2,0) )
        return false;
    
    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3DObject(); 
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;


    return true;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// the sample framework will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
void CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps )
{
    // If device doesn't support HWVP then switch to SWVP.
    if( (pCaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 )
        pDeviceSettings->BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    else
        pDeviceSettings->BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    
    // This application is designed to work on a pure device by not using 
    // IDirect3D9::Get*() methods, so create a pure device if supported and using HWVP.
    if ((pCaps->DevCaps & D3DDEVCAPS_PUREDEVICE) != 0 && 
        (pDeviceSettings->BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0 )
        pDeviceSettings->BehaviorFlags |= D3DCREATE_PUREDEVICE;

    // Debugging vertex shaders requires either REF or software vertex processing 
    // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
    if( pDeviceSettings->DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->DeviceType = D3DDEVTYPE_REF;
#endif
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc )
{
    HRESULT hr;
    WCHAR strPath[MAX_PATH];
    

    // Create the vertex declaration
    V_RETURN( pd3dDevice->CreateVertexDeclaration( VertexElements, &g_pVertexDecl ) );

    // Create the vertex buffer for the triangles / quads
    V_RETURN( pd3dDevice->CreateVertexBuffer( 3 * sizeof(Vertex), D3DUSAGE_WRITEONLY, 
                                              0, D3DPOOL_MANAGED, &g_pVBTriangles, NULL ) );

    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Arial", &g_pFont ) );

    // Create the spheres
    V_RETURN( DXUTFindDXSDKMediaFileCch( strPath, MAX_PATH, L"spherehigh.x" ) );
    V_RETURN( LoadMesh( pd3dDevice, strPath, &g_pMeshSphereHigh ) );

    V_RETURN( DXUTFindDXSDKMediaFileCch( strPath, MAX_PATH, L"spherelow.x" ) );
    V_RETURN( LoadMesh( pd3dDevice, strPath, &g_pMeshSphereLow ) );

    V_RETURN( DXUTFindDXSDKMediaFileCch( strPath, MAX_PATH, L"quadhigh.x" ) );
    V_RETURN( LoadMesh( pd3dDevice, strPath, &g_pMeshQuadHigh ) );

    V_RETURN( DXUTFindDXSDKMediaFileCch( strPath, MAX_PATH, L"quadlow.x" ) );
    V_RETURN( LoadMesh( pd3dDevice, strPath, &g_pMeshQuadLow ) );

    
    // Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the 
    // shader debugger. Debugging vertex shaders requires either REF or software vertex 
    // processing, and debugging pixel shaders requires REF.  The 
    // D3DXSHADER_FORCE_*_SOFTWARE_NOOPT flag improves the debug experience in the 
    // shader debugger.  It enables source level debugging, prevents instruction 
    // reordering, prevents dead code elimination, and forces the compiler to compile 
    // against the next higher available software target, which ensures that the 
    // unoptimized shaders do not exceed the shader model limitations.  Setting these 
    // flags will cause slower rendering since the shaders will be unoptimized and 
    // forced into software.  See the DirectX documentation for more information about 
    // using the shader debugger.
    DWORD dwShaderFlags = 0;
    #ifdef DEBUG_VS
        dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
    #endif
    #ifdef DEBUG_PS
        dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
    #endif

    // Read the D3DX effect file
    V_RETURN( DXUTFindDXSDKMediaFileCch( strPath, MAX_PATH, L"AntiAlias.fx" ) );

    // If this fails, there should be debug output as to 
    // they the .fx file failed to compile
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, strPath, NULL, NULL, dwShaderFlags, 
                                        NULL, &g_pEffect, NULL ) );


    // Load the textures
    V_RETURN( DXUTFindDXSDKMediaFileCch( strPath, MAX_PATH, L"checker.tga" ) );
    V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, strPath, &g_pCheckerTexture ) );

    V_RETURN( DXUTFindDXSDKMediaFileCch( strPath, MAX_PATH, L"triangle.tga" ) );
    V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, strPath, &g_pTriangleTexture ) );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: LoadMesh()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, LPD3DXMESH* ppMesh )
{
    LPD3DXMESH pMesh = NULL;
    WCHAR str[MAX_PATH];
    HRESULT hr;

    if( ppMesh == NULL )
        return E_INVALIDARG;

    DXUTFindDXSDKMediaFileCch( str, MAX_PATH, strFileName );
    hr = D3DXLoadMeshFromX(str, D3DXMESH_MANAGED, 
         pd3dDevice, NULL, NULL, NULL, NULL, &pMesh);
    if( FAILED(hr) || (pMesh == NULL) )
        return hr;

    DWORD *rgdwAdjacency = NULL;

    // Make sure there are normals which are required for lighting
    if( !(pMesh->GetFVF() & D3DFVF_NORMAL) )
    {
        LPD3DXMESH pTempMesh;
        hr = pMesh->CloneMeshFVF( pMesh->GetOptions(), 
                                  pMesh->GetFVF() | D3DFVF_NORMAL, 
                                  pd3dDevice, &pTempMesh );
        if( FAILED(hr) )
            return hr;

        D3DXComputeNormals( pTempMesh, NULL );

        SAFE_RELEASE( pMesh );
        pMesh = pTempMesh;
    }

    // Optimze the mesh to make it fast for the user's graphics card
    rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
    if( rgdwAdjacency == NULL )
        return E_OUTOFMEMORY;
    pMesh->ConvertPointRepsToAdjacency(NULL, rgdwAdjacency);
    pMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL);
    delete []rgdwAdjacency;

    *ppMesh = pMesh;

    return S_OK;
}



//--------------------------------------------------------------------------------------
// Set the vertices for the triangles scene
//--------------------------------------------------------------------------------------
HRESULT FillVertexBuffer()
{
    HRESULT hr;

    Vertex* pVertex = NULL;
    V_RETURN( g_pVBTriangles->Lock( 0, 0, (void**) &pVertex, 0 ) );
    
    float fTexel = 1.0f / 128;
    int   nBorder = 5;

    pVertex->Position = D3DXVECTOR3( 1, 1, 0 );
    pVertex->Diffuse = D3DCOLOR_ARGB( 255, 0, 0, 0 );
    pVertex->TexCoord = D3DXVECTOR2( (128 - nBorder) * fTexel, (128 - nBorder) * fTexel );
    pVertex++;

    pVertex->Position = D3DXVECTOR3( 0, 1, 0 );
    pVertex->Diffuse = D3DCOLOR_ARGB( 255, 0, 0, 0 );
    pVertex->TexCoord = D3DXVECTOR2( nBorder * fTexel, (128 - nBorder) * fTexel );
    pVertex++;

    pVertex->Position = D3DXVECTOR3( 0, 0, 0 );
    pVertex->Diffuse = D3DCOLOR_ARGB( 255, 0, 0, 0 );
    pVertex->TexCoord = D3DXVECTOR2( nBorder * fTexel, nBorder * fTexel );
    pVertex++;

    g_pVBTriangles->Unlock();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Moves the world-space XY plane into screen-space for pixel-perfect perspective
//--------------------------------------------------------------------------------------
HRESULT CalculateViewAndProjection( D3DXMATRIX* pViewOut, D3DXMATRIX* pProjectionOut, float fovy, float zNearOffset, float zFarOffset )
{
    if( pViewOut == NULL ||
        pProjectionOut == NULL )
        return E_INVALIDARG;

    // Get back buffer description and determine aspect ratio
    const D3DSURFACE_DESC* pBackBufferSurfaceDesc = DXUTGetBackBufferSurfaceDesc();
    float Width = (float)pBackBufferSurfaceDesc->Width;
    float Height = (float)pBackBufferSurfaceDesc->Height;
    float fAspectRatio = Width/Height;

    // Determine the correct Z depth to completely fill the frustum
    float yScale = 1 / tanf( fovy/2 );
    float z = yScale * Height / 2;
    
    // Calculate perspective projection
    D3DXMatrixPerspectiveFovLH( pProjectionOut, fovy, fAspectRatio, z + zNearOffset, z + zFarOffset );

    // Initialize the view matrix as a rotation and translation from "screen-coordinates"
    // in world space (the XY plane from the perspective of Z+) to a plane that's centered
    // along Z+
    D3DXMatrixIdentity( pViewOut );
    pViewOut->_22 = -1;
    pViewOut->_33 = -1;
    pViewOut->_41 = -(Width/2);
    pViewOut->_42 = (Height/2);
    pViewOut->_43 = z;

    
    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc )
{
    HRESULT hr;

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );
    if( g_pEffect )
        V_RETURN( g_pEffect->OnResetDevice() );

    V_RETURN( FillVertexBuffer() );
    
    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    const D3DCOLOR backColor = D3DCOLOR_ARGB(255, 150, 150, 150);
    const D3DCOLOR bottomColor = D3DCOLOR_ARGB(255, 100, 100, 100);

    g_HUD.SetBackgroundColors( bottomColor, bottomColor, backColor, backColor );
    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - GUI_WIDTH, 0 );
    g_HUD.SetSize( GUI_WIDTH, pBackBufferSurfaceDesc->Height );
    
    InitializeUI();

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT InitializeUI()
{
    HRESULT hr;

    g_D3DDeviceSettings = DXUTGetDeviceSettings();
    
    CD3DEnumDeviceSettingsCombo* pDeviceSettingsCombo = GetCurrentDeviceSettingsCombo();
    if( pDeviceSettingsCombo == NULL )
        return E_FAIL;

    CDXUTComboBox* pMultisampleTypeCombo = g_HUD.GetComboBox( IDC_MULTISAMPLE_TYPE );
    pMultisampleTypeCombo->RemoveAllItems();

    for( int ims=0; ims < pDeviceSettingsCombo->multiSampleTypeList.GetSize(); ims++ )
    {
        D3DMULTISAMPLE_TYPE msType = pDeviceSettingsCombo->multiSampleTypeList.GetAt( ims );

        bool bConflictFound = false;
        for( int iConf = 0; iConf < pDeviceSettingsCombo->DSMSConflictList.GetSize(); iConf++ )
        {
            CD3DEnumDSMSConflict DSMSConf = pDeviceSettingsCombo->DSMSConflictList.GetAt( iConf );
            if( DSMSConf.DSFormat == g_D3DDeviceSettings.pp.AutoDepthStencilFormat &&
                DSMSConf.MSType == msType )
            {
                bConflictFound = true;
                break;
            }
        }

        if( !bConflictFound )
            AddMultisampleType( msType );
    }

    CDXUTComboBox* pMultisampleQualityCombo = g_HUD.GetComboBox( IDC_MULTISAMPLE_TYPE );
    pMultisampleQualityCombo->SetSelectedByData( ULongToPtr(g_D3DDeviceSettings.pp.MultiSampleType) );

    hr = OnMultisampleTypeChanged();
    if( FAILED(hr) )
        return hr;

    return S_OK;
}


//-------------------------------------------------------------------------------------
CD3DEnumDeviceSettingsCombo* GetCurrentDeviceSettingsCombo()
{
    CD3DEnumeration* pD3DEnum = DXUTGetEnumeration();
    return pD3DEnum->GetDeviceSettingsCombo( g_D3DDeviceSettings.AdapterOrdinal,
                                             g_D3DDeviceSettings.DeviceType,
                                             g_D3DDeviceSettings.AdapterFormat,
                                             g_D3DDeviceSettings.pp.BackBufferFormat,
                                             (g_D3DDeviceSettings.pp.Windowed == TRUE) );
}


//-------------------------------------------------------------------------------------
HRESULT OnMultisampleTypeChanged()
{
    HRESULT hr = S_OK;

    D3DMULTISAMPLE_TYPE multisampleType = GetSelectedMultisampleType();
    g_D3DDeviceSettings.pp.MultiSampleType = multisampleType;

    // If multisampling is enabled, then centroid is a meaningful option.
    g_HUD.GetCheckBox( IDC_CENTROID )->SetEnabled( multisampleType != D3DMULTISAMPLE_NONE );
    g_HUD.GetCheckBox( IDC_CENTROID )->SetChecked( multisampleType != D3DMULTISAMPLE_NONE && g_bCentroid );

    CD3DEnumDeviceSettingsCombo* pDeviceSettingsCombo = GetCurrentDeviceSettingsCombo();
    if( pDeviceSettingsCombo == NULL )
        return E_FAIL;

    DWORD dwMaxQuality = 0;
    for( int iType = 0; iType < pDeviceSettingsCombo->multiSampleTypeList.GetSize(); iType++ )
    {
        D3DMULTISAMPLE_TYPE msType = pDeviceSettingsCombo->multiSampleTypeList.GetAt( iType );
        if( msType == multisampleType )
        {
            dwMaxQuality = pDeviceSettingsCombo->multiSampleQualityList.GetAt( iType );
            break;
        }
    }
   
    // DXUTSETTINGSDLG_MULTISAMPLE_QUALITY
    CDXUTComboBox* pMultisampleQualityCombo = g_HUD.GetComboBox( IDC_MULTISAMPLE_QUALITY );
    pMultisampleQualityCombo->RemoveAllItems();

    for( UINT iQuality = 0; iQuality < dwMaxQuality; iQuality++ )
    {
        AddMultisampleQuality( iQuality );
    }

    pMultisampleQualityCombo->SetSelectedByData( ULongToPtr(g_D3DDeviceSettings.pp.MultiSampleQuality) );

    hr = OnMultisampleQualityChanged();
    if( FAILED(hr) )
        return hr;

    return S_OK;
}


//-------------------------------------------------------------------------------------
HRESULT OnMultisampleQualityChanged()
{
    g_D3DDeviceSettings.pp.MultiSampleQuality = GetSelectedMultisampleQuality();
    
    // Change the device if current settings don't match the UI settings
    DXUTDeviceSettings curDeviceSettings = DXUTGetDeviceSettings();
    if( curDeviceSettings.pp.MultiSampleQuality != g_D3DDeviceSettings.pp.MultiSampleQuality ||
        curDeviceSettings.pp.MultiSampleType != g_D3DDeviceSettings.pp.MultiSampleType )
    {
        if( g_D3DDeviceSettings.pp.MultiSampleType != D3DMULTISAMPLE_NONE )
            g_D3DDeviceSettings.pp.Flags &= ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

        DXUTCreateDeviceFromSettings( &g_D3DDeviceSettings );
    }

    return S_OK;
}

//-------------------------------------------------------------------------------------
void AddMultisampleType( D3DMULTISAMPLE_TYPE type )
{
    CDXUTComboBox* pComboBox = g_HUD.GetComboBox( IDC_MULTISAMPLE_TYPE );
    
    if( !pComboBox->ContainsItem( DXUTMultisampleTypeToString(type) ) )
        pComboBox->AddItem( DXUTMultisampleTypeToString(type), ULongToPtr(type) );
}


//-------------------------------------------------------------------------------------
D3DMULTISAMPLE_TYPE GetSelectedMultisampleType()
{
    CDXUTComboBox* pComboBox = g_HUD.GetComboBox( IDC_MULTISAMPLE_TYPE );
    
    return (D3DMULTISAMPLE_TYPE) PtrToUlong( pComboBox->GetSelectedData() ); 
}


//-------------------------------------------------------------------------------------
void AddMultisampleQuality( DWORD dwQuality )
{
    CDXUTComboBox* pComboBox = g_HUD.GetComboBox( IDC_MULTISAMPLE_QUALITY );
        
    WCHAR strQuality[50];
    StringCchPrintf( strQuality, 50, L"%d", dwQuality );
    strQuality[49] = 0;

    if( !pComboBox->ContainsItem( strQuality ) )
        pComboBox->AddItem( strQuality, ULongToPtr(dwQuality) );
}


//-------------------------------------------------------------------------------------
DWORD GetSelectedMultisampleQuality()
{
    CDXUTComboBox* pComboBox = g_HUD.GetComboBox( IDC_MULTISAMPLE_QUALITY );
    
    return PtrToUlong( pComboBox->GetSelectedData() ); 
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime )
{
}


//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, the sample framework will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime )
{
    HRESULT hr;
    
    // Determine the active technique from the UI settings
    StringCchCopyA( g_strActiveTechnique, MAX_PATH, "Texture" );

    if( g_HUD.GetRadioButton( IDC_FILTER_POINT )->GetChecked() )
        StringCchCatA( g_strActiveTechnique, MAX_PATH, "Point" );
    else if( g_HUD.GetRadioButton( IDC_FILTER_LINEAR )->GetChecked() )
        StringCchCatA( g_strActiveTechnique, MAX_PATH, "Linear" );
    else if( g_HUD.GetRadioButton( IDC_FILTER_ANISOTROPIC )->GetChecked() )
        StringCchCatA( g_strActiveTechnique, MAX_PATH, "Anisotropic" );
    else
        return; //error

    if( g_HUD.GetCheckBox( IDC_CENTROID )->GetChecked() )
        StringCchCatA( g_strActiveTechnique, MAX_PATH, "Centroid" );

    // Get the current rotation matrix
    if( g_HUD.GetRadioButton( IDC_ROTATION_RPM )->GetChecked() )
    {
        float fRotate = (float) _wtof( g_HUD.GetEditBox( IDC_RPM )->GetText() );
        D3DXMatrixRotationY( &g_mRotation, ((float)fTime * 2.0f * D3DX_PI * fRotate) / (60.0f) );
    }
    else if( g_HUD.GetRadioButton( IDC_ROTATION_DEGREES )->GetChecked() )
    {
        float fRotate = (float) _wtof( g_HUD.GetEditBox( IDC_DEGREES )->GetText() );
        D3DXMatrixRotationY( &g_mRotation, fRotate * (D3DX_PI / 180.0f) );
    }

    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 255, 255, 255), 1.0f, 0) );
    

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        V( pd3dDevice->SetStreamSource( 0, g_pVBTriangles, 0, sizeof(Vertex) ) );
        V( pd3dDevice->SetVertexDeclaration( g_pVertexDecl ) );

        DXUTComboBoxItem* pSelectedItem = g_HUD.GetComboBox( IDC_SCENE )->GetSelectedItem();
        
        if( 0 == lstrcmp( L"Triangles", pSelectedItem->strText ) )
            RenderSceneTriangles( pd3dDevice, fTime, fElapsedTime );
        else if( 0 == lstrcmp( L"Quads", pSelectedItem->strText ) )
            RenderSceneQuads( pd3dDevice, fTime, fElapsedTime );
        else if( 0 == lstrcmp( L"Spheres", pSelectedItem->strText ) )
            RenderSceneSpheres( pd3dDevice, fTime, fElapsedTime );

        
        
        DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" ); // These events are to help PIX identify what the code is doing
        RenderText();
        V( g_HUD.OnRender( fElapsedTime ) );
        V( g_DialogLabels.OnRender( fElapsedTime ) );
        DXUT_EndPerfEvent();

        V( pd3dDevice->EndScene() );
    }


}


//-------------------------------------------------------------------------------------
void RenderSceneTriangles( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime )
{
    HRESULT hr;

    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProjection;
    D3DXMATRIXA16 mWorldViewProjection;
    D3DXMATRIXA16 mRotation;
    D3DXMATRIXA16 mScaling;
    D3DXMATRIXA16 mTranslation;

    CalculateViewAndProjection( &mView, &mProjection, D3DX_PI/4, -100, 100 );

    // Place labels within the scene
    g_DialogLabels.GetStatic( IDC_LABEL0 )->SetLocation( 25, 75 );
    g_DialogLabels.GetStatic( IDC_LABEL0 )->SetText( L"Solid Color Fill" );

    g_DialogLabels.GetStatic( IDC_LABEL1 )->SetLocation( 175, 75 );
    g_DialogLabels.GetStatic( IDC_LABEL1 )->SetText( L"Texturing Artifacts" );

    for( int iTriangle=0; iTriangle < 4; iTriangle++ )
    {
        switch( iTriangle )
        {
            case 0:
                D3DXMatrixIdentity( &mRotation );
                D3DXMatrixTranslation( &mTranslation, 75.0f +0.1f, 125.0f -0.5f, 0 );
                V( g_pEffect->SetTechnique( "Color" ) );
                break;

            case 1:
                mRotation = g_mRotation;
                D3DXMatrixTranslation( &mTranslation, 75.0f +0.1f, 275.0f -0.5f, 0 );
                V( g_pEffect->SetTechnique( "Color" ) );
                break;
        
            case 2:
                D3DXMatrixIdentity( &mRotation );
                D3DXMatrixTranslation( &mTranslation, 225.0f +0.1f, 125.0f -0.5f, 0 );
                V( g_pEffect->SetTechnique( g_strActiveTechnique ) );
                break;

            case 3:
                mRotation = g_mRotation;
                D3DXMatrixTranslation( &mTranslation, 225.0f +0.1f, 275.0f -0.5f, 0 );
                V( g_pEffect->SetTechnique( g_strActiveTechnique ) );
                break;
        }

        const float Width = 40;
        const float Height = 100;
    
        D3DXMatrixScaling( &mScaling, Width, Height, 1 );
        mWorld = mScaling * mRotation * mTranslation;
        mWorldViewProjection = mWorld * mView * mProjection;
        
        V( g_pEffect->SetMatrix( "g_mWorldViewProjection", &mWorldViewProjection ) );
        V( g_pEffect->SetTexture( "g_tDiffuse", g_pTriangleTexture ) );
        

        UINT NumPasses;
        V( g_pEffect->Begin( &NumPasses, 0 ) );

        for( UINT iPass=0; iPass < NumPasses; iPass++ )
        {
            V( g_pEffect->BeginPass( iPass ) );

            V( pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 1 ) );

            V( g_pEffect->EndPass() );
        }

        V( g_pEffect->End() );
    }
}


//-------------------------------------------------------------------------------------
void RenderSceneQuads( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime )
{
    HRESULT hr;

    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProjection;
    D3DXMATRIXA16 mWorldViewProjection;
    D3DXMATRIXA16 mScaling;
    D3DXMATRIXA16 mTranslation;

    const D3DSURFACE_DESC* pDesc = DXUTGetBackBufferSurfaceDesc();
    const float totalWidth = (float)pDesc->Width - GUI_WIDTH;
    const float radius = .2f * totalWidth;

    // Place labels within the scene
    g_DialogLabels.GetStatic( IDC_LABEL0 )->SetLocation( (int)( .25f * totalWidth - 50 ), (int)( pDesc->Height / 2.0f - radius - 50 ) );
    g_DialogLabels.GetStatic( IDC_LABEL0 )->SetText( L"2 Triangles" );

    g_DialogLabels.GetStatic( IDC_LABEL1 )->SetLocation( (int)( .75f * totalWidth - 50 ), (int)( pDesc->Height / 2.0f - radius - 50 ) );
    g_DialogLabels.GetStatic( IDC_LABEL1 )->SetText( L"8192 Triangles" );

    CalculateViewAndProjection( &mView, &mProjection, D3DX_PI/4, -300, 300 );
    
    V( g_pEffect->SetTechnique( g_strActiveTechnique ) );            
    V( g_pEffect->SetTexture( "g_tDiffuse", g_pCheckerTexture ) );
            
    for( int iQuad=0; iQuad < 2; iQuad++ )
    {
        ID3DXMesh* pMesh = NULL;

        switch( iQuad )
        {
            case 0:
                D3DXMatrixTranslation( &mTranslation, .25f * totalWidth, pDesc->Height / 2.0f, 0 );
                pMesh = g_pMeshQuadLow;
                break;

            case 1:
                D3DXMatrixTranslation( &mTranslation, .75f * totalWidth, pDesc->Height / 2.0f, 0 );
                pMesh = g_pMeshQuadHigh;
                break;
        }

        D3DXMatrixScaling( &mScaling, 2*radius, 2*radius, 2*radius );
        mWorld = mScaling * g_mRotation * mTranslation;
        mWorldViewProjection = mWorld * mView * mProjection;

        V( g_pEffect->SetMatrix( "g_mWorldViewProjection", &mWorldViewProjection ) );
        
        UINT NumPasses;
        V( g_pEffect->Begin( &NumPasses, 0 ) );

        for( UINT iPass=0; iPass < NumPasses; iPass++ )
        {
            V( g_pEffect->BeginPass( iPass ) );

            V( pMesh->DrawSubset( 0 ) );

            V( g_pEffect->EndPass() );
        }

        V( g_pEffect->End() );
    }
}


//-------------------------------------------------------------------------------------
void RenderSceneSpheres( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime )
{
    HRESULT hr;

    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProjection;
    D3DXMATRIXA16 mWorldViewProjection;
    D3DXMATRIXA16 mScaling;
    D3DXMATRIXA16 mTranslation;

    const D3DSURFACE_DESC* pDesc = DXUTGetBackBufferSurfaceDesc();
    const float totalWidth = (float)pDesc->Width - GUI_WIDTH;
    const float radius = .2f * totalWidth;

    // Place labels within the scene
    g_DialogLabels.GetStatic( IDC_LABEL0 )->SetLocation( (int)( .25f * totalWidth - 50 ), (int)( pDesc->Height / 2.0f - radius - 50 ) );
    g_DialogLabels.GetStatic( IDC_LABEL0 )->SetText( L"180 Triangles" );

    g_DialogLabels.GetStatic( IDC_LABEL1 )->SetLocation( (int)( .75f * totalWidth - 50 ), (int)( pDesc->Height / 2.0f - radius - 50 ) );
    g_DialogLabels.GetStatic( IDC_LABEL1 )->SetText( L"8064 Triangles" );

    CalculateViewAndProjection( &mView, &mProjection, D3DX_PI/4, -300, 300 );
    
    V( g_pEffect->SetTechnique( g_strActiveTechnique ) );            
    V( g_pEffect->SetTexture( "g_tDiffuse", g_pCheckerTexture ) );
            
    for( int iSphere=0; iSphere < 2; iSphere++ )
    {
        ID3DXMesh* pMesh = NULL;

        switch( iSphere )
        {
            case 0:
                D3DXMatrixTranslation( &mTranslation, .25f * totalWidth, pDesc->Height / 2.0f, 0 );
                pMesh = g_pMeshSphereLow;
                break;

            case 1:
                D3DXMatrixTranslation( &mTranslation, .75f * totalWidth, pDesc->Height / 2.0f, 0 );
                pMesh = g_pMeshSphereHigh;
                break;
        }

        D3DXMatrixScaling( &mScaling, radius, radius, radius );
        mWorld = mScaling * g_mRotation * mTranslation;
        mWorldViewProjection = mWorld * mView * mProjection;

        V( g_pEffect->SetMatrix( "g_mWorldViewProjection", &mWorldViewProjection ) );
        
        UINT NumPasses;
        V( g_pEffect->Begin( &NumPasses, 0 ) );

        for( UINT iPass=0; iPass < NumPasses; iPass++ )
        {
            V( g_pEffect->BeginPass( iPass ) );

            V( pMesh->DrawSubset( 0 ) );

            V( g_pEffect->EndPass() );
        }

        V( g_pEffect->End() );
    }
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 0.3f, 0.3f, 0.7f, 1.0f ) );
    txtHelper.DrawTextLine( DXUTGetFrameStats() );
    txtHelper.DrawTextLine( DXUTGetDeviceStats() );

    txtHelper.SetForegroundColor( D3DXCOLOR( 0.4f, 0.4f, 0.6f, 1.0f ) );
    txtHelper.DrawTextLine( L"Please see the AntiAlias documentation" );
    
   
    txtHelper.End();
}


//--------------------------------------------------------------------------------------
// Before handling window messages, the sample framework passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then the sample framework will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing )
{
    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    return 0;
}


//--------------------------------------------------------------------------------------
// As a convenience, the sample framework inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown )
{
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:      DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:             DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:          DXUTSetShowSettingsDialog( !DXUTGetShowSettingsDialog() ); break;
        case IDC_MULTISAMPLE_TYPE:      OnMultisampleTypeChanged(); break;
        case IDC_MULTISAMPLE_QUALITY:   OnMultisampleQualityChanged(); break;
        case IDC_CENTROID:              g_bCentroid = ((CDXUTCheckBox*)pControl)->GetChecked(); break;
    }
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice()
{
    if( g_pFont )
        g_pFont->OnLostDevice();
    if( g_pEffect )
        g_pEffect->OnLostDevice();
    SAFE_RELEASE( g_pTextSprite ); 
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice()
{
    SAFE_RELEASE( g_pEffect );
    SAFE_RELEASE( g_pFont );
    SAFE_RELEASE( g_pVertexDecl );
    SAFE_RELEASE( g_pVBTriangles );
    SAFE_RELEASE( g_pMeshSphereHigh );
    SAFE_RELEASE( g_pMeshSphereLow );
    SAFE_RELEASE( g_pMeshQuadHigh );
    SAFE_RELEASE( g_pMeshQuadLow );
    SAFE_RELEASE( g_pCheckerTexture );
    SAFE_RELEASE( g_pTriangleTexture );
}



