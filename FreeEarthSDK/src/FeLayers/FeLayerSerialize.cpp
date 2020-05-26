#include <osgEarth/XmlUtils>
#include <LayerSys.h>
#include <FeLayerSerialize.h>


using namespace FeLayers;


// 序列化用到的字段名
//const char* g_szMapLayerSys = "map_layer_sys";
const char* g_szLayerList = "layer_list";
const char* g_szLayer = "layer";


CFeConfig getConfigFrom(const FeUtil::TiXmlElement* pElement)
{
	CFeConfig conf( pElement->Value() );

	const FeUtil::TiXmlElement* child = pElement->FirstChildElement();
	while ( child )
	{
		conf.add( getConfigFrom(child) );
		child = child->NextSiblingElement();
	}

	const char* pText = pElement->GetText();
	if( pText )
	{
		conf.value() = pText;
	}

	return conf;
}

void setConfigTo(const CFeConfig& conf, FeUtil::TiXmlElement* pElement)
{
	pElement->SetValue(conf.key().c_str());

	if ( !conf.value().empty() )
	{
		FeUtil::TiXmlText text(conf.value().c_str());
		pElement->InsertEndChild( text );
	}

	for( CFeConfigSet::const_iterator j = conf.children().begin(); j != conf.children().end(); j++ )
	{
		FeUtil::TiXmlElement ele((*j).key().c_str());
		setConfigTo((*j), &ele);

		pElement->InsertEndChild( ele );
	}
}

CFeLayerSerialize::CFeLayerSerialize( CLayerSys* pLayerSys ) 
	: m_opLayerSys(pLayerSys)
{

}

CFeConfig CFeLayerSerialize::GetLayersConfig(const CLayersSet& layerSet)
{
	CFeConfig layersConfig(g_szLayerList);

	for(CLayersSet::const_iterator iter = layerSet.begin(); iter != layerSet.end(); iter++)
	{
		CFeLayerConfig configLayer = (*iter)->GetLayerConfig();
		layersConfig.addObj(g_szLayer, configLayer);
	}

	return layersConfig;
}

void FeLayers::CFeLayerSerialize::AddLayersFromConfig( const CFeConfig& layersConf )
{
	CFeConfigSet layerList = layersConf.children(g_szLayer);
	for(CFeConfigSet::iterator iter = layerList.begin(); iter != layerList.end(); iter++)
	{
		CFeLayerConfig layerConfig(*iter);
		m_opLayerSys->AddLayer(layerConfig);
	}
}

void CFeLayerSerialize::SaveTo( const CLayersSet& layerSet, const std::string& filePath )
{
	CFeConfig layersConf = GetLayersConfig(layerSet);
	osgEarth::XmlDocument doc(layersConf);

	std::ofstream out;
	out.open(filePath.c_str(), std::ios_base::trunc);
	doc.store(out);
	out.close();
}

void FeLayers::CFeLayerSerialize::SaveTo( const CLayerIDSet& layerIDSet, const std::string& filePath )
{
	CLayersSet layerSet, layersToSave;
	m_opLayerSys->GetLayers(layerSet);

	CLayerIDSet layersID;
	layersID.assign(layerIDSet.begin(), layerIDSet.end());

	for(CLayersSet::iterator iter = layerSet.begin(); iter != layerSet.end(); iter++)
	{
		CLayerIDSet::iterator iterID = std::find(layersID.begin(), layersID.end(), (*iter)->GetLayerUID());

		if( iterID != layersID.end() )
		{
			layersToSave.push_back(*iter);
			layersID.erase(iterID);
		}
	}

	SaveTo(layersToSave, filePath);
}

void FeLayers::CFeLayerSerialize::SaveTo( const std::string& filePath )
{
	CLayersSet layerSet;
	m_opLayerSys->GetLayers(layerSet);
	SaveTo(layerSet, filePath);
}

bool FeLayers::CFeLayerSerialize::LoadFrom( const std::string& filePath )
{
	osgEarth::ReadResult r = osgEarth::URI(filePath).readString();
	if ( r.failed() ) return false;

	std::stringstream in(r.getString());

	CFeConfig doc;
	doc.fromXML(in);

	CFeConfig layersConfig(g_szLayerList);
	if(!doc.getObjIfSet(layersConfig.key(), layersConfig)) return false;

	AddLayersFromConfig(layersConfig);

	return true;
}

void FeLayers::CFeLayerSerialize::SaveTo( FeUtil::TiXmlElement* pRootElement )
{
	CLayersSet layerSet;
	m_opLayerSys->GetLayers(layerSet);
	CFeConfig layersConfig = GetLayersConfig(layerSet);

	FeUtil::TiXmlElement layerSysElem(g_szLayerList);
	setConfigTo(layersConfig, &layerSysElem);

	pRootElement->InsertEndChild(layerSysElem);
}

bool FeLayers::CFeLayerSerialize::LoadFrom( FeUtil::TiXmlElement* pRootElement )
{
	FeUtil::TiXmlElement* pLayerSysElem = pRootElement->FirstChildElement(g_szLayerList);
	if(!pLayerSysElem) return false;

	CFeConfig layersConfig = getConfigFrom( pLayerSysElem );
	AddLayersFromConfig(layersConfig);

	return true;
}

