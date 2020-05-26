#version $GLSL_VERSION_STR 
$GLSL_DEFAULT_PRECISION_FLOAT

#pragma vp_entryPoint "oe_lighting_vertex"
#pragma vp_location   "vertex_view"
#pragma vp_order      "0.0"

varying vec3 oe_nmap_light;
varying vec3 oe_nmap_view;
varying vec3 oe_Normal;

uniform bool oe_mode_GL_LIGHTING;

void oe_lighting_vertex(inout vec4 VertexVIEW)
{
    if (oe_mode_GL_LIGHTING)
    {
        vec3 tangent = normalize(cross(gl_Normal, vec3(0,-1,0)));

        vec3 n = oe_Normal; //normalize(gl_NormalMatrix * gl_Normal);
        vec3 t = normalize(gl_NormalMatrix * tangent);
        vec3 b = cross(n, t);

        vec3 tmp = gl_LightSource[0].position.xyz;
        oe_nmap_light.x = dot(tmp, t);
        oe_nmap_light.y = dot(tmp, b);
        oe_nmap_light.z = dot(tmp, n);

        tmp = -VertexVIEW.xyz;
        oe_nmap_view.x = dot(tmp, t);
        oe_nmap_view.y = dot(tmp, b);
        oe_nmap_view.z = dot(tmp, n);
    }
};


//varying vec3 oe_Normal;
//varying vec4 oe_layer_tilec;
//uniform mat4 oe_nmap_normalTexMatrix;
//varying mat3 oe_nmap_TBN;
//void oe_lighting_vertex(inout vec4 VertexMODEL)
//{
//	//oe_nmap_normalCoords = oe_nmap_normalTexMatrix * oe_layer_tilec;
//	// form the matrix that will transform a normal vector from
//	// tangent space to model space in the fragment shader.
//	// We expect the vertex normal to be a simple UP vector.
//	vec3 B = vec3(0,1,0);
//	vec3 N = gl_Normal;
//	vec3 T = normalize(cross(B,N));
//	oe_nmap_TBN = gl_NormalMatrix * mat3(T, B, N);
//};