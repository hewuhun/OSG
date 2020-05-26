#include <HDRLighting.h>
#include <osgDB/ReadFile>
#include <osg/Uniform>
#include <osgViewer/CompositeViewer>
#include <FeUtils/PathRegistry.h>
#include <osg/ShapeDrawable>

using namespace FeHDRLighting;

static const char *HDRSceneVertSource = {
	"varying vec3 vNormal;\n"
	"varying vec4 vVertexPosInCam;\n"
	"void main(void)\n"
	"{\n"
	"    gl_FrontColor = gl_Color;\n"
	"    gl_BackColor = gl_Color;\n"
	"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"}\n"
};

static const char *HDRSceneFragSource = {

	"void main(void)\n"
	"{\n"
	"	gl_FragColor = gl_Color;	\n"
	"	gl_FragColor.xyz *= 60;	\n"
	"}\n"
};

static const char *HDRModelSceneVertSource = {
	"varying vec3 vNormal;\n"
	"varying vec4 vVertexPosInCam;\n"
	"void main(void)\n"
	"{\n"
	"    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"    gl_FrontColor = gl_Color;\n"
	"    gl_BackColor = gl_Color;\n"
	"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"    vVertexPosInCam = gl_ModelViewMatrix * gl_Vertex;\n"
	"    vNormal = normalize(gl_NormalMatrix * gl_Normal);\n"
	"}\n"
};

//fix
static const char *HDRModelSceneFragSource = {
	"const int NUM_LIGHTS = 10; \n"
	"uniform bool uEnableLighting; \n"
	"uniform sampler2D textureMap;\n"
	"uniform vec3 uEmissive;\n"
	"uniform vec4 uAvLightPositionView[NUM_LIGHTS];\n"
	"uniform vec4 uAvLightIntensity[NUM_LIGHTS];\n"
	"uniform vec3 uAvLightColor[NUM_LIGHTS];\n"
	"uniform float uDiffuseCoefficient;\n" 
	"uniform float uPhongCoefficient;\n" 
	"uniform float uPhongExponent;\n"
	"uniform float ulightDiffRadius;\n"
	"uniform bool uRenderLight; \n"
	"varying vec3 vNormal;\n"
	"varying vec4 vVertexPosInCam;\n"
	"void main(void)\n"
	"{\n"
	"	vec4 texColor = texture2D(textureMap,gl_TexCoord[0].xy);\n"
	"	vec4 vColor = vec4(texColor.x * gl_Color.x,texColor.y * gl_Color.y,texColor.z * gl_Color.z,texColor.w * gl_Color.w);\n"
	"if (!uEnableLighting)\n"
	"{                                               \n"
	"	gl_FragColor = vColor;						 \n"
	"}												 \n"
	"else											 \n"
	"{												 \n"
	"	vec3 lightDir = normalize(gl_LightSource[0].position.xyz);\n"
	"   float ambient = 0.2;\n"
	"   float diff = clamp(dot(vNormal, lightDir),0,1 - ambient);\n"
	"	vec3 fColor = vColor.xyz * ambient + vColor.xyz * diff;\n"
	"	vec3 lColor = vec3(0,0,0);\n"

	"   if(uRenderLight)\n"
	"   {\n"
	"   vec3 vPointToCamera = normalize(-vVertexPosInCam.xyz);                                     \n"
	//"   vec3 vIntensity = vec3(0.02, 0.02, 0.02);												\n"
	"   																							\n"
	//"   vIntensity += uEmissive; 																	\n"

	"   for(int iLight = 0; iLight < NUM_LIGHTS; iLight++)											\n"
	"   {																							\n"
	"       if(abs(uAvLightIntensity[iLight].x - 0.0) < 0.00001)                                   \n"
	"			continue;	\n"
	"   	float fDistance = distance(uAvLightPositionView[iLight].xyz, vVertexPosInCam.xyz);		\n"
	"       if(fDistance > ulightDiffRadius)\n"
	"			continue;	\n"
	"       vec3 vlc = uAvLightColor[iLight] * (1 - ambient);                                      \n"
	"       vec3 cIntensity = vec3(0.0, 0.0, 0.0);												\n"
	"   	vec3 vLightToPoint = normalize(vVertexPosInCam.xyz - uAvLightPositionView[iLight].xyz);	\n"
	"   	vec3 vReflection   = normalize(reflect(vLightToPoint,vNormal));									\n"
	"   	float fPhongValue = clamp(dot(vReflection, vPointToCamera),0,1);						\n"
	"   																							\n"
	"   	float fDiffuse = uDiffuseCoefficient * clamp(dot(vNormal, -vLightToPoint),0,1);			\n"
	"   	float fSpecular = uPhongCoefficient * pow(fPhongValue, uPhongExponent);					\n"
	"   																							\n"
	"        float pow = clamp(1 - fDistance / ulightDiffRadius,0,1);                                          \n"
	"        cIntensity = (fDiffuse + fSpecular) * uAvLightIntensity[iLight] * pow;				\n"
	"        vlc *= cIntensity;																	\n"
	"        lColor += vlc;																	    \n"

	"        break; \n"
	//"       fDistance /= 1000;\n"
	//"   	vIntensity += (fDiffuse + fSpecular) * uAvLightIntensity[iLight]/(fDistance * fDistance);\n"
	"   }																							\n"
	//"   lColor *= vIntensity;																	\n"
	"   }\n"

	"	gl_FragColor = vec4(fColor + lColor,vColor.w);\n"
	"}												 \n"
	"}\n"
};
//fix

static const char *DownScale4x4VertSource = {
	"void main(void)\n"
	"{\n"
	"    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"    gl_Position = gl_Vertex;\n"
	"}\n"
};

static const char *DownScale4x4FragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec2 uSampleOffsets[MAX_SAMPLES];\n"
	"uniform sampler2D originalMap;\n"
	"void main(void)\n"
	"{\n"
		"vec4 sample = 0.0;\n"

		"for( int i=0; i < MAX_SAMPLES; i++ )\n"
		"{\n"
		"	sample += texture2D( originalMap, gl_TexCoord[0].xy + uSampleOffsets[i] );\n"
		"}\n"

		"gl_FragColor = sample / MAX_SAMPLES;\n"
	"}\n"
};

static const char *SampleLumInitialFragSource = {
	"vec3 LUMINANCE_VECTOR  = vec3(0.2125, 0.7154, 0.0721);\n"
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec2 uSampleOffsets[MAX_SAMPLES];\n"
	"uniform sampler2D originalMap;\n"
	"void main(void)\n"
	"{\n"
	"vec3 vSample = vec3(0,0,0);                                                              \n"
	"float fLogLumSum = 0.0;														   \n"
	"																				   \n"
	"for(int iSample = 0; iSample < 9; iSample++)									   \n"
	"{																				   \n"
	"	vSample = texture2D(originalMap, gl_TexCoord[0].xy + uSampleOffsets[iSample]).xyz; \n"
	"	fLogLumSum += log(dot(vSample, LUMINANCE_VECTOR) + 0.0001);				   \n"
	"}																				   \n"
	"																				   \n"
	"fLogLumSum /= 9;																   \n"
	"																				   \n"
	"gl_FragColor = vec4(fLogLumSum, fLogLumSum, fLogLumSum, 1.0);					   \n"
	"}\n"
};

static const char *SampleLumIterativeFragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec2 uSampleOffsets[MAX_SAMPLES];\n"
	"uniform sampler2D originalMap;\n"
	"void main(void)\n"
	"{\n"
	"float sample = 0.0;\n"

	"for( int i=0; i < MAX_SAMPLES; i++ )\n"
	"{\n"
	"	sample += texture2D( originalMap, gl_TexCoord[0].xy + uSampleOffsets[i] );\n"
	"}\n"

	"sample /= MAX_SAMPLES;\n"

	"gl_FragColor = vec4(sample,sample,sample,1.0);\n"
	"}\n"
};

static const char *SampleLumFinalFragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec2 uSampleOffsets[MAX_SAMPLES];\n"
	"uniform sampler2D originalMap;\n"
	"void main(void)\n"
	"{\n"
	"float sample = 0.0;\n"

	"for( int i=0; i < MAX_SAMPLES; i++ )\n"
	"{\n"
	"	sample += texture2D( originalMap, gl_TexCoord[0].xy + uSampleOffsets[i] );\n"
	"}\n"

	"sample = exp(sample / 16);\n"

	"gl_FragColor = vec4(sample,sample,sample,1.0);\n"
	"}\n"
};

static const char *AdapteFragSource = {
	"uniform float u_elapsedTime;\n"
	"uniform sampler2D tex0;\n"
	"uniform sampler2D tex1;\n"
	"void main(void)\n"
	"{\n"
		"float fAdaptedLum = texture2D(tex0, vec2(0.5, 0.5)).x;                                                          \n"
		"float fCurrentLum = texture2D(tex1, vec2(0.5, 0.5)).x;															 \n"
		"float fNewAdaptation = fAdaptedLum + (fCurrentLum - fAdaptedLum) * ( 1 - pow( 0.98, 300 * u_elapsedTime ) );\n"
		"gl_FragColor = vec4(fNewAdaptation,fNewAdaptation,fNewAdaptation,1.0);\n"
		//"gl_FragColor = vec4(1.0,0.0,0.0,1.0);\n"
	"}\n"
};

static const char *BrightPassFilterFragSource = {
	"const float BRIGHT_PASS_THRESHOLD  = 5.0;\n"
	"const float BRIGHT_PASS_OFFSET = 10.0;\n"
	"uniform float u_middleGray;\n"
	"uniform sampler2D tex0;\n"
	"uniform sampler2D tex1;\n"
	"void main(void)\n"
	"{\n"
		"vec4 vSample = texture2D( tex0, gl_TexCoord[0].xy );    \n"
		//"float  fAdaptedLum = texture2D( tex1, vec2(0.5, 0.5) ).x; \n"
		"float  fAdaptedLum = 0.0; \n"

		"vSample.xyz *= u_middleGray / (fAdaptedLum + 0.001);	 \n"

		"vSample.xyz -= BRIGHT_PASS_THRESHOLD;					 \n"

		"vSample = max(vSample, 0.0);							 \n"

		"vSample.xyz /= (BRIGHT_PASS_OFFSET + vSample);			 \n"

		"gl_FragColor = vSample;								 \n"
	"}\n"
};

static const char *GaussBlur5x5FragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec2 uSampleOffsets[MAX_SAMPLES];\n"
	"uniform vec4 uSampleWeights[MAX_SAMPLES];\n"
	"uniform sampler2D tex0;\n"
	"void main(void)\n"
	"{\n"
		"vec4 sample = vec4(0.0,0.0,0.0,0.0);                                                       \n"

		"for( int i = 0; i < 12; i++ )																\n"
		"{																							\n"
		"	sample += uSampleWeights[i] * texture2D( tex0, gl_TexCoord[0].xy + uSampleOffsets[i] );	\n"
		"}																							\n"

		"gl_FragColor = sample;																		\n"
	"}\n"
};

static const char *DownScale2x2FragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec2 uSampleOffsets[MAX_SAMPLES];\n"
	"uniform sampler2D tex0;\n"
	"void main(void)\n"
	"{\n"
		"vec4 sample = vec4(0.0,0.0,0.0,0.0);                                   \n"

		"for( int i = 0; i < 4; i++ )												\n"
		"{																		\n"
		"	sample += texture2D( tex0, gl_TexCoord[0].xy + uSampleOffsets[i] );	\n"
		"}																		\n"
	
		"gl_FragColor = sample / 4;												\n"
	"}\n"
};

static const char *BloomFragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec2 uSampleOffsets[MAX_SAMPLES];\n"
	"uniform vec4 uSampleWeights[MAX_SAMPLES];\n"
	"uniform sampler2D tex0;\n"
	"void main(void)\n"
	"{\n"
		"vec4 vSample = vec4(0.0,0.0,0.0,0.0);                            \n"
		"vec4 vColor = vec4(0.0,0.0,0.0,0.0);							  \n"
	
		"vec2 vSamplePosition;											  \n"
	
		"for(int iSample = 0; iSample < 15; iSample++)					  \n"
		"{																  \n"
		"	vSamplePosition = gl_TexCoord[0].xy + uSampleOffsets[iSample];\n"
		"	vColor = texture2D(tex0, vSamplePosition);					  \n"
		"	vSample += uSampleWeights[iSample] * vColor;				  \n"
		"}											\n"

		"gl_FragColor = vSample;	\n"
	"}\n"
};

static const char *StarFragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec2 uSampleOffsets[MAX_SAMPLES];\n"
	"uniform vec4 uSampleWeights[MAX_SAMPLES];\n"
	"uniform sampler2D tex0;\n"
	"void main(void)\n"
	"{\n"
		"vec4 vSample = vec4(0.0,0.0,0.0,0.0);                            \n"
		"vec4 vColor = vec4(0.0,0.0,0.0,0.0);							  \n"
	
		"vec2 vSamplePosition;											  \n"
	
		"for(int iSample = 0; iSample < 8; iSample++)					  \n"
		"{																  \n"
		"	vSamplePosition = gl_TexCoord[0].xy + uSampleOffsets[iSample];\n"
		"	vSample = texture2D(tex0, vSamplePosition);					  \n"
		//"	vColor += uSampleWeights[iSample] * vSample;				  \n"
		"	vColor += 1.0 * vSample;				  \n"
		"}																  \n"

		"gl_FragColor = vColor;											  \n"
	"}\n"
};

static const char *MergeTextures_1FragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec4 uSampleWeights[MAX_SAMPLES];\n"
	"uniform sampler2D tex0;\n"
	"void main(void)\n"
	"{\n"
		"vec4 vColor = vec4(0.0,0.0,0.0,0.0);                              \n"

		"vColor += uSampleWeights[0] * texture2D(tex0, gl_TexCoord[0].xy); \n"

		"gl_FragColor = vColor;											   \n"
	"}\n"
};

static const char *MergeTextures_2FragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec4 uSampleWeights[MAX_SAMPLES];\n"
	"uniform sampler2D tex0;\n"
	"uniform sampler2D tex1;\n"
	"void main(void)\n"
	"{\n"
	"vec4 vColor = vec4(0.0,0.0,0.0,0.0);                              \n"

	"vColor += uSampleWeights[0] * texture2D(tex0, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[1] * texture2D(tex1, gl_TexCoord[0].xy); \n"

	"gl_FragColor = vColor;											   \n"
	"}\n"
};

static const char *MergeTextures_3FragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec4 uSampleWeights[MAX_SAMPLES];\n"
	"uniform sampler2D tex0;\n"
	"uniform sampler2D tex1;\n"
	"uniform sampler2D tex2;\n"
	"void main(void)\n"
	"{\n"
	"vec4 vColor = vec4(0.0,0.0,0.0,0.0);                              \n"

	"vColor += uSampleWeights[0] * texture2D(tex0, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[1] * texture2D(tex1, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[2] * texture2D(tex2, gl_TexCoord[0].xy); \n"

	"gl_FragColor = vColor;											   \n"
	"}\n"
};

