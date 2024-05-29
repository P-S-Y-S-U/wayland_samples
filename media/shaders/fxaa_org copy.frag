#ifdef GL_ES	
	precision mediump float;
#endif

uniform lowp sampler2D source;                                                   
uniform vec2 g_Resolution;                                                    
varying highp vec2 qt_TexCoord0;                                                                                                                                  
                                                                                                                                                                  
uniform float m_Subpix;                                                                                                                           
uniform float m_EdgeThreshold;                                                   
uniform float m_EdgeThresholdMin;                                                                                                                                 

uniform highp float zoomFactor;                          
uniform highp float dx;                                   
uniform highp float dy; 

#define u_texture source                                                          
#define v_texCoords qt_TexCoord0

#define FXAA_GREEN_AS_LUMA 1

#define FXAA_PC 1
#define FXAA_GLSL_120 1
#define HIGH_QUALITY

#ifdef HIGH_QUALITY
	#define FXAA_QUALITY__PRESET 39
#else
	#ifdef LOW_QUALITY
		#define FXAA_QUALITY__PRESET 12
	#else
		#define FXAA_QUALITY__PRESET 25
	#endif
#endif


#ifndef FXAA_PS3
    #define FXAA_PS3 0
#endif

#ifndef FXAA_360
    #define FXAA_360 0
#endif

#ifndef FXAA_360_OPT
    #define FXAA_360_OPT 0
#endif

#ifndef FXAA_PC
    #define FXAA_PC 0
#endif

#ifndef FXAA_PC_CONSOLE
    #define FXAA_PC_CONSOLE 0
#endif

#ifndef FXAA_GLSL_120
    #define FXAA_GLSL_120 0
#endif

#ifndef FXAA_GLSL_130
    #define FXAA_GLSL_130 0
#endif

#ifndef FXAA_HLSL_3
    #define FXAA_HLSL_3 0
#endif

#ifndef FXAA_HLSL_4
    #define FXAA_HLSL_4 0
#endif

#ifndef FXAA_HLSL_5
    #define FXAA_HLSL_5 0
#endif

#ifndef FXAA_GREEN_AS_LUMA
    #define FXAA_GREEN_AS_LUMA 0
#endif

#ifndef FXAA_EARLY_EXIT
    #define FXAA_EARLY_EXIT 1
#endif

#ifndef FXAA_DISCARD
    #define FXAA_DISCARD 0
#endif

#ifndef FXAA_FAST_PIXEL_OFFSET
    #ifdef GL_EXT_gpu_shader4
        #define FXAA_FAST_PIXEL_OFFSET 1
    #endif
    #ifdef GL_NV_gpu_shader5
        #define FXAA_FAST_PIXEL_OFFSET 1
    #endif
    #ifdef GL_ARB_gpu_shader5
        #define FXAA_FAST_PIXEL_OFFSET 1
    #endif
    #ifndef FXAA_FAST_PIXEL_OFFSET
        #define FXAA_FAST_PIXEL_OFFSET 0
    #endif
#endif

#ifndef FXAA_GATHER4_ALPHA
    #if (FXAA_HLSL_5 == 1)
        #define FXAA_GATHER4_ALPHA 1
    #endif
    #ifdef GL_ARB_gpu_shader5
        #define FXAA_GATHER4_ALPHA 1
    #endif
    #ifdef GL_NV_gpu_shader5
        #define FXAA_GATHER4_ALPHA 1
    #endif
    #ifndef FXAA_GATHER4_ALPHA
        #define FXAA_GATHER4_ALPHA 0
    #endif
#endif

#ifndef FXAA_CONSOLE__PS3_EDGE_SHARPNESS
    #if 1
        #define FXAA_CONSOLE__PS3_EDGE_SHARPNESS 8.0
    #endif
    #if 0
        #define FXAA_CONSOLE__PS3_EDGE_SHARPNESS 4.0
    #endif
    #if 0
        #define FXAA_CONSOLE__PS3_EDGE_SHARPNESS 2.0
    #endif
#endif

#ifndef FXAA_CONSOLE__PS3_EDGE_THRESHOLD
    #if 1
        #define FXAA_CONSOLE__PS3_EDGE_THRESHOLD 0.125
    #else
        #define FXAA_CONSOLE__PS3_EDGE_THRESHOLD 0.25
    #endif
#endif

#ifndef FXAA_QUALITY__PRESET 
    #define FXAA_QUALITY__PRESET 12
#endif

#if (FXAA_QUALITY__PRESET == 10)
    #define FXAA_QUALITY__PS 3
    #define FXAA_QUALITY__P0 1.5
    #define FXAA_QUALITY__P1 3.0
    #define FXAA_QUALITY__P2 12.0
#endif

#if (FXAA_QUALITY__PRESET == 11)
    #define FXAA_QUALITY__PS 4
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 3.0
    #define FXAA_QUALITY__P3 12.0
#endif

#if (FXAA_QUALITY__PRESET == 12)
    #define FXAA_QUALITY__PS 5
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 2.0
    #define FXAA_QUALITY__P3 4.0
    #define FXAA_QUALITY__P4 12.0
#endif

#if (FXAA_QUALITY__PRESET == 13)
    #define FXAA_QUALITY__PS 6
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 2.0
    #define FXAA_QUALITY__P3 2.0
    #define FXAA_QUALITY__P4 4.0
    #define FXAA_QUALITY__P5 12.0
#endif

#if (FXAA_QUALITY__PRESET == 14)
    #define FXAA_QUALITY__PS 7
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 2.0
    #define FXAA_QUALITY__P3 2.0
    #define FXAA_QUALITY__P4 2.0
    #define FXAA_QUALITY__P5 4.0
    #define FXAA_QUALITY__P6 12.0
#endif

#if (FXAA_QUALITY__PRESET == 15)
    #define FXAA_QUALITY__PS 8
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 2.0
    #define FXAA_QUALITY__P3 2.0
    #define FXAA_QUALITY__P4 2.0
    #define FXAA_QUALITY__P5 2.0
    #define FXAA_QUALITY__P6 4.0
    #define FXAA_QUALITY__P7 12.0
#endif

#if (FXAA_QUALITY__PRESET == 20)
    #define FXAA_QUALITY__PS 3
    #define FXAA_QUALITY__P0 1.5
    #define FXAA_QUALITY__P1 2.0
    #define FXAA_QUALITY__P2 8.0
#endif

#if (FXAA_QUALITY__PRESET == 21)
    #define FXAA_QUALITY__PS 4
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 2.0
    #define FXAA_QUALITY__P3 8.0
#endif

#if (FXAA_QUALITY__PRESET == 22)
    #define FXAA_QUALITY__PS 5
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 2.0
    #define FXAA_QUALITY__P3 2.0
    #define FXAA_QUALITY__P4 8.0
