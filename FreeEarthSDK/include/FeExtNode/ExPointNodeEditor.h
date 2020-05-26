/**************************************************************************************************
* @file ExPointNodeEditor.h
* @note 点图元编辑器
* @author g00034
* @data 2016-8-3
**************************************************************************************************/
#ifndef FE_EXTERN_POINT_EDITOR_H
#define FE_EXTERN_POINT_EDITOR_H


#include <osg/LineWidth>
#include <osgViewer/Viewer>

namespace FeExtNode
{
	template<class T>
	class FEEXTNODE_EXPORT CExPointNodeEditor : public osg::Group
	{
	public:
		CExPointNodeEditor(T* pNode);

		~CExPointNodeEditor();

	public:
		void traverse(osg::NodeVisitor& nv);

	protected:
		osg::Geode* CreateEditNode();

	protected:
		bool Handle(osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
		bool MouseRelease(osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
		bool LeftMouseDown(osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
		bool MouseDrag(osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
		bool isPickNode( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

	protected:
		osg::observer_ptr<T>		        m_opPointNode;

		bool								m_bPick;
	};

	template<typename T>
	CExPointNodeEditor<T>::CExPointNodeEditor(T* pNode)
		:osg::Group()
		, m_opPointNode(pNode)
		, m_bPick(false)
	{
		setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal() + 1);
		/*m_opPointNode->GetPlaceNode()->getAttachPoint()->*/addChild(CreateEditNode());
	}

	template<class T>
	CExPointNodeEditor<T>::~CExPointNodeEditor()
	{

	}

	template<class T>
	void CExPointNodeEditor<T>::traverse(osg::NodeVisitor& nv)
	{
		if (nv.getVisitorType() == osg::NodeVisitor::EVENT_VISITOR)
		{
			osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(&nv);
			if (ev)
			{
				for (osgGA::EventQueue::Events::iterator itr = ev->getEvents().begin();
					itr != ev->getEvents().end();
					++itr)
				{
					osgGA::GUIEventAdapter* ea = itr->get()->asGUIEventAdapter();
					if (Handle(*ea, *(ev->getActionAdapter())))
					{
						ea->setHandled(true);
					}
				}
			}
		}

		osg::Group::traverse(nv);
	}

	template<class T>
	bool CExPointNodeEditor<T>::Handle(osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		switch (ea.getEventType())
		{
		case osgGA::GUIEventAdapter::PUSH:
			{
				if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
				{
					return LeftMouseDown(ea, aa);
				}
			}
			break;

		case osgGA::GUIEventAdapter::RELEASE:

			return MouseRelease(ea, aa);
			break;

		case osgGA::GUIEventAdapter::DRAG:

			return MouseDrag(ea, aa);
			break;

		default:
			break;
		}

		return false;
	}

	template<class T>
	bool CExPointNodeEditor<T>::isPickNode( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		osgViewer::View *pViewer =dynamic_cast<osgViewer::View *>(&aa);
		osgUtil::LineSegmentIntersector::Intersections intersections;

		if (pViewer && pViewer->computeIntersections(ea,intersections))
		{
			for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
				hitr != intersections.end();
				++hitr)
			{
				osg::NodePath nodePath = hitr->nodePath;
				for (osg::NodePath::iterator itr = nodePath.begin(); itr != nodePath.end(); itr++)
				{
					CExternNode* pIExternNode = dynamic_cast<CExternNode*>(*itr);
					if (NULL != pIExternNode)
					{
						/// 标牌节点
						T* node = dynamic_cast<T*>(*itr);

						if(node == m_opPointNode.get())
						{
							return true;
						}
					}
				}
			}
		}

		return false;
	}

	template<class T>
	bool CExPointNodeEditor<T>::LeftMouseDown(osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		osgViewer::Viewer* pViewer = dynamic_cast<osgViewer::Viewer*>(aa.asView());
		if (pViewer)
		{
			if(isPickNode(ea, aa))
			{
				m_bPick = true;

				return true;
			}
		}

		return false;
	}

	template<class T>
	bool CExPointNodeEditor<T>::MouseRelease(osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		m_bPick = false;

		return false;
	}

	template<class T>
	bool CExPointNodeEditor<T>::MouseDrag(osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		if (m_opPointNode.valid() && m_bPick)
		{
			osgEarth::MapNode* pMapNode = m_opPointNode->GetMapNode();
			if (pMapNode)
			{
				osg::Vec3d world;
				pMapNode->getTerrain()->getWorldCoordsUnderMouse(aa.asView(), ea.getX(), ea.getY(), world);

				double lat, lon, hei;
				pMapNode->getMapSRS()->getEllipsoid()->convertXYZToLatLongHeight(world.x(), world.y(), world.z(), lat, lon, hei);
				lon = osg::RadiansToDegrees(lon);
				lat = osg::RadiansToDegrees(lat);

				osgEarth::GeoPoint point = m_opPointNode->GetPosition();
				point.x() = lon;
				point.y() = lat;
				point.z() = hei;

				m_opPointNode->SetPosition(point);
			}

			return true;
		}

		return false;
	}

	template<class T>
	osg::Geode* CExPointNodeEditor<T>::CreateEditNode()
	{
		osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;

		osg::ref_ptr<osg::Vec3Array> vecBorder = new osg::Vec3Array;
		osg::ref_ptr<osg::Vec3Array> vecSubBorder = new osg::Vec3Array;

		// 左边
		vecBorder->push_back(osg::Vec3(-20, +40, 0));
		vecBorder->push_back(osg::Vec3(-20, 0, 0));

		// 下边
		vecBorder->push_back(osg::Vec3(-20, 0, 0));
		vecBorder->push_back(osg::Vec3(+20, 0, 0));

		// 右边
		vecBorder->push_back(osg::Vec3(+20, 0, 0));
		vecBorder->push_back(osg::Vec3(+20, +40, 0));

		// 上边
		vecBorder->push_back(osg::Vec3(+20, +40, 0));
		vecBorder->push_back(osg::Vec3(-20, +40, 0));

		// 左横线
		vecSubBorder->push_back(osg::Vec3(-25, 20, 0));
		vecSubBorder->push_back(osg::Vec3(-15, 20, 0));

		// 下横线
		vecSubBorder->push_back(osg::Vec3(0, 5, 0));
		vecSubBorder->push_back(osg::Vec3(0, -5, 0));

		// 右横线
		vecSubBorder->push_back(osg::Vec3(15, 20, 0));
		vecSubBorder->push_back(osg::Vec3(25, 20, 0));

		// 上横线
		vecSubBorder->push_back(osg::Vec3(0, +45, 0));
		vecSubBorder->push_back(osg::Vec3(0, +35, 0));

		osg::ref_ptr<osg::Geometry> pGeoBorder = new osg::Geometry;
		osg::ref_ptr<osg::Geometry> pGeoSubBorder = new osg::Geometry;
		pGeoBorder->setVertexArray(vecBorder.get());
		pGeoSubBorder->setVertexArray(vecSubBorder.get());

		osg::ref_ptr<osg::DrawArrays> rpBorder = new osg::DrawArrays(GL_LINES, 0, vecBorder->size());
		osg::ref_ptr<osg::DrawArrays> rpSubBorder = new osg::DrawArrays(GL_LINES, 0, vecSubBorder->size());

		pGeoBorder->addPrimitiveSet(rpBorder);
		pGeoSubBorder->addPrimitiveSet(rpSubBorder);

		pGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pGeode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		pGeode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

		osg::ref_ptr<osg::LineWidth> rpBorderLineWidth = new osg::LineWidth(4.0);
		osg::ref_ptr<osg::LineWidth> rpSubBorderLineWidth = new osg::LineWidth(2.0);

		pGeoBorder->getOrCreateStateSet()->setAttributeAndModes(rpBorderLineWidth, osg::StateAttribute::ON);
		pGeoSubBorder->getOrCreateStateSet()->setAttributeAndModes(rpSubBorderLineWidth, osg::StateAttribute::ON);

		osg::ref_ptr<osg::Vec4dArray> vecColor = new osg::Vec4dArray;


		vecColor->push_back(osg::Vec4d(1.0, 1.0, 0.0, 0.8));
		pGeoBorder->setColorArray(vecColor);
		pGeoSubBorder->setColorArray(vecColor);

		pGeoBorder->setColorBinding(osg::Geometry::BIND_OVERALL);
		pGeoSubBorder->setColorBinding(osg::Geometry::BIND_OVERALL);

		pGeode->addDrawable(pGeoBorder);
		pGeode->addDrawable(pGeoSubBorder);

		return pGeode.release();
	}

}

#endif //FE_EXTERN_POINT_EDITOR_H
