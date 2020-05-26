#include <osgEarthSymbology/LineSymbol>
#include <osgEarthSymbology/PolygonSymbol>

#include <FeUtils/RenderContext.h>

#include <FeLayerModel.h>

using namespace FeLayers;

/**
  * @class CModelStyleParser
  * @brief 内部类，用于解析矢量图层样式
  * @author g00034
*/
class CModelStyleParser
{
public:
	/**  
	  * @brief 解析为CSS格式的配置
	  * @return 图层样式配置
	*/
	static osgEarth::Config GetCssStyleConfig(CFeLayerModel* pModelLayer)
	{
		//配置styles配置项
		osgEarth::Config stylesConfig("styles");
		osgEarth::Config styleConfig("style");

		std::stringstream strStyle;
		{
			strStyle << "lo{" 
				<< "altitude-clamping: none;"
				<< "altitude-technique: map;";

			// 点
			if( pModelLayer->HasPointFeature() )
			{
				/**
				注意： 在内部解析时，根据“:”冒号区分关键字，所以如果包含绝对路径时（如： text-font:d:/font.ttf）则会解析错误
					如果给绝对路径增加引号（如： text-font:"d:/font.ttf"）则会将引号字符也保存，影响后期的使用
					如：导致字体获取错误
				*/
				strStyle << "text-content: [NAME]"
					<< ";text-size:" << pModelLayer->GetFontSize()
					<< ";text-fill:" << colorToString( pModelLayer->GetFontColor() )
					<< ";text-font:" << "\"" << pModelLayer->GetFontName() << "\""
					<< ";text-encoding:utf-8"

					<< ";icon:" << "\"" <<  pModelLayer->GetPointIconPath() << "\""
					<< ";icon-placement: vertex"
					<< ";icon-scale: 1.0";
					//<< ";icon-declutter: true";
			}
			// 线
			if( pModelLayer->HasLineFeature() )
			{
				strStyle << "stroke-width:" << pModelLayer->GetLineWidth()
					<< ";stroke-opacity:" << pModelLayer->GetLineColor().a()
					<< ";stroke:" << colorToString( pModelLayer->GetLineColor() )
					<< ";stroke-stipple:" << pModelLayer->GetLineStipple();
			}
			// 面
			if( pModelLayer->HasPolygonFeature() )
			{
				strStyle << "fill:" << colorToString( pModelLayer->GetFillColor() )
					<< ";fill-opacity:" << pModelLayer->GetFillColor().a();
			}

			strStyle << ";}";
		}

		styleConfig.value() = strStyle.str();
		styleConfig.add("type", "text/css");

		stylesConfig.add(styleConfig);
		return stylesConfig;
	}

	/**  
	  * @brief 直接解析为样式配置
	  * @return 图层样式配置
	*/
	static osgEarth::Config GetStyleConfig(CFeLayerModel* pModelLayer)
	{
		osgEarth::Config stylesConfig("styles");
		osg::ref_ptr<osgEarth::Symbology::StyleSheet> rpStyleSheet = new osgEarth::Symbology::StyleSheet;
		
		// 点
		if( pModelLayer->HasPointFeature() )
		{
			rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::TextSymbol>(
				)->content() = osgEarth::StringExpression("[NAME]");
			rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::TextSymbol>(
				)->size() = pModelLayer->GetFontSize();
			rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::TextSymbol>(
				)->fill()->color() = pModelLayer->GetFontColor();
			rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::TextSymbol>(
				)->halo()->color() = pModelLayer->GetFontHaloColor();
			rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::TextSymbol>(
				)->font() = pModelLayer->GetFontName();
			rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::TextSymbol>(
				)->encoding() = osgEarth::Symbology::TextSymbol::ENCODING_UTF8;

			if ( pModelLayer->GetPointIconVisible() )
			{
				rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::IconSymbol>(
					)->url()->setLiteral(pModelLayer->GetPointIconPath());
				rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::IconSymbol>(
					)->placement() = osgEarth::Symbology::IconSymbol::PLACEMENT_VERTEX;
				rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::IconSymbol>(
					)->scale()->setLiteral(1.0);
			}
		}

		// 线
		if( pModelLayer->HasLineFeature() )
		{
			rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::LineSymbol>(
				)->stroke()->width() = pModelLayer->GetLineWidth();
			rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::LineSymbol>(
				)->stroke()->color() = pModelLayer->GetLineColor();
			rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::LineSymbol>(
				)->stroke()->stipplePattern() = pModelLayer->GetLineStipple();
			rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::LineSymbol>(
				)->stroke()->widthUnits() = osgEarth::Symbology::Units::PIXELS;

			// 贴地技术
			rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::AltitudeSymbol>(
				)->clamping() = osgEarth::AltitudeSymbol::CLAMP_TO_TERRAIN;
			rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::AltitudeSymbol>(
				)->technique() = osgEarth::AltitudeSymbol::TECHNIQUE_GPU;
		}

		// 面
		if( pModelLayer->HasPolygonFeature() )
		{
			rpStyleSheet->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::PolygonSymbol>(
				)->fill()->color() = pModelLayer->GetFillColor();
		}

		

		stylesConfig.merge(rpStyleSheet->getConfig());
		return stylesConfig;
	}

private:
	static std::string colorToString(const osg::Vec4f& clr)
	{
		char szClr[256] = {0,};
		sprintf(szClr, "#%02X%02X%02X", (int)(clr.r()*255), (int)(clr.g()*255), (int)(clr.b()*255));
		return szClr;
	}
};


