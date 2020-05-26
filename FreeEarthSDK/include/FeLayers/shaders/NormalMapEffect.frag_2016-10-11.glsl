#version $GLSL_VERSION_STR
$GLSL_DEFAULT_PRECISION_FLOAT

#pragma vp_entryPoint "oe_lighting_fragment"
#pragma vp_location   "fragment_lighting"
#pragma vp_order      "0.0"

uniform sampler2D oe_nmap_normalTex; 
uniform float oe_nmap_startlod; 
uniform bool oe_mode_GL_LIGHTING; 
uniform vec4 oe_tile_key; 
varying vec4 oe_layer_tilec; 
varying vec4 oe_layer_tilecbump; //by y00001 BUMP特性
varying vec3 oe_nmap_light; 
varying vec3 oe_nmap_view; 

void oe_lighting_fragment(inout vec4 color) 
{
    if (oe_mode_GL_LIGHTING) 
    { 
        vec3 L = normalize(oe_nmap_light); 
        vec3 N = normalize(texture2D(oe_nmap_normalTex, oe_layer_tilecbump.st).xyz * 2.0 - 1.0);  //by y00001 BUMP特性
        vec3 V = normalize(oe_nmap_view); 

        vec4 ambient  = gl_LightSource[0].ambient * gl_FrontMaterial.ambient;  //by y00001根据L的值，将其从0到0.3映射，级别0~7级都为0，8~11级为从0到0.3
        float factor = 0.0;
        factor = 0.01 * oe_tile_key.z; 
        if(factor>0.2) factor=0.2; 
        float D = max(dot(L, N), factor); 
        vec4 diffuse  = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * D; 

		//float S = pow(clamp(dot(reflect(-L,N),V),0.0,1.0), gl_FrontMaterial.shininess); 
		//vec4 specular = gl_LightSource[0].specular * gl_FrontMaterial.specular * S; 
		float factorDiffuse = 1.0; 
		if(oe_tile_key.z <= 13.0) {factorDiffuse = 1.2;}

		//if(factorDiffuse<=1.0) factorDiffuse = 1.0; 

        color.rgb = (ambient.rgb*color.rgb) + (diffuse.rgb*color.rgb*factorDiffuse);// + specular.rgb; 
    } 
};

//varying vec4 oe_layer_tilecbump; 
//vec3 oe_global_Normal;
//uniform vec4 oe_tile_key;
//uniform sampler2D oe_nmap_normalTex;
//varying mat3 oe_nmap_TBN; 
//
//void oe_lighting_fragment(inout vec4 color)
//{
//	vec4 encodedNormal = texture2D(oe_nmap_normalTex, oe_layer_tilecbump.st);
//	vec3 normalTangent = normalize(encodedNormal.xyz*2.0-1.0);
//	oe_global_Normal = normalize(oe_nmap_TBN * normalTangent);
//};