static const char *MergeTextures_4FragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec4 uSampleWeights[MAX_SAMPLES];\n"
	"uniform sampler2D tex0;\n"
	"uniform sampler2D tex1;\n"
	"uniform sampler2D tex2;\n"
	"uniform sampler2D tex3;\n"
	"void main(void)\n"
	"{\n"
	"vec4 vColor = vec4(0.0,0.0,0.0,0.0);                              \n"

	"vColor += uSampleWeights[0] * texture2D(tex0, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[1] * texture2D(tex1, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[2] * texture2D(tex2, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[3] * texture2D(tex3, gl_TexCoord[0].xy); \n"

	"gl_FragColor = vColor;											   \n"
	"}\n"
};

static const char *MergeTextures_5FragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec4 uSampleWeights[MAX_SAMPLES];\n"
	"uniform sampler2D tex0;\n"
	"uniform sampler2D tex1;\n"
	"uniform sampler2D tex2;\n"
	"uniform sampler2D tex3;\n"
	"uniform sampler2D tex4;\n"
	"void main(void)\n"
	"{\n"
	"vec4 vColor = vec4(0.0,0.0,0.0,0.0);                              \n"

	"vColor += uSampleWeights[0] * texture2D(tex0, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[1] * texture2D(tex1, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[2] * texture2D(tex2, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[3] * texture2D(tex3, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[4] * texture2D(tex4, gl_TexCoord[0].xy); \n"

	"gl_FragColor = vColor;											   \n"
	"}\n"
};

static const char *MergeTextures_6FragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec4 uSampleWeights[MAX_SAMPLES];\n"
	"uniform sampler2D tex0;\n"
	"uniform sampler2D tex1;\n"
	"uniform sampler2D tex2;\n"
	"uniform sampler2D tex3;\n"
	"uniform sampler2D tex4;\n"
	"uniform sampler2D tex5;\n"
	"void main(void)\n"
	"{\n"
	"vec4 vColor = vec4(0.0,0.0,0.0,0.0);                              \n"

	"vColor += uSampleWeights[0] * texture2D(tex0, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[1] * texture2D(tex1, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[2] * texture2D(tex2, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[3] * texture2D(tex3, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[4] * texture2D(tex4, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[5] * texture2D(tex5, gl_TexCoord[0].xy); \n"

	"gl_FragColor = vColor;											   \n"
	"}\n"
};

static const char *MergeTextures_7FragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec4 uSampleWeights[MAX_SAMPLES];\n"
	"uniform sampler2D tex0;\n"
	"uniform sampler2D tex1;\n"
	"uniform sampler2D tex2;\n"
	"uniform sampler2D tex3;\n"
	"uniform sampler2D tex4;\n"
	"uniform sampler2D tex5;\n"
	"uniform sampler2D tex6;\n"
	"void main(void)\n"
	"{\n"
	"vec4 vColor = vec4(0.0,0.0,0.0,0.0);                              \n"

	"vColor += uSampleWeights[0] * texture2D(tex0, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[1] * texture2D(tex1, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[2] * texture2D(tex2, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[3] * texture2D(tex3, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[4] * texture2D(tex4, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[5] * texture2D(tex5, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[6] * texture2D(tex6, gl_TexCoord[0].xy); \n"

	"gl_FragColor = vColor;											   \n"
	"}\n"
};

static const char *MergeTextures_8FragSource = {
	"const int MAX_SAMPLES = 16;\n"
	"uniform vec4 uSampleWeights[MAX_SAMPLES];\n"
	"uniform sampler2D tex0;\n"
	"uniform sampler2D tex1;\n"
	"uniform sampler2D tex2;\n"
	"uniform sampler2D tex3;\n"
	"uniform sampler2D tex4;\n"
	"uniform sampler2D tex5;\n"
	"uniform sampler2D tex6;\n"
	"uniform sampler2D tex7;\n"
	"void main(void)\n"
	"{\n"
	"vec4 vColor = vec4(0.0,0.0,0.0,0.0);                              \n"

	"vColor += uSampleWeights[0] * texture2D(tex0, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[1] * texture2D(tex1, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[2] * texture2D(tex2, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[3] * texture2D(tex3, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[4] * texture2D(tex4, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[5] * texture2D(tex5, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[6] * texture2D(tex6, gl_TexCoord[0].xy); \n"
	"vColor += uSampleWeights[7] * texture2D(tex7, gl_TexCoord[0].xy); \n"

	"gl_FragColor = vColor;											   \n"
	"}\n"
};

static const char *FinalFragSource = {
	"vec3 LUMINANCE_VECTOR  = vec3(0.2125, 0.7154, 0.0721);\n"
	"vec3 BLUE_SHIFT_VECTOR = float3(1.05, 0.97, 1.27);\n"
	"uniform float u_middleGray;\n"
	"uniform float u_starScale;\n"
	"uniform float u_bloomScale;\n"
	"uniform float u_lightPow;\n"
	"uniform sampler2D tex0;\n"
	"uniform sampler2D tex1;\n"
	"uniform sampler2D tex2;\n"
	"uniform sampler2D tex3;\n"
	"void main(void)\n"
	"{\n"
	"vec4 vSample = texture2D(tex0, gl_TexCoord[0].xy);                            \n"
	"vec4 vBloom = texture2D(tex1, gl_TexCoord[0].xy);							   \n"
	"vec4 vStar = texture2D(tex2, gl_TexCoord[0].xy);							   \n"
	"float fAdaptedLum = texture2D(tex3, vec2(0.5, 0.5));						   \n"
	"																			   \n"
	/*"if( g_bEnableBlueShift )													   \n"
	"{																		   \n"
	"	float fBlueShiftCoefficient = 1.0f - (fAdaptedLum + 1.5)/4.1;			   \n"
	"	fBlueShiftCoefficient = clamp(fBlueShiftCoefficient,0,1);				   \n"
	"																			   \n"
	"	vec3 vRodColor = dot( vSample.xyz, LUMINANCE_VECTOR ) * BLUE_SHIFT_VECTOR; \n"
	"	vSample.xyz = mix( vSample.xyz, vRodColor, fBlueShiftCoefficient );		   \n"
	"}																		   \n"*/
	"																			   \n"
	//"if( g_bEnableToneMap )													   \n"
	//"{																		   \n"
	//"	vSample.xyz *= u_middleGray / (fAdaptedLum + 0.001);					   \n"
	//"	vSample.xyz /= (1.0 + vSample);											   \n"
	//"}  																		   \n"
	"																			   \n"
	"vSample += u_starScale * vStar;											   \n"
	"vSample += u_bloomScale * vBloom;											   \n"
	"																			   \n"
	"gl_FragColor = vSample;													   \n"
	"}\n"
};

//#define HUDTEST

FeHDRLighting::FeHDRLightingNode::FeHDRLightingNode(FeUtil::CRenderContext* rc,osg::Node* lightingModel,EGLARELIBTYPE lt,float starLengthScale,float starScale,float bloomScale)
	:m_rpModelNode(NULL)
	,m_enableLighting(true)
	,m_rpUpdateCallback(NULL)
	,m_minLightBaseValue(0)
    ,m_maxLightBaseValue(63)
	,m_emissiveCoefficient(39.78f)
	,m_emissive(0,0,0)
	,m_opRenderContext(rc)
	,m_phongCoefficient(1.0)
	,m_diffuseCoefficient(2.0)
	,m_phongExponent(5.0)
	,m_rpSceneCamera(NULL)
	,m_rpSceneTex(NULL)
	,m_rpHud(NULL)
	,m_islastFrameActive(false)
	,m_visibleMultiple(100)
	,m_rpDownScale4x4Cam(NULL)
	,m_rpDownScale4x4Tex(NULL)
	,m_curRenderBinNum(-100)
	,m_adaptationInvalid(false)
	,m_texAdaptedLuminanceLast(NULL)
	,m_texAdaptedLuminanceCur(NULL)
	,m_AdaptedLuminanceCam(NULL)
	,m_lastFrameTime(0)
	,m_texBrightPass(NULL)
	,m_texBrightPassCam(NULL)
	,m_keyValue(6.0)
	,m_minKeyValue(0.0)
	,m_maxKeyValue(100.0)
	,m_modelSceneTex(NULL)
	,m_rpModelSceneCam(NULL)
	,m_texStarSource(NULL)
	,m_texStarSourceCam(NULL)
	,m_texBloomSource(NULL)
	,m_texBloomSourceCam(NULL)
	,m_texStarCamGroup(NULL)
	,m_starCamGroupNodeCallback(NULL)
	,m_glareType(lt)
	,m_finalSceneCam(NULL)
	,m_starScale(starScale)
	,m_bloomScale(bloomScale)
	,m_starLengthScale(starLengthScale)
	,m_rpLightingModel(lightingModel)
	,m_lightDiffRadius(100)
	,m_enableBlooming(true)
{
	for (int i = 0;i < NUM_LIGHTS;i++)
	{
		m_lightBaseValue[i] = 35;
		m_avLightPosition[i] = osg::Vec4(0,0,10,1);
		m_lightRadius[i] = 0;
		m_lightPow[i] = 1.0;
		m_lightColor[i] = osg::Vec3(1.0,1.0,1.0);
	}

	for (int i = 0;i < NUM_BLOOM_TEXTURES;i++)
	{
		m_texBloom[i] = NULL;
	}
	for (int i = 0;i < NUM_STAR_TEXTURES;i++)
	{
		m_texStar[i] = NULL;
	}
	for (int i = 0;i < 8;i++)
	{
		m_texStarAdd[i][0] = NULL;
		m_texStarAdd[i][1] = NULL;
	}
	for (int i = 0;i < NUM_BLOOM_TEXTURES;i++)
	{
		m_texBloomCam[i] = NULL;
	}
	for (int i = 0;i < NUM_STAR_TEXTURES;i++)
	{
		m_texStarCam[i] = NULL;
	}
	for (int i = 0;i < 8;i++)
	{
		m_texStarAddCam[i][0] = NULL;
		m_texStarAddCam[i][1] = NULL;
	}

	creatScene();

#ifdef HUDTEST
	addHud();
#endif

	updateLight();

	m_rpUpdateCallback = new MyUpdateCallback(this);
	addCullCallback(m_rpUpdateCallback);

	m_rpPreUpdateCallback = new MyPreUpdateCallback(this);
	osgViewer::CompositeViewer* vv = dynamic_cast<osgViewer::CompositeViewer*>(rc->GetView()->getViewerBase());
	if(vv)
	{
		vv->addPreUpdateCallback(m_rpPreUpdateCallback);
	}

}

FeHDRLighting::FeHDRLightingNode::~FeHDRLightingNode()
{
	
	removeSceneCamFromRoot();
}

void FeHDRLighting::FeHDRLightingNode::creatScene()
{
	createBloomTextures();

	m_rpModelNode = new osg::Group();
	osg::Program* program = new osg::Program;
	program->addShader( new osg::Shader( osg::Shader::VERTEX, HDRSceneVertSource ) );
	program->addShader( new osg::Shader( osg::Shader::FRAGMENT, HDRSceneFragSource ) );
	m_rpModelNode->getOrCreateStateSet()->setAttributeAndModes( program, osg::StateAttribute::ON );

	if(m_rpLightingModel.valid())
	{
		osg::Program* program = new osg::Program;
		program->addShader( new osg::Shader( osg::Shader::VERTEX, HDRModelSceneVertSource ) );
		program->addShader( new osg::Shader( osg::Shader::FRAGMENT, HDRModelSceneFragSource ) );
		m_rpLightingModel->getOrCreateStateSet()->setAttributeAndModes( program, osg::StateAttribute::ON );
	}

	initUniform();
	addChild(m_rpModelNode);

	//fix
	for (int i = 0;i < NUM_LIGHTS;i++)
	{
		//osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3d(0,0,0),1));
		float lp = osg::clampBetween(m_lightPow[i],0.0f,1.0f);
		LightSphereGeomerty* sd = new LightSphereGeomerty();
		osg::Geode* gn = new osg::Geode();
		gn->addDrawable(sd);

		m_rpLightNode[i] = new osg::MatrixTransform();
		osg::Matrix mat;
		mat.makeIdentity();
		m_rpLightNode[i]->setMatrix(mat);
		m_rpLightNode[i]->addChild(gn);

		m_rpModelNode->addChild(m_rpLightNode[i]);
	}
	//fix
}

void FeHDRLighting::FeHDRLightingNode::initUniform()
{
	if(m_rpLightingModel.valid())
	{
		m_rpLightingModel->getOrCreateStateSet()->addUniform( new osg::Uniform("textureMap", 0) );
		m_rpLightingModel->getOrCreateStateSet()->addUniform( new osg::Uniform("uEnableLighting", true) );
		m_rpLightingModel->getOrCreateStateSet()->addUniform( new osg::Uniform("uEmissive", m_emissive) );
		m_rpLightingModel->getOrCreateStateSet()->addUniform( new osg::Uniform("uRenderLight", true) );
		osg::Uniform*  uAvLightPositionView = new osg::Uniform();
		uAvLightPositionView->setName("uAvLightPositionView");
		uAvLightPositionView->setNumElements(NUM_LIGHTS);
		uAvLightPositionView->setType(osg::Uniform::FLOAT_VEC4);
		m_rpLightingModel->getOrCreateStateSet()->addUniform( uAvLightPositionView);	

		osg::Uniform*  uAvLightIntensity = new osg::Uniform();
		uAvLightIntensity->setName("uAvLightIntensity");
		uAvLightIntensity->setNumElements(NUM_LIGHTS);
		uAvLightIntensity->setType(osg::Uniform::FLOAT_VEC4);
		m_rpLightingModel->getOrCreateStateSet()->addUniform( uAvLightIntensity );	

		osg::Uniform*  uAvLightColor = new osg::Uniform();
		uAvLightColor->setName("uAvLightColor");
		uAvLightColor->setNumElements(NUM_LIGHTS);
		uAvLightColor->setType(osg::Uniform::FLOAT_VEC3);
		m_rpLightingModel->getOrCreateStateSet()->addUniform( uAvLightColor );	

		m_rpLightingModel->getOrCreateStateSet()->addUniform( new osg::Uniform("uDiffuseCoefficient", m_diffuseCoefficient) );
		m_rpLightingModel->getOrCreateStateSet()->addUniform( new osg::Uniform("uPhongCoefficient", m_phongCoefficient) );
		m_rpLightingModel->getOrCreateStateSet()->addUniform( new osg::Uniform("uPhongExponent", m_phongExponent) );
		m_rpLightingModel->getOrCreateStateSet()->addUniform( new osg::Uniform("ulightDiffRadius", m_lightDiffRadius) );
		
	}
}

