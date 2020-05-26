#version $GLSL_VERSION_STR
$GLSL_DEFAULT_PRECISION_FLOAT

#pragma vp_entryPoint "atmos_fragment_main"
#pragma vp_location   "fragment_lighting"
#pragma vp_order      "0.8"

uniform bool oe_mode_GL_LIGHTING;  
uniform float atmos_exposure;    // scene exposure (ground level)
varying vec3 atmos_lightDir;     // light direction (view coords)
varying vec3 atmos_color;        // atmospheric lighting color
varying vec3 atmos_atten;        // atmospheric lighting attentuation factor
varying vec3 atmos_up;           // earth up vector at fragment (in view coords)
varying float atmos_space;       // camera altitude (0=ground, 1=atmos outer radius)
varying vec3 atmos_vert;  

vec3 oe_global_Normal;           // surface normal (from osgEarth)

uniform sampler2D atmos_s2Tex0;                                                                                                                  
uniform sampler2D atmos_s2Tex1;                                                                                                                  
varying vec4 oe_layer_tc_t;                                                                                                                      
varying float fBump;  
varying float outerRadius;                                                                                                                            
                                                                                                                                                 

void atmos_fragment_main(inout vec4 color)  
{  
    if ( oe_mode_GL_LIGHTING == false )
	{
		return;
	}
	
	//h00017
	//if(outerRadius > 0.0)
	//{
	//    color.rgb = color.rgb * fBump * gl_LightSource[0].ambient.rgb * 0.7 + color.rgb * 0.3; 
	//}
	//else
	{
	    color.rgb =  atmos_color + color.rgb * fBump * max(atmos_atten, gl_LightSource[0].ambient.rgb) * 0.7+color.rgb * 0.3;
	}
}