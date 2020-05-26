#include <FeUtils/RenderContext.h>
#include <FeUtils/StrUtil.h>

namespace FeUtil
{
	CRenderContext::CRenderContext(  )
		:osg::Referenced()
		,m_opRoot(NULL)
		,m_opMapNode(NULL)
		,m_opCompositeViewer(NULL)  
		,m_opTriton(NULL)
		,m_camAltFromTerrain(0)
		,m_opEarthRotationMT(NULL)
	{

	}

	CRenderContext::CRenderContext( osgViewer::CompositeViewer* pViewer, osg::Group* pRoot, osgEarth::MapNode* pMapNode )
		:osg::Referenced()
		,m_opRoot(pRoot)
		,m_opMapNode(pMapNode)
		,m_opCompositeViewer(pViewer)
		,m_opTriton(NULL)
		,m_camAltFromTerrain(0)
	{

	}

	CRenderContext::~CRenderContext()
	{

	}

	osgEarth::MapNode* CRenderContext::GetMapNode()
	{
		return m_opMapNode.get();
	}

	osg::Group* CRenderContext::GetRoot()
	{
		return m_opRoot.get();
	}

	osgViewer::View* CRenderContext::GetView( int nIndex /*= 0*/ )
	{
		if (m_opCompositeViewer.valid())
		{
			return m_opCompositeViewer->getView(nIndex);
		}
	}

	osgViewer::CompositeViewer* CRenderContext::GetViewer()
	{
		return m_opCompositeViewer.get();
	}

	void CRenderContext::SetViewer( osgViewer::CompositeViewer* pViewer )
	{
		m_opCompositeViewer = pViewer;
	}

	void CRenderContext::SetRoot( osg::Group* pRoot )
	{
		m_opRoot = pRoot;
	}

	void CRenderContext::SetMapNode( osgEarth::MapNode* pMapNode )
	{
		m_opMapNode = pMapNode;
	}

	osg::GraphicsContext* CRenderContext::GetGraphicsWindow(int nIndex /*= 0*/)
	{
		osgViewer::View* pView = GetView(nIndex);
		if (pView && pView->getCamera())
		{
			return pView->getCamera()->getGraphicsContext();
		}

		return NULL;
	}

	osg::Camera* CRenderContext::GetCamera(int nIndex /*= 0*/)
	{
		osgViewer::View* pView = GetView(nIndex);
		if (pView && pView->getCamera())
		{
			return pView->getCamera();
		}

		return NULL;
	}

	bool CRenderContext::AddEventHandler(osgGA::GUIEventHandler* pHandler, int nIndex /*= 0*/)
	{
		osgViewer::View* pView = GetView(nIndex);
		if (pView)
		{
			if (std::find(pView->getEventHandlers().begin(),pView->getEventHandlers().end(), pHandler) == pView->getEventHandlers().end())
			{
				pView->addEventHandler(pHandler);
				return true;
			}
		}

		return false;
	}

	bool CRenderContext::RemoveEventHandler(osgGA::GUIEventHandler* pHandler, int nIndex /*= 0*/)
	{
		osgViewer::View* pView = GetView(nIndex);
		if (pView)
		{
			if (std::find(pView->getEventHandlers().begin(),pView->getEventHandlers().end(), pHandler) != pView->getEventHandlers().end())
			{
				pView->removeEventHandler(pHandler);
				return true;
			}
		}

		return false;
	}

	const osgEarth::SpatialReference* CRenderContext::GetMapSRS()
	{
		if(m_opMapNode.valid())
		{
			return m_opMapNode->getMapSRS();
		}

		return NULL;
	}

	osg::Node* CRenderContext::GetTritonNode()
	{
		return m_opTriton.get();
	}

	void CRenderContext::SetTritonNode( osg::Node* pNode )
	{
		m_opTriton = pNode;
	}

	void CRenderContext::SetEarthRotationMT( osg::MatrixTransform* pErMT )
	{
		m_opEarthRotationMT = pErMT;
	}

	osg::MatrixTransform* CRenderContext::GetEarthRotationMT()
	{
		return m_opEarthRotationMT.get();
	}

}