void FeHDRLighting::FeHDRLightingNode::updateUniform()
{
	if(m_rpLightingModel.valid())
	{
		osg::Uniform* uEnableLighting = m_rpLightingModel->getOrCreateStateSet()->getUniform("uEnableLighting");
		if(uEnableLighting)
		{
			uEnableLighting->set(m_enableLighting);
		}

		osg::Uniform* uEmissive = m_rpLightingModel->getOrCreateStateSet()->getUniform("uEmissive");
		if(uEmissive)
		{
			uEmissive->set(m_emissive);
		}

		osg::Uniform* ulightDiffRadius = m_rpLightingModel->getOrCreateStateSet()->getUniform("ulightDiffRadius");
		if(ulightDiffRadius)
		{
			ulightDiffRadius->set(m_lightDiffRadius);
		}
		

		osg::Uniform* uAvLightPositionView = m_rpLightingModel->getOrCreateStateSet()->getUniform("uAvLightPositionView");
		if(uAvLightPositionView && m_opRenderContext.valid())
		{
			osg::Camera* cam = m_opRenderContext->GetCamera();
			if(cam)
			{
				for (int i = 0;i < NUM_LIGHTS;i++)
				{
					osg::Matrix transMat;
					transMat.makeIdentity();

					osg::MatrixList ml = getWorldMatrices();
					if(!ml.empty())
						transMat = ml[0];

					osg::Matrix viewMat = cam->getViewMatrix();

					osg::Vec4 viewPos = m_avLightPosition[i] * transMat * viewMat;

					uAvLightPositionView->setElement(i,viewPos);
				}
			}
		}

		osg::Uniform* uAvLightIntensity = m_rpLightingModel->getOrCreateStateSet()->getUniform("uAvLightIntensity");
		if(uAvLightIntensity)
		{
			for (int i = 0;i < NUM_LIGHTS;i++)
			{
				uAvLightIntensity->setElement(i,m_avLightIntensity[i]);
			}
		}

		osg::Uniform* uAvLightColor = m_rpLightingModel->getOrCreateStateSet()->getUniform("uAvLightColor");
		if(uAvLightColor)
		{
			for (int i = 0;i < NUM_LIGHTS;i++)
			{
				uAvLightColor->setElement(i,m_lightColor[i] * m_lightPow[i]);
			}
		}

		osg::Uniform* uDiffuseCoefficient = m_rpLightingModel->getOrCreateStateSet()->getUniform("uDiffuseCoefficient");
		if(uDiffuseCoefficient)
		{
			uDiffuseCoefficient->set(m_diffuseCoefficient);
		}
		osg::Uniform* uPhongCoefficient = m_rpLightingModel->getOrCreateStateSet()->getUniform("uPhongCoefficient");
		if(uPhongCoefficient)
		{
			uPhongCoefficient->set(m_phongCoefficient);
		}
		osg::Uniform* uPhongExponent = m_rpLightingModel->getOrCreateStateSet()->getUniform("uPhongExponent");
		if(uPhongExponent)
		{
			uPhongExponent->set(m_phongExponent);
		}

		if(m_opRenderContext.valid() && m_AdaptedLuminanceCam.valid())
		{
			osg::Uniform* u_elapsedTime = m_AdaptedLuminanceCam->getStateSet()->getUniform("u_elapsedTime");
			if(u_elapsedTime)
			{
				osgViewer::View* vv = m_opRenderContext->GetView();
				if(vv)
				{
					double curTime = vv->getFrameStamp()->getReferenceTime();
					float et = curTime - m_lastFrameTime;
					u_elapsedTime->set(et);
					m_lastFrameTime = curTime;
				}
			}	
		}

		if(m_texBrightPassCam.valid())
		{
			osg::Uniform* u_middleGray = m_texBrightPassCam->getStateSet()->getUniform("u_middleGray");
			if(u_middleGray)
			{
				u_middleGray->set(m_keyValue / 100.0f);
			}
		}
		
		if(m_finalSceneCam.valid())
		{
			osg::Uniform* u_middleGray = m_finalSceneCam->getStateSet()->getUniform("u_middleGray");
			if(u_middleGray)
			{
				u_middleGray->set(m_keyValue / 100.0f);
			}

			osg::Uniform* u_starScale = m_finalSceneCam->getStateSet()->getUniform("u_starScale");
			if(u_starScale)
			{
				u_starScale->set(m_starScale);
			}

			osg::Uniform* u_bloomScale = m_finalSceneCam->getStateSet()->getUniform("u_bloomScale");
			if(u_bloomScale)
			{
				u_bloomScale->set(m_bloomScale);
			}
		}
	}
}

void FeHDRLighting::FeHDRLightingNode::updateLight()
{
	for (int i = 0;i < NUM_LIGHTS;i++)
	{
		if(m_lightBaseValue[i] == (int)0)
		{
			m_avLightIntensity[i].x() = (int)0;
		}
		else
		{
			m_nLightMantissa[i] = 1 + ( m_lightBaseValue[i] ) % 9;
			m_nLightLogIntensity[i] = -4 + ( m_lightBaseValue[i] ) / 9;

			m_avLightIntensity[i].x() = m_nLightMantissa[i] * ( float )pow( 10.0f, m_nLightLogIntensity[i] );
			m_avLightIntensity[i].y() = m_nLightMantissa[i] * ( float )pow( 10.0f, m_nLightLogIntensity[i] );
			m_avLightIntensity[i].z() = m_nLightMantissa[i] * ( float )pow( 10.0f, m_nLightLogIntensity[i] );
			m_avLightIntensity[i].w() = 1.0f;
		}
			
		if(m_lightRadius[i] < 0.0f || osg::equivalent(m_lightRadius[i],0.0f))
		{
			if(m_rpLightNode[i].valid())
			{
				m_rpLightNode[i]->setNodeMask(0);
			}
		}
		else
		{
			if(m_rpLightNode[i].valid())
			{
				m_rpLightNode[i]->setNodeMask(1);
				osg::Matrix scaleMat;
				scaleMat.makeScale(m_lightRadius[i],m_lightRadius[i],m_lightRadius[i]);
				osg::Matrix transMat;
				transMat.makeTranslate(m_avLightPosition[i].x(),m_avLightPosition[i].y(),m_avLightPosition[i].z());

				m_rpLightNode[i]->setMatrix(scaleMat * transMat);
			}
		}

		if(m_rpLightNode[i].valid())
		{
			if(m_rpLightNode[i]->getNumChildren() > 0)
			{
				osg::Geode* gn = dynamic_cast<osg::Geode*>(m_rpLightNode[i]->getChild(0));
				if(gn)
				{
					if(gn->getNumDrawables() > 0)
					{
						//fix
						LightSphereGeomerty* sp = dynamic_cast<LightSphereGeomerty*>(gn->getDrawable(0));
						if(sp)
						{
							float lp = osg::clampBetween(m_lightPow[i],0.0f,1.0f);
							sp->SetColor(osg::Vec4(m_lightColor[i].x() * lp,m_lightColor[i].y() * lp,m_lightColor[i].z() * lp,1.0));
						}
						//fix
					}
				}
			}
		}
	}	
}

void FeHDRLighting::FeHDRLightingNode::initSceneCamera()
{
	if(m_rpSceneTex == NULL && m_rpSceneCamera == NULL)
	{
		int texWidth = 0;
		int texHeight = 0;
		if(m_opRenderContext.valid())
		{
			osg::Camera* cam = m_opRenderContext->GetCamera();
			if(cam)
			{
				osg::Viewport* vp = cam->getViewport();
				if(vp)
				{
					texWidth = vp->width();
					texHeight = vp->height();
				}
			}
		}

		m_rpSceneTex = new osg::Texture2D();
		m_rpSceneTex->setTextureSize(texWidth, texHeight); 
		m_rpSceneTex->setInternalFormat(GL_RGBA16F_ARB); 
		m_rpSceneTex->setSourceFormat(GL_LUMINANCE); 
		m_rpSceneTex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
		m_rpSceneTex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
		m_rpSceneTex->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
		m_rpSceneTex->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);

		m_rpSceneCamera = new osg::Camera();
		m_rpSceneCamera->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
		m_rpSceneCamera->setName("hdr_light_camera");
		m_rpSceneCamera->setReferenceFrame(osg::Transform::RELATIVE_RF); 
		m_rpSceneCamera->setViewport(0, 0, texWidth, texHeight); 
		m_rpSceneCamera->setRenderOrder(osg::Camera::PRE_RENDER); 
		m_rpSceneCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
		m_rpSceneCamera->attach(osg::Camera::COLOR_BUFFER, m_rpSceneTex); 
		m_rpSceneCamera->setAllowEventFocus(false);
		m_rpSceneCamera->setInheritanceMask(osg::CullSettings::COMPUTE_NEAR_FAR_MODE |
			osg::CullSettings::CULLING_MODE                            |
			osg::CullSettings::LOD_SCALE                               |
			osg::CullSettings::SMALL_FEATURE_CULLING_PIXEL_SIZE        |
			osg::CullSettings::CLAMP_PROJECTION_MATRIX_CALLBACK        |
			osg::CullSettings::NEAR_FAR_RATIO                          |
			osg::CullSettings::IMPOSTOR_ACTIVE                         |
			osg::CullSettings::DEPTH_SORT_IMPOSTOR_SPRITES             |
			osg::CullSettings::IMPOSTOR_PIXEL_ERROR_THRESHOLD          |
			osg::CullSettings::NUM_FRAMES_TO_KEEP_IMPOSTORS_SPRITES    |
			osg::CullSettings::CULL_MASK                               |
			osg::CullSettings::CULL_MASK_LEFT                          |
			osg::CullSettings::CULL_MASK_RIGHT                         |
			osg::CullSettings::CLEAR_COLOR                             |
			osg::CullSettings::CLEAR_MASK                              |
			osg::CullSettings::LIGHTING_MODE                           |
			osg::CullSettings::LIGHT                                   );

		//MySceneCamDrawCallback* cb = new MySceneCamDrawCallback(this);
		//m_rpSceneCamera->setInitialDrawCallback(cb);
	}
}

void FeHDRLighting::FeHDRLightingNode::addHud()
{
	float w = 800;
	float h = 600;

	m_rpHud = new osg::Camera();
	m_rpHud->setProjectionMatrix(osg::Matrix::ortho2D(0,w,0,h));
	m_rpHud->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	m_rpHud->setViewMatrix(osg::Matrix::identity());
	m_rpHud->setClearMask(GL_DEPTH_BUFFER_BIT);
	m_rpHud->setRenderOrder(osg::Camera::POST_RENDER);
	m_rpHud->setAllowEventFocus(false);

	osg::Geometry* gm = new osg::Geometry();

	float sc = 3.0;
	osg::Vec3Array* va = new osg::Vec3Array();
	va->push_back(osg::Vec3(0,0,0));
	va->push_back(osg::Vec3(w / sc,0,0));
	va->push_back(osg::Vec3(w / sc,h / sc,0));
	va->push_back(osg::Vec3(0,h / sc,0));

	osg::Vec2Array* ta = new osg::Vec2Array();
	ta->push_back(osg::Vec2(0,0));
	ta->push_back(osg::Vec2(1,0));
	ta->push_back(osg::Vec2(1,1));
	ta->push_back(osg::Vec2(0,1));

	osg::DrawElementsUByte* ia = new osg::DrawElementsUByte(osg::PrimitiveSet::TRIANGLES);
	ia->push_back(0);
	ia->push_back(1);
	ia->push_back(2);
	ia->push_back(0);
	ia->push_back(2);
	ia->push_back(3);

	gm->setVertexArray(va);
	gm->addPrimitiveSet(ia);
	gm->setTexCoordArray(0,ta);

	//gm->getOrCreateStateSet()->setTextureAttributeAndModes(0,m_texStarAdd[0][1]);
	//gm->getOrCreateStateSet()->setTextureAttributeAndModes(0,m_texStar[0]);
	gm->getOrCreateStateSet()->setTextureAttributeAndModes(0,m_modelSceneTex);
	gm->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	osg::Geode* gn = new osg::Geode();
	gn->addDrawable(gm);

	m_rpHud->addChild(gn);

	//if(m_opRenderContext.valid())
	//	m_opRenderContext->GetRoot()->addChild(m_rpHud);
}

void FeHDRLighting::FeHDRLightingNode::insertSceneCamToRoot()
{
	if(m_opRenderContext.valid() && m_rpSceneCamera.valid())
	{
		osg::Group* root = m_opRenderContext->GetRoot();
		if(root)
		{
			if(!root->containsNode(m_rpSceneCamera))
			{
				m_rpSceneCamera->removeChildren(0,m_rpSceneCamera->getNumChildren());

				for (int i = 0;i < root->getNumChildren();i++)
				{	
					m_rpSceneCamera->addChild(root->getChild(i));
				}

				root->removeChildren(0,root->getNumChildren());

				root->addChild(m_rpSceneCamera);
			}
			else
			{
			}

			if(m_rpModelSceneCam.valid())
			{
				if(!root->containsNode(m_rpModelSceneCam))
				{
					root->addChild(m_rpModelSceneCam);
				}
			}

			if(m_rpDownScale4x4Cam.valid())
			{
				if(!root->containsNode(m_rpDownScale4x4Cam))
				{
					root->addChild(m_rpDownScale4x4Cam);
				}
			}
			/*for (int i = NUM_TONEMAP_TEXTURES - 1;i >= 0;i--)
			{
				if(m_toneMapCam[i].valid())
				{
					if(!root->containsNode(m_toneMapCam[i]))
					{
						root->addChild(m_toneMapCam[i]);
					}
				}
			}*/
			/*if(m_AdaptedLuminanceCam.valid())
			{
				if(!root->containsNode(m_AdaptedLuminanceCam))
				{
					root->addChild(m_AdaptedLuminanceCam);
				}
			}*/
			if(m_texBrightPassCam.valid())
			{
				if(!root->containsNode(m_texBrightPassCam))
				{
					root->addChild(m_texBrightPassCam);
				}
			}
			if(m_texStarSourceCam.valid())
			{
				if(!root->containsNode(m_texStarSourceCam))
				{
					root->addChild(m_texStarSourceCam);
				}
			}
			if(m_texBloomSourceCam.valid())
			{
				if(!root->containsNode(m_texBloomSourceCam))
				{
					root->addChild(m_texBloomSourceCam);
				}
			}
			for (int i = NUM_BLOOM_TEXTURES - 1;i >= 0;i--)
			{
				if(m_texBloomCam[i].valid())
				{
					if(!root->containsNode(m_texBloomCam[i]))
					{
						root->addChild(m_texBloomCam[i]);
					}
				}
			}
			if(m_texStarCamGroup.valid())
			{
				if(!root->containsNode(m_texStarCamGroup))
				{
					root->addChild(m_texStarCamGroup);
				}
			}
			if(m_finalSceneCam.valid())
			{
				if(!root->containsNode(m_finalSceneCam))
				{
					root->addChild(m_finalSceneCam);
				}
			}

#ifdef HUDTEST
			if(m_rpHud.valid())
			{
				if(!root->containsNode(m_rpHud))
				{
					root->addChild(m_rpHud);
				}
			}
#endif
			
		}
	}
}

