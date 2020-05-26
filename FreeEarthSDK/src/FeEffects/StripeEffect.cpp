#include <FeEffects/StripeEffect.h>
#include <osg/Depth>
#include <osg/LineSegment>
#include "FeUtils/CoordConverter.h"

namespace FeEffect
{

	CStripeEffect::CStripeEffect(FeUtil::CRenderContext* pRenderContext)
		:CFreeEffect()
		,m_vecColor(1.0,0.0,0.0,0.15)
		,m_rpGeom(NULL)
		,m_rpGnode(NULL)
		,m_rpDrawElementsUShort(NULL)
		,m_opRenderContext(pRenderContext)
	{
		CreateEffect();
	}

	CStripeEffect::~CStripeEffect()
	{

	}

	bool CStripeEffect::CreateEffect()
	{
		this->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		this->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		this->getOrCreateStateSet()->setMode(GL_SMOOTH, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		osg::ref_ptr<osg::Depth> pDepth = new osg::Depth();
		pDepth->setWriteMask(false);
		this->getOrCreateStateSet()->setAttribute(pDepth, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		//面
		m_rpGnode = new osg::Geode;
		m_rpGeom = new osg::Geometry;

		m_rpGeom->setDataVariance(osg::Object::DYNAMIC);
		m_rpGeom->setUseDisplayList(false);
		m_rpGeom->setUseVertexBufferObjects(true);

		//面顶点序列
		osg::ref_ptr<osg::Vec3dArray> rpVertex = new osg::Vec3dArray;
		m_rpGeom->setVertexArray(rpVertex);

		//面颜色序列
		osg::ref_ptr<osg::Vec4dArray> rpColorArray = new osg::Vec4dArray;
		rpColorArray->push_back(m_vecColor);
		m_rpGeom->setColorArray(rpColorArray);
		m_rpGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

		m_rpDrawElementsUShort = new osg::DrawArrays;
		m_rpGeom->addPrimitiveSet(m_rpDrawElementsUShort);

		m_rpGnode->addDrawable(m_rpGeom);
		this->addChild(m_rpGnode);
		return true;
	}

	bool CStripeEffect::UpdateEffect(int nNum)
	{
		if (m_rpDrawElementsUShort.valid())
		{
			if (nNum*2>=4)
			{
				//m_rpDrawElementsUShort->clear();
				//for (int i = 0;i<nNum*2;++i)
				//{
					m_rpDrawElementsUShort->set(GL_QUAD_STRIP,0,nNum*2);
				//}
				m_rpGeom->dirtyDisplayList();
				m_rpGeom->dirtyBound();
			}
			return true;
		}
		return false;
	}

	bool CStripeEffect::SetVetex( osg::Vec3dArray* pVertex )
	{
		osg::Vec3dArray * pVertexArray = dynamic_cast<osg::Vec3dArray *>(m_rpGeom->getVertexArray());
		if (m_rpGeom.valid()&&pVertexArray)
		{
			pVertexArray->clear();
			for (int i=0;i<pVertex->size();++i)
			{
				osg::Vec3d vecLLH = pVertex->at(i);
				vecLLH.z()=0;
				FeUtil::DegreeLL2LLH(m_opRenderContext.get(),vecLLH);
				vecLLH.z() += 10000;
				osg::Vec3d vecXYZ;
				FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),vecLLH,vecXYZ);
				//pVertex->at(i) = vecXYZ;
				pVertexArray->push_back(vecXYZ);
			}
			//m_rpGeom->setVertexArray(pVertex);
			pVertexArray->dirty();
			return true;
		}
		return false;	
	}

}