CFeLayerModel::CFeLayerModel( const CFeLayerModelConfig& cfg )
	: SuperClass(m_layerConfig)
{
	m_layerConfig = cfg;

	// 创建model配置项
	osgEarth::Config modelConfig = CreateModelConfig();
	osgEarth::ModelLayerOptions layerOpt( modelConfig );

	m_rpLayerEarth = new osgEarth::ModelLayer(layerOpt);
	m_rpLayerEarth->setVisibleRange(m_layerConfig.visibleConfig().minVisibleRange().get(), m_layerConfig.visibleConfig().maxVisibleRange().get());
}

CFeLayerModel::~CFeLayerModel()
{
	
}

osgEarth::Config CFeLayerModel::CreateModelConfig()
{
	// 构造features配置项
	osgEarth::Config featuresConfig;
	featuresConfig.key() = "features";
	featuresConfig.merge(m_layerConfig.sourceConfig().getConfig());

	// 构造layout配置项
	osgEarth::Config layoutConfig("layout");
	osg::ref_ptr<osgEarth::Features::FeatureDisplayLayout> rpLayout = new osgEarth::Features::FeatureDisplayLayout;
	rpLayout->minRange() = 0;//m_layerConfig.minVisibleRange();
	rpLayout->maxRange() = FLT_MAX;//m_layerConfig.maxVisibleRange();
	//rpLayout->tileSize() = 15;
	//osgEarth::Features::FeatureLevel fl(m_layerConfig.minVisibleRange().get(), m_layerConfig.maxVisibleRange().get());
	osgEarth::Features::FeatureLevel fl(0, FLT_MAX);
	rpLayout->addLevel(fl);
	layoutConfig.merge(rpLayout->getConfig());
	
	// 构造styles配置项
	osgEarth::Config stylesConfig = CModelStyleParser::GetStyleConfig(this);

	// 构造model配置项
	osgEarth::Config modelConfig("model");
	modelConfig.add("driver", "feature_geom");
	modelConfig.add(featuresConfig);
	modelConfig.add(layoutConfig);
	modelConfig.add(stylesConfig);

	CFeConfig cfg = m_layerConfig.getConfig();
	cfg.remove("min_range");
	cfg.remove("max_range");

	modelConfig.merge(cfg);
	

	return modelConfig;
}

void CFeLayerModel::PostAddLayer( osgEarth::MapNode* pMapNode )
{
	if( pMapNode )
	{
		// 获取矢量图层 FeatureModelGraph 节点
		osg::Node* pModelLayerNode = pMapNode->getModelLayerNode(GetEarthLayer());
		m_opFeatureModelGraph = osgEarth::findTopMostNodeOfType<osgEarth::Features::FeatureModelGraph>(pModelLayerNode);
	}
}

