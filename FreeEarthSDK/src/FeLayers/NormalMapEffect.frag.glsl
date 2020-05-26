#version $GLSL_VERSION_STR
$GLSL_DEFAULT_PRECISION_FLOAT

#pragma vp_entryPoint "oe_lighting_fragment"
#pragma vp_location   "fragment_lighting"
#pragma vp_order      "0.0"

///���淨�߹���ͼ
uniform sampler2D oe_nmap_normalTex; 

///ȫ�ֱ��������ڱ��浱ǰ�����Ƿ���
uniform bool oe_mode_GL_LIGHTING; 

///vec4���ͣ����ڱ��浱ǰ��Ƭ��X��Y��L����Ƭ��Χ��İ뾶
uniform vec4 oe_tile_key; 

///���߹�����ͼ����������
varying vec4 oe_layer_tilecbump; 

///���ڱ��������Ĺ��յķ���
varying vec3 oe_nmap_light; 

void oe_lighting_fragment(inout vec4 color) 
{
	//�����ǰ�������տ�������ʵ�ַ��߹���Ч��
	if (oe_mode_GL_LIGHTING) 
	{ 
		vec3 L = normalize(oe_nmap_light); 
		vec3 N = normalize(texture2D(oe_nmap_normalTex, oe_layer_tilecbump.st).xyz * 2.0-1.0); 

		//������
		vec4 ambient  = gl_LightSource[0].ambient * gl_FrontMaterial.ambient;  

		//ɢ��⣺��ҪӰ�취�߹���Ч���Ĺ�
		vec4 diffuse  = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * max(dot(L, N), 0.2); 

		//�����
		vec4 specular = gl_LightSource[0].specular * gl_FrontMaterial.specular ; 

		color.rgb = (ambient.rgb*color.rgb) + (diffuse.rgb*color.rgb) + (specular.rgb); 
	} 
};