#version $GLSL_VERSION_STR 
$GLSL_DEFAULT_PRECISION_FLOAT

#pragma vp_entryPoint "oe_lighting_vertex"
#pragma vp_location   "vertex_view"
#pragma vp_order      "0.0"
	
uniform bool oe_mode_GL_LIGHTING;

varying vec3 oe_Normal;
varying vec3 oe_nmap_light;

void oe_lighting_vertex(inout vec4 VertexVIEW)
{
    if (oe_mode_GL_LIGHTING)
    {
        vec3 tangent = normalize(cross(gl_Normal, vec3(0,-1,0)));

        vec3 n = oe_Normal; 
        vec3 t = normalize(gl_NormalMatrix * tangent);
        vec3 b = cross(n, t);

        oe_nmap_light.x = dot(gl_LightSource[0].position.xyz, t);
        oe_nmap_light.y = dot(gl_LightSource[0].position.xyz, b);
        oe_nmap_light.z = dot(gl_LightSource[0].position.xyz, n);
    }
};