FeLayers::CFeLayerConfig CFeLayerModel::GetLayerConfig()
{
	return m_layerConfig;
}

osgEarth::Features::FeatureModelGraph* CFeLayerModel::GetLayerFeatureModel()
{
	return m_opFeatureModelGraph.get();
}

void CFeLayerModel::CheckAndValidFeatureType()
{
	if ( GetLayerFeatureModel()
		&& !m_layerConfig.featureSymbol().HasLineType() 
		&& !m_layerConfig.featureSymbol().HasPolygonType() 
		&& !m_layerConfig.featureSymbol().HasPointType() )
	{
		// 如果当前图层配置中不含任何矢量类型（通常为网络数据，如：WFS）
		// osgEarth 中会进行相关处理，将数据中包含的图元类型保存到默认样式中
		// 在此获取
		osgEarth::Symbology::Style* style = GetLayerFeatureModel()->getStyles()->getDefaultStyle();
		if( style->has<osgEarth::Symbology::LineSymbol>() )
		{
			m_layerConfig.featureSymbol().AddFeatureType(CFeFeatureSymbol::FT_LINE);

			// 使用默认线属性
			m_layerConfig.featureSymbol().lineColor() = style->getOrCreateSymbol<
				osgEarth::Symbology::LineSymbol>()->stroke()->color();
			m_layerConfig.featureSymbol().lineWidth() = style->getOrCreateSymbol<
				osgEarth::Symbology::LineSymbol>()->stroke()->width();
			m_layerConfig.featureSymbol().lineStipple() = style->getOrCreateSymbol<
				osgEarth::Symbology::LineSymbol>()->stroke()->stipple();
		}
		if( style->has<osgEarth::Symbology::PointSymbol>() )
		{
			m_layerConfig.featureSymbol().AddFeatureType(CFeFeatureSymbol::FT_POINT);

			// 设置文字属性
			style->getOrCreateSymbol<osgEarth::Symbology::TextSymbol>(
				)->content() = osgEarth::StringExpression("[NAME]");
			style->getOrCreateSymbol<osgEarth::Symbology::TextSymbol>(
				)->encoding() = osgEarth::Symbology::TextSymbol::ENCODING_UTF8;
		}
// 		if( style->has<osgEarth::Symbology::PolygonSymbol>() )
// 		{
// 			m_layerConfig.featureSymbol().AddFeatureType(CFeFeatureSymbol::FT_POLYGON);
// 
// 			// 使用默认面属性
// 			m_layerConfig.featureSymbol().fillColor() = style->getOrCreateSymbol<
// 				osgEarth::Symbology::PolygonSymbol>()->fill()->color();
// 		}
	}
}

bool CFeLayerModel::HasLineFeature()
{
	CheckAndValidFeatureType();

	return m_layerConfig.featureSymbol().HasLineType();
}

bool CFeLayerModel::HasPolygonFeature()
{
// 	CheckAndValidFeatureType();
// 
// 	return m_layerConfig.featureSymbol().IsPolygonType();

	// 三维矢量不支持面
	return false;
}

bool CFeLayerModel::HasPointFeature()
{
	CheckAndValidFeatureType();

	return m_layerConfig.featureSymbol().HasPointType();
}

void CFeLayerModel::SetLineColor( const osg::Vec4f& color )
{
	if( GetLayerFeatureModel() && HasLineFeature() )
	{
		m_layerConfig.featureSymbol().lineColor() = color;
		
		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<
			osgEarth::Symbology::LineSymbol>()->stroke()->color() = color;

		// 贴地技术
		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::AltitudeSymbol>(
			)->clamping() = osgEarth::AltitudeSymbol::CLAMP_TO_TERRAIN;
		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::AltitudeSymbol>(
			)->technique() = osgEarth::AltitudeSymbol::TECHNIQUE_GPU;

		GetLayerFeatureModel()->dirty();
	}
}

osg::Vec4f CFeLayerModel::GetLineColor() const
{
	return m_layerConfig.featureSymbol().lineColor().get();
}

