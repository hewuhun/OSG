#include <FeKits/graticule/FeGraticule.h>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthSymbology/TextSymbol>
#include <osgEarthSymbology/Geometry>
#include <osgDB/ConvertUTF>
#include <FeUtils/CoordConverter.h>

namespace FeKit
{
	FeGraticuleItem::FeGraticuleItem(MapNode* mapnode, float f, float w, Color c) :
osg::Group(),
	m_lonlatDelta(f),
	m_lineColor(c),
	m_lineWidth(w),
	m_mapnode(mapnode)
{
	osg::DisplaySettings::instance()->setNumMultiSamples(4);
	BuildGraticuleItem();
}

FeGraticuleItem::~FeGraticuleItem()
{

}

bool FeGraticuleItem::BuildGraticuleItem()
{
	Style linestyle;
	linestyle.getOrCreate<LineSymbol>()->stroke()->color() = m_lineColor;
	linestyle.getOrCreate<LineSymbol>()->stroke()->width() = m_lineWidth;
	linestyle.getOrCreate<AltitudeSymbol>()->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN;
	linestyle.getOrCreate<AltitudeSymbol>()->verticalOffset() = RELATIVE_ALTITUDE;
	linestyle.getOrCreate<AltitudeSymbol>()->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_GPU;

	Style labelStyle;
	labelStyle.getOrCreate<TextSymbol>()->alignment() = TextSymbol::ALIGN_CENTER_BOTTOM_BASE_LINE;
	labelStyle.getOrCreate<TextSymbol>()->fill()->color() = Color::White;
	labelStyle.getOrCreate<TextSymbol>()->halo()->color() = Color::Black;
	labelStyle.getOrCreate<TextSymbol>()->font() = "font/simhei.ttf";
	labelStyle.getOrCreate<TextSymbol>()->size() = 17.0;
	labelStyle.getOrCreate<TextSymbol>()->encoding() = osgEarth::Symbology::TextSymbol::ENCODING_UTF8;
	labelStyle.getOrCreate<TextSymbol>()->declutter() = true;
	labelStyle.getOrCreate<TextSymbol>()->occlusionCull() = true;

	const SpatialReference* geoSRS = m_mapnode->getMapSRS()->getGeographicSRS();

	bool drawln = true;

	float fdelta = INTERPLATOR_VALUE;

	osgEarth::Symbology::GeometryCollection gc;
	//创建纬线圈集合
	for (float i=-90.0; i<90.0; i+=m_lonlatDelta)
	{
		if (std::abs(i+90)<0.00001 || std::abs(i)<0.00001)
		{
			continue;
		}
		//画线
		osg::ref_ptr<Polygon> py = new Polygon();
		for (float j=0.0; j<360.0; j+=fdelta)
		{
			py->push_back(osg::Vec3d(j, i, RELATIVE_ALTITUDE));
		}
		osg::ref_ptr<Feature> feature = NULL;
		if (m_mapnode.valid())
		{
			feature = new Feature(py, geoSRS, osgEarth::Symbology::Style());
			feature->geoInterp() = GEOINTERP_RHUMB_LINE;
			m_featureList.push_back(feature);
		}
		//        gc.push_back(py);
		//画标注
		if (drawln)
		{
			std::stringstream ss;
			if (i<0)
			{
				ss<<"S"<<std::abs(i)<<"°";
			}
			else
			{
				ss<<"N"<<std::abs(i)<<"°";
			}
			std::string str = osgDB::convertStringFromCurrentCodePageToUTF8(ss.str());
			osg::ref_ptr<PlaceNode> ln = new PlaceNode(m_mapnode.get(), GeoPoint(geoSRS,0.0, i, RELATIVE_ALTITUDE, osgEarth::ALTMODE_RELATIVE), str, labelStyle);
			m_latplacenodeList.push_back(ln);
			addChild(ln);
		}
		drawln = !drawln;
	}

	drawln = false;
	//创建经线圈集合

	for (float i=0; i<=180.0; i+=m_lonlatDelta)
	{
		if (std::abs(i)<0.00001 || std::abs(i-180.0)<0.00001
			|| std::abs(i-23.5)<0.00001 || std::abs(i+23.5)<0.00001
			|| std::abs(i-66.5667)<0.00001 || std::abs(i+66.5667)<0.00001 )
		{
			continue;
		}
		osg::ref_ptr<LineString> py = new LineString();
		for (float j=-90.0; j<=90.0; j+=fdelta)
		{
			py->push_back(osg::Vec3d(i, j, RELATIVE_ALTITUDE));
		}
		for (float j=90.0; j>=-90.0; j-=fdelta)
		{
			py->push_back(osg::Vec3d(i+180, j, RELATIVE_ALTITUDE));
		}
		osg::ref_ptr<Feature> feature = NULL;
		if (m_mapnode.valid())
		{
			feature = new Feature(py, geoSRS, osgEarth::Symbology::Style());
			feature->geoInterp() = GEOINTERP_RHUMB_LINE;
			m_featureList.push_back(feature);
		}
		//        gc.push_back(py);
		//画标注
		if (drawln)
		{
			std::stringstream ss, oss;
			ss<<"E"<<std::abs(i)<<"°";
			oss<<"W"<<std::abs(180-i)<<"°";
			std::string str = osgDB::convertStringFromCurrentCodePageToUTF8(ss.str());
			std::string ostr = osgDB::convertStringFromCurrentCodePageToUTF8(oss.str());
			osg::ref_ptr<PlaceNode> ln = new PlaceNode(m_mapnode.get(), GeoPoint(geoSRS, i, 0.0, RELATIVE_ALTITUDE, osgEarth::ALTMODE_RELATIVE), str, labelStyle);
			osg::ref_ptr<PlaceNode> oln = new PlaceNode(m_mapnode.get(), GeoPoint(geoSRS, i+180, 0.0, RELATIVE_ALTITUDE, osgEarth::ALTMODE_RELATIVE), ostr, labelStyle);
			m_lonplacenodeList.push_back(ln);
			m_lonplacenodeList.push_back(oln);
			addChild(ln);
			addChild(oln);
		}
		drawln = !drawln;

	}
	//    osg::ref_ptr<osgEarth::Symbology::MultiGeometry> mg = new osgEarth::Symbology::MultiGeometry(gc);
	//    osg::ref_ptr<Feature> feature = new Feature(mg.get(), m_mapnode->getMapSRS(), linestyle);
	osg::ref_ptr<FeatureNode> featureNode = new FeatureNode(m_mapnode.get(), m_featureList, linestyle);
	featureNode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	featureNode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	featureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
	//   featureNode->getOrCreateStateSet()->setRenderBinDetails(GRATICULE_RENDER_ORDER, "DepthSortedBin");

	std::for_each(m_latplacenodeList.begin(), m_latplacenodeList.end(), ActivateOpaqueOnType(typeid(PlaceNode)));
	std::for_each(m_lonplacenodeList.begin(), m_lonplacenodeList.end(), ActivateOpaqueOnType(typeid(PlaceNode)));

	addChild(featureNode);

	return true;
}
//////////////////////////////////////////////////////////////////////////

FeLOD::FeLOD(osgViewer::View* v, FeKit::FreeEarthManipulator* em) : 
osg::LOD(),
	m_view(v),
	m_em(em),
	m_activePlaceNodeSetNo(FELOD_INVALUE_NO)
{

}

using namespace osg;
void FeLOD::traverse( NodeVisitor & nv )
{
	switch(nv.getTraversalMode())
	{
	case(NodeVisitor::TRAVERSE_ALL_CHILDREN):
		std::for_each(_children.begin(),_children.end(),NodeAcceptOp(nv));
		break;
	case(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
		{
			float required_range = 0;
			if (_rangeMode==DISTANCE_FROM_EYE_POINT)
			{
				double dis = 1.0;
				if (m_em.valid())
				{
					dis = m_em->getViewpoint().getRange();
				}
				osg::Vec3d eye, center, up;
				m_view->getCamera()->getViewMatrixAsLookAt(eye, center, up, dis);
				osg::Vec3d cameraPosLLH;
				FeUtil::XYZ2DegreeLLH(m_em->GetRenderContext(),eye, cameraPosLLH);
				osg::Vec3d cameraLooktoLLH;
				FeUtil::XYZ2DegreeLLH(m_em->GetRenderContext(),center, cameraLooktoLLH);

				double pitch = m_em->getViewpoint().getPitch();
				double lat;
				float pitchrate = cos(osg::DegreesToRadians(std::abs(pitch)));
				lat = cameraPosLLH.y()*(1-pitchrate)+cameraLooktoLLH.y()*pitchrate;

				float rate = 1.0;
				if (lat > 70.0 || lat < -70.0)
				{
					rate =  std::sin(osg::DegreesToRadians(std::abs(lat))) + std::abs(lat/70.0);
				}

				required_range = nv.getDistanceToViewPoint(getCenter(),true) * rate ;
			}
			else
			{
				osg::CullStack* cullStack = dynamic_cast<osg::CullStack*>(&nv);
				if (cullStack && cullStack->getLODScale())
				{
					required_range = cullStack->clampedPixelSize(getBound()) / cullStack->getLODScale();
				}
				else
				{
					for(unsigned int i=0;i<_rangeList.size();++i)
					{
						required_range = osg::maximum(required_range,_rangeList[i].first);
					}
				}
			}

			unsigned int numChildren = _children.size();
			if (_rangeList.size()<numChildren) numChildren=_rangeList.size();

			for(unsigned int i=0;i<numChildren;++i)
			{
				if (_rangeList[i].first<=required_range && required_range<_rangeList[i].second)
				{
					if (m_activePlaceNodeSetNo != i)
					{
						m_activePlaceNodeSetNo = i;
					}           
					_children[i]->setNodeMask(~0x00);
					_children[i]->accept(nv);
				}
				else
				{
					_children[i]->setNodeMask(0x00);
				}
			}
			break;
		}
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////

FeGraticule::FeGraticule(MapNode* mapnode, osgViewer::View* v, FeKit::FreeEarthManipulator* em) 
	: osg::Group()
	,FeKit::CKitsCtrl(NULL)
	,m_lod(NULL)
	,m_opEm(em)
	,m_opView(v)
	,m_opPth(NULL)
{
	m_lod = new FeLOD(v, em);
	m_lod->setCenter(osg::Vec3d(0,0,0));
	addChild(m_lod);
	BuildGraticule(mapnode);
	InitPosTransHandle(v, em);
	m_bShow ? Show() : Hide();
}

FeGraticule::~FeGraticule()
{

}

bool FeGraticule::BuildGraticule(MapNode* mapnode)
{
	if (mapnode == NULL)
	{
		return false;
	}
	//创建网格
	float delta[] = { 10.0, 5.0, 2.5, 1.0, 0.5, 0.25};
	float range[] = { FLT_MAX , 18000000, 12000000, 9000000, 6800000, 6500000, 0.0};
	for (int i=0; i<6; i++)
	{
		float d = delta[i];
		osg::ref_ptr<FeGraticuleItem> item = new FeGraticuleItem(mapnode, d, 0.5, Color(1.0,1.0,1.0,0.5));
		m_fegraticuleitemList.push_back(item);
		m_lod->addChild(item);
		m_lod->setRange(i, range[i+1], range[i]);
	}
	//创建特殊意义的网格线
	Style linestyle;
	linestyle.getOrCreate<LineSymbol>()->stroke()->color() = Color(1.0, 1.0, 0.0, 0.5);
	linestyle.getOrCreate<LineSymbol>()->stroke()->width() = 1.5;
	linestyle.getOrCreate<AltitudeSymbol>()->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN;
	linestyle.getOrCreate<AltitudeSymbol>()->verticalOffset() = RELATIVE_ALTITUDE;
	linestyle.getOrCreate<AltitudeSymbol>()->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_GPU;

	Style labelStyle;
	labelStyle.getOrCreate<TextSymbol>()->alignment() = TextSymbol::ALIGN_CENTER_BOTTOM_BASE_LINE;
	labelStyle.getOrCreate<TextSymbol>()->fill()->color() = Color::Yellow;
	labelStyle.getOrCreate<TextSymbol>()->halo()->color() = Color::Black;
	labelStyle.getOrCreate<TextSymbol>()->size() = 18.0;
	labelStyle.getOrCreate<TextSymbol>()->font() = "font/simhei.ttf";
	labelStyle.getOrCreate<TextSymbol>()->encoding() = osgEarth::Symbology::TextSymbol::ENCODING_UTF8;
	labelStyle.getOrCreate<TextSymbol>()->declutter() = true;
	labelStyle.getOrCreate<TextSymbol>()->occlusionCull() = true;
	///script SetCVar("cameradistanceMax",50)
	const SpatialReference* geoSRS = mapnode->getMapSRS()->getGeographicSRS();

	float fdelta = INTERPLATOR_VALUE;

	FeatureList templist;
	//1.南北极圈，南北回归线，赤道
	float latvalue[] = {66.5667, -66.5667, 23.5, -23.5, 0.0};
	std::string lat_name[] = {"北极圈","南极圈","北回归线","南回归线","赤道"};
	for (int i=0; i<5; i++)
	{
		//画线
		osg::ref_ptr<Polygon> py = new Polygon();
		for (float j=0.0; j<360.0; j+=fdelta)
		{
			py->push_back(osg::Vec3d(j, latvalue[i], RELATIVE_ALTITUDE));
		}
		osg::ref_ptr<Feature> feature = NULL;

		feature = new Feature(py, geoSRS, osgEarth::Symbology::Style());
		feature->geoInterp() = GEOINTERP_RHUMB_LINE;
		templist.push_back(feature);

		//画标注
		std::string str = osgDB::convertStringFromCurrentCodePageToUTF8(lat_name[i].c_str());
		osg::ref_ptr<PlaceNode> ln = new PlaceNode(mapnode, GeoPoint(geoSRS,0.0, latvalue[i], RELATIVE_ALTITUDE, osgEarth::ALTMODE_RELATIVE), str, labelStyle);
		m_specialLatplacenodeList.push_back(ln);
		addChild(ln);
	}
	//2.本初子午线、逆子午线
	float lonvalue[] = {0.0, 180.0};
	std::string lon_name[] = {"本初子午线", "逆子午线"};
	for (int i=0; i<2; i++)
	{
		osg::ref_ptr<LineString> py = new LineString();
		for (float j=-90.0; j<=90.0; j+=fdelta)
		{
			py->push_back(osg::Vec3d(lonvalue[i], j, RELATIVE_ALTITUDE));
		}
		for (float j=90.0; j>=-90.0; j-=fdelta)
		{
			py->push_back(osg::Vec3d(lonvalue[i]+180, j, RELATIVE_ALTITUDE));
		}
		osg::ref_ptr<Feature> feature = NULL;

		feature = new Feature(py, geoSRS, osgEarth::Symbology::Style());
		feature->geoInterp() = GEOINTERP_RHUMB_LINE;
		templist.push_back(feature);

		std::string str = osgDB::convertStringFromCurrentCodePageToUTF8(lon_name[i].c_str());
		osg::ref_ptr<PlaceNode> ln = new PlaceNode(mapnode, GeoPoint(geoSRS, lonvalue[i], 85.0, RELATIVE_ALTITUDE, osgEarth::ALTMODE_RELATIVE), str, labelStyle);
		m_specialLonplacenodeList.push_back(ln);
		addChild(ln);
	}

	osg::ref_ptr<FeatureNode> featureNode = new FeatureNode(mapnode, templist, linestyle);
	featureNode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	featureNode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	featureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
	//   featureNode->getOrCreateStateSet()->setRenderBinDetails(GRATICULE_RENDER_ORDER, "DepthSortedBin");

	std::for_each(m_specialLatplacenodeList.begin(), m_specialLatplacenodeList.end(), ActivateOpaqueOnType(typeid(PlaceNode)));
	std::for_each(m_specialLonplacenodeList.begin(), m_specialLonplacenodeList.end(), ActivateOpaqueOnType(typeid(PlaceNode)));

	addChild(featureNode);

	return true;
}

bool FeGraticule::InitPosTransHandle(osgViewer::View* v, FeKit::FreeEarthManipulator* em)
{
	if (v == NULL && em == NULL)
	{
		return false;
	}
	if(!m_opPth.valid())
	{
		m_opPth = new PositionTransHandler(this, em);
	}

	v->addEventHandler(m_opPth);

	return true;
}

void FeGraticule::Show()
{
	m_bShow = true;
	setNodeMask(0xffffffff);

	if(!m_opView.valid())
	{
		return;
	}

	if(m_opPth.valid())
	{
		m_opView->removeEventHandler(m_opPth);
		m_opPth = NULL;
	}

	m_opPth = new PositionTransHandler(this, m_opEm.get());
	m_opView->addEventHandler(m_opPth);
}

void FeGraticule::Hide()
{
	m_bShow = false;
	setNodeMask(0.00000000);

	if(!m_opView.valid())
	{
		return;
	}

	if(m_opPth.valid())
	{
		m_opView->removeEventHandler(m_opPth);
		m_opPth = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////

PositionTransHandler::PositionTransHandler(FeGraticule* ffg, FeKit::FreeEarthManipulator* em) :
osgGA::GUIEventHandler(),
	m_ffg(ffg),
	m_view(NULL),
	m_em(em),
	m_emStateSet(EmStateSet(-999,-999,-999))
{

}

PositionTransHandler::~PositionTransHandler()
{

}

bool PositionTransHandler::handle( const GUIEventAdapter& ea,GUIActionAdapter& aa)
{
	if (!m_view.valid())
	{
		m_view = dynamic_cast<osgViewer::View*>(aa.asView());
		return false;
	}
	if (!m_ffg.valid())
	{
		return false;
	}

	float heading = m_em->getViewpoint().getHeading();
	float pitch = m_em->getViewpoint().getPitch();
	float range = m_em->getViewpoint().getRange();

	if (std::abs(m_emStateSet.m_heading - heading)<0.00001 
		&& std::abs(m_emStateSet.m_pitch-pitch)<0.00001 
		&& std::abs(m_emStateSet.m_range-range)<0.00001)
	{
		return false;
	}

	m_emStateSet.m_heading = heading;
	m_emStateSet.m_pitch = pitch;
	m_emStateSet.m_range = range;

	double dis = 1.0;
	if (m_em.valid())
	{
		dis = m_em->getViewpoint().getRange();
	}
	osg::Vec3d eye, center, up;
	m_view->getCamera()->getViewMatrixAsLookAt(eye, center, up, dis);
	osg::Vec3d cameraPosLLH;
	FeUtil::XYZ2DegreeLLH(m_em->GetRenderContext(),eye, cameraPosLLH);
	osg::Vec3d cameraLooktoLLH;
	FeUtil::XYZ2DegreeLLH(m_em->GetRenderContext(),center, cameraLooktoLLH);

#if 1
	double lon, lat;
	float rate = cos(osg::DegreesToRadians(std::abs(pitch)));
	lon = cameraPosLLH.x()*(1-rate)+cameraLooktoLLH.x()*rate;
	lat = cameraPosLLH.y()*(1-rate)+cameraLooktoLLH.y()*rate;

	if ((lat < -89.0) || (lat > 89.0))
	{
		lat = (lat < -89.0) ? -89.0: 89.0;
	}
	unsigned int cullconst = 1;
	unsigned int cullnum = 0;
	if (lat < -85.0 || lat > 85.0)
	{
		cullconst = CULLLABEL_CONST(3*cullconst);
	}
	else if (lat < -75.0 || lat > 75.0)
	{
		cullconst = CULLLABEL_CONST(cullconst);
	}
	unsigned int acNo = m_ffg->m_lod->getActiveChildNo();
	if (acNo < 0 || acNo == FELOD_INVALUE_NO)
	{
		acNo = 0;
	}
	FeGraticuleItem* fgi = m_ffg->m_fegraticuleitemList.at(acNo);
	if (fgi == NULL)
	{
		return false;
	}

	int w = m_view->getCamera()->getViewport()->width();
	int h = m_view->getCamera()->getViewport()->height();
	int size = fgi->m_lonplacenodeList.size();
	for (int i=0; i<size; i++)
	{
		PlaceNode* pn = fgi->m_lonplacenodeList.at(i);
		if (pn)
		{
			pn->setNodeMask(0x00);
			if (cullnum%cullconst==0)
			{
				cullnum = 0;
				GeoPoint pos = pn->getPosition();
				pos.y() = lat;
				//计算下pn坐标是否在屏幕内
				osg::Vec3d worldPos;
				FeUtil::DegreeLLH2XYZ(m_em->GetRenderContext(),pos.vec3d(), worldPos);
				osg::Vec2d screenPos;
				FeUtil::ConvertLocalWorldCoordToScreen(m_em->GetRenderContext(),worldPos, screenPos);
				bool isIn = (screenPos.x() > 0 && screenPos.x() < w && screenPos.y() > 0 && screenPos.y() < h) ? true : false;
				double d3 = osgEarth::GeoMath::distance(cameraPosLLH, pos.vec3d(), pos.getSRS());
				if (isIn )
				{
					pn->setPosition(pos); 
					pn->setNodeMask(~0x00);
				}
			}
			cullnum++;
		}
	}
	size = fgi->m_latplacenodeList.size();
	for (int i=0; i<size; i++)
	{
		PlaceNode* pn = fgi->m_latplacenodeList.at(i);
		if (pn)
		{
			pn->setNodeMask(0x00);
			GeoPoint pos = pn->getPosition();
			pos.x() = lon;
			osg::Vec3d worldPos;
			FeUtil::DegreeLLH2XYZ(m_em->GetRenderContext(),pos.vec3d(), worldPos);
			osg::Vec2d screenPos;
			FeUtil::ConvertLocalWorldCoordToScreen(m_em->GetRenderContext(),worldPos, screenPos);
			bool isIn = (screenPos.x() > 0 && screenPos.x() < w && screenPos.y() > 0 && screenPos.y() < h) ? true : false;
			double d3 = osgEarth::GeoMath::distance(cameraPosLLH, pos.vec3d(), pos.getSRS());
			if (isIn)
			{
				pn->setPosition(pos); 
				pn->setNodeMask(~0x00);
			}   
		}
	}

	//处理特殊经纬线
	size = m_ffg->m_specialLonplacenodeList.size();
	for (int i=0; i<size; i++)
	{
		PlaceNode* pn = m_ffg->m_specialLonplacenodeList.at(i);
		if (pn)
		{
			GeoPoint pos = pn->getPosition();
			pos.y() = lat;
			pn->setPosition(pos);       
		}
	}

	size = m_ffg->m_specialLatplacenodeList.size();
	for (int i=0; i<size; i++)
	{
		PlaceNode* pn = m_ffg->m_specialLatplacenodeList.at(i);
		if (pn)
		{
			GeoPoint pos = pn->getPosition();
			pos.x() = lon;
			pn->setPosition(pos);       
		}
	}

#endif
	return false;
}
}