#endif

#if (FXAA_QUALITY__PRESET == 23)
    #define FXAA_QUALITY__PS 6
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 2.0
    #define FXAA_QUALITY__P3 2.0
    #define FXAA_QUALITY__P4 2.0
    #define FXAA_QUALITY__P5 8.0
#endif

#if (FXAA_QUALITY__PRESET == 24)
    #define FXAA_QUALITY__PS 7
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 2.0
    #define FXAA_QUALITY__P3 2.0
    #define FXAA_QUALITY__P4 2.0
    #define FXAA_QUALITY__P5 3.0
    #define FXAA_QUALITY__P6 8.0
#endif

#if (FXAA_QUALITY__PRESET == 25)
    #define FXAA_QUALITY__PS 8
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 2.0
    #define FXAA_QUALITY__P3 2.0
    #define FXAA_QUALITY__P4 2.0
    #define FXAA_QUALITY__P5 2.0
    #define FXAA_QUALITY__P6 4.0
    #define FXAA_QUALITY__P7 8.0
#endif

#if (FXAA_QUALITY__PRESET == 26)
    #define FXAA_QUALITY__PS 9
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 2.0
    #define FXAA_QUALITY__P3 2.0
    #define FXAA_QUALITY__P4 2.0
    #define FXAA_QUALITY__P5 2.0
    #define FXAA_QUALITY__P6 2.0
    #define FXAA_QUALITY__P7 4.0
    #define FXAA_QUALITY__P8 8.0
#endif

#if (FXAA_QUALITY__PRESET == 27)
    #define FXAA_QUALITY__PS 10
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 2.0
    #define FXAA_QUALITY__P3 2.0
    #define FXAA_QUALITY__P4 2.0
    #define FXAA_QUALITY__P5 2.0
    #define FXAA_QUALITY__P6 2.0
    #define FXAA_QUALITY__P7 2.0
    #define FXAA_QUALITY__P8 4.0
    #define FXAA_QUALITY__P9 8.0
#endif

#if (FXAA_QUALITY__PRESET == 28)
    #define FXAA_QUALITY__PS 11
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 2.0
    #define FXAA_QUALITY__P3 2.0
    #define FXAA_QUALITY__P4 2.0
    #define FXAA_QUALITY__P5 2.0
    #define FXAA_QUALITY__P6 2.0
    #define FXAA_QUALITY__P7 2.0
    #define FXAA_QUALITY__P8 2.0
    #define FXAA_QUALITY__P9 4.0
    #define FXAA_QUALITY__P10 8.0
#endif

#if (FXAA_QUALITY__PRESET == 29)
    #define FXAA_QUALITY__PS 12
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.5
    #define FXAA_QUALITY__P2 2.0
    #define FXAA_QUALITY__P3 2.0
    #define FXAA_QUALITY__P4 2.0
    #define FXAA_QUALITY__P5 2.0
    #define FXAA_QUALITY__P6 2.0
    #define FXAA_QUALITY__P7 2.0
    #define FXAA_QUALITY__P8 2.0
    #define FXAA_QUALITY__P9 2.0
    #define FXAA_QUALITY__P10 4.0
    #define FXAA_QUALITY__P11 8.0
#endif

#if (FXAA_QUALITY__PRESET == 39)
    #define FXAA_QUALITY__PS 12
    #define FXAA_QUALITY__P0 1.0
    #define FXAA_QUALITY__P1 1.0
    #define FXAA_QUALITY__P2 1.0
    #define FXAA_QUALITY__P3 1.0
    #define FXAA_QUALITY__P4 1.0
    #define FXAA_QUALITY__P5 1.5
    #define FXAA_QUALITY__P6 2.0
    #define FXAA_QUALITY__P7 2.0
    #define FXAA_QUALITY__P8 2.0
    #define FXAA_QUALITY__P9 2.0
    #define FXAA_QUALITY__P10 4.0
    #define FXAA_QUALITY__P11 8.0
#endif



#if (FXAA_GLSL_120 == 1) || (FXAA_GLSL_130 == 1)
    #define FxaaBool bool
    #define FxaaDiscard discard
    #define FxaaFloat float
    #define FxaaFloat2 vec2
    #define FxaaFloat3 vec3
    #define FxaaFloat4 vec4
    #define FxaaHalf float
    #define FxaaHalf2 vec2
    #define FxaaHalf3 vec3
    #define FxaaHalf4 vec4
    #define FxaaInt2 vec2
    #define FxaaSat(x) clamp(x, 0.0, 1.0)
    #define FxaaTex sampler2D
#else
    #define FxaaBool bool
    #define FxaaDiscard clip(-1)
    #define FxaaFloat float
    #define FxaaFloat2 float2
    #define FxaaFloat3 float3
    #define FxaaFloat4 float4
    #define FxaaHalf half
    #define FxaaHalf2 half2
    #define FxaaHalf3 half3
    #define FxaaHalf4 half4
    #define FxaaSat(x) saturate(x)
#endif

#if (FXAA_GLSL_120 == 1)
    #define FxaaTexTop(t, p) texture2D(t, p)
    #if (FXAA_FAST_PIXEL_OFFSET == 1)
        #define FxaaTexOff(t, p, o, r) texture2DLodOffset(t, p, 0.0, o)
    #else
        #define FxaaTexOff(t, p, o, r) texture2D(t, p + (o * r))
    #endif
    #if (FXAA_GATHER4_ALPHA == 1)
        #define FxaaTexAlpha4(t, p) textureGather(t, p, 3)
        #define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)
        #define FxaaTexGreen4(t, p) textureGather(t, p, 1)
        #define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)
    #endif
#endif

#if (FXAA_GLSL_130 == 1)
    #define FxaaTexTop(t, p) textureLod(t, p, 0.0)
    #define FxaaTexOff(t, p, o, r) textureLodOffset(t, p, 0.0, o)
    #if (FXAA_GATHER4_ALPHA == 1)
        #define FxaaTexAlpha4(t, p) textureGather(t, p, 3)
        #define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)
        #define FxaaTexGreen4(t, p) textureGather(t, p, 1)
        #define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)
    #endif
#endif

#if (FXAA_HLSL_3 == 1) || (FXAA_360 == 1) || (FXAA_PS3 == 1)
    #define FxaaInt2 float2
    #define FxaaTex sampler2D
    #define FxaaTexTop(t, p) tex2Dlod(t, float4(p, 0.0, 0.0))
    #define FxaaTexOff(t, p, o, r) tex2Dlod(t, float4(p + (o * r), 0, 0))
#endif