void FeHDRLighting::FeHDRLightingNode::removeSceneCamFromRoot()
{
	if(m_opRenderContext.valid() && m_rpSceneCamera.valid())
	{
		osg::Group* root = m_opRenderContext->GetRoot();
		if(root)
		{
			if(root->containsNode(m_rpSceneCamera))
			{
				for (int i = 0;i < m_rpSceneCamera->getNumChildren();i++)
				{
					if(!root->containsNode(m_rpSceneCamera->getChild(i)))
						root->addChild(m_rpSceneCamera->getChild(i));
				}

				m_rpSceneCamera->removeChildren(0,m_rpSceneCamera->getNumChildren());

				root->removeChild(m_rpSceneCamera);
			}

			if(m_rpModelSceneCam.valid())
			{
				if(root->containsNode(m_rpModelSceneCam))
				{
					root->removeChild(m_rpModelSceneCam);
				}
			}
			if(m_rpDownScale4x4Cam.valid())
			{
				if(root->containsNode(m_rpDownScale4x4Cam))
				{
					root->removeChild(m_rpDownScale4x4Cam);
				}
			}
			/*for (int i = 0;i < NUM_TONEMAP_TEXTURES;i++)
			{
			if(m_toneMapCam[i].valid())
			{
			if(root->containsNode(m_toneMapCam[i]))
			{
			root->removeChild(m_toneMapCam[i]);
			}
			}
			}*/
			/*if(m_AdaptedLuminanceCam.valid())
			{
				if(root->containsNode(m_AdaptedLuminanceCam))
				{
					root->removeChild(m_AdaptedLuminanceCam);
				}
			}*/
			if(m_texBrightPassCam.valid())
			{
				if(root->containsNode(m_texBrightPassCam))
				{
					root->removeChild(m_texBrightPassCam);
				}
			}
			if(m_texStarSourceCam.valid())
			{
				if(root->containsNode(m_texStarSourceCam))
				{
					root->removeChild(m_texStarSourceCam);
				}
			}
			if(m_texBloomSourceCam.valid())
			{
				if(root->containsNode(m_texBloomSourceCam))
				{
					root->removeChild(m_texBloomSourceCam);
				}
			}
			for (int i = 0;i < NUM_BLOOM_TEXTURES;i++)
			{
				if(m_texBloomCam[i].valid())
				{
					if(root->containsNode(m_texBloomCam[i]))
					{
						root->removeChild(m_texBloomCam[i]);
					}
				}
			}
			if(m_texStarCamGroup.valid())
			{
				if(root->containsNode(m_texStarCamGroup))
				{
					root->removeChild(m_texStarCamGroup);
				}
			}
			if(m_finalSceneCam.valid())
			{
				if(root->containsNode(m_finalSceneCam))
				{
					root->removeChild(m_finalSceneCam);
				}
			}

#ifdef HUDTEST
			if(m_rpHud.valid())
			{
				if(root->containsNode(m_rpHud))
				{
					root->removeChild(m_rpHud);
				}
			}
#endif
		}
	}
}

void FeHDRLighting::FeHDRLightingNode::preUpdate()
{
	updateBloomTextureSize();

	if(checkSceneCamWithRoot())
		insertSceneCamToRoot();
	else
		removeSceneCamFromRoot();

	//updateAdaptationTarget();
	addChildToModelSceneCam();

	m_islastFrameActive = false;
	m_adaptationInvalid = true;
}

bool FeHDRLighting::FeHDRLightingNode::checkSceneCamWithRoot()
{
	if(m_islastFrameActive && m_enableLighting && m_enableBlooming)
	{
		osg::Camera* cam = m_opRenderContext->GetCamera();
		if(cam)
		{
			osg::Vec3d camPos,tmp;
			cam->getViewMatrixAsLookAt(camPos,tmp,tmp);

			osg::Vec3d nodeCenter = getBound().center();
			osg::Matrix transMat;
			transMat.makeIdentity();
			osg::MatrixList ml = getWorldMatrices();
			if(!ml.empty())
				transMat = ml[0];
			nodeCenter = nodeCenter * transMat;

			double rad = getBound().radius();

			if((camPos - nodeCenter).length() < m_visibleMultiple * rad)
				return true;
		}
	}

	return false;
}

void FeHDRLighting::FeHDRLightingNode::initDownScale4x4Cam()
{
	if(m_rpDownScale4x4Tex == NULL && m_texBrightPass == NULL && m_texStarSource == NULL && m_texBloomSource == NULL && m_rpDownScale4x4Cam == NULL)
	{
		if(m_modelSceneTex.valid())
		{
			m_rpDownScale4x4Tex = new osg::Texture2D();
			float cropWidth = m_modelSceneTex->getTextureWidth() - m_modelSceneTex->getTextureWidth() % 8;
			float cropHeight = m_modelSceneTex->getTextureHeight() - m_modelSceneTex->getTextureHeight() % 8;
			m_rpDownScale4x4Tex->setTextureSize(cropWidth / 4, cropHeight / 4); 
			m_rpDownScale4x4Tex->setInternalFormat(GL_RGBA16F_ARB); 
			m_rpDownScale4x4Tex->setSourceFormat(GL_LUMINANCE); 
			m_rpDownScale4x4Tex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
			m_rpDownScale4x4Tex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
			m_rpDownScale4x4Tex->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
			m_rpDownScale4x4Tex->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);

			m_texBrightPass = new osg::Texture2D();
			m_texBrightPass->setTextureSize(cropWidth / 4 + 2, cropHeight / 4 + 2); 
			m_texBrightPass->setInternalFormat(GL_RGBA16F_ARB); 
			m_texBrightPass->setSourceFormat(GL_LUMINANCE); 
			m_texBrightPass->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
			m_texBrightPass->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
			m_texBrightPass->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
			m_texBrightPass->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);

			m_texStarSource = new osg::Texture2D();
			m_texStarSource->setTextureSize(cropWidth / 4 + 2, cropHeight / 4 + 2); 
			m_texStarSource->setInternalFormat(GL_RGBA16F_ARB); 
			m_texStarSource->setSourceFormat(GL_LUMINANCE); 
			m_texStarSource->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
			m_texStarSource->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
			m_texStarSource->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
			m_texStarSource->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);

			m_texBloomSource = new osg::Texture2D();
			m_texBloomSource->setTextureSize(cropWidth / 8 + 2, cropHeight / 8 + 2); 
			m_texBloomSource->setInternalFormat(GL_RGBA16F_ARB); 
			m_texBloomSource->setSourceFormat(GL_LUMINANCE); 
			m_texBloomSource->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
			m_texBloomSource->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
			m_texBloomSource->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
			m_texBloomSource->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);

			for (int i = 0;i < NUM_BLOOM_TEXTURES;i++)
			{
				if(m_texBloom[i] == NULL)
				{
					m_texBloom[i] = new osg::Texture2D();
					m_texBloom[i]->setTextureSize(cropWidth / 8 + 2, cropHeight / 8 + 2); 
					m_texBloom[i]->setInternalFormat(GL_RGBA16F_ARB); 
					m_texBloom[i]->setSourceFormat(GL_LUMINANCE); 
					m_texBloom[i]->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
					m_texBloom[i]->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
					m_texBloom[i]->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
					m_texBloom[i]->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);
				}		
			}

			for (int i = 0;i < NUM_STAR_TEXTURES;i++)
			{
				if(m_texStar[i] == NULL)
				{
					m_texStar[i] = new osg::Texture2D();
					m_texStar[i]->setTextureSize(cropWidth / 4, cropHeight / 4); 
					m_texStar[i]->setInternalFormat(GL_RGBA16F_ARB); 
					m_texStar[i]->setSourceFormat(GL_LUMINANCE); 
					m_texStar[i]->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
					m_texStar[i]->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
					m_texStar[i]->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
					m_texStar[i]->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);
				}
			}

			for (int i = 0;i < 8;i++)
			{
				if(m_texStarAdd[i][0] == NULL)
				{
					m_texStarAdd[i][0] = new osg::Texture2D();
					m_texStarAdd[i][0]->setTextureSize(cropWidth / 4, cropHeight / 4); 
					m_texStarAdd[i][0]->setInternalFormat(GL_RGBA16F_ARB); 
					m_texStarAdd[i][0]->setSourceFormat(GL_LUMINANCE); 
					m_texStarAdd[i][0]->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
					m_texStarAdd[i][0]->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
					m_texStarAdd[i][0]->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
					m_texStarAdd[i][0]->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);
				}
				
				if(m_texStarAdd[i][1] == NULL)
				{
					m_texStarAdd[i][1] = new osg::Texture2D();
					m_texStarAdd[i][1]->setTextureSize(cropWidth / 4, cropHeight / 4); 
					m_texStarAdd[i][1]->setInternalFormat(GL_RGBA16F_ARB); 
					m_texStarAdd[i][1]->setSourceFormat(GL_LUMINANCE); 
					m_texStarAdd[i][1]->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
					m_texStarAdd[i][1]->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
					m_texStarAdd[i][1]->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
					m_texStarAdd[i][1]->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);
				}
			}

			osg::Vec4d rectSrc;
			rectSrc.x() = (m_modelSceneTex->getTextureWidth() - cropWidth) / 2;
			rectSrc.y() = (m_modelSceneTex->getTextureHeight() - cropHeight) / 2;
			rectSrc.z() = rectSrc.x() + cropWidth;
			rectSrc.w() = rectSrc.y() + cropHeight;
			osg::Vec4d coords;
			getTextureCoords( m_modelSceneTex, &rectSrc, m_rpDownScale4x4Tex, NULL, coords );

			osg::StateSet* ss = new osg::StateSet();

			int textUnit = 0;
			ss->setTextureAttributeAndModes(textUnit,m_modelSceneTex,osg::StateAttribute::ON);
			osg::Uniform* uSampleOffsets = new osg::Uniform();
			uSampleOffsets->setName("uSampleOffsets");
			uSampleOffsets->setNumElements(MAX_SAMPLES);
			uSampleOffsets->setType(osg::Uniform::FLOAT_VEC2);

			osg::Vec2 avSampleOffsets[MAX_SAMPLES];
			getSampleOffsets_DownScale4x4( m_modelSceneTex->getTextureWidth(), m_modelSceneTex->getTextureHeight(), avSampleOffsets );
			for (int i = 0;i < MAX_SAMPLES;i++)
			{
				uSampleOffsets->setElement(i,avSampleOffsets[i]);
			}
			ss->addUniform(uSampleOffsets);

			osg::Program* program = new osg::Program;
			program->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
			program->addShader( new osg::Shader( osg::Shader::FRAGMENT, DownScale4x4FragSource ) );
			ss->setAttributeAndModes( program, osg::StateAttribute::ON );
			ss->setRenderBinDetails(m_curRenderBinNum++,"");
			ss->addUniform( new osg::Uniform("originalMap", textUnit) );

			m_rpDownScale4x4Cam = new osg::Camera();
			m_rpDownScale4x4Cam->setStateSet(ss);
			m_rpDownScale4x4Cam->setRenderOrder( osg::Camera::PRE_RENDER );
			m_rpDownScale4x4Cam->setInheritanceMask(osg::CullSettings::NO_VARIABLES);
			m_rpDownScale4x4Cam->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
			m_rpDownScale4x4Cam->setReferenceFrame(osg::Transform::RELATIVE_RF); 
			m_rpDownScale4x4Cam->setViewport(0, 0, m_rpDownScale4x4Tex->getTextureWidth(), m_rpDownScale4x4Tex->getTextureHeight()); 
			m_rpDownScale4x4Cam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
			m_rpDownScale4x4Cam->attach(osg::Camera::COLOR_BUFFER, m_rpDownScale4x4Tex); 
			m_rpDownScale4x4Cam->setAllowEventFocus(false);

			osg::Geode* gn = creatScreenQuadNode(coords.x(),coords.z(),coords.y(),coords.w());
			m_rpDownScale4x4Cam->addChild( gn );
		}
	}
}

osg::Geode* FeHDRLighting::FeHDRLightingNode::creatScreenQuadNode(float leftU,float rightU,float topV,float bottomV)
{
	osg::Geometry* geom = new osg::Geometry;

	double nearDepth = 0.0;
	osg::Vec3dArray* coords = new osg::Vec3dArray(4);
	(*coords)[0] = osg::Vec3d(-1,-1,nearDepth);
	(*coords)[1] = osg::Vec3d(1,-1,nearDepth);
	(*coords)[2] = osg::Vec3d(1,1,nearDepth);
	(*coords)[3] = osg::Vec3d(-1,1,nearDepth);
	geom->setVertexArray(coords);

	osg::Vec2dArray* tcoords = new osg::Vec2dArray(4);
	(*tcoords)[0].set(leftU,topV);
	(*tcoords)[1].set(rightU,topV);
	(*tcoords)[2].set(rightU,bottomV);
	(*tcoords)[3].set(leftU,bottomV);

	/*(*tcoords)[0].set(leftU,bottomV);
	(*tcoords)[1].set(rightU,bottomV);
	(*tcoords)[2].set(rightU,topV);
	(*tcoords)[3].set(leftU,topV);*/

	geom->setTexCoordArray(0,tcoords);
	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

	geom->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
	geom->setCullingActive(false);

	osg::Geode* gn = new osg::Geode();
	gn->addChild(geom);

	return gn;
}

void FeHDRLighting::FeHDRLightingNode::getTextureCoords( osg::ref_ptr<osg::Texture2D>& rpSrcTex,osg::Vec4d* srcRect,osg::ref_ptr<osg::Texture2D>& rpDestTex,osg::Vec4d* destRect,osg::Vec4d& outCoord )
{
	float tU, tV;

	if( !rpSrcTex.valid() || !rpDestTex.valid())
		return;

	outCoord.x() = 0.0;
	outCoord.y() = 0.0;
	outCoord.z() = 1.0;
	outCoord.w() = 1.0;

	if( srcRect != NULL )
	{
		tU = 1.0f / rpSrcTex->getTextureWidth();
		tV = 1.0f / rpSrcTex->getTextureHeight();

		outCoord.x() += srcRect->x() * tU;
		outCoord.y() += srcRect->y() * tV;
		outCoord.z() -= ( rpSrcTex->getTextureWidth() - srcRect->z() ) * tU;
		outCoord.w() -= ( rpSrcTex->getTextureHeight() - srcRect->w() ) * tV;
	}

	if( destRect != NULL )
	{
		tU = 1.0f / rpDestTex->getTextureWidth();
		tV = 1.0f / rpDestTex->getTextureHeight();

		outCoord.x() -= destRect->x() * tU;
		outCoord.y() -= destRect->y() * tV;
		outCoord.z() += ( rpDestTex->getTextureWidth() - destRect->z() ) * tU;
		outCoord.w() += ( rpDestTex->getTextureHeight() - destRect->w() ) * tV;
	}
}

