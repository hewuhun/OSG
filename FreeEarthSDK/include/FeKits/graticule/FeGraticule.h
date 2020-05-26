#ifndef FE_GRATICULE_H
#define FE_GRATICULE_H

#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthFeatures/Feature>
#include <osgEarthAnnotation/PlaceNode>
#include <osgEarthUtil/EarthManipulator>

#include <osgGA/GUIEventHandler>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>

#include <osg/LOD>

#include <osgViewer/View>

#include <FeKits/Export.h>
#include <FeKits/KitsCtrl.h>
#include <FeKits/manipulator/FreeEarthManipulator.h>

using namespace osgEarth;
using namespace osgEarth::Util;
using namespace osgEarth::Annotation;
using namespace osgEarth::Features;
using namespace osgEarth::Symbology;
using namespace osgGA;

//#ifndef __SOME_PARAM
//#define __SOME_PARAM
#define GRATICULE_RENDER_ORDER 998
#define FELOD_INVALUE_NO -1
#define RELATIVE_ALTITUDE 2000.0
#define LABEL_RELATIVE_ALTITUDE 2000.0
#define INTERPLATOR_VALUE 0.5
#define CULLLABEL_CONST(x) ((int)x+2)

//#endif

typedef std::vector<osg::ref_ptr<PlaceNode> >  PlaneNodeList;

namespace FeKit
{
	class FeGraticule;
	class PositionTransHandler;

	//每级网格几何体
	class FeGraticuleItem : public osg::Group
	{
		friend class FeGraticule;
		friend class PositionTransHandler;
	public:
		FeGraticuleItem(MapNode*, float, float, Color);
		~FeGraticuleItem();

	protected:
		FeatureList m_featureList;
		PlaneNodeList m_lonplacenodeList;
		PlaneNodeList m_latplacenodeList;
		float m_lonlatDelta;
		float m_lineWidth;
		Color m_lineColor;
		osg::observer_ptr<MapNode> m_mapnode;
	private:
		bool BuildGraticuleItem();

	};

	//LOD
	class FeLOD : public osg::LOD
	{
		friend class PositionTransHandler;
	public:
		FeLOD(osgViewer::View*, FeKit::FreeEarthManipulator*);
	protected:
		unsigned int getActiveChildNo() const { return m_activePlaceNodeSetNo; };
	private:
		void traverse  ( osg::NodeVisitor & nv  ) ;
		unsigned int m_activePlaceNodeSetNo;
		osg::observer_ptr<osgViewer::View> m_view;
		osg::observer_ptr<FeKit::FreeEarthManipulator> m_em;
	};

    typedef std::vector<osg::ref_ptr<FeGraticuleItem> > FeGraticuleItemList;

	//整体网格,外部导出
	class FEKIT_EXPORT FeGraticule : public osg::Group,  public CKitsCtrl
	{
		friend class PositionTransHandler;
	public:
		FeGraticule(MapNode*, osgViewer::View*, FeKit::FreeEarthManipulator*);
		~FeGraticule();

		virtual void Show();
		virtual void Hide();

	protected:
		osg::ref_ptr<FeLOD> m_lod;

		osg::observer_ptr<osgViewer::View> m_opView;
		osg::observer_ptr<FeKit::FreeEarthManipulator> m_opEm;
		osg::ref_ptr<PositionTransHandler> m_opPth;

	private:
		bool BuildGraticule(MapNode*);
		bool InitPosTransHandle(osgViewer::View*, FeKit::FreeEarthManipulator*);

		FeGraticuleItemList m_fegraticuleitemList;
		PlaneNodeList m_specialLonplacenodeList;
		PlaneNodeList m_specialLatplacenodeList;
	};

	class PositionTransHandler : public GUIEventHandler
	{
	public:
		PositionTransHandler(FeGraticule* ffg, FeKit::FreeEarthManipulator* em);
		~PositionTransHandler();
	protected:
		virtual bool handle(const GUIEventAdapter&,GUIActionAdapter&);

	private:
		osg::observer_ptr<osgViewer::View> m_view;
		osg::observer_ptr<FeKit::FreeEarthManipulator> m_em;
		osg::observer_ptr<FeGraticule> m_ffg;

		typedef struct EmStateSet
		{
			EmStateSet(float p, float r, float h)
			{
				m_heading = h; m_pitch = p; m_range = r;
			}
			float m_pitch;
			float m_range;
			float m_heading;
		}EmStateSet;
		EmStateSet m_emStateSet;
	};

	struct ActivateOpaqueOnType
	{
		ActivateOpaqueOnType(const std::type_info& t): _t(t) {}

		void operator()(osg::ref_ptr<PlaceNode>& nptr) const
		{
			const PlaceNode* ptr = nptr.get();
			if(typeid(*ptr)==_t)
			{
				osg::StateSet* ss = nptr->getOrCreateStateSet();
				if(ss) {
					ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
					ss->setRenderBinDetails(GRATICULE_RENDER_ORDER+1, "RenderBin");
				}
			}
		}

		const std::type_info&  _t;

	protected:

		ActivateOpaqueOnType& operator = (const ActivateOpaqueOnType&) { return *this; }
	};
}

#endif //FE_GRATICULE_H