void CFeLayerModel::SetFillColor( const osg::Vec4f& color )
{
	if( GetLayerFeatureModel() && HasPolygonFeature() )
	{
		m_layerConfig.featureSymbol().fillColor() = color;

		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<
			osgEarth::Symbology::PolygonSymbol>()->fill()->color() = color;

		// 贴地技术
		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::AltitudeSymbol>(
			)->clamping() = osgEarth::AltitudeSymbol::CLAMP_TO_TERRAIN;
		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::AltitudeSymbol>(
			)->technique() = osgEarth::AltitudeSymbol::TECHNIQUE_GPU;

		GetLayerFeatureModel()->dirty();
	}
}

osg::Vec4f CFeLayerModel::GetFillColor() const
{
	return m_layerConfig.featureSymbol().fillColor().get();
}

void CFeLayerModel::SetLineWidth( float width )
{
	if( GetLayerFeatureModel() && HasLineFeature() )
	{
		m_layerConfig.featureSymbol().lineWidth() = width;

		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<
			osgEarth::Symbology::LineSymbol>()->stroke()->width() = width;

		// 贴地技术
		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::AltitudeSymbol>(
			)->clamping() = osgEarth::AltitudeSymbol::CLAMP_TO_TERRAIN;
		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::AltitudeSymbol>(
			)->technique() = osgEarth::AltitudeSymbol::TECHNIQUE_GPU;

		GetLayerFeatureModel()->dirty();
	}
}

float CFeLayerModel::GetLineWidth() const
{
	return m_layerConfig.featureSymbol().lineWidth().get();
}

void CFeLayerModel::SetLineStipple( unsigned short stipple )
{
	if( GetLayerFeatureModel() && HasLineFeature() )
	{
		m_layerConfig.featureSymbol().lineStipple() = stipple;

		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<
			osgEarth::Symbology::LineSymbol>()->stroke()->stipplePattern() = stipple;

		// 贴地技术
		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::AltitudeSymbol>(
			)->clamping() = osgEarth::AltitudeSymbol::CLAMP_TO_TERRAIN;
		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<osgEarth::Symbology::AltitudeSymbol>(
			)->technique() = osgEarth::AltitudeSymbol::TECHNIQUE_GPU;

		GetLayerFeatureModel()->dirty();
	}
}

unsigned short CFeLayerModel::GetLineStipple() const
{
	return m_layerConfig.featureSymbol().lineStipple().get();
}

void CFeLayerModel::SetFontSize( float fHeight )
{
	if( GetLayerFeatureModel() && HasPointFeature() )
	{
		m_layerConfig.featureSymbol().fontSize() = fHeight;

		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<
			osgEarth::Symbology::TextSymbol>()->size() = fHeight;

		GetLayerFeatureModel()->dirty();
	}
}

float CFeLayerModel::GetFontSize()
{
	return m_layerConfig.featureSymbol().fontSize().get();
}

void CFeLayerModel::SetFontName( const std::string& name )
{
	if( GetLayerFeatureModel() && HasPointFeature() )
	{
		m_layerConfig.featureSymbol().fontName() = name;

		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<
			osgEarth::Symbology::TextSymbol>()->font() = name;

		GetLayerFeatureModel()->dirty();
	}
}

std::string CFeLayerModel::GetFontName()
{
	return m_layerConfig.featureSymbol().fontName().get();
}

void CFeLayerModel::SetFontColor( const osg::Vec4f& color )
{
	if( GetLayerFeatureModel() && HasPointFeature() )
	{
		m_layerConfig.featureSymbol().fontColor() = color;

		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<
			osgEarth::Symbology::TextSymbol>()->fill()->color() = color;

		GetLayerFeatureModel()->dirty();
	}
}

osg::Vec4f CFeLayerModel::GetFontColor()
{
	return m_layerConfig.featureSymbol().fontColor().get();
}

void CFeLayerModel::SetPointIconPath( const std::string& name )
{
	if( GetLayerFeatureModel() && HasPointFeature() )
	{
		m_layerConfig.featureSymbol().pointIconPath() = name;

		if( GetPointIconVisible() )
		{
			GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<
				osgEarth::Symbology::IconSymbol>()->url()->setLiteral(name);
			GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<
				osgEarth::Symbology::IconSymbol>()->placement() = osgEarth::Symbology::IconSymbol::PLACEMENT_VERTEX;
			GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<
				osgEarth::Symbology::IconSymbol>()->scale()->setLiteral(1.0);

			GetLayerFeatureModel()->dirty();
		}
	}
}

