/**************************************************************************************************
* @file Shaders.h
* @note FeLayers库中Shader的管理类
* @author c00005
* @data 2016-8-13
**************************************************************************************************/
#ifndef FE_LAYERS_SHADERS_H
#define FE_LAYERS_SHADERS_H 1

#include <FeLayers/Export.h>

#include <osgEarth/ShaderLoader>

namespace FeLayers
{
    struct FELAYERS_EXPORT Shaders : public osgEarth::ShaderPackage
	{
        Shaders();

		std::string
			NormalMapEffect_Vertex,
			NormalMapEffect_Fragment;
	};	
}

#endif // FE_LAYERS_SHADERS_H