void FeHDRLighting::FeHDRLightingNode::getSampleOffsets_DownScale4x4( int dwWidth, int dwHeight, osg::Vec2 avSampleOffsets[] )
{
	if( NULL == avSampleOffsets )
		return;

	float tU = 1.0f / dwWidth;
	float tV = 1.0f / dwHeight;

	int index = 0;
	for( int y = 0; y < 4; y++ )
	{
		for( int x = 0; x < 4; x++ )
		{
			avSampleOffsets[ index ].x() = ( x - 1.5f ) * tU;
			avSampleOffsets[ index ].y() = ( y - 1.5f ) * tV;

			index++;
		}
	}
}

void FeHDRLighting::FeHDRLightingNode::measureLuminance()
{
	int i, x, y, index;
	osg::Vec2 avSampleOffsets[MAX_SAMPLES];

	int dwCurTexture = NUM_TONEMAP_TEXTURES - 1;

	for( i = 0; i < NUM_TONEMAP_TEXTURES; i++ )
	{
		int iSampleLen = 1 << ( 2 * i );

		m_apTexToneMap[i] = new osg::Texture2D();
		m_apTexToneMap[i]->setTextureSize(iSampleLen, iSampleLen); 
		m_apTexToneMap[i]->setInternalFormat(GL_LUMINANCE16F_ARB); 
		m_apTexToneMap[i]->setSourceFormat(GL_LUMINANCE); 
		m_apTexToneMap[i]->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
		m_apTexToneMap[i]->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
		m_apTexToneMap[i]->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
		m_apTexToneMap[i]->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);

		m_toneMapCam[i] = new osg::Camera();
		m_toneMapCam[i]->setRenderOrder( osg::Camera::PRE_RENDER );
		m_toneMapCam[i]->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
		m_toneMapCam[i]->setInheritanceMask(osg::CullSettings::NO_VARIABLES);
		m_toneMapCam[i]->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
		m_toneMapCam[i]->setReferenceFrame(osg::Transform::RELATIVE_RF); 
		m_toneMapCam[i]->setViewport(0, 0, m_apTexToneMap[i]->getTextureWidth(), m_apTexToneMap[i]->getTextureHeight()); 
		m_toneMapCam[i]->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
		m_toneMapCam[i]->attach(osg::Camera::COLOR_BUFFER, m_apTexToneMap[i]); 
		m_toneMapCam[i]->setAllowEventFocus(false);
	}

	float tU, tV;
	tU = 1.0f / ( 3.0f * m_apTexToneMap[dwCurTexture]->getTextureWidth() );
	tV = 1.0f / ( 3.0f * m_apTexToneMap[dwCurTexture]->getTextureHeight() );

	index = 0;
	for( x = -1; x <= 1; x++ )
	{
		for( y = -1; y <= 1; y++ )
		{
			avSampleOffsets[index].x() = x * tU;
			avSampleOffsets[index].y() = y * tV;

			index++;
		}
	}

	osg::Program* program = new osg::Program;
	program->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
	program->addShader( new osg::Shader( osg::Shader::FRAGMENT, SampleLumInitialFragSource ) );
	m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->setAttributeAndModes( program, osg::StateAttribute::ON );
	m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
	m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->addUniform( new osg::Uniform("originalMap", 0) );
	m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->setTextureAttributeAndModes(0,m_rpDownScale4x4Tex,osg::StateAttribute::ON);
	osg::Uniform* uSampleOffsets = new osg::Uniform();
	uSampleOffsets->setName("uSampleOffsets");
	uSampleOffsets->setNumElements(MAX_SAMPLES);
	uSampleOffsets->setType(osg::Uniform::FLOAT_VEC2);
	for (int i = 0;i < MAX_SAMPLES;i++)
	{
		uSampleOffsets->setElement(i,avSampleOffsets[i]);
	}
	m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->addUniform(uSampleOffsets);
	osg::Geode* gn = creatScreenQuadNode(0.0,1.0,0.0,1.0);
	m_toneMapCam[dwCurTexture]->addChild(gn);
	
	dwCurTexture--;

	while( dwCurTexture > 0 )
	{
		getSampleOffsets_DownScale4x4( m_apTexToneMap[dwCurTexture + 1]->getTextureWidth(), m_apTexToneMap[dwCurTexture + 1]->getTextureHeight(), avSampleOffsets );

		osg::Program* programLocal = new osg::Program;
		programLocal->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
		programLocal->addShader( new osg::Shader( osg::Shader::FRAGMENT, SampleLumIterativeFragSource ) );
		m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->setAttributeAndModes( programLocal, osg::StateAttribute::ON );
		m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
		m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->addUniform( new osg::Uniform("originalMap", 0) );
		m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->setTextureAttributeAndModes(0,m_apTexToneMap[dwCurTexture + 1],osg::StateAttribute::ON);
		osg::Uniform* uSampleOffsetsLocal = new osg::Uniform();
		uSampleOffsetsLocal->setName("uSampleOffsets");
		uSampleOffsetsLocal->setNumElements(MAX_SAMPLES);
		uSampleOffsetsLocal->setType(osg::Uniform::FLOAT_VEC2);
		for (int i = 0;i < MAX_SAMPLES;i++)
		{
			uSampleOffsetsLocal->setElement(i,avSampleOffsets[i]);
		}
		m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->addUniform(uSampleOffsetsLocal);
		osg::Geode* gn0 = creatScreenQuadNode(0.0,1.0,0.0,1.0);
		m_toneMapCam[dwCurTexture]->addChild(gn0);

		dwCurTexture--;
	}

	getSampleOffsets_DownScale4x4( m_apTexToneMap[1]->getTextureWidth(), m_apTexToneMap[1]->getTextureHeight(), avSampleOffsets );

	osg::Program* programFinal = new osg::Program;
	programFinal->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
	programFinal->addShader( new osg::Shader( osg::Shader::FRAGMENT, SampleLumFinalFragSource ) );
	m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->setAttributeAndModes( programFinal, osg::StateAttribute::ON );
	m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
	m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->addUniform( new osg::Uniform("originalMap", 0) );
	m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->setTextureAttributeAndModes(0,m_apTexToneMap[1],osg::StateAttribute::ON);
	osg::Uniform* uSampleOffsetsFinal = new osg::Uniform();
	uSampleOffsetsFinal->setName("uSampleOffsets");
	uSampleOffsetsFinal->setNumElements(MAX_SAMPLES);
	uSampleOffsetsFinal->setType(osg::Uniform::FLOAT_VEC2);
	for (int i = 0;i < MAX_SAMPLES;i++)
	{
		uSampleOffsetsFinal->setElement(i,avSampleOffsets[i]);
	}
	m_toneMapCam[dwCurTexture]->getOrCreateStateSet()->addUniform(uSampleOffsetsFinal);
	osg::Geode* gn1 = creatScreenQuadNode(0.0,1.0,0.0,1.0);
	m_toneMapCam[dwCurTexture]->addChild(gn1);
}

void FeHDRLighting::FeHDRLightingNode::initAdaptation()
{
	if(m_apTexToneMap[0].valid())
	{
		m_texAdaptedLuminanceCur = new osg::Texture2D();
		m_texAdaptedLuminanceCur->setTextureSize(1, 1); 
		m_texAdaptedLuminanceCur->setInternalFormat(GL_RGBA16F_ARB); 
		m_texAdaptedLuminanceCur->setSourceFormat(GL_LUMINANCE); 
		m_texAdaptedLuminanceCur->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
		m_texAdaptedLuminanceCur->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
		m_texAdaptedLuminanceCur->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
		m_texAdaptedLuminanceCur->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);

		m_texAdaptedLuminanceLast = new osg::Texture2D();
		m_texAdaptedLuminanceLast->setTextureSize(1, 1); 
		m_texAdaptedLuminanceLast->setInternalFormat(GL_RGBA16F_ARB); 
		m_texAdaptedLuminanceLast->setSourceFormat(GL_LUMINANCE); 
		m_texAdaptedLuminanceLast->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
		m_texAdaptedLuminanceLast->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
		m_texAdaptedLuminanceLast->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
		m_texAdaptedLuminanceLast->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);
		

		m_AdaptedLuminanceCam = new osg::Camera();
		m_AdaptedLuminanceCam->setRenderOrder( osg::Camera::PRE_RENDER );
		m_AdaptedLuminanceCam->setInheritanceMask(osg::CullSettings::NO_VARIABLES);
		m_AdaptedLuminanceCam->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
		m_AdaptedLuminanceCam->setReferenceFrame(osg::Transform::RELATIVE_RF); 
		m_AdaptedLuminanceCam->setViewport(0, 0, m_texAdaptedLuminanceCur->getTextureWidth(), m_texAdaptedLuminanceCur->getTextureHeight()); 
		m_AdaptedLuminanceCam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
		m_AdaptedLuminanceCam->attach(osg::Camera::COLOR_BUFFER, m_texAdaptedLuminanceCur); 
		m_AdaptedLuminanceCam->setAllowEventFocus(false);

		osg::Geode* gn = creatScreenQuadNode(0.0,1.0,0.0,1.0);
		m_AdaptedLuminanceCam->addChild( gn );

		int textUnit0 = 0;
		m_AdaptedLuminanceCam->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit0,m_texAdaptedLuminanceLast,osg::StateAttribute::ON);
		int textUnit1 = 1;
		m_AdaptedLuminanceCam->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit1,m_apTexToneMap[0],osg::StateAttribute::ON);

		osg::Program* program = new osg::Program;
		program->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
		program->addShader( new osg::Shader( osg::Shader::FRAGMENT, AdapteFragSource ) );
		m_AdaptedLuminanceCam->getOrCreateStateSet()->setAttributeAndModes( program, osg::StateAttribute::ON );
		m_AdaptedLuminanceCam->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
		m_AdaptedLuminanceCam->getOrCreateStateSet()->addUniform( new osg::Uniform("tex0", textUnit0) );
		m_AdaptedLuminanceCam->getOrCreateStateSet()->addUniform( new osg::Uniform("tex1", textUnit1) );
		m_AdaptedLuminanceCam->getOrCreateStateSet()->addUniform( new osg::Uniform("u_elapsedTime", 0.0f) );
	}
}

void FeHDRLighting::FeHDRLightingNode::updateAdaptationTarget()
{
	if(m_texAdaptedLuminanceCur.valid() && m_texAdaptedLuminanceLast.valid() && m_AdaptedLuminanceCam.valid())
	{
		//osg::ref_ptr<osg::Texture2D> tempTex = m_texAdaptedLuminanceLast;
		//m_texAdaptedLuminanceLast = m_texAdaptedLuminanceCur;
		//m_texAdaptedLuminanceCur = tempTex;

		m_texAdaptedLuminanceLast = new osg::Texture2D(*m_texAdaptedLuminanceCur.get(),osg::CopyOp::DEEP_COPY_ALL);
		m_AdaptedLuminanceCam->getOrCreateStateSet()->setTextureAttributeAndModes(0,m_texAdaptedLuminanceLast,osg::StateAttribute::ON);
	}
}

void FeHDRLighting::FeHDRLightingNode::initSceneScaledToBrightPass()
{
	if(m_texBrightPassCam == NULL)
	{
		if(m_texBrightPass.valid() && m_rpDownScale4x4Tex.valid())// && m_texAdaptedLuminanceCur.valid())
		{
			m_texBrightPassCam = new osg::Camera();
			m_texBrightPassCam->setRenderOrder( osg::Camera::PRE_RENDER );
			m_texBrightPassCam->setInheritanceMask(osg::CullSettings::NO_VARIABLES);
			m_texBrightPassCam->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
			m_texBrightPassCam->setReferenceFrame(osg::Transform::RELATIVE_RF); 
			m_texBrightPassCam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
			m_texBrightPassCam->attach(osg::Camera::COLOR_BUFFER, m_texBrightPass); 
			m_texBrightPassCam->setViewport(0,0,m_texBrightPass->getTextureWidth(),m_texBrightPass->getTextureHeight());
			m_texBrightPassCam->setAllowEventFocus(false);

			osg::Geode* gn = creatScreenQuadNode(0.0,1.0,0.0,1.0);
			m_texBrightPassCam->addChild( gn );

			int textUnit0 = 0;
			m_texBrightPassCam->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit0,m_rpDownScale4x4Tex,osg::StateAttribute::ON);
			//int textUnit1 = 1;
			//m_texBrightPassCam->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit1,m_texAdaptedLuminanceCur,osg::StateAttribute::ON);
			//m_texBrightPassCam->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit1,m_texAdaptedLuminanceCur,osg::StateAttribute::ON);

			osg::Program* program = new osg::Program;
			program->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
			program->addShader( new osg::Shader( osg::Shader::FRAGMENT, BrightPassFilterFragSource ) );
			m_texBrightPassCam->getOrCreateStateSet()->setAttributeAndModes( program, osg::StateAttribute::ON );
			m_texBrightPassCam->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
			m_texBrightPassCam->getOrCreateStateSet()->addUniform( new osg::Uniform("tex0", textUnit0) );
			//m_texBrightPassCam->getOrCreateStateSet()->addUniform( new osg::Uniform("tex1", textUnit1) );
			m_texBrightPassCam->getOrCreateStateSet()->addUniform( new osg::Uniform("u_middleGray", 0.0f) );
		}
	}
}

void FeHDRLighting::FeHDRLightingNode::initModelSceneCamera()
{
	if(m_modelSceneTex == NULL && m_rpModelSceneCam == NULL)
	{
		if(m_rpSceneTex.valid())
		{
			m_modelSceneTex = new osg::Texture2D();
			m_modelSceneTex->setTextureSize(m_rpSceneTex->getTextureWidth(), m_rpSceneTex->getTextureHeight()); 
			m_modelSceneTex->setInternalFormat(GL_RGBA16F_ARB); 
			m_modelSceneTex->setSourceFormat(GL_LUMINANCE); 
			m_modelSceneTex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
			m_modelSceneTex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
			m_modelSceneTex->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
			m_modelSceneTex->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);

			m_rpModelSceneCam = new osg::Camera();
			m_rpModelSceneCam->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
			m_rpModelSceneCam->setReferenceFrame(osg::Transform::RELATIVE_RF); 
			m_rpModelSceneCam->setViewport(0, 0, m_modelSceneTex->getTextureWidth(), m_modelSceneTex->getTextureHeight()); 
			m_rpModelSceneCam->setRenderOrder(osg::Camera::PRE_RENDER); 
			m_rpModelSceneCam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
			m_rpModelSceneCam->attach(osg::Camera::COLOR_BUFFER, m_modelSceneTex); 
			m_rpModelSceneCam->setAllowEventFocus(false);
			m_rpModelSceneCam->setInheritanceMask(osg::CullSettings::COMPUTE_NEAR_FAR_MODE |
				osg::CullSettings::CULLING_MODE                            |
				osg::CullSettings::LOD_SCALE                               |
				osg::CullSettings::SMALL_FEATURE_CULLING_PIXEL_SIZE        |
				osg::CullSettings::CLAMP_PROJECTION_MATRIX_CALLBACK        |
				osg::CullSettings::NEAR_FAR_RATIO                          |
				osg::CullSettings::IMPOSTOR_ACTIVE                         |
				osg::CullSettings::DEPTH_SORT_IMPOSTOR_SPRITES             |
				osg::CullSettings::IMPOSTOR_PIXEL_ERROR_THRESHOLD          |
				osg::CullSettings::NUM_FRAMES_TO_KEEP_IMPOSTORS_SPRITES    |
				osg::CullSettings::CULL_MASK                               |
				osg::CullSettings::CULL_MASK_LEFT                          |
				osg::CullSettings::CULL_MASK_RIGHT                         |
				osg::CullSettings::CLEAR_COLOR                             |
				osg::CullSettings::CLEAR_MASK                              |
				osg::CullSettings::LIGHTING_MODE                           |
				osg::CullSettings::LIGHT                                   );

			MyModelSceneCamDrawCallback* cb = new MyModelSceneCamDrawCallback(this);
			m_rpModelSceneCam->setInitialDrawCallback(cb);
		}
	}	
}