std::string CFeLayerModel::GetPointIconPath()
{
	return m_layerConfig.featureSymbol().pointIconPath().get();
}


void CFeLayerModel::SetPointIconVisible( bool bVisible )
{
	if( GetLayerFeatureModel() && HasPointFeature() )
	{
		m_layerConfig.featureSymbol().pointIconVisible() = bVisible;

		if(bVisible)
		{
			SetPointIconPath(GetPointIconPath());
		}
		else
		{
			GetLayerFeatureModel()->getStyles()->getDefaultStyle()->removeSymbol(
				GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getSymbol<osgEarth::Symbology::IconSymbol>());
		}

		GetLayerFeatureModel()->dirty();
	}
}

bool CFeLayerModel::GetPointIconVisible()
{
	return m_layerConfig.featureSymbol().pointIconVisible().get();
}

void CFeLayerModel::SetVisibleRange( float minVal, float maxVal )
{
	if( m_rpLayerEarth.valid() )
	{
		//SuperClass::SetVisibleRange(minVal, maxVal);
		m_layerConfig.visibleConfig().minVisibleRange() = minVal;
		m_layerConfig.visibleConfig().maxVisibleRange() = maxVal;

		m_rpLayerEarth->setVisibleRange(minVal, maxVal);
	}
}

void FeLayers::CFeLayerModel::setOpacity( float opacity )
{
	m_layerConfig.visibleConfig().opacity() = opacity;
	if(m_rpLayerEarth.valid()) m_rpLayerEarth->setOpacity(opacity);
}

float FeLayers::CFeLayerModel::getOpacity() const
{
	return m_layerConfig.visibleConfig().opacity().get();
}

void FeLayers::CFeLayerModel::GetVisibleRange( float& minVal, float& maxVal )
{
	minVal = m_layerConfig.visibleConfig().minVisibleRange().get();
	maxVal = m_layerConfig.visibleConfig().maxVisibleRange().get();
}

void FeLayers::CFeLayerModel::SetFontHaloColor( const osg::Vec4f& color )
{
	if( GetLayerFeatureModel() && HasPointFeature() )
	{
		m_layerConfig.featureSymbol().fontHaloColor() = color;

		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<
			osgEarth::Symbology::TextSymbol>()->halo()->color() = color;

		GetLayerFeatureModel()->dirty();
	}
}

osg::Vec4f FeLayers::CFeLayerModel::GetFontHaloColor()
{
	return m_layerConfig.featureSymbol().fontHaloColor().get();
}

void FeLayers::CFeLayerModel::SetAltitudeClamp( AltitudeClamp clamp )
{
	if( GetLayerFeatureModel() )
	{
		m_layerConfig.featureSymbol().Clamp() = clamp;

// 		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<
// 			osgEarth::Symbology::AltitudeSymbol>()->clamping() = clamp;
// 
// 		GetLayerFeatureModel()->dirty();
	}
}

FeLayers::AltitudeClamp FeLayers::CFeLayerModel::GetAltitudeClamp()
{
	return static_cast<AltitudeClamp>(m_layerConfig.featureSymbol().Clamp().get());
}

void FeLayers::CFeLayerModel::SetAltitudeTechnique( AltitudeTechnique technique )
{
	if( GetLayerFeatureModel() )
	{
		m_layerConfig.featureSymbol().Technique() = technique;

// 		GetLayerFeatureModel()->getStyles()->getDefaultStyle()->getOrCreateSymbol<
// 			osgEarth::Symbology::AltitudeSymbol>()->technique() = technique;
// 
// 		GetLayerFeatureModel()->dirty();
	}
}

FeLayers::AltitudeTechnique FeLayers::CFeLayerModel::GetAltitudeTechnique()
{
	return static_cast<AltitudeTechnique>(m_layerConfig.featureSymbol().Technique().get());
}