#if (FXAA_HLSL_4 == 1)
    #define FxaaInt2 int2
    struct FxaaTex { SamplerState smpl; Texture2D tex; };
    #define FxaaTexTop(t, p) t.tex.SampleLevel(t.smpl, p, 0.0)
    #define FxaaTexOff(t, p, o, r) t.tex.SampleLevel(t.smpl, p, 0.0, o)
#endif

#if (FXAA_HLSL_5 == 1)
    #define FxaaInt2 int2
    struct FxaaTex { SamplerState smpl; Texture2D tex; };
    #define FxaaTexTop(t, p) t.tex.SampleLevel(t.smpl, p, 0.0)
    #define FxaaTexOff(t, p, o, r) t.tex.SampleLevel(t.smpl, p, 0.0, o)
    #define FxaaTexAlpha4(t, p) t.tex.GatherAlpha(t.smpl, p)
    #define FxaaTexOffAlpha4(t, p, o) t.tex.GatherAlpha(t.smpl, p, o)
    #define FxaaTexGreen4(t, p) t.tex.GatherGreen(t.smpl, p)
    #define FxaaTexOffGreen4(t, p, o) t.tex.GatherGreen(t.smpl, p, o)
#endif


#if (FXAA_GREEN_AS_LUMA == 0)
    FxaaFloat FxaaLuma(FxaaFloat4 rgba) { return rgba.w; }
#else
    FxaaFloat FxaaLuma(FxaaFloat4 rgba) { return rgba.y; }
#endif    



#if (FXAA_PC == 1)

