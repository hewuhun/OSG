#include <FeLayers/NormalMapEffect.h>

#include <FeUtils/logger/LoggerDef.h>

#include <osgEarth/Registry>
#include <osgEarth/Capabilities>
#include <osgEarth/VirtualProgram>
#include <osgEarth/TerrainEngineNode>

#include "Shaders.h"

using namespace osgEarth;
using namespace osgEarth::Util;

namespace FeLayers
{
	CNormalMapEffect::CNormalMapEffect()
		:TerrainEffect()
	{

	}

	CNormalMapEffect::CNormalMapEffect(const Config& conf, Map* map)
		:TerrainEffect()
	{
		conf.getIfSet( "layer", m_strLayerName );

		if ( map && m_strLayerName.isSet() )
		{
			setNormalMapLayer( map->getImageLayerByName(*m_strLayerName) );
		}
	}

	CNormalMapEffect::~CNormalMapEffect()
	{

	}

	Config CNormalMapEffect::getConfig() const
	{
		optional<std::string> layername;

		if ( m_opImageLayer.valid() && !m_opImageLayer->getName().empty() )
			layername = m_opImageLayer->getName();

		Config conf("normal_map");
		conf.addIfSet( "layer", layername );

		return conf;
	}

	void CNormalMapEffect::setNormalMapLayer( ImageLayer* layer )
	{
		m_opImageLayer = layer;
	}

	ImageLayer* CNormalMapEffect::getNormalMapLayer()
	{
		return m_opImageLayer.get();
	}

	void CNormalMapEffect::onInstall(TerrainEngineNode* engine)
	{
		if ( engine )
		{
			osg::StateSet* stateset = engine->getOrCreateStateSet();
			if ( m_opImageLayer.valid() )
			{
               // LOG(LOG_INFO) << "安装法线光照特效";
				int unit = m_opImageLayer->shareImageUnit().value();
				m_rpSamplerUniform = stateset->getOrCreateUniform("oe_nmap_normalTex", osg::Uniform::SAMPLER_2D);
				m_rpSamplerUniform->set(unit);
			}

			VirtualProgram* vp = VirtualProgram::getOrCreate(stateset);

			FeLayers::Shaders pkg;
			pkg.load(vp, pkg.NormalMapEffect_Vertex);
			pkg.load(vp, pkg.NormalMapEffect_Fragment);
		}
	}

	void CNormalMapEffect::onUninstall(TerrainEngineNode* engine)
	{
		if ( engine )
		{
			osg::StateSet* stateset = engine->getStateSet();
			if ( stateset )
			{
                //LOG(LOG_INFO) << "卸载法线光照特效";
				stateset->removeUniform( m_rpSamplerUniform.get() );

				VirtualProgram* vp = VirtualProgram::get(stateset);
				if ( vp )
				{
					Shaders pkg;
					pkg.unload(vp, pkg.NormalMapEffect_Vertex);
					pkg.unload(vp, pkg.NormalMapEffect_Fragment);
				}
			}
		}
	}
}
