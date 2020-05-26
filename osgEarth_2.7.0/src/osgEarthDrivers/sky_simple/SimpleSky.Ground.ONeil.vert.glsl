#version $GLSL_VERSION_STR
$GLSL_DEFAULT_PRECISION_FLOAT

#pragma vp_entryPoint "atmos_vertex_main"
#pragma vp_location   "vertex_view"
#pragma vp_order      "0.5"

uniform bool oe_mode_GL_LIGHTING;  

uniform mat4 osg_ViewMatrixInverse;   // world camera position in [3].xyz  
uniform mat4 osg_ViewMatrix;          // GL view matrix  
uniform vec3 atmos_v3LightDir;        // The direction vector to the light source  
uniform vec3 atmos_v3InvWavelength;   // 1 / pow(wavelength,4) for the rgb channels  
uniform float atmos_fOuterRadius;     // Outer atmosphere radius  
uniform float atmos_fOuterRadius2;    // fOuterRadius^2  		
uniform float atmos_fInnerRadius;     // Inner planetary radius  
uniform float atmos_fInnerRadius2;    // fInnerRadius^2  
uniform float atmos_fKrESun;          // Kr * ESun  	
uniform float atmos_fKmESun;          // Km * ESun  		
uniform float atmos_fKr4PI;           // Kr * 4 * PI  	
uniform float atmos_fKm4PI;           // Km * 4 * PI  		
uniform float atmos_fScale;           // 1 / (fOuterRadius - fInnerRadius)  	
uniform float atmos_fScaleDepth;      // The scale depth  
uniform float atmos_fScaleOverScaleDepth;     // fScale / fScaleDepth  	
uniform int atmos_nSamples;  	
uniform float atmos_fSamples;  

varying vec4 oe_layer_tc_t;                                                                                                                            
varying float fBump;                                                                                                                                   
varying vec3 atmos_color;  
varying vec3 atmos_atten;  
varying vec3 atmos_lightDir;  
varying float outerRadius; 
                                                                                                                                                       
                                                                                                                                                       
float scale(float fCos)                                                                                                                                
{                                                                                                                                                      
	float x = 1.0 - fCos;                                                                                                                               
	return atmos_fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));                                                               
}                                                                                                                                                      
                                                                                                                                                       
