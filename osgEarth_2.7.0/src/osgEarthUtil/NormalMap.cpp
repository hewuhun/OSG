/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
* Copyright 2008-2012 Pelican Mapping
* http://osgearth.org
*
* osgEarth is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*/
#include <osgEarthUtil/NormalMap>
#include <osgEarth/Registry>
#include <osgEarth/Capabilities>
#include <osgEarth/VirtualProgram>
#include <osgEarth/TerrainEngineNode>

#define LC "[NormalMap] "

using namespace osgEarth;
using namespace osgEarth::Util;

namespace
{
	const char* vs = 
		"#version " GLSL_VERSION_STR "\n"
		GLSL_DEFAULT_PRECISION_FLOAT "\n"
		"	varying vec3 oe_Normal;\n"
		"varying vec4 oe_layer_tilec;\n"
		"uniform mat4 oe_nmap_normalTexMatrix;\n"
		"//varying vec4 oe_nmap_normalCoords;\n"
		"varying mat3 oe_nmap_TBN;\n"
		"void oe_lighting_vertex(inout vec4 VertexMODEL)\n"
		"{\n"
		"	//oe_nmap_normalCoords = oe_nmap_normalTexMatrix * oe_layer_tilec;\n"
		"	// form the matrix that will transform a normal vector from\n"
		"	// tangent space to model space in the fragment shader.\n"
		"	// We expect the vertex normal to be a simple UP vector.\n"
		"	vec3 B = vec3(0,1,0);\n"
		"	vec3 N = oe_Normal;\n"
		"	vec3 T = normalize(cross(B,N));\n"
		"	oe_nmap_TBN = gl_NormalMatrix * mat3(T, B, N);\n"
		"}\n";


	const char* fs =
		"#version " GLSL_VERSION_STR "\n"
		GLSL_DEFAULT_PRECISION_FLOAT "\n"
		"varying vec4 oe_layer_tilecbump; \n"
		"vec3 oe_global_Normal;\n"
		"uniform vec4 oe_tile_key;\n"
		"uniform sampler2D oe_nmap_normalTex;\n"
		"varying mat3 oe_nmap_TBN; \n"
		"void oe_lighting_fragment(inout vec4 color)\n"
		"{\n"
		"	vec4 encodedNormal = texture2D(oe_nmap_normalTex, oe_layer_tilecbump.st);\n"
		"	vec3 normalTangent = normalize(encodedNormal.xyz*2.0-1.0);\n"
		"	oe_global_Normal = normalize(oe_nmap_TBN * normalTangent);\n"
		"}\n";
}
//namespace
//{
//    const char* vs =
//        "#version " GLSL_VERSION_STR "\n"
//        GLSL_DEFAULT_PRECISION_FLOAT "\n"
//
//        "varying vec3 oe_nmap_light; \n"
//        "varying vec3 oe_nmap_view; \n"
//        "varying vec3 oe_Normal; \n"
//        "uniform bool oe_mode_GL_LIGHTING; \n"
//
//        "void oe_lighting_vertex(inout vec4 VertexVIEW) \n"
//        "{ \n"
//        "    if (oe_mode_GL_LIGHTING) \n"
//        "    { \n"
//        "        vec3 tangent = normalize(cross(gl_Normal, vec3(0,-1,0))); \n"
//
//        "        vec3 n = oe_Normal; \n" //normalize(gl_NormalMatrix * gl_Normal); \n"
//        "        vec3 t = normalize(gl_NormalMatrix * tangent); \n"
//        "        vec3 b = cross(n, t); \n"
//
//        "        vec3 tmp = gl_LightSource[0].position.xyz; \n"
//        "        oe_nmap_light.x = dot(tmp, t); \n"
//        "        oe_nmap_light.y = dot(tmp, b); \n"
//        "        oe_nmap_light.z = dot(tmp, n); \n"
//
//        "        tmp = -VertexVIEW.xyz; \n"
//        "        oe_nmap_view.x = dot(tmp, t); \n"
//        "        oe_nmap_view.y = dot(tmp, b); \n"
//        "        oe_nmap_view.z = dot(tmp, n); \n"
//        "    } \n"
//        "} \n";
//
//    const char* fs =
//        "#version " GLSL_VERSION_STR "\n"
//        GLSL_DEFAULT_PRECISION_FLOAT "\n"
//
//        "uniform sampler2D oe_nmap_tex; \n"
//        "uniform float oe_nmap_startlod; \n"
//        "uniform bool oe_mode_GL_LIGHTING; \n"
//        "uniform vec4 oe_tile_key; \n"
//        "varying vec4 oe_layer_tilec; \n"
//        "varying vec4 oe_layer_tilecbump;\n" //by y00001 BUMP特性
//        "varying vec3 oe_nmap_light; \n"
//        "varying vec3 oe_nmap_view; \n"
//
//        "void oe_lighting_fragment(inout vec4 color) \n"
//        "{\n"
//        "    if (oe_mode_GL_LIGHTING) \n"
//        "    { \n"
//        "        vec3 L = normalize(oe_nmap_light); \n"
//        "        vec3 N = normalize(texture2D(oe_nmap_tex, oe_layer_tilecbump.st).xyz * 2.0 - 1.0); \n" //by y00001 BUMP特性
//        "        vec3 V = normalize(oe_nmap_view); \n"
//
//        "        vec4 ambient  = gl_LightSource[0].ambient * gl_FrontMaterial.ambient; \n" //by y00001根据L的值，将其从0到0.3映射，级别0~7级都为0，8~11级为从0到0.3
//        "        float factor = 0.0;\n"
//        "        factor = 0.01 * oe_tile_key.z; \n"
//        "        if(factor>0.2) factor=0.2; \n"
//        "        float D = max(dot(L, N), factor); \n"
//        "        vec4 diffuse  = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * D; \n"
//       
//        //"        float S = pow(clamp(dot(reflect(-L,N),V),0.0,1.0), gl_FrontMaterial.shininess); \n"
//        //"        vec4 specular = gl_LightSource[0].specular * gl_FrontMaterial.specular * S; \n"
//        "  float factorDiffuse = 1.0; \n"
//        "  if(oe_tile_key.z <= 13.0) {factorDiffuse = 1.2;}\n"
//
//        //"  if(factorDiffuse<=1.0) factorDiffuse = 1.0; \n"
//
//
//        "        color.rgb = (ambient.rgb*color.rgb) + (diffuse.rgb*color.rgb*factorDiffuse);" // + specular.rgb; \n"
//        "    } \n"
//        "}\n";
//}


