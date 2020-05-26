#include <FeKits/weather/GlobalFog.h>

#include <osgEarth/Registry>
#include <osgEarth/Capabilities>
#include <osgEarth/VirtualProgram>
#include <osgEarth/TerrainEngineNode>

#define LC "[Fog] "

using namespace osgEarth;
using namespace osgEarth::Util;

namespace
{
    const char* vs =
        "#version " GLSL_VERSION_STR "\n"
        GLSL_DEFAULT_PRECISION_FLOAT "\n"

        "varying float fogFactor;\n"
        "attribute vec4 oe_terrain_attr; \n"
        "void oe_globalfog_vertex(inout vec4 VertexVIEW) \n"
        "{ \n"        
        "    vec3  upVector  = oe_terrain_attr.xyz; \n"
        "    float elev       = oe_terrain_attr.w; \n" //当前海拔高度
        "    vec3  v3Pos       = VertexVIEW.xyz; \n" //视点位置
        "    float z = length( VertexVIEW.xyz );\n"
        //"    const float LOG2 = 1.442695;\n"        
        //"    fogFactor = exp2( -gl_Fog.density * gl_Fog.density * z * z * LOG2 );\n"
        "    fogFactor = 0.0; \n"
        "    float dl = 6000.0 - elev; \n"
        "    if((upVector.x>3000.0)) \n"
        "    { \n"
        "        fogFactor = 0.5; \n"
        "    } \n"
        "    fogFactor = clamp(fogFactor, 0.0, 1.0);\n"
        "} \n";

    const char* fs =
        "#version " GLSL_VERSION_STR "\n"
        GLSL_DEFAULT_PRECISION_FLOAT "\n"

        "varying float fogFactor;\n"

        "void oe_globalfog_frag(inout vec4 color) \n"
        "{ \n"        
        "    color.rgb = mix(color.rgb, vec3(1.0, 1.0, 1.0), fogFactor);\n"
        "} \n";
}

namespace FeKit
{
    CGlobalFogEffect::CGlobalFogEffect()
    {

    }

    CGlobalFogEffect::~CGlobalFogEffect()
    {
        detach();
    }

    void CGlobalFogEffect::attach( osg::StateSet* stateSet )
    {
        VirtualProgram* vp = VirtualProgram::getOrCreate( stateSet );
        vp->setFunction( "oe_globalfog_vertex", vs, ShaderComp::LOCATION_VERTEX_VIEW );
        vp->setFunction( "oe_globalfog_frag", fs, ShaderComp::LOCATION_FRAGMENT_LIGHTING );
        _statesets.push_back(stateSet);
    }

    void CGlobalFogEffect::detach( osg::StateSet* stateSet )
    {
        VirtualProgram* vp = VirtualProgram::get(stateSet);
        if ( vp )
        {
            vp->removeShader( "oe_globalfog_vertex" );
            vp->removeShader( "oe_globalfog_frag" );
        }
    }

    void CGlobalFogEffect::detach()
    {
        for (StateSetList::iterator it = _statesets.begin(); it != _statesets.end(); ++it)
        {
            osg::ref_ptr<osg::StateSet> stateset;
            if ( (*it).lock(stateset) )
            {
                detach( stateset );
                (*it) = 0L;
            }
        }
    }

}