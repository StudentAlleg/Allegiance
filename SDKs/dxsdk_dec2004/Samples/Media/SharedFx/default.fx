// Default effect for handling materials embedded within x-files which have no effect instance
// Copyright (c) Microsoft Corporation. All rights reserved.

// Single diffuse texture map (optional)
texture Texture : TEXTURE < string name = ""; >;

// Fixed-function material properties
float4 Diffuse : DIFFUSE = { 0, 0, 0, 1 };   
float4 Ambient : AMBIENT = { 0, 0, 0, 1 };
float4 Specular : SPECULAR = { 0, 0, 0, 1 };
float4 Emissive : EMISSIVE = { 0, 0, 0, 1 }; 
float  Power : POWER = 1;

// Directional light
float4 LightColor : LIGHTCOLOR = { 1, 1, 1, 1 };
float3 LightDirection : LIGHTDIRECTION = { 0, 0, -1 };   
        
// Transformation matrices    
float4x4 World : WORLD;         
float4x4 View : VIEW;     
float4x4 Projection : PROJECTION; 


//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
sampler TextureSampler = 
sampler_state
{
    Texture = <Texture>;    
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};


//--------------------------------------------------------------------------------------
// Name: VS
// Type: Vertex Shader
// Desc: Projection transform
//--------------------------------------------------------------------------------------
void VS( float4 posObj: POSITION,
         float3 normalObj: NORMAL,
         float2 texCoordIn: TEXCOORD0,
         out float4 posProj: POSITION,
         out float4 colorOut: COLOR0,
         out float2 texCoordOut: TEXCOORD0 )
{
    // Transform the position into projected space for display and world space for lighting
    posProj = mul( posObj, World );
    posProj = mul( posProj, View );
    posProj = mul( posProj, Projection );
   
    // Transform the normal into world space for lighting
    float3 normalWorld = mul( normalObj, (float3x3)World );
    
    // Compute ambient and diffuse lighting
    colorOut = LightColor * Ambient;
    colorOut += LightColor * Diffuse * saturate( dot( LightDirection, normalWorld ) );
   
    // Pass the texture coordinate
    texCoordOut = texCoordIn;
}



//--------------------------------------------------------------------------------------
// Name: PS
// Type: Pixel Shader
// Desc: Modulate the texture by the vertex color
//--------------------------------------------------------------------------------------
void PS( float4 colorIn: COLOR0,
         float2 texCoord: TEXCOORD0,
         out float4 colorOut: COLOR0,
	 uniform bool textureEnabled )
{  
    colorOut = colorIn;

    // Sample and modulate the texture
    if( textureEnabled )
        colorOut *= tex2D( TextureSampler, texCoord );
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique TextureEnabled
{
    pass P0
    {
        VertexShader = compile vs_1_1 VS();    
        PixelShader = compile ps_1_1 PS( true );    
    }
}

technique TextureDisabled
{
    pass P0
    {
        VertexShader = compile vs_1_1 VS();    
        PixelShader = compile ps_1_1 PS( false );     
    }
}