FxaaFloat4 FxaaPixelShader(
    FxaaFloat2 pos,

    FxaaFloat4 fxaaConsolePosPos,

    FxaaTex tex,

    FxaaTex fxaaConsole360TexExpBiasNegOne,

    FxaaTex fxaaConsole360TexExpBiasNegTwo,

    FxaaFloat2 fxaaQualityRcpFrame,
 
    FxaaFloat4 fxaaConsoleRcpFrameOpt,

    FxaaFloat4 fxaaConsoleRcpFrameOpt2,

    FxaaFloat4 fxaaConsole360RcpFrameOpt2,

    FxaaFloat fxaaQualitySubpix,

    FxaaFloat fxaaQualityEdgeThreshold,

    FxaaFloat fxaaQualityEdgeThresholdMin,
 
    FxaaFloat fxaaConsoleEdgeSharpness,

    FxaaFloat fxaaConsoleEdgeThreshold,

    FxaaFloat fxaaConsoleEdgeThresholdMin,
 
    FxaaFloat4 fxaaConsole360ConstDir
) {

    FxaaFloat2 posM;
    posM.x = pos.x;
    posM.y = pos.y;
    #if (FXAA_GATHER4_ALPHA == 1)
        #if (FXAA_DISCARD == 0)
            FxaaFloat4 rgbyM = FxaaTexTop(tex, posM);
            #if (FXAA_GREEN_AS_LUMA == 0)
                #define lumaM rgbyM.w
            #else
                #define lumaM rgbyM.y
            #endif
        #endif
        #if (FXAA_GREEN_AS_LUMA == 0)
            FxaaFloat4 luma4A = FxaaTexAlpha4(tex, posM);
            FxaaFloat4 luma4B = FxaaTexOffAlpha4(tex, posM, FxaaInt2(-1, -1));
        #else
            FxaaFloat4 luma4A = FxaaTexGreen4(tex, posM);
            FxaaFloat4 luma4B = FxaaTexOffGreen4(tex, posM, FxaaInt2(-1, -1));
        #endif
        #if (FXAA_DISCARD == 1)
            #define lumaM luma4A.w
        #endif
        #define lumaE luma4A.z
        #define lumaS luma4A.x
        #define lumaSE luma4A.y
        #define lumaNW luma4B.w
        #define lumaN luma4B.z
        #define lumaW luma4B.x
    #else
        FxaaFloat4 rgbyM = FxaaTexTop(tex, posM);
        #if (FXAA_GREEN_AS_LUMA == 0)
            #define lumaM rgbyM.w
        #else
            #define lumaM rgbyM.y
        #endif
        FxaaFloat lumaS = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0, 1), fxaaQualityRcpFrame.xy));
        FxaaFloat lumaE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 0), fxaaQualityRcpFrame.xy));
        FxaaFloat lumaN = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0,-1), fxaaQualityRcpFrame.xy));
        FxaaFloat lumaW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 0), fxaaQualityRcpFrame.xy));
    #endif

    FxaaFloat maxSM = max(lumaS, lumaM);
    FxaaFloat minSM = min(lumaS, lumaM);
    FxaaFloat maxESM = max(lumaE, maxSM);
    FxaaFloat minESM = min(lumaE, minSM);
    FxaaFloat maxWN = max(lumaN, lumaW);
    FxaaFloat minWN = min(lumaN, lumaW);
    FxaaFloat rangeMax = max(maxWN, maxESM);
    FxaaFloat rangeMin = min(minWN, minESM);
    FxaaFloat rangeMaxScaled = rangeMax * fxaaQualityEdgeThreshold;
    FxaaFloat range = rangeMax - rangeMin;
    FxaaFloat rangeMaxClamped = max(fxaaQualityEdgeThresholdMin, rangeMaxScaled);
    FxaaBool earlyExit = range < rangeMaxClamped;

    if(earlyExit)
        #if (FXAA_DISCARD == 1)
            FxaaDiscard;
        #else
            return rgbyM;
        #endif

    #if (FXAA_GATHER4_ALPHA == 0)
        FxaaFloat lumaNW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1,-1), fxaaQualityRcpFrame.xy));
        FxaaFloat lumaSE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 1), fxaaQualityRcpFrame.xy));
        FxaaFloat lumaNE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1,-1), fxaaQualityRcpFrame.xy));
        FxaaFloat lumaSW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 1), fxaaQualityRcpFrame.xy));
    #else
        FxaaFloat lumaNE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(1, -1), fxaaQualityRcpFrame.xy));
        FxaaFloat lumaSW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 1), fxaaQualityRcpFrame.xy));
    #endif

    FxaaFloat lumaNS = lumaN + lumaS;
    FxaaFloat lumaWE = lumaW + lumaE;
    FxaaFloat subpixRcpRange = 1.0/range;
    FxaaFloat subpixNSWE = lumaNS + lumaWE;
    FxaaFloat edgeHorz1 = (-2.0 * lumaM) + lumaNS;
    FxaaFloat edgeVert1 = (-2.0 * lumaM) + lumaWE;

    FxaaFloat lumaNESE = lumaNE + lumaSE;
    FxaaFloat lumaNWNE = lumaNW + lumaNE;
    FxaaFloat edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
    FxaaFloat edgeVert2 = (-2.0 * lumaN) + lumaNWNE;

    FxaaFloat lumaNWSW = lumaNW + lumaSW;
    FxaaFloat lumaSWSE = lumaSW + lumaSE;
    FxaaFloat edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
    FxaaFloat edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);
    FxaaFloat edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
    FxaaFloat edgeVert3 = (-2.0 * lumaS) + lumaSWSE;
    FxaaFloat edgeHorz = abs(edgeHorz3) + edgeHorz4;
    FxaaFloat edgeVert = abs(edgeVert3) + edgeVert4;

    FxaaFloat subpixNWSWNESE = lumaNWSW + lumaNESE;
    FxaaFloat lengthSign = fxaaQualityRcpFrame.x;
    FxaaBool horzSpan = edgeHorz >= edgeVert;
    FxaaFloat subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;

    if(!horzSpan) lumaN = lumaW;
    if(!horzSpan) lumaS = lumaE;
    if(horzSpan) lengthSign = fxaaQualityRcpFrame.y;
    FxaaFloat subpixB = (subpixA * (1.0/12.0)) - lumaM;

    FxaaFloat gradientN = lumaN - lumaM;
    FxaaFloat gradientS = lumaS - lumaM;
    FxaaFloat lumaNN = lumaN + lumaM;
    FxaaFloat lumaSS = lumaS + lumaM;
    FxaaBool pairN = abs(gradientN) >= abs(gradientS);
    FxaaFloat gradient = max(abs(gradientN), abs(gradientS));
    if(pairN) lengthSign = -lengthSign;
    FxaaFloat subpixC = FxaaSat(abs(subpixB) * subpixRcpRange);

    FxaaFloat2 posB;
    posB.x = posM.x;
    posB.y = posM.y;
    FxaaFloat2 offNP;
    offNP.x = (!horzSpan) ? 0.0 : fxaaQualityRcpFrame.x;
    offNP.y = ( horzSpan) ? 0.0 : fxaaQualityRcpFrame.y;
    if(!horzSpan) posB.x += lengthSign * 0.5;
    if( horzSpan) posB.y += lengthSign * 0.5;

    FxaaFloat2 posN;
    posN.x = posB.x - offNP.x * FXAA_QUALITY__P0;
    posN.y = posB.y - offNP.y * FXAA_QUALITY__P0;
    FxaaFloat2 posP;
    posP.x = posB.x + offNP.x * FXAA_QUALITY__P0;
    posP.y = posB.y + offNP.y * FXAA_QUALITY__P0;
    FxaaFloat subpixD = ((-2.0)*subpixC) + 3.0;
    FxaaFloat lumaEndN = FxaaLuma(FxaaTexTop(tex, posN));
    FxaaFloat subpixE = subpixC * subpixC;
    FxaaFloat lumaEndP = FxaaLuma(FxaaTexTop(tex, posP));

    if(!pairN) lumaNN = lumaSS;
    FxaaFloat gradientScaled = gradient * 1.0/4.0;
    FxaaFloat lumaMM = lumaM - lumaNN * 0.5;
    FxaaFloat subpixF = subpixD * subpixE;
    FxaaBool lumaMLTZero = lumaMM < 0.0;

    lumaEndN -= lumaNN * 0.5;
    lumaEndP -= lumaNN * 0.5;
    FxaaBool doneN = abs(lumaEndN) >= gradientScaled;
    FxaaBool doneP = abs(lumaEndP) >= gradientScaled;
    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P1;
    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P1;
    FxaaBool doneNP = (!doneN) || (!doneP);
    if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P1;
    if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P1;

    if(doneNP) {
        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
        doneN = abs(lumaEndN) >= gradientScaled;
        doneP = abs(lumaEndP) >= gradientScaled;
        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P2;
        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P2;
        doneNP = (!doneN) || (!doneP);
        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P2;
        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P2;

        #if (FXAA_QUALITY__PS > 3)
        if(doneNP) {
            if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
            if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
            doneN = abs(lumaEndN) >= gradientScaled;
            doneP = abs(lumaEndP) >= gradientScaled;
            if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P3;
            if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P3;
            doneNP = (!doneN) || (!doneP);
            if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P3;
            if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P3;

            #if (FXAA_QUALITY__PS > 4)
            if(doneNP) {
                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                doneN = abs(lumaEndN) >= gradientScaled;
                doneP = abs(lumaEndP) >= gradientScaled;
                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P4;
                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P4;
                doneNP = (!doneN) || (!doneP);
                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P4;
                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P4;

                #if (FXAA_QUALITY__PS > 5)
                if(doneNP) {
                    if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                    if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                    if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                    if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                    doneN = abs(lumaEndN) >= gradientScaled;
                    doneP = abs(lumaEndP) >= gradientScaled;
                    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P5;
                    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P5;
                    doneNP = (!doneN) || (!doneP);
                    if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P5;
                    if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P5;

                    #if (FXAA_QUALITY__PS > 6)
                    if(doneNP) {
                        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                        doneN = abs(lumaEndN) >= gradientScaled;
                        doneP = abs(lumaEndP) >= gradientScaled;
                        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P6;
                        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P6;
                        doneNP = (!doneN) || (!doneP);
                        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P6;
                        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P6;

                        #if (FXAA_QUALITY__PS > 7)
                        if(doneNP) {
                            if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                            if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                            doneN = abs(lumaEndN) >= gradientScaled;
                            doneP = abs(lumaEndP) >= gradientScaled;
                            if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P7;
                            if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P7;
                            doneNP = (!doneN) || (!doneP);
                            if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P7;
                            if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P7;

    #if (FXAA_QUALITY__PS > 8)
    if(doneNP) {
        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
        doneN = abs(lumaEndN) >= gradientScaled;
        doneP = abs(lumaEndP) >= gradientScaled;
        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P8;
        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P8;
        doneNP = (!doneN) || (!doneP);
        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P8;
        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P8;

        #if (FXAA_QUALITY__PS > 9)
        if(doneNP) {
            if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
            if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
            doneN = abs(lumaEndN) >= gradientScaled;
            doneP = abs(lumaEndP) >= gradientScaled;
            if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P9;
            if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P9;
            doneNP = (!doneN) || (!doneP);
            if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P9;
            if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P9;

            #if (FXAA_QUALITY__PS > 10)
            if(doneNP) {
                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                doneN = abs(lumaEndN) >= gradientScaled;
                doneP = abs(lumaEndP) >= gradientScaled;
                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P10;
                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P10;
                doneNP = (!doneN) || (!doneP);
                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P10;
                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P10;

                #if (FXAA_QUALITY__PS > 11)
                if(doneNP) {
                    if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                    if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                    if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                    if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                    doneN = abs(lumaEndN) >= gradientScaled;
                    doneP = abs(lumaEndP) >= gradientScaled;
                    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P11;
                    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P11;
                    doneNP = (!doneN) || (!doneP);
                    if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P11;
                    if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P11;

                    #if (FXAA_QUALITY__PS > 12)
                    if(doneNP) {
                        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                        doneN = abs(lumaEndN) >= gradientScaled;
                        doneP = abs(lumaEndP) >= gradientScaled;
                        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P12;
                        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P12;
                        doneNP = (!doneN) || (!doneP);
                        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P12;
                        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P12;

                    }
                    #endif

                }
                #endif

            }
            #endif

        }
        #endif

    }
    #endif

                        }
                        #endif

                    }
                    #endif

                }
                #endif

            }
            #endif

        }
        #endif

    }

    FxaaFloat dstN = posM.x - posN.x;
    FxaaFloat dstP = posP.x - posM.x;
    if(!horzSpan) dstN = posM.y - posN.y;
    if(!horzSpan) dstP = posP.y - posM.y;

    FxaaBool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
    FxaaFloat spanLength = (dstP + dstN);
    FxaaBool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
    FxaaFloat spanLengthRcp = 1.0/spanLength;

    FxaaBool directionN = dstN < dstP;
    FxaaFloat dst = min(dstN, dstP);
    FxaaBool goodSpan = directionN ? goodSpanN : goodSpanP;
    FxaaFloat subpixG = subpixF * subpixF;
    FxaaFloat pixelOffset = (dst * (-spanLengthRcp)) + 0.5;
    FxaaFloat subpixH = subpixG * fxaaQualitySubpix;

    FxaaFloat pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
    FxaaFloat pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
    if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
    if( horzSpan) posM.y += pixelOffsetSubpix * lengthSign;
    #if (FXAA_DISCARD == 1)
        return FxaaTexTop(tex, posM);
    #else
        return FxaaFloat4(FxaaTexTop(tex, posM).xyz, lumaM);
    #endif
}