NormalMap::NormalMap() :
TerrainEffect(),
_startLOD    ( 0 )
{
    init();
}

NormalMap::NormalMap(const Config& conf, Map* map) :
TerrainEffect(),
_startLOD    ( 0 )
{
    mergeConfig(conf);

    if ( map && _layerName.isSet() )
    {
        setNormalMapLayer( map->getImageLayerByName(*_layerName) );
    }

    init();
}


void
NormalMap::init()
{
    _startLODUniform = new osg::Uniform(osg::Uniform::FLOAT, "oe_nmap_startlod");
    _startLODUniform->set( 0.0f );
}


void
NormalMap::setStartLOD(unsigned value)
{
    _startLOD = value;
    _startLODUniform->set( (float)value );
}


NormalMap::~NormalMap()
{
    //nop
}

void
NormalMap::onInstall(TerrainEngineNode* engine)
{
    if ( engine )
    {
        osg::StateSet* stateset = engine->getOrCreateStateSet();
        if ( _layer.valid() )
        {
            OE_NOTICE << LC << "Installing layer " << _layer->getName() << " as normal map" << std::endl;
            int unit = *_layer->shareImageUnit();
            _samplerUniform = stateset->getOrCreateUniform("oe_nmap_normalTex", osg::Uniform::SAMPLER_2D);
            _samplerUniform->set(unit);
        }
        
        stateset->addUniform( _startLODUniform.get() );

        VirtualProgram* vp = VirtualProgram::getOrCreate(stateset);

        // these special (built-in) function names are for the main lighting shaders.
        // using them here will override the default lighting.
        vp->setFunction( "oe_lighting_vertex",   vs, ShaderComp::LOCATION_VERTEX_VIEW, 0.0 );
        vp->setFunction( "oe_lighting_fragment", fs, ShaderComp::LOCATION_FRAGMENT_LIGHTING, 0.0 );
    }
}


void
NormalMap::onUninstall(TerrainEngineNode* engine)
{
    if ( engine )
    {
        osg::StateSet* stateset = engine->getStateSet();
        if ( stateset )
        {
            stateset->removeUniform( _samplerUniform.get() );
            stateset->removeUniform( _startLODUniform.get() );
            VirtualProgram* vp = VirtualProgram::get(stateset);
            if ( vp )
            {
                vp->removeShader( "oe_lighting_vertex" );
                vp->removeShader( "oe_lighting_fragment" );
            }
        }
    }
}


//-------------------------------------------------------------

void
NormalMap::mergeConfig(const Config& conf)
{
    conf.getIfSet( "layer",       _layerName );
    conf.getIfSet( "start_level", _startLOD );
    conf.getIfSet( "start_lod",   _startLOD );
}

Config
NormalMap::getConfig() const
{
    optional<std::string> layername;

    if ( _layer.valid() && !_layer->getName().empty() )
        layername = _layer->getName();

    Config conf("normal_map");
    conf.addIfSet( "layer",       layername );
    conf.addIfSet( "start_level", _startLOD );
    return conf;
}