void FeHDRLighting::FeHDRLightingNode::addChildToModelSceneCam()
{
	if(m_rpModelSceneCam.valid())
	{
		if(m_rpModelSceneCam->getNumChildren() == 0)
		{
			if(m_rpModelNode.valid())
			{
				osg::Matrix mat;
				osg::MatrixList ml = m_rpModelNode->getWorldMatrices();
				if(!ml.empty())
					mat = ml[0];

				osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform();
				mt->setMatrix(mat);
				mt->addChild(m_rpModelNode);

				m_rpModelSceneCam->addChild(mt);
			}

			if(m_rpLightingModel.valid())
			{
				osg::Matrix mat;
				osg::MatrixList ml = m_rpLightingModel->getWorldMatrices();
				if(!ml.empty())
					mat = ml[0];

				osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform();
				mt->setMatrix(mat);
				mt->addChild(m_rpLightingModel.get());

				//m_rpModelSceneCam->addChild(mt);
			}
		}
	}
}

void FeHDRLighting::FeHDRLightingNode::initBrightPassToStarSource()
{
	if(m_texStarSourceCam == NULL)
	{
		if(m_texStarSource.valid())
		{
			m_texStarSourceCam = new osg::Camera();
			m_texStarSourceCam->setRenderOrder( osg::Camera::PRE_RENDER );
			m_texStarSourceCam->setInheritanceMask(osg::CullSettings::NO_VARIABLES);
			m_texStarSourceCam->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
			m_texStarSourceCam->setReferenceFrame(osg::Transform::RELATIVE_RF); 
			m_texStarSourceCam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
			m_texStarSourceCam->attach(osg::Camera::COLOR_BUFFER, m_texStarSource); 
			m_texStarSourceCam->setViewport(0,0,m_texStarSource->getTextureWidth(),m_texStarSource->getTextureHeight());
			m_texStarSourceCam->setAllowEventFocus(false);

			osg::Geode* gn = creatScreenQuadNode(0.0,1.0,0.0,1.0);
			m_texStarSourceCam->addChild( gn );

			osg::Vec2 avSampleOffsets[MAX_SAMPLES];
			osg::Vec4 avSampleWeights[MAX_SAMPLES];
			getSampleOffsetsGaussBlur5x5( m_texBrightPass->getTextureWidth(), m_texBrightPass->getTextureHeight(), avSampleOffsets, avSampleWeights );

			int textUnit0 = 0;
			m_texStarSourceCam->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit0,m_texBrightPass,osg::StateAttribute::ON);

			osg::Program* program = new osg::Program;
			program->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
			program->addShader( new osg::Shader( osg::Shader::FRAGMENT, GaussBlur5x5FragSource ) );
			m_texStarSourceCam->getOrCreateStateSet()->setAttributeAndModes( program, osg::StateAttribute::ON );
			m_texStarSourceCam->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
			m_texStarSourceCam->getOrCreateStateSet()->addUniform( new osg::Uniform("tex0", textUnit0) );

			osg::Uniform* uSampleOffsets = new osg::Uniform();
			uSampleOffsets->setName("uSampleOffsets");
			uSampleOffsets->setNumElements(MAX_SAMPLES);
			uSampleOffsets->setType(osg::Uniform::FLOAT_VEC2);
			for (int i = 0;i < MAX_SAMPLES;i++)
			{
				uSampleOffsets->setElement(i,avSampleOffsets[i]);
			}
			m_texStarSourceCam->getOrCreateStateSet()->addUniform(uSampleOffsets);

			osg::Uniform* uSampleWeights = new osg::Uniform();
			uSampleWeights->setName("uSampleWeights");
			uSampleWeights->setNumElements(MAX_SAMPLES);
			uSampleWeights->setType(osg::Uniform::FLOAT_VEC4);
			for (int i = 0;i < MAX_SAMPLES;i++)
			{
				uSampleWeights->setElement(i,avSampleWeights[i]);
			}
			m_texStarSourceCam->getOrCreateStateSet()->addUniform(uSampleWeights);
		}
	}
}

void FeHDRLighting::FeHDRLightingNode::getSampleOffsetsGaussBlur5x5( unsigned int dwD3DTexWidth, unsigned int dwD3DTexHeight, osg::Vec2* avTexCoordOffset, osg::Vec4* avSampleWeights, float fMultiplier /*= 1.0f*/ )
{
	float tu = 1.0f / ( float )dwD3DTexWidth;
	float tv = 1.0f / ( float )dwD3DTexHeight;

	osg::Vec4 vWhite( 1.0f, 1.0f, 1.0f, 1.0f );

	float totalWeight = 0.0f;
	int index = 0;
	for( int x = -2; x <= 2; x++ )
	{
		for( int y = -2; y <= 2; y++ )
		{
			if( abs( x ) + abs( y ) > 2 )
				continue;

			avTexCoordOffset[index] = osg::Vec2( x * tu, y * tv );
			avSampleWeights[index] = vWhite * gaussianDistribution( ( float )x, ( float )y, 1.0f );
			totalWeight += avSampleWeights[index].x();

			index++;
		}
	}

	for( int i = 0; i < index; i++ )
	{
		avSampleWeights[i] /= totalWeight;
		avSampleWeights[i] *= fMultiplier;
	}
}

float FeHDRLighting::FeHDRLightingNode::gaussianDistribution( float x, float y, float rho )
{
	float g = 1.0f / sqrtf( 2.0f * osg::PI * rho * rho );
	g *= expf( -( x * x + y * y ) / ( 2 * rho * rho ) );

	return g;
}

void FeHDRLighting::FeHDRLightingNode::initStarSourceToBloomSource()
{
	if(m_texBloomSourceCam == NULL)
	{
		if(m_texBloomSource.valid() && m_texStarSource.valid() && m_texBrightPass.valid())
		{
			m_texBloomSourceCam = new osg::Camera();
			m_texBloomSourceCam->setRenderOrder( osg::Camera::PRE_RENDER );
			m_texBloomSourceCam->setInheritanceMask(osg::CullSettings::NO_VARIABLES);
			m_texBloomSourceCam->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
			m_texBloomSourceCam->setReferenceFrame(osg::Transform::RELATIVE_RF); 
			m_texBloomSourceCam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
			m_texBloomSourceCam->attach(osg::Camera::COLOR_BUFFER, m_texBloomSource); 
			m_texBloomSourceCam->setViewport(0,0,m_texBloomSource->getTextureWidth(),m_texBloomSource->getTextureHeight());
			m_texBloomSourceCam->setAllowEventFocus(false);

			osg::Geode* gn = creatScreenQuadNode(0.0,1.0,0.0,1.0);
			m_texBloomSourceCam->addChild( gn );

			int textUnit0 = 0;
			m_texBloomSourceCam->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit0,m_texStarSource,osg::StateAttribute::ON);

			osg::Program* program = new osg::Program;
			program->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
			program->addShader( new osg::Shader( osg::Shader::FRAGMENT, DownScale2x2FragSource ) );
			m_texBloomSourceCam->getOrCreateStateSet()->setAttributeAndModes( program, osg::StateAttribute::ON );
			m_texBloomSourceCam->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
			m_texBloomSourceCam->getOrCreateStateSet()->addUniform( new osg::Uniform("tex0", textUnit0) );

			osg::Vec2 avSampleOffsets[MAX_SAMPLES];
			getSampleOffsetsDownScale2x2( m_texBrightPass->getTextureWidth(), m_texBrightPass->getTextureHeight(), avSampleOffsets);
			osg::Uniform* uSampleOffsets = new osg::Uniform();
			uSampleOffsets->setName("uSampleOffsets");
			uSampleOffsets->setNumElements(MAX_SAMPLES);
			uSampleOffsets->setType(osg::Uniform::FLOAT_VEC2);
			for (int i = 0;i < MAX_SAMPLES;i++)
			{
				uSampleOffsets->setElement(i,avSampleOffsets[i]);
			}
			m_texBloomSourceCam->getOrCreateStateSet()->addUniform(uSampleOffsets);

			MyBloomCamDrawCallback* cb = new MyBloomCamDrawCallback(this);
			m_texBloomSourceCam->setInitialDrawCallback(cb);

		}
	}
}

void FeHDRLighting::FeHDRLightingNode::getSampleOffsetsDownScale2x2( unsigned int dwWidth, unsigned int dwHeight, osg::Vec2 avSampleOffsets[] )
{
	if( NULL == avSampleOffsets )
		return;

	float tU = 1.0f / dwWidth;
	float tV = 1.0f / dwHeight;

	int index = 0;
	for( int y = 0; y < 2; y++ )
	{
		for( int x = 0; x < 2; x++ )
		{
			avSampleOffsets[ index ].x() = ( x - 0.5f ) * tU;
			avSampleOffsets[ index ].y() = ( y - 0.5f ) * tV;

			index++;
		}
	}
}

void FeHDRLighting::FeHDRLightingNode::initBloom()
{
	for (int i = 0;i < NUM_BLOOM_TEXTURES;i++)
	{
		if(!m_texBloom[i].valid())
			return;
	}

	if(!m_texBloomSource.valid())
		return;

	for (int i = 0;i < NUM_BLOOM_TEXTURES;i++)
	{
		if(m_texBloomCam[i] == NULL)
		{
			m_texBloomCam[i] = new osg::Camera();
			m_texBloomCam[i]->setRenderOrder( osg::Camera::PRE_RENDER );
			m_texBloomCam[i]->setInheritanceMask(osg::CullSettings::NO_VARIABLES);
			m_texBloomCam[i]->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
			m_texBloomCam[i]->setReferenceFrame(osg::Transform::RELATIVE_RF); 
			m_texBloomCam[i]->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
			m_texBloomCam[i]->attach(osg::Camera::COLOR_BUFFER, m_texBloom[i]); 
			m_texBloomCam[i]->setAllowEventFocus(false);
			m_texBloomCam[i]->setViewport(0, 0, m_texBloom[i]->getTextureWidth(), m_texBloom[i]->getTextureHeight()); 
		}	
	}

	osg::Geode* gn0 = creatScreenQuadNode(0.0,1.0,0.0,1.0);
	m_texBloomCam[2]->addChild( gn0 );

	osg::Vec2 avSampleOffsets[MAX_SAMPLES];
	osg::Vec4 avSampleWeights[MAX_SAMPLES];
	getSampleOffsetsGaussBlur5x5( m_texBloomSource->getTextureWidth(), m_texBloomSource->getTextureHeight(), avSampleOffsets, avSampleWeights );

	int textUnit0 = 0;
	m_texBloomCam[2]->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit0,m_texBloomSource,osg::StateAttribute::ON);

	osg::Program* program0 = new osg::Program;
	program0->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
	program0->addShader( new osg::Shader( osg::Shader::FRAGMENT, GaussBlur5x5FragSource ) );
	m_texBloomCam[2]->getOrCreateStateSet()->setAttributeAndModes( program0, osg::StateAttribute::ON );
	m_texBloomCam[2]->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
	m_texBloomCam[2]->getOrCreateStateSet()->addUniform( new osg::Uniform("tex0", textUnit0) );

	osg::Uniform* uSampleOffsets0 = new osg::Uniform();
	uSampleOffsets0->setName("uSampleOffsets");
	uSampleOffsets0->setNumElements(MAX_SAMPLES);
	uSampleOffsets0->setType(osg::Uniform::FLOAT_VEC2);
	for (int i = 0;i < MAX_SAMPLES;i++)
	{
		uSampleOffsets0->setElement(i,avSampleOffsets[i]);
	}
	m_texBloomCam[2]->getOrCreateStateSet()->addUniform(uSampleOffsets0);

	osg::Uniform* uSampleWeights0 = new osg::Uniform();
	uSampleWeights0->setName("uSampleWeights");
	uSampleWeights0->setNumElements(MAX_SAMPLES);
	uSampleWeights0->setType(osg::Uniform::FLOAT_VEC4);
	for (int i = 0;i < MAX_SAMPLES;i++)
	{
		uSampleWeights0->setElement(i,avSampleWeights[i]);
	}
	m_texBloomCam[2]->getOrCreateStateSet()->addUniform(uSampleWeights0);

	osg::Geode* gn1 = creatScreenQuadNode(0.0,1.0,0.0,1.0);
	m_texBloomCam[1]->addChild( gn1 );

	float afSampleOffsets[MAX_SAMPLES];
	getSampleOffsetsBloom( m_texBloom[2]->getTextureWidth(), afSampleOffsets, avSampleWeights, 3.0f, 2.0f );
	for( int i = 0; i < MAX_SAMPLES; i++ )
	{
		avSampleOffsets[i] = osg::Vec2( afSampleOffsets[i], 0.0f );
	}

	textUnit0 = 0;
	m_texBloomCam[1]->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit0,m_texBloom[2],osg::StateAttribute::ON);

	osg::Program* program1 = new osg::Program;
	program1->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
	program1->addShader( new osg::Shader( osg::Shader::FRAGMENT, BloomFragSource ) );
	m_texBloomCam[1]->getOrCreateStateSet()->setAttributeAndModes( program1, osg::StateAttribute::ON );
	m_texBloomCam[1]->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
	m_texBloomCam[1]->getOrCreateStateSet()->addUniform( new osg::Uniform("tex0", textUnit0) );

	osg::Uniform* uSampleOffsets1 = new osg::Uniform();
	uSampleOffsets1->setName("uSampleOffsets");
	uSampleOffsets1->setNumElements(MAX_SAMPLES);
	uSampleOffsets1->setType(osg::Uniform::FLOAT_VEC2);
	for (int i = 0;i < MAX_SAMPLES;i++)
	{
		uSampleOffsets1->setElement(i,avSampleOffsets[i]);
	}
	m_texBloomCam[1]->getOrCreateStateSet()->addUniform(uSampleOffsets1);

	osg::Uniform* uSampleWeights1 = new osg::Uniform();
	uSampleWeights1->setName("uSampleWeights");
	uSampleWeights1->setNumElements(MAX_SAMPLES);
	uSampleWeights1->setType(osg::Uniform::FLOAT_VEC4);
	for (int i = 0;i < MAX_SAMPLES;i++)
	{
		uSampleWeights1->setElement(i,avSampleWeights[i]);
	}
	m_texBloomCam[1]->getOrCreateStateSet()->addUniform(uSampleWeights1);

	osg::Geode* gn2 = creatScreenQuadNode(0.0,1.0,0.0,1.0);
	m_texBloomCam[0]->addChild( gn2 );

	getSampleOffsetsBloom( m_texBloom[1]->getTextureHeight(), afSampleOffsets, avSampleWeights, 3.0f, 2.0f );
	for( int i = 0; i < MAX_SAMPLES; i++ )
	{
		avSampleOffsets[i] = osg::Vec2( 0.0f, afSampleOffsets[i] );
	}

	textUnit0 = 0;
	m_texBloomCam[0]->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit0,m_texBloom[1],osg::StateAttribute::ON);

	osg::Program* program2 = new osg::Program;
	program2->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
	program2->addShader( new osg::Shader( osg::Shader::FRAGMENT, BloomFragSource ) );
	m_texBloomCam[0]->getOrCreateStateSet()->setAttributeAndModes( program2, osg::StateAttribute::ON );
	m_texBloomCam[0]->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
	m_texBloomCam[0]->getOrCreateStateSet()->addUniform( new osg::Uniform("tex0", textUnit0) );

	osg::Uniform* uSampleOffsets2 = new osg::Uniform();
	uSampleOffsets2->setName("uSampleOffsets");
	uSampleOffsets2->setNumElements(MAX_SAMPLES);
	uSampleOffsets2->setType(osg::Uniform::FLOAT_VEC2);
	for (int i = 0;i < MAX_SAMPLES;i++)
	{
		uSampleOffsets2->setElement(i,avSampleOffsets[i]);
	}
	m_texBloomCam[0]->getOrCreateStateSet()->addUniform(uSampleOffsets2);

	osg::Uniform* uSampleWeights2 = new osg::Uniform();
	uSampleWeights2->setName("uSampleWeights");
	uSampleWeights2->setNumElements(MAX_SAMPLES);
	uSampleWeights2->setType(osg::Uniform::FLOAT_VEC4);
	for (int i = 0;i < MAX_SAMPLES;i++)
	{
		uSampleWeights2->setElement(i,avSampleWeights[i]);
	}
	m_texBloomCam[0]->getOrCreateStateSet()->addUniform(uSampleWeights2);
}