#endif




#if (FXAA_PC_CONSOLE == 1)

FxaaFloat4 FxaaPixelShader(

    FxaaFloat2 pos,
    FxaaFloat4 fxaaConsolePosPos,
    FxaaTex tex,
    FxaaTex fxaaConsole360TexExpBiasNegOne,
    FxaaTex fxaaConsole360TexExpBiasNegTwo,
    FxaaFloat2 fxaaQualityRcpFrame,
    FxaaFloat4 fxaaConsoleRcpFrameOpt,
    FxaaFloat4 fxaaConsoleRcpFrameOpt2,
    FxaaFloat4 fxaaConsole360RcpFrameOpt2,
    FxaaFloat fxaaQualitySubpix,
    FxaaFloat fxaaQualityEdgeThreshold,
    FxaaFloat fxaaQualityEdgeThresholdMin,
    FxaaFloat fxaaConsoleEdgeSharpness,
    FxaaFloat fxaaConsoleEdgeThreshold,
    FxaaFloat fxaaConsoleEdgeThresholdMin,
    FxaaFloat4 fxaaConsole360ConstDir
) {

    FxaaFloat lumaNw = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.xy));
    FxaaFloat lumaSw = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.xw));
    FxaaFloat lumaNe = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.zy));
    FxaaFloat lumaSe = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.zw));

    FxaaFloat4 rgbyM = FxaaTexTop(tex, pos.xy);
    #if (FXAA_GREEN_AS_LUMA == 0)
        FxaaFloat lumaM = rgbyM.w;
    #else
        FxaaFloat lumaM = rgbyM.y;
    #endif

    FxaaFloat lumaMaxNwSw = max(lumaNw, lumaSw);
    lumaNe += 1.0/384.0;
    FxaaFloat lumaMinNwSw = min(lumaNw, lumaSw);

    FxaaFloat lumaMaxNeSe = max(lumaNe, lumaSe);
    FxaaFloat lumaMinNeSe = min(lumaNe, lumaSe);

    FxaaFloat lumaMax = max(lumaMaxNeSe, lumaMaxNwSw);
    FxaaFloat lumaMin = min(lumaMinNeSe, lumaMinNwSw);

    FxaaFloat lumaMaxScaled = lumaMax * fxaaConsoleEdgeThreshold;

    FxaaFloat lumaMinM = min(lumaMin, lumaM);
    FxaaFloat lumaMaxScaledClamped = max(fxaaConsoleEdgeThresholdMin, lumaMaxScaled);
    FxaaFloat lumaMaxM = max(lumaMax, lumaM);
    FxaaFloat dirSwMinusNe = lumaSw - lumaNe;
    FxaaFloat lumaMaxSubMinM = lumaMaxM - lumaMinM;
    FxaaFloat dirSeMinusNw = lumaSe - lumaNw;
    if(lumaMaxSubMinM < lumaMaxScaledClamped) return rgbyM;

    FxaaFloat2 dir;
    dir.x = dirSwMinusNe + dirSeMinusNw;
    dir.y = dirSwMinusNe - dirSeMinusNw;

    FxaaFloat2 dir1 = normalize(dir.xy);
    FxaaFloat4 rgbyN1 = FxaaTexTop(tex, pos.xy - dir1 * fxaaConsoleRcpFrameOpt.zw);
    FxaaFloat4 rgbyP1 = FxaaTexTop(tex, pos.xy + dir1 * fxaaConsoleRcpFrameOpt.zw);

    FxaaFloat dirAbsMinTimesC = min(abs(dir1.x), abs(dir1.y)) * fxaaConsoleEdgeSharpness;
    FxaaFloat2 dir2 = clamp(dir1.xy / dirAbsMinTimesC, -2.0, 2.0);

    FxaaFloat4 rgbyN2 = FxaaTexTop(tex, pos.xy - dir2 * fxaaConsoleRcpFrameOpt2.zw);
    FxaaFloat4 rgbyP2 = FxaaTexTop(tex, pos.xy + dir2 * fxaaConsoleRcpFrameOpt2.zw);

    FxaaFloat4 rgbyA = rgbyN1 + rgbyP1;
    FxaaFloat4 rgbyB = ((rgbyN2 + rgbyP2) * 0.25) + (rgbyA * 0.25);

    #if (FXAA_GREEN_AS_LUMA == 0)
        FxaaBool twoTap = (rgbyB.w < lumaMin) || (rgbyB.w > lumaMax);
    #else
        FxaaBool twoTap = (rgbyB.y < lumaMin) || (rgbyB.y > lumaMax);
    #endif
    if(twoTap) rgbyB.xyz = rgbyA.xyz * 0.5;
    return rgbyB; }

