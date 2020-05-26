#version 110
#pragma vp_entryPoint "oe_nmap_fragment"
#pragma vp_location   "fragment_coloring"
#pragma vp_order      "0.2"

// stage global:
varying vec4 oe_layer_tilecbump; 

vec3 oe_global_Normal;

uniform vec4 oe_tile_key;
uniform sampler2D oe_nmap_normalTex;
//varying vec4 oe_nmap_normalCoords;
varying mat3 oe_nmap_TBN; 

void oe_nmap_fragment(inout vec4 color)
{
    vec4 encodedNormal = texture2D(oe_nmap_normalTex, oe_layer_tilecbump.st);
    vec3 normalTangent = normalize(encodedNormal.xyz*2.0-1.0);
    oe_global_Normal = normalize(oe_nmap_TBN * normalTangent);

    // visualize curvature:
    //color.rgb = vec3(0,0,0);
    //if(decoded.a >= 0.4) color.r = 1.0;
    //if(decoded.a <= -0.4) color.b = 1.0;

    // visualize normals:
    //color.rgb = encodedNormal.xyz;
}


//uniform vec4 oe_tile_key;
//uniform sampler2D oe_nmap_normalTex;
//
//varying vec4 oe_layer_tilecbump; 
//varying vec3 oe_nmap_light;
//
//void oe_nmap_fragment(inout vec4 color)
//{
//	vec3 L = normalize(oe_nmap_light);
//    vec3 N = normalize(texture2D(oe_nmap_normalTex, oe_layer_tilecbump.st).xyz * 2.0 - 1.0);
//
//	//by y00001根据L的值，将其从0到0.3映射，级别0~7级都为0，8~11级为从0到0.3
//    vec4 ambient  = gl_LightSource[0].ambient * gl_FrontMaterial.ambient;
//
//    float factor = 0.0;
//    factor = 0.01 * oe_tile_key.z; 
//    if(factor>0.2) 
//	{
//		factor=0.2;
//	}
//	 
//    float D = max(dot(L, N), factor); 
//    vec4 diffuse  = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * D; 
//   
//     float factorDiffuse = 1.0; 
//     if(oe_tile_key.z <= 13.0) 
//	 {
//		factorDiffuse = 1.2;
//	 }
//
//	 //color.rgb = (ambient.rgb*color.rgb) + (diffuse.rgb*color.rgb*factorDiffuse);
//	 color.rgb = N;
//}