void FeHDRLighting::FeHDRLightingNode::getSampleOffsetsBloom( unsigned int dwD3DTexSize, float afTexCoordOffset[15], osg::Vec4* avColorWeight, float fDeviation, float fMultiplier /*= 1.0f */ )
{
	int i = 0;
	float tu = 1.0f / ( float )dwD3DTexSize;

	float weight = fMultiplier * gaussianDistribution( 0, 0, fDeviation );
	avColorWeight[0] = osg::Vec4( weight, weight, weight, 1.0f );

	afTexCoordOffset[0] = 0.0f;

	for( i = 1; i < 8; i++ )
	{
		weight = fMultiplier * gaussianDistribution( ( float )i, 0, fDeviation );
		afTexCoordOffset[i] = i * tu;

		avColorWeight[i] = osg::Vec4( weight, weight, weight, 1.0f );
	}

	for( i = 8; i < 15; i++ )
	{
		avColorWeight[i] = avColorWeight[i - 7];
		afTexCoordOffset[i] = -afTexCoordOffset[i - 7];
	}
}

bool ColorLerp (osg::Vec4 *pOut, const osg::Vec4 *pC1, const osg::Vec4 *pC2, float s)
{
	if(!pOut || !pC1 || !pC2)
		return false;

	pOut->x() = pC1->x() + s * (pC2->x() - pC1->x());
	pOut->y() = pC1->y() + s * (pC2->y() - pC1->y());
	pOut->z() = pC1->z() + s * (pC2->z() - pC1->z());
	pOut->a() = pC1->a() + s * (pC2->a() - pC1->a());

	return true;
}

void FeHDRLighting::FeHDRLightingNode::initStar()
{
	if(!m_texStarSource.valid())
		return;

	for (int i = 0;i < NUM_STAR_TEXTURES;i++)
	{
		if(!m_texStar[i].valid())
			return;
	}

	m_glareDef.Initialize(m_glareType);

	m_texStarCamGroup = new osg::Group();
	m_starCamGroupNodeCallback = new MyStarCamGroupNodeCallback(this);
	m_texStarCamGroup->setCullCallback(m_starCamGroupNodeCallback);

	for (int i = 0;i < NUM_STAR_TEXTURES;i++)
	{
		if(m_texStarCam[i] == NULL)
		{
			m_texStarCam[i] = new osg::Camera();
			m_texStarCam[i]->setRenderOrder( osg::Camera::PRE_RENDER );
			m_texStarCam[i]->setInheritanceMask(osg::CullSettings::NO_VARIABLES);
			m_texStarCam[i]->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
			m_texStarCam[i]->setReferenceFrame(osg::Transform::RELATIVE_RF); 
			m_texStarCam[i]->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
			m_texStarCam[i]->attach(osg::Camera::COLOR_BUFFER, m_texStar[i]); 
			m_texStarCam[i]->setAllowEventFocus(false);
			m_texStarCam[i]->setViewport(0, 0, m_texStar[i]->getTextureWidth(), m_texStar[i]->getTextureHeight()); 
		}
	}

	for (int i = 0;i < 8;i++)
	{
		if(m_texStarAddCam[i][0] == NULL)
		{
			m_texStarAddCam[i][0] = new osg::Camera();
			m_texStarAddCam[i][0]->setRenderOrder( osg::Camera::PRE_RENDER );
			m_texStarAddCam[i][0]->setInheritanceMask(osg::CullSettings::NO_VARIABLES);
			m_texStarAddCam[i][0]->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
			m_texStarAddCam[i][0]->setReferenceFrame(osg::Transform::RELATIVE_RF); 
			m_texStarAddCam[i][0]->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
			m_texStarAddCam[i][0]->attach(osg::Camera::COLOR_BUFFER, m_texStarAdd[i][0]); 
			m_texStarAddCam[i][0]->setAllowEventFocus(false);
			m_texStarAddCam[i][0]->setViewport(0, 0, m_texStarAdd[i][0]->getTextureWidth(), m_texStarAdd[i][0]->getTextureHeight()); 
		}
		
		if(m_texStarAddCam[i][1] == NULL)
		{
			m_texStarAddCam[i][1] = new osg::Camera();
			m_texStarAddCam[i][1]->setRenderOrder( osg::Camera::PRE_RENDER );
			m_texStarAddCam[i][1]->setInheritanceMask(osg::CullSettings::NO_VARIABLES);
			m_texStarAddCam[i][1]->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
			m_texStarAddCam[i][1]->setReferenceFrame(osg::Transform::RELATIVE_RF); 
			m_texStarAddCam[i][1]->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
			m_texStarAddCam[i][1]->attach(osg::Camera::COLOR_BUFFER, m_texStarAdd[i][1]); 
			m_texStarAddCam[i][1]->setAllowEventFocus(false);
			m_texStarAddCam[i][1]->setViewport(0, 0, m_texStarAdd[i][1]->getTextureWidth(), m_texStarAdd[i][1]->getTextureHeight()); 
		}
	}

	bool hasCallback = false;

	const CStarDef& starDef = m_glareDef.m_starDef;
	const float fTanFoV = atanf( osg::PI / 8 );
	const osg::Vec4 vWhite( 1.0f, 1.0f, 1.0f, 1.0f );
	static const int s_maxPasses = 3;
	static const int nSamples = 8;
	static osg::Vec4 s_aaColor[s_maxPasses][8];
	static const osg::Vec4 s_colorWhite( 0.63f, 0.63f, 0.63f, 0.0f );

	osg::Vec4 avSampleWeights[MAX_SAMPLES];
	osg::Vec2 avSampleOffsets[MAX_SAMPLES];

	float srcW = m_texStarSource->getTextureWidth();
	float srcH = m_texStarSource->getTextureHeight();

	for( int p = 0; p < s_maxPasses; p ++ )
	{
		float ratio;
		ratio = ( float )( p + 1 ) / ( float )s_maxPasses;

		for( int s = 0; s < nSamples; s ++ )
		{
			osg::Vec4 chromaticAberrColor;
			ColorLerp( &chromaticAberrColor,
				&( CStarDef::GetChromaticAberrationColor( s ) ),
				&s_colorWhite,
				ratio );

			ColorLerp( &( s_aaColor[p][s] ),
				&s_colorWhite, &chromaticAberrColor,
				m_glareDef.m_fChromaticAberration );
		}
	}

	float radOffset;
	radOffset = m_glareDef.m_fStarInclination + starDef.m_fInclination;

	osg::observer_ptr<osg::Texture2D> pTexSource;

	for( int d = 0; d < starDef.m_nStarLines; d ++ )
	{
		const STARLINE& starLine = starDef.m_pStarLine[d];

		pTexSource = m_texStarSource;

		float rad;
		rad = radOffset + starLine.fInclination;
		float sn, cs;
		sn = sinf( rad ), cs = cosf( rad );
		osg::Vec2 vtStepUV;
		vtStepUV.x() = sn / srcW * starLine.fSampleLength * m_starLengthScale;
		vtStepUV.y() = cs / srcH * starLine.fSampleLength * m_starLengthScale;

		float attnPowScale;
		attnPowScale = ( fTanFoV + 0.1f ) * 1.0f *
			( (srcW - 2) + (srcH - 2) ) / ( srcW + srcH ) * 1.2f;

		int iWorkTexture;
		iWorkTexture = 1;
		for( int p = 0; p < starLine.nPasses; p ++ )
		{
			int texIndex = 0;
			bool isLastPass = false;
			if( p == starLine.nPasses - 1 )
			{
				texIndex = d + 4;
				isLastPass = true;
			}
			else
			{
				texIndex = iWorkTexture;
			}

			for( int i = 0; i < nSamples; i ++ )
			{
				float lum;
				lum = powf( starLine.fAttenuation, attnPowScale * i );

				avSampleWeights[i] = s_aaColor[starLine.nPasses - 1 - p][i] *
					lum * ( p + 1.0f ) * 0.5f;


				avSampleOffsets[i].x() = vtStepUV.x() * i;
				avSampleOffsets[i].y() = vtStepUV.y() * i;
				if( fabs( avSampleOffsets[i].x() ) >= 0.9f ||
					fabs( avSampleOffsets[i].y() ) >= 0.9f )
				{
					avSampleOffsets[i].x() = 0.0f;
					avSampleOffsets[i].y() = 0.0f;
					avSampleWeights[i] *= 0.0f;
				}

			}

			if(isLastPass)
			{
				osg::Geode* gn = creatScreenQuadNode(0.0,1.0,0.0,1.0);
				m_texStarCam[texIndex]->addChild( gn );

				int textUnit0 = 0;
				m_texStarCam[texIndex]->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit0,pTexSource.get(),osg::StateAttribute::ON);

				osg::Program* program0 = new osg::Program;
				program0->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
				program0->addShader( new osg::Shader( osg::Shader::FRAGMENT, StarFragSource ) );
				m_texStarCam[texIndex]->getOrCreateStateSet()->setAttributeAndModes( program0, osg::StateAttribute::ON );
				m_texStarCam[texIndex]->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
				m_texStarCam[texIndex]->getOrCreateStateSet()->addUniform( new osg::Uniform("tex0", textUnit0) );

				osg::Uniform* uSampleOffsets0 = new osg::Uniform();
				uSampleOffsets0->setName("uSampleOffsets");
				uSampleOffsets0->setNumElements(MAX_SAMPLES);
				uSampleOffsets0->setType(osg::Uniform::FLOAT_VEC2);
				for (int i = 0;i < MAX_SAMPLES;i++)
				{
					uSampleOffsets0->setElement(i,avSampleOffsets[i]);
				}
				m_texStarCam[texIndex]->getOrCreateStateSet()->addUniform(uSampleOffsets0);

				osg::Uniform* uSampleWeights0 = new osg::Uniform();
				uSampleWeights0->setName("uSampleWeights");
				uSampleWeights0->setNumElements(MAX_SAMPLES);
				uSampleWeights0->setType(osg::Uniform::FLOAT_VEC4);
				for (int i = 0;i < MAX_SAMPLES;i++)
				{
					uSampleWeights0->setElement(i,avSampleWeights[i]);
				}
				m_texStarCam[texIndex]->getOrCreateStateSet()->addUniform(uSampleWeights0);

				m_texStarCamGroup->addChild(m_texStarCam[texIndex]);
			}
			else
			{
				osg::Geode* gn = creatScreenQuadNode(0.0,1.0,0.0,1.0);
				m_texStarAddCam[d][texIndex - 1]->addChild( gn );

				int textUnit0 = 0;
				m_texStarAddCam[d][texIndex - 1]->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit0,pTexSource.get(),osg::StateAttribute::ON);

				osg::Program* program0 = new osg::Program;
				program0->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
				program0->addShader( new osg::Shader( osg::Shader::FRAGMENT, StarFragSource ) );
				m_texStarAddCam[d][texIndex - 1]->getOrCreateStateSet()->setAttributeAndModes( program0, osg::StateAttribute::ON );
				m_texStarAddCam[d][texIndex - 1]->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
				m_texStarAddCam[d][texIndex - 1]->getOrCreateStateSet()->addUniform( new osg::Uniform("tex0", textUnit0) );

				osg::Uniform* uSampleOffsets0 = new osg::Uniform();
				uSampleOffsets0->setName("uSampleOffsets");
				uSampleOffsets0->setNumElements(MAX_SAMPLES);
				uSampleOffsets0->setType(osg::Uniform::FLOAT_VEC2);
				for (int i = 0;i < MAX_SAMPLES;i++)
				{
					uSampleOffsets0->setElement(i,avSampleOffsets[i]);
				}
				m_texStarAddCam[d][texIndex - 1]->getOrCreateStateSet()->addUniform(uSampleOffsets0);

				osg::Uniform* uSampleWeights0 = new osg::Uniform();
				uSampleWeights0->setName("uSampleWeights");
				uSampleWeights0->setNumElements(MAX_SAMPLES);
				uSampleWeights0->setType(osg::Uniform::FLOAT_VEC4);
				for (int i = 0;i < MAX_SAMPLES;i++)
				{
					uSampleWeights0->setElement(i,avSampleWeights[i]);
				}
				m_texStarAddCam[d][texIndex - 1]->getOrCreateStateSet()->addUniform(uSampleWeights0);

				m_texStarCamGroup->addChild(m_texStarAddCam[d][texIndex - 1]);
			}
			

			if(!hasCallback)
			{
				if(!isLastPass)
				{
					MyStarCamDrawCallback* cb = new MyStarCamDrawCallback(this);
					m_texStarAddCam[d][texIndex - 1]->setInitialDrawCallback(cb);

					hasCallback = true;
				}	
			}

			vtStepUV *= nSamples;
			attnPowScale *= nSamples;

			pTexSource = m_texStarAdd[d][iWorkTexture - 1];

			iWorkTexture += 1;
			if( iWorkTexture > 2 )
			{
				iWorkTexture = 1;
			}
		}
	}

	for( int i = 0; i < starDef.m_nStarLines; i++ )
	{
		m_texStarCam[0]->getOrCreateStateSet()->setTextureAttributeAndModes(i,m_texStar[i + 4],osg::StateAttribute::ON);
		avSampleWeights[i] = vWhite * 1.0f / starDef.m_nStarLines;
	}

	char *strTechnique = "";
	switch(starDef.m_nStarLines)
	{
	case 1:
		strTechnique = const_cast<char*>(MergeTextures_1FragSource);
		break;
	case 2:
		strTechnique = const_cast<char*>(MergeTextures_2FragSource);
		break;
	case 3:
		strTechnique = const_cast<char*>(MergeTextures_3FragSource);
		break;
	case 4:
		strTechnique = const_cast<char*>(MergeTextures_4FragSource);
		break;
	case 5:
		strTechnique = const_cast<char*>(MergeTextures_5FragSource);
		break;
	case 6:
		strTechnique = const_cast<char*>(MergeTextures_6FragSource);
		break;
	case 7:
		strTechnique = const_cast<char*>(MergeTextures_7FragSource);
		break;
	case 8:
		strTechnique = const_cast<char*>(MergeTextures_8FragSource);
		break;
	}

	osg::Geode* gn1 = creatScreenQuadNode(0.0,1.0,0.0,1.0);
	m_texStarCam[0]->addChild( gn1 );

	osg::Program* program1 = new osg::Program;
	program1->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
	program1->addShader( new osg::Shader( osg::Shader::FRAGMENT, strTechnique ) );
	m_texStarCam[0]->getOrCreateStateSet()->setAttributeAndModes( program1, osg::StateAttribute::ON );
	m_texStarCam[0]->getOrCreateStateSet()->setRenderBinDetails(m_curRenderBinNum++,"");
	m_texStarCam[0]->getOrCreateStateSet()->addUniform( new osg::Uniform("tex0", 0) );
	m_texStarCam[0]->getOrCreateStateSet()->addUniform( new osg::Uniform("tex1", 1) );
	m_texStarCam[0]->getOrCreateStateSet()->addUniform( new osg::Uniform("tex2", 2) );
	m_texStarCam[0]->getOrCreateStateSet()->addUniform( new osg::Uniform("tex3", 3) );
	m_texStarCam[0]->getOrCreateStateSet()->addUniform( new osg::Uniform("tex4", 4) );
	m_texStarCam[0]->getOrCreateStateSet()->addUniform( new osg::Uniform("tex5", 5) );
	m_texStarCam[0]->getOrCreateStateSet()->addUniform( new osg::Uniform("tex6", 6) );
	m_texStarCam[0]->getOrCreateStateSet()->addUniform( new osg::Uniform("tex7", 7) );

	osg::Uniform* uSampleWeights1 = new osg::Uniform();
	uSampleWeights1->setName("uSampleWeights");
	uSampleWeights1->setNumElements(MAX_SAMPLES);
	uSampleWeights1->setType(osg::Uniform::FLOAT_VEC4);
	for (int i = 0;i < MAX_SAMPLES;i++)
	{
		uSampleWeights1->setElement(i,avSampleWeights[i]);
	}
	m_texStarCam[0]->getOrCreateStateSet()->addUniform(uSampleWeights1);

	m_texStarCamGroup->addChild(m_texStarCam[0]);
}