#endif



#if (FXAA_360 == 1)

[reduceTempRegUsage(4)]
float4 FxaaPixelShader(

    FxaaFloat2 pos,
    FxaaFloat4 fxaaConsolePosPos,
    FxaaTex tex,
    FxaaTex fxaaConsole360TexExpBiasNegOne,
    FxaaTex fxaaConsole360TexExpBiasNegTwo,
    FxaaFloat2 fxaaQualityRcpFrame,
    FxaaFloat4 fxaaConsoleRcpFrameOpt,
    FxaaFloat4 fxaaConsoleRcpFrameOpt2,
    FxaaFloat4 fxaaConsole360RcpFrameOpt2,
    FxaaFloat fxaaQualitySubpix,
    FxaaFloat fxaaQualityEdgeThreshold,
    FxaaFloat fxaaQualityEdgeThresholdMin,
    FxaaFloat fxaaConsoleEdgeSharpness,
    FxaaFloat fxaaConsoleEdgeThreshold,
    FxaaFloat fxaaConsoleEdgeThresholdMin,
    FxaaFloat4 fxaaConsole360ConstDir
) {

    float4 lumaNwNeSwSe;
    #if (FXAA_GREEN_AS_LUMA == 0)
        asm { 
            tfetch2D lumaNwNeSwSe.w___, tex, pos.xy, OffsetX = -0.5, OffsetY = -0.5, UseComputedLOD=false
            tfetch2D lumaNwNeSwSe._w__, tex, pos.xy, OffsetX =  0.5, OffsetY = -0.5, UseComputedLOD=false
            tfetch2D lumaNwNeSwSe.__w_, tex, pos.xy, OffsetX = -0.5, OffsetY =  0.5, UseComputedLOD=false
            tfetch2D lumaNwNeSwSe.___w, tex, pos.xy, OffsetX =  0.5, OffsetY =  0.5, UseComputedLOD=false
        };
    #else
        asm { 
            tfetch2D lumaNwNeSwSe.y___, tex, pos.xy, OffsetX = -0.5, OffsetY = -0.5, UseComputedLOD=false
            tfetch2D lumaNwNeSwSe._y__, tex, pos.xy, OffsetX =  0.5, OffsetY = -0.5, UseComputedLOD=false
            tfetch2D lumaNwNeSwSe.__y_, tex, pos.xy, OffsetX = -0.5, OffsetY =  0.5, UseComputedLOD=false
            tfetch2D lumaNwNeSwSe.___y, tex, pos.xy, OffsetX =  0.5, OffsetY =  0.5, UseComputedLOD=false
        };
    #endif

    lumaNwNeSwSe.y += 1.0/384.0;
    float2 lumaMinTemp = min(lumaNwNeSwSe.xy, lumaNwNeSwSe.zw);
    float2 lumaMaxTemp = max(lumaNwNeSwSe.xy, lumaNwNeSwSe.zw);
    float lumaMin = min(lumaMinTemp.x, lumaMinTemp.y);
    float lumaMax = max(lumaMaxTemp.x, lumaMaxTemp.y);

    float4 rgbyM = tex2Dlod(tex, float4(pos.xy, 0.0, 0.0));
    #if (FXAA_GREEN_AS_LUMA == 0)
        float lumaMinM = min(lumaMin, rgbyM.w);
        float lumaMaxM = max(lumaMax, rgbyM.w);
    #else
        float lumaMinM = min(lumaMin, rgbyM.y);
        float lumaMaxM = max(lumaMax, rgbyM.y);
    #endif        
    if((lumaMaxM - lumaMinM) < max(fxaaConsoleEdgeThresholdMin, lumaMax * fxaaConsoleEdgeThreshold)) return rgbyM;

    float2 dir;
    dir.x = dot(lumaNwNeSwSe, fxaaConsole360ConstDir.yyxx);
    dir.y = dot(lumaNwNeSwSe, fxaaConsole360ConstDir.xyxy);
    dir = normalize(dir);

    float4 dir1 = dir.xyxy * fxaaConsoleRcpFrameOpt.xyzw;

    float4 dir2;
    float dirAbsMinTimesC = min(abs(dir.x), abs(dir.y)) * fxaaConsoleEdgeSharpness;
    dir2 = saturate(fxaaConsole360ConstDir.zzww * dir.xyxy / dirAbsMinTimesC + 0.5);
    dir2 = dir2 * fxaaConsole360RcpFrameOpt2.xyxy + fxaaConsole360RcpFrameOpt2.zwzw;

    float4 rgbyN1 = tex2Dlod(fxaaConsole360TexExpBiasNegOne, float4(pos.xy + dir1.xy, 0.0, 0.0));
    float4 rgbyP1 = tex2Dlod(fxaaConsole360TexExpBiasNegOne, float4(pos.xy + dir1.zw, 0.0, 0.0));
    float4 rgbyN2 = tex2Dlod(fxaaConsole360TexExpBiasNegTwo, float4(pos.xy + dir2.xy, 0.0, 0.0));
    float4 rgbyP2 = tex2Dlod(fxaaConsole360TexExpBiasNegTwo, float4(pos.xy + dir2.zw, 0.0, 0.0));

    float4 rgbyA = rgbyN1 + rgbyP1;
    float4 rgbyB = rgbyN2 + rgbyP2 + rgbyA * 0.5;

    float4 rgbyR = ((FxaaLuma(rgbyB) - lumaMax) > 0.0) ? rgbyA : rgbyB; 
    rgbyR = ((FxaaLuma(rgbyB) - lumaMin) > 0.0) ? rgbyR : rgbyA; 
    return rgbyR; }

#endif



#if (FXAA_PS3 == 1) && (FXAA_EARLY_EXIT == 0)

#pragma regcount 7
#pragma disablepc all
#pragma option O3
#pragma option OutColorPrec=fp16
#pragma texformat default RGBA8