void atmos_vertex_main(inout vec4 vertex)                                                                                                              
{                                                                                                                                                      
	// Get the ray from the camera to the vertex and its length (which is the far point of the ray passing through the atmosphere)                      
	vec4 v4Pos = gl_ModelViewMatrix * gl_Vertex;                                                                                                        
	vec3 v3Pos = (osg_ViewMatrixInverse *v4Pos).xyz;                                                                                                          
	vec3 v3CameraPos = (osg_ViewMatrixInverse*vec4(0,0,0,1)).xyz;                                                                                             
	vec3 v3Ray = v3Pos - v3CameraPos;                                                                                                                   
	float fFar = length(v3Ray);                                                                                                                         
	v3Ray /= fFar;                                                                                                                                      
                                                                                                                                                       
	vec3 v3EarthPos = vec3(0, 0, 0);                                                                                                                    
	float fCameraHeight = length(v3EarthPos-v3CameraPos);                                                                                               
	vec3 v3Attenuate;                                                                                                                                   
	vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);   

	float atmos_space = max(0.0, (fCameraHeight-atmos_fInnerRadius)/(atmos_fOuterRadius-atmos_fInnerRadius));   
	fBump = clamp(1.0-atmos_space, 0.6, 0.8);

	//h00017
	outerRadius = 1.0;
	vec3 lightDir = normalize(atmos_v3LightDir); \n
    vec3 posDir = normalize(v3Pos);\n
	float lightDot = clamp(dot(lightDir, posDir) * 5.0,0.0,1.0);\n
	fBump = lightDot;    
	                                                                                                      
	if(fCameraHeight > atmos_fOuterRadius)                                                                                                              
	{                                                                                                                                                   
		// Calculate the closest intersection of the ray with the outer atmosphere (which is the near point of the ray passing through the atmosphere)  
		float B = 2.0 * dot(v3CameraPos, v3Ray);                                                                                                        
		float C = fCameraHeight*fCameraHeight - atmos_fOuterRadius2;                                                                                    
		float fDet = max(0.0, B*B - 4.0 * C);                                                                                                           
		float fNear = 0.5 * (-B - sqrt(fDet));                                                                                                          
                                                                                                                                                       
		// Calculate the ray's starting position, then calculate its scattering offset                                                                  
		vec3 v3Start = v3CameraPos + v3Ray * fNear;                                                                                                     
		fFar -= fNear;                                                                                                                                  
		float fDepth = exp((atmos_fInnerRadius - atmos_fOuterRadius) / atmos_fScaleDepth);                                                              
		float fCameraAngle = dot(-v3Ray, v3Pos) / length(v3Pos);                                                                                        
		float fLightAngle = dot(atmos_v3LightDir, v3Pos) / length(v3Pos);                                                                                     
		float fCameraScale = scale(fCameraAngle);                                                                                                       
		float fLightScale = scale(fLightAngle);                                                                                                         
		float fCameraOffset = fDepth*fCameraScale;                                                                                                      
		float fTemp = (fLightScale + fCameraScale);                                                                                                     
                                                                                                                                                       
		// Initialize the scattering loop variables                                                                                                     
		float fSampleLength = fFar / atmos_fSamples;                                                                                                    
		float fScaledLength = fSampleLength * atmos_fScale;                                                                                             
		vec3 v3SampleRay = v3Ray * fSampleLength;                                                                                                       
		vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;                                                                                               
                                                                                                                                                       
		// Now loop through the sample rays                                                                                                             
		for(int i=0; i<atmos_nSamples; i++)                                                                                                             
		{                                                                                                                                               
			float fHeight = length(v3SamplePoint);                                                                                                      
			float fDepth = exp(atmos_fScaleOverScaleDepth * (atmos_fInnerRadius - fHeight));                                                            
			float fScatter = fDepth*fTemp - fCameraOffset;                                                                                              
			v3Attenuate = exp(-fScatter * (atmos_v3InvWavelength * atmos_fKr4PI + atmos_fKm4PI));                                                       
			v3FrontColor += v3Attenuate * (fDepth * fScaledLength);                                                                                     
			v3SamplePoint += v3SampleRay;                                                                                                               
		}                                                                                                                                               
		//fBump = 1.0;       	                                                                                                                       
	}                                                                                                                                                   
	else                                                                                                                                                
	{                                                                                                                                                   
		// Calculate the ray's starting position, then calculate its scattering offset                                                                  
		vec3 v3Start = v3CameraPos;                                                                                                                     
		float fDepth = exp((atmos_fInnerRadius - fCameraHeight) / atmos_fScaleDepth);                                                                   
		float fCameraAngle = dot(-v3Ray, v3Pos) / length(v3Pos);                                                                                        
		float fLightAngle = dot(atmos_v3LightDir, v3Pos) / length(v3Pos);                                                                                     
		float fCameraScale = scale(fCameraAngle);                                                                                                       
		float fLightScale = scale(fLightAngle);                                                                                                         
		float fCameraOffset = fDepth*fCameraScale;                                                                                                      
		float fTemp = (fLightScale + fCameraScale);                                                                                                     
                                                                                                                                                       
		// Initialize the scattering loop variables                                                                                                     
		float fSampleLength = fFar / atmos_fSamples;                                                                                                    
		float fScaledLength = fSampleLength * atmos_fScale;                                                                                             
		vec3 v3SampleRay = v3Ray * fSampleLength;                                                                                                       
		vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;                                                                                               
                                                                                                                                                       
		// Now loop through the sample rays                                                                                                             
		for(int i=0; i<atmos_nSamples; i++)                                                                                                             
		{                                                                                                                                               
			float fHeight = length(v3SamplePoint);                                                                                                      
			float fDepth = exp(atmos_fScaleOverScaleDepth * (atmos_fInnerRadius - fHeight));                                                            
			float fScatter = fDepth*fTemp - fCameraOffset;                                                                                              
			v3Attenuate = exp(-fScatter * (atmos_v3InvWavelength * atmos_fKr4PI + atmos_fKm4PI));                                                       
			v3FrontColor += v3Attenuate * (fDepth * fScaledLength);                                                                                     
			v3SamplePoint += v3SampleRay;                                                                                                               
		}                                                                                                                                               
                                                                                                                                                       
		//Calculate bump                                                                                                                                
		vec3 v3Normal = normalize(gl_NormalMatrix*gl_Normal);                                                                                           
		//fBump = sqrt(max(0.1, dot(atmos_v3LightDir, v3Normal)));    
		
		outerRadius = -1.0;                                                                                          
	}                                                                                                                                                   
	//gl_FrontColor.rgb = v3FrontColor * (atmos_v3InvWavelength * atmos_fKrESun + atmos_fKmESun);                                                         
	//gl_FrontColor.a = 0.1;                                                                                                                              
	//gl_FrontSecondaryColor.a = 1.0;                                                                                                                   
	// Calculate the attenuation factor for the ground                                                                                                  
	//gl_FrontSecondaryColor.rgb = v3Attenuate;                                                                                                           
                                                                                                                                                       
	//gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;                                                                                             
	//gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;                                                                                           
	//gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord1;                                                                                           
	//oe_layer_tc_t = gl_MultiTexCoord0;                                                                                                                  
   atmos_color = v3FrontColor * (atmos_v3InvWavelength * atmos_fKrESun + atmos_fKmESun);  
   atmos_atten = v3Attenuate;  
}                                                                                                                                                      ;