void FeHDRLighting::FeHDRLightingNode::initFinalSceneNode()
{
	if(m_finalSceneCam == NULL)
	{
		if(m_rpSceneTex.valid() && m_texStar[0].valid() && m_texBloom[0])// && m_apTexToneMap[0].valid())
		{
			m_finalSceneCam = new osg::Camera();

			osg::Geode* gn = creatScreenQuadNode(0.0,1.0,0.0,1.0);

			osg::StateSet* ss = new osg::StateSet();
			ss->setTextureAttributeAndModes(0,m_rpSceneTex,osg::StateAttribute::ON);
			ss->setTextureAttributeAndModes(1,m_texBloom[0],osg::StateAttribute::ON);
			ss->setTextureAttributeAndModes(2,m_texStar[0],osg::StateAttribute::ON);
			//ss->setTextureAttributeAndModes(3,m_apTexToneMap[0],osg::StateAttribute::ON);

			m_texBloom[0]->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
			m_texBloom[0]->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
			m_texStar[0]->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
			m_texStar[0]->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

			osg::Program* program = new osg::Program;
			program->addShader( new osg::Shader( osg::Shader::VERTEX, DownScale4x4VertSource ) );
			program->addShader( new osg::Shader( osg::Shader::FRAGMENT, FinalFragSource ) );
			ss->setAttributeAndModes( program, osg::StateAttribute::ON );
			ss->addUniform( new osg::Uniform("tex0", 0) );
			ss->addUniform( new osg::Uniform("tex1", 1) );
			ss->addUniform( new osg::Uniform("tex2", 2) );
			ss->addUniform( new osg::Uniform("tex3", 3) );

			ss->addUniform( new osg::Uniform("u_middleGray", 0.0f) );
			ss->addUniform( new osg::Uniform("u_starScale", m_starScale) );
			ss->addUniform( new osg::Uniform("u_bloomScale", m_bloomScale) );

			m_finalSceneCam->setStateSet(ss);
			m_finalSceneCam->setRenderOrder( osg::Camera::POST_RENDER );
			m_finalSceneCam->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
			m_finalSceneCam->addChild( gn );
		}
	}
}

void FeHDRLighting::FeHDRLightingNode::setLight( unsigned int index,const HDRLightParam& p )
{
	if(index < NUM_LIGHTS)
	{
		m_lightPow[index] = p.lightPow;
		m_avLightPosition[index].x() = p.lightPos.x();
		m_avLightPosition[index].y() = p.lightPos.y();
		m_avLightPosition[index].z() = p.lightPos.z();
		m_lightRadius[index] = p.lightRadius;
		m_lightColor[index] = p.lightColor;

		updateLight();
	}
}

FeHDRLighting::HDRLightParam FeHDRLighting::FeHDRLightingNode::getLight( unsigned int index )
{
	HDRLightParam p;
	if(index < NUM_LIGHTS)
	{
		p.lightPow = m_lightPow[index];
		p.lightPos.x() = m_avLightPosition[index].x();
		p.lightPos.y() = m_avLightPosition[index].y();
		p.lightPos.z() = m_avLightPosition[index].z();
		p.lightRadius = m_lightRadius[index];
		p.lightColor = m_lightColor[index];
	}

	return p;
}

void FeHDRLighting::FeHDRLightingNode::createBloomTextures()
{
	initSceneCamera();
	initModelSceneCamera();
	initDownScale4x4Cam();
	//measureLuminance();
	//initAdaptation();
	initSceneScaledToBrightPass();
	initBrightPassToStarSource();
	initStarSourceToBloomSource();
	initBloom();
	initStar();
	initFinalSceneNode();
}

void FeHDRLighting::FeHDRLightingNode::deleteBloomTextures()
{
	removeSceneCamFromRoot();
	m_rpSceneCamera = NULL;
	m_rpDownScale4x4Cam = NULL;
	m_texBrightPassCam = NULL;
	m_texStarSourceCam = NULL;
	m_texBloomSourceCam = NULL;
	for (int i = 0;i < NUM_BLOOM_TEXTURES;i++)
	{
		m_texBloomCam[i] = NULL;
	}
	m_texStarCamGroup = NULL;
	for (int i = 0;i < NUM_STAR_TEXTURES;i++)
	{
		m_texStarCam[i] = NULL;
	}
	for (int i = 0;i < 8;i++)
	{
		m_texStarAddCam[i][0] = NULL;
		m_texStarAddCam[i][1] = NULL;
	}
	m_finalSceneCam = NULL;
	m_rpModelSceneCam = NULL;

	m_rpSceneTex = NULL;
	m_modelSceneTex = NULL;
	m_rpDownScale4x4Tex = NULL;
	m_texBrightPass = NULL;
	m_texStarSource = NULL;
	m_texBloomSource = NULL;
	for (int i = 0;i < NUM_BLOOM_TEXTURES;i++)
	{
		m_texBloom[i] = NULL;
	}
	for (int i = 0;i < NUM_STAR_TEXTURES;i++)
	{
		m_texStar[i] = NULL;
	}
	for (int i = 0;i < 8;i++)
	{
		m_texStarAdd[i][0] = NULL;
		m_texStarAdd[i][1] = NULL;
	}
	
}

void FeHDRLighting::FeHDRLightingNode::updateBloomTextureSize()
{
	int texWidth = 0;
	int texHeight = 0;
	if(m_opRenderContext.valid())
	{
		osg::Camera* cam = m_opRenderContext->GetCamera();
		if(cam)
		{
			osg::Viewport* vp = cam->getViewport();
			if(vp)
			{
				texWidth = vp->width();
				texHeight = vp->height();
			}
		}

		if(m_rpSceneTex.valid())
		{
			if(m_rpSceneTex->getTextureWidth() != texWidth || m_rpSceneTex->getTextureHeight() != texHeight)
			{
				deleteBloomTextures();
				createBloomTextures();
			}
		}
	}
}

void FeHDRLighting::GeometryVisitor::apply( osg::Geometry& geometry )
{
}

FeHDRLighting::GeometryVisitor::GeometryVisitor( osg::Node* n )
	:m_opModelNode(n)
{

}

void FeHDRLighting::MyGeometryDrawCallback::drawImplementation( osg::RenderInfo& renderInfo,const osg::Drawable* drawable ) const
{
	if(drawable)
	{	
		drawable->drawImplementation(renderInfo);
	}
}

FeHDRLighting::MyGeometryDrawCallback::MyGeometryDrawCallback( osg::Node* n )
	:m_opModelNode(n)
{

}

FeHDRLighting::MyUpdateCallback::MyUpdateCallback( FeHDRLightingNode* ln )
	:m_opFeHDRLightingNode(ln)
{

}

void FeHDRLighting::MyUpdateCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
	if(m_opFeHDRLightingNode.valid())
	{
		m_opFeHDRLightingNode->updateUniform();

		m_opFeHDRLightingNode->m_islastFrameActive = true;
	}

	traverse(node,nv);
}

void FeHDRLighting::MyPreUpdateCallback::customOperator( const osgViewer::ViewerBase* viewerBase )
{
	if(m_opHDRLightingNode.valid())
	{
		m_opHDRLightingNode->preUpdate();
	}	
}

void FeHDRLighting::MyStarCamGroupNodeCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
	if(m_opHDRLightingNode.valid())
	{
		if( m_opHDRLightingNode->m_glareDef.m_fGlareLuminance <= 0.0f ||
			m_opHDRLightingNode->m_glareDef.m_fStarLuminance <= 0.0f )
		{
			return;
		}
		else
		{
			traverse(node,nv);
		}
	}
}

void FeHDRLighting::MyStarCamDrawCallback::operator()( osg::RenderInfo& renderInfo ) const
{
	if(m_opHDRLightingNode.valid())
	{
		if(m_opHDRLightingNode->m_texStarSource.valid())
		{
			m_opHDRLightingNode->m_texStarSource->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
			m_opHDRLightingNode->m_texStarSource->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		}
	}
}

void FeHDRLighting::MyBloomCamDrawCallback::operator()( osg::RenderInfo& renderInfo ) const
{
	if(m_opHDRLightingNode.valid())
	{
		if(m_opHDRLightingNode->m_texStarSource.valid())
		{
			m_opHDRLightingNode->m_texStarSource->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
			m_opHDRLightingNode->m_texStarSource->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
		}
	}
}

void FeHDRLighting::MySceneCamDrawCallback::operator()( osg::RenderInfo& renderInfo ) const
{
	if(m_opHDRLightingNode.valid() && m_opHDRLightingNode->m_rpLightingModel.valid())
	{
		osg::Uniform* uRenderLight = m_opHDRLightingNode->m_rpLightingModel->getOrCreateStateSet()->getUniform("uRenderLight");
		if(uRenderLight)
		{
			uRenderLight->set(false);
		}
	}
}

void FeHDRLighting::MyModelSceneCamDrawCallback::operator()( osg::RenderInfo& renderInfo ) const
{
	if(m_opHDRLightingNode.valid() && m_opHDRLightingNode->m_rpLightingModel.valid())
	{
		osg::Uniform* uRenderLight = m_opHDRLightingNode->m_rpLightingModel->getOrCreateStateSet()->getUniform("uRenderLight");
		if(uRenderLight)
		{
			uRenderLight->set(true);
		}
	}
}


FeHDRLighting::LightSphereGeomerty::LightSphereGeomerty(float r,int row /*= 10*/,int col /*= 10*/ )
	:m_row(row)
    ,m_col(col)
    ,m_radius(r)
{
	setUseDisplayList(false);
	setUseVertexBufferObjects(true);
	setDataVariance(osg::Object::DYNAMIC);

	CreateSphere();
}

void FeHDRLighting::LightSphereGeomerty::CreateSphere()
{
	osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array();
	osg::ref_ptr<osg::DrawElementsUShort> ia = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES);
	
	float colAngle = ( 2 * osg::PI ) / ((float)(m_col));
	float rowAngle = osg::PI / ((float)m_row);

	osg::Vec3 zAxis(0,0,m_radius);
	osg::Vec3 xAxis(1,0,0);

	for (int c = 0;c < m_col + 1;c++)
	{
		osg::Matrix cm = osg::Matrix::rotate(c * colAngle,zAxis);
		osg::Vec3 curAxis = xAxis * cm;
		curAxis.normalize();
		for (int r = 0;r < m_row + 1;r++)
		{
			osg::Matrix rm = osg::Matrix::rotate(r * rowAngle,curAxis);
			osg::Vec3 curPoint = zAxis * rm;

			va->push_back(curPoint);

			if(c > 0 && r > 0)
			{
				ia->push_back((c - 1) * (m_row + 1) + (r - 1));
				ia->push_back(c * (m_row + 1) + (r - 1));
				ia->push_back(c * (m_row + 1) + r);

				ia->push_back((c - 1) * (m_row + 1) + (r - 1));
				ia->push_back(c * (m_row + 1) + r);
				ia->push_back((c - 1) * (m_row + 1) + r);
			}
		}
	}

	osg::ref_ptr<osg::Vec4Array> ca = new osg::Vec4Array();
	ca->push_back(m_color);

	setVertexArray(va);
	addPrimitiveSet(ia);
	setColorArray(ca,osg::Array::BIND_OVERALL);
}

void FeHDRLighting::LightSphereGeomerty::SetColor( osg::Vec4 c )
{
	m_color = c;

	osg::Vec4Array* ca = dynamic_cast<osg::Vec4Array*>(getColorArray());
	if(ca)
	{
		ca->clear();
		ca->push_back(m_color);

		ca->dirty();
	}
}