half4 FxaaPixelShader(

    FxaaFloat2 pos,
    FxaaFloat4 fxaaConsolePosPos,
    FxaaTex tex,
    FxaaTex fxaaConsole360TexExpBiasNegOne,
    FxaaTex fxaaConsole360TexExpBiasNegTwo,
    FxaaFloat2 fxaaQualityRcpFrame,
    FxaaFloat4 fxaaConsoleRcpFrameOpt,
    FxaaFloat4 fxaaConsoleRcpFrameOpt2,
    FxaaFloat4 fxaaConsole360RcpFrameOpt2,
    FxaaFloat fxaaQualitySubpix,
    FxaaFloat fxaaQualityEdgeThreshold,
    FxaaFloat fxaaQualityEdgeThresholdMin,
    FxaaFloat fxaaConsoleEdgeSharpness,
    FxaaFloat fxaaConsoleEdgeThreshold,
    FxaaFloat fxaaConsoleEdgeThresholdMin,
    FxaaFloat4 fxaaConsole360ConstDir
) {


    half4 dir;
    half4 lumaNe = h4tex2Dlod(tex, half4(fxaaConsolePosPos.zy, 0, 0));
    #if (FXAA_GREEN_AS_LUMA == 0)
        lumaNe.w += half(1.0/512.0);
        dir.x = -lumaNe.w;
        dir.z = -lumaNe.w;
    #else
        lumaNe.y += half(1.0/512.0);
        dir.x = -lumaNe.y;
        dir.z = -lumaNe.y;
    #endif


    half4 lumaSw = h4tex2Dlod(tex, half4(fxaaConsolePosPos.xw, 0, 0));
    #if (FXAA_GREEN_AS_LUMA == 0)
        dir.x += lumaSw.w;
        dir.z += lumaSw.w;
    #else
        dir.x += lumaSw.y;
        dir.z += lumaSw.y;
    #endif        


    half4 lumaNw = h4tex2Dlod(tex, half4(fxaaConsolePosPos.xy, 0, 0));
    #if (FXAA_GREEN_AS_LUMA == 0)
        dir.x -= lumaNw.w;
        dir.z += lumaNw.w;
    #else
        dir.x -= lumaNw.y;
        dir.z += lumaNw.y;
    #endif


    half4 lumaSe = h4tex2Dlod(tex, half4(fxaaConsolePosPos.zw, 0, 0));
    #if (FXAA_GREEN_AS_LUMA == 0)
        dir.x += lumaSe.w;
        dir.z -= lumaSe.w;
    #else
        dir.x += lumaSe.y;
        dir.z -= lumaSe.y;
    #endif


    half4 dir1_pos;
    dir1_pos.xy = normalize(dir.xyz).xz;
    half dirAbsMinTimesC = min(abs(dir1_pos.x), abs(dir1_pos.y)) * half(FXAA_CONSOLE__PS3_EDGE_SHARPNESS);


    half4 dir2_pos;
    dir2_pos.xy = clamp(dir1_pos.xy / dirAbsMinTimesC, half(-2.0), half(2.0));
    dir1_pos.zw = pos.xy;
    dir2_pos.zw = pos.xy;
    half4 temp1N;
    temp1N.xy = dir1_pos.zw - dir1_pos.xy * fxaaConsoleRcpFrameOpt.zw;


    temp1N = h4tex2Dlod(tex, half4(temp1N.xy, 0.0, 0.0));
    half4 rgby1;
    rgby1.xy = dir1_pos.zw + dir1_pos.xy * fxaaConsoleRcpFrameOpt.zw;


    rgby1 = h4tex2Dlod(tex, half4(rgby1.xy, 0.0, 0.0));
    rgby1 = (temp1N + rgby1) * 0.5;


    half4 temp2N;
    temp2N.xy = dir2_pos.zw - dir2_pos.xy * fxaaConsoleRcpFrameOpt2.zw;
    temp2N = h4tex2Dlod(tex, half4(temp2N.xy, 0.0, 0.0));


    half4 rgby2;
    rgby2.xy = dir2_pos.zw + dir2_pos.xy * fxaaConsoleRcpFrameOpt2.zw;
    rgby2 = h4tex2Dlod(tex, half4(rgby2.xy, 0.0, 0.0));
    rgby2 = (temp2N + rgby2) * 0.5;

    #if (FXAA_GREEN_AS_LUMA == 0)
        half lumaMin = min(min(lumaNw.w, lumaSw.w), min(lumaNe.w, lumaSe.w));
        half lumaMax = max(max(lumaNw.w, lumaSw.w), max(lumaNe.w, lumaSe.w));
    #else
        half lumaMin = min(min(lumaNw.y, lumaSw.y), min(lumaNe.y, lumaSe.y));
        half lumaMax = max(max(lumaNw.y, lumaSw.y), max(lumaNe.y, lumaSe.y));
    #endif        
    rgby2 = (rgby2 + rgby1) * 0.5;


    #if (FXAA_GREEN_AS_LUMA == 0)
        bool twoTapLt = rgby2.w < lumaMin;
        bool twoTapGt = rgby2.w > lumaMax;
    #else
        bool twoTapLt = rgby2.y < lumaMin;
        bool twoTapGt = rgby2.y > lumaMax;
    #endif


    if(twoTapLt || twoTapGt) rgby2 = rgby1;

    return rgby2; }

#endif




#if (FXAA_PS3 == 1) && (FXAA_EARLY_EXIT == 1)

#pragma regcount 7
#pragma disablepc all
#pragma option O2
#pragma option OutColorPrec=fp16
#pragma texformat default RGBA8

half4 FxaaPixelShader(

    FxaaFloat2 pos,
    FxaaFloat4 fxaaConsolePosPos,
    FxaaTex tex,
    FxaaTex fxaaConsole360TexExpBiasNegOne,
    FxaaTex fxaaConsole360TexExpBiasNegTwo,
    FxaaFloat2 fxaaQualityRcpFrame,
    FxaaFloat4 fxaaConsoleRcpFrameOpt,
    FxaaFloat4 fxaaConsoleRcpFrameOpt2,
    FxaaFloat4 fxaaConsole360RcpFrameOpt2,
    FxaaFloat fxaaQualitySubpix,
    FxaaFloat fxaaQualityEdgeThreshold,
    FxaaFloat fxaaQualityEdgeThresholdMin,
    FxaaFloat fxaaConsoleEdgeSharpness,
    FxaaFloat fxaaConsoleEdgeThreshold,
    FxaaFloat fxaaConsoleEdgeThresholdMin,
    FxaaFloat4 fxaaConsole360ConstDir
) {


    half4 rgbyNe = h4tex2Dlod(tex, half4(fxaaConsolePosPos.zy, 0, 0));
    #if (FXAA_GREEN_AS_LUMA == 0)
        half lumaNe = rgbyNe.w + half(1.0/512.0);
    #else
        half lumaNe = rgbyNe.y + half(1.0/512.0);
    #endif


    half4 lumaSw = h4tex2Dlod(tex, half4(fxaaConsolePosPos.xw, 0, 0));
    #if (FXAA_GREEN_AS_LUMA == 0)
        half lumaSwNegNe = lumaSw.w - lumaNe;
    #else
        half lumaSwNegNe = lumaSw.y - lumaNe;
    #endif


    half4 lumaNw = h4tex2Dlod(tex, half4(fxaaConsolePosPos.xy, 0, 0));
    #if (FXAA_GREEN_AS_LUMA == 0)
        half lumaMaxNwSw = max(lumaNw.w, lumaSw.w);
        half lumaMinNwSw = min(lumaNw.w, lumaSw.w);
    #else
        half lumaMaxNwSw = max(lumaNw.y, lumaSw.y);
        half lumaMinNwSw = min(lumaNw.y, lumaSw.y);
    #endif

    half4 lumaSe = h4tex2Dlod(tex, half4(fxaaConsolePosPos.zw, 0, 0));
    #if (FXAA_GREEN_AS_LUMA == 0)
        half dirZ =  lumaNw.w + lumaSwNegNe;
        half dirX = -lumaNw.w + lumaSwNegNe;
    #else
        half dirZ =  lumaNw.y + lumaSwNegNe;
        half dirX = -lumaNw.y + lumaSwNegNe;
    #endif


    half3 dir;
    dir.y = 0.0;
    #if (FXAA_GREEN_AS_LUMA == 0)
        dir.x =  lumaSe.w + dirX;
        dir.z = -lumaSe.w + dirZ;
        half lumaMinNeSe = min(lumaNe, lumaSe.w);
    #else
        dir.x =  lumaSe.y + dirX;
        dir.z = -lumaSe.y + dirZ;
        half lumaMinNeSe = min(lumaNe, lumaSe.y);
    #endif


    half4 dir1_pos;
    dir1_pos.xy = normalize(dir).xz;
    half dirAbsMinTimes8 = min(abs(dir1_pos.x), abs(dir1_pos.y)) * half(FXAA_CONSOLE__PS3_EDGE_SHARPNESS);


    half4 dir2_pos;
    dir2_pos.xy = clamp(dir1_pos.xy / dirAbsMinTimes8, half(-2.0), half(2.0));
    dir1_pos.zw = pos.xy;
    dir2_pos.zw = pos.xy;
    #if (FXAA_GREEN_AS_LUMA == 0)
        half lumaMaxNeSe = max(lumaNe, lumaSe.w);
    #else
        half lumaMaxNeSe = max(lumaNe, lumaSe.y);
    #endif


    half4 temp1N;
    temp1N.xy = dir1_pos.zw - dir1_pos.xy * fxaaConsoleRcpFrameOpt.zw;
    temp1N = h4tex2Dlod(tex, half4(temp1N.xy, 0.0, 0.0));
    half lumaMax = max(lumaMaxNwSw, lumaMaxNeSe);
    half lumaMin = min(lumaMinNwSw, lumaMinNeSe);


    half4 rgby1;
    rgby1.xy = dir1_pos.zw + dir1_pos.xy * fxaaConsoleRcpFrameOpt.zw;
    rgby1 = h4tex2Dlod(tex, half4(rgby1.xy, 0.0, 0.0));
    rgby1 = (temp1N + rgby1) * 0.5;


    half4 rgbyM = h4tex2Dlod(tex, half4(pos.xy, 0.0, 0.0));
    #if (FXAA_GREEN_AS_LUMA == 0)
        half lumaMaxM = max(lumaMax, rgbyM.w);
        half lumaMinM = min(lumaMin, rgbyM.w);
    #else
        half lumaMaxM = max(lumaMax, rgbyM.y);
        half lumaMinM = min(lumaMin, rgbyM.y);
    #endif


    half4 temp2N;
    temp2N.xy = dir2_pos.zw - dir2_pos.xy * fxaaConsoleRcpFrameOpt2.zw;
    temp2N = h4tex2Dlod(tex, half4(temp2N.xy, 0.0, 0.0));
    half4 rgby2;
    rgby2.xy = dir2_pos.zw + dir2_pos.xy * fxaaConsoleRcpFrameOpt2.zw;
    half lumaRangeM = (lumaMaxM - lumaMinM) / FXAA_CONSOLE__PS3_EDGE_THRESHOLD;


    rgby2 = h4tex2Dlod(tex, half4(rgby2.xy, 0.0, 0.0));
    rgby2 = (temp2N + rgby2) * 0.5;

    rgby2 = (rgby2 + rgby1) * 0.5;


    #if (FXAA_GREEN_AS_LUMA == 0)
        bool twoTapLt = rgby2.w < lumaMin;
        bool twoTapGt = rgby2.w > lumaMax;
    #else
        bool twoTapLt = rgby2.y < lumaMin;
        bool twoTapGt = rgby2.y > lumaMax;
    #endif
    bool earlyExit = lumaRangeM < lumaMax;
    bool twoTap = twoTapLt || twoTapGt;


    if(twoTap) rgby2 = rgby1;
    if(earlyExit) rgby2 = rgbyM;

    return rgby2; }

#endif


vec2 getZoomUv( vec2 orgUv, vec2 move, float zoomFactor )
{
    highp vec2 uv;
    uv += move;
    uv *= zoomFactor;

    return uv;
}

void main()
{
	FxaaFloat2 _pos = v_texCoords;
	FxaaFloat4 fxaaConsolePosPos = vec4(0);

	FxaaFloat2 fxaaQualityRcpFrame = 1.0/g_Resolution;
	FxaaFloat4 fxaaConsoleRcpFrameOpt = vec4(0);
	FxaaFloat4 fxaaConsoleRcpFrameOpt2 = vec4(0);
	FxaaFloat4 fxaaConsole360RcpFrameOpt2 = vec4(0);
	FxaaFloat fxaaQualitySubpix = m_Subpix;
	FxaaFloat fxaaQualityEdgeThreshold = m_EdgeThreshold;
	FxaaFloat fxaaQualityEdgeThresholdMin = m_EdgeThresholdMin;
	FxaaFloat fxaaConsoleEdgeSharpness = 0.0;
	FxaaFloat fxaaConsoleEdgeThreshold = 0.0;
	FxaaFloat fxaaConsoleEdgeThresholdMin = 0.0;
	FxaaFloat4 fxaaConsole360ConstDir = vec4(0);
    
    _pos = getZoomUv(_pos, vec2(dx, dy), zoomFactor);

    FxaaFloat4 orgSample = FxaaTexTop(u_texture, _pos);

    gl_FragColor = vec4( FxaaPixelShader(_pos, fxaaConsolePosPos, u_texture, u_texture, u_texture, fxaaQualityRcpFrame, 
    				fxaaConsoleRcpFrameOpt, fxaaConsoleRcpFrameOpt2, fxaaConsole360RcpFrameOpt2, fxaaQualitySubpix, fxaaQualityEdgeThreshold, 
    				fxaaQualityEdgeThresholdMin, fxaaConsoleEdgeSharpness, fxaaConsoleEdgeThreshold, fxaaConsoleEdgeThresholdMin, fxaaConsole360ConstDir).xyz, orgSample.w );
}