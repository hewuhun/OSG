#version $GLSL_VERSION_STR
$GLSL_DEFAULT_PRECISION_FLOAT

#pragma vp_entryPoint "oe_lighting_fragment"
#pragma vp_location   "fragment_lighting"
#pragma vp_order      "0.0"

///保存法线光照图
uniform sampler2D oe_nmap_normalTex; 

///全局变量，用于保存当前光照是否开启
uniform bool oe_mode_GL_LIGHTING; 

///vec4类型，用于保存当前瓦片的X、Y、L和瓦片包围球的半径
uniform vec4 oe_tile_key; 

///法线光照贴图的纹理坐标
varying vec4 oe_layer_tilecbump; 

///用于保存计算出的光照的方向
varying vec3 oe_nmap_light; 

void oe_lighting_fragment(inout vec4 color) 
{
	//如果当前场景光照开启，则实现法线光照效果
	if (oe_mode_GL_LIGHTING) 
	{ 
		vec3 L = normalize(oe_nmap_light); 
		vec3 N = normalize(texture2D(oe_nmap_normalTex, oe_layer_tilecbump.st).xyz * 2.0-1.0); 

		//环境光
		vec4 ambient  = gl_LightSource[0].ambient * gl_FrontMaterial.ambient;  

		//散射光：主要影响法线光照效果的光
		vec4 diffuse  = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * max(dot(L, N), 0.2); 

		//镜面光
		vec4 specular = gl_LightSource[0].specular * gl_FrontMaterial.specular ; 

		color.rgb = (ambient.rgb*color.rgb) + (diffuse.rgb*color.rgb) + (specular.rgb); 
	} 
};