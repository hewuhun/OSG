#include <FeKits/IR/SulIR.h>
#include <FeUtils/DeviceInfoUtil.h>
#include "osgEarth/NodeUtils"
#include "osgEarth/JsonUtils"

namespace FeKit
{
	CSulIR::CSulIR( 
		FeUtil::CRenderContext* pRenderContext,
		osg::Node* pNode,
		double dWidthRadio,
		double dHeightRadio, 
		float dIR /*= 0.6*/,
		sigma::uint32 nSamplerTex /*= 32*/,
		FeKit::CSulTexCam::ESETUP setup /*= CSulTexCam::STANDARD*/ )
		:osg::Group(),
		FeKit::CKitsCtrl(NULL),
		m_opRC(pRenderContext),
		m_opViewer(dynamic_cast<osgViewer::Viewer*>(pRenderContext->GetView())),
		m_opEarthNode(NULL),
		m_fIR(dIR),
		m_nSamplerTex(nSamplerTex),
		m_eSetup(setup),
		m_dWidthRadio(dWidthRadio),
		m_dHeightRadio(dHeightRadio)
	{
		CreateIR(m_dWidthRadio, m_dHeightRadio);
	}

	CSulIR::~CSulIR()
	{

	}

	void CSulIR::CreateIR(int w, int h)
	{
		if(!m_rpIR.valid())
		{                            
			m_rpIR = new osg::Group;//新建一个group
		}

		m_rpRtt = osgEarth::findRelativeNodeOfType<CSulTexCam>(m_rpIR.get());
		if (m_rpRtt.valid())
		{
			m_rpRtt->removeChild(0, m_rpRtt->getNumChildren());
			m_opEarthNode = NULL;
			m_rpIR->removeChild(0, m_rpIR->getNumChildren());
			m_rpRtt = NULL;  
		}
		m_rpRtt = new FeKit::CSulTexCam(m_opViewer.get(), w ,h, m_eSetup);
		m_rpIR->addChild(m_rpRtt.get());

		osg::ref_ptr<CSulPostFilterNightVision> rpPostFilter = osgEarth::findRelativeNodeOfType<CSulPostFilterNightVision>(m_rpIR.get());
		if (m_rpIR.valid())
		{
			m_rpIR->removeChild(rpPostFilter);
			rpPostFilter =NULL;
		}
		rpPostFilter = new FeKit::CSulPostFilterNightVision(m_fIR);
		rpPostFilter->in( m_rpRtt->getTexture(), m_nSamplerTex, w ,h );
		m_rpIR->addChild(rpPostFilter.get());

		if (false == containsNode(m_rpIR))
		{
			addChild(m_rpIR.get());
		}
	}

	void CSulIR::Show()
	{
		m_bShow = true;
		if (m_rpRtt.valid() && m_opRC.valid())
		{
			m_rpRtt->addChild(m_opRC->GetMapNode());
		}
		setNodeMask(~0x00);
	}

	void CSulIR::Hide()
	{
		m_bShow = false;
		if (m_rpRtt.valid())
		{
			m_rpRtt->removeChild(0, m_rpRtt->getNumChildren());
		}
		setNodeMask(0x00);
	}

	void CSulIR::ResizeKit( int w, int h)
	{
		if (m_rpRtt.valid())
		{
			m_rpRtt->setViewport(0,0,w*m_dWidthRadio,h*m_dHeightRadio);
		}

		osg::ref_ptr<CSulPostFilterNightVision> rpPostFilter = osgEarth::findRelativeNodeOfType<CSulPostFilterNightVision>(m_rpIR.get());
		if (m_rpIR.valid())
		{
			m_rpIR->removeChild(rpPostFilter);
			rpPostFilter =NULL;
		}

		osg::ref_ptr<CSulPostFilterNightVision> newrpPostFilter = new FeKit::CSulPostFilterNightVision(m_fIR);
		newrpPostFilter->in( m_rpRtt->getTexture(), m_nSamplerTex, w ,h );
		m_rpIR->addChild(newrpPostFilter.get());

		CreateIR(w*m_dWidthRadio,h*m_dHeightRadio);

		if(m_bShow)
		{
			Show();
		}
	}

	void CSulIR::AddNode( osg::Node* obj )
	{
		// 		if (m_rpIR.valid() &&  m_rpIR->getChild(0)&&m_rpIR->getChild(0)->asGroup())
		// 		{
		//             CSulTexCam* p =  osgEarth::findRelativeNodeOfType<CSulTexCam>(m_rpIR);
		// 			if (p)
		// 			{
		//                 p->addChild(obj);
		// 			}
		// 		}
	}

}