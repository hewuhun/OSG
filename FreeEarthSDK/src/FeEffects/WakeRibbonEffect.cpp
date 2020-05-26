#include <FeEffects/WakeRibbonEffect.h>

#include <osg/MatrixTransform>
#include <osg/LineWidth>
#include <osg/Multisample>

#include <FeUtils/CoordConverter.h>

#include <iostream>

namespace FeEffect
{
	CWakeRibbonEffect::CWakeRibbonEffect()
		:FeEffect::CFreeEffect()
		,m_rpGeometry(NULL)
		,m_rpDrawArrays(NULL)
		,m_rpVertexArray(NULL)
		,m_rpColorArray(NULL)
		,m_rpSelfMt(NULL)
		,m_opTransMT(NULL)
		,m_opRotateMt(NULL)
		,m_opScaleMt(NULL)
		,m_nSize(1000)
		,m_dWidth(1.0)
		,m_vecCenter(0, 0, 0)
		,m_vecColor(0.0, 1.0, 1.0, 1.0)
		,m_vecFirstPosition(0, 0, 0)
	{
		m_rpCallback = new CWakeRibbonCallback(this);

		m_rpSelfMt = new osg::MatrixTransform;
		addChild(m_rpSelfMt);
	}

	CWakeRibbonEffect::~CWakeRibbonEffect()
	{
		m_rpCallback = NULL;
	}

	bool CWakeRibbonEffect::CreateEffect()
	{
		if(m_rpSelfMt.valid())
		{
			m_rpSelfMt->removeChildren(0, getNumChildren());
		}

		osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
		if(false == m_rpGeometry.valid())
		{
			m_rpGeometry = new osg::Geometry;
		}
		if(false == m_rpVertexArray.valid())
		{
			m_rpVertexArray = new osg::Vec3dArray;
		}
		m_rpVertexArray->clear();

		if(false == m_rpColorArray.valid())
		{
			m_rpColorArray = new osg::Vec4dArray;
		}
		m_rpColorArray->clear();

		m_rpGeometry->setDataVariance(osg::Object::DYNAMIC);
		m_rpGeometry->setUseDisplayList(false);
		m_rpGeometry->setUseVertexBufferObjects(true);

		m_rpGeometry->setColorArray(m_rpColorArray.get());
		m_rpGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
		m_rpGeometry->setVertexArray(m_rpVertexArray.get());
		m_rpGeometry->setNormalArray(m_rpVertexArray.get());
		m_rpGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

		if(!m_rpDrawArrays.valid())
		{
			m_rpDrawArrays = new osg::DrawArrays();
		}

		m_rpDrawArrays->set(GL_QUAD_STRIP, 0, m_rpVertexArray->size());
		m_rpGeometry->addPrimitiveSet(m_rpDrawArrays);

		m_rpGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		m_rpGeometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		m_rpGeometry->getOrCreateStateSet()->setRenderBinDetails(11, "RenderBin");//osg::StateSet::TRANSPARENT_BIN);

		pGeode->addDrawable(m_rpGeometry.get());
		pGeode->getOrCreateStateSet()->setMode(GL_LINE_SMOOTH,osg::StateAttribute::ON);
		pGeode->getOrCreateStateSet()->setMode(GL_POINT_SMOOTH,osg::StateAttribute::ON);

		m_rpSelfMt->addChild(pGeode.get());

		return true;
	}

	void CWakeRibbonEffect::ClearLine()
	{
		if(m_rpColorArray.valid())
		{
			m_rpColorArray->clear();
		}
		if(m_rpVertexArray.valid())
		{
			m_rpVertexArray->clear();
		}
	}

	void CWakeRibbonEffect::SetMatrixTransform( osg::MatrixTransform *pTransMT)
	{
		m_opTransMT = pTransMT;
	}

	void CWakeRibbonEffect::SetColor( osg::Vec4d vecColor )
	{
		m_vecColor = vecColor;
		if(m_rpColorArray.valid())
		{
			osg::Vec4dArray::iterator itr = m_rpColorArray->begin();
			for(;itr!=m_rpColorArray->end();++itr)
			{
				itr->x() = m_vecColor.x();
				itr->y() = m_vecColor.y();
				itr->z() = m_vecColor.z();
			}
		}
	}

	void CWakeRibbonEffect::SetCenter( osg::Vec3d vecCenter )
	{
		m_vecCenter = vecCenter;
	}

	void CWakeRibbonEffect::SetWidth( double dWidth )
	{
		m_dWidth = dWidth;
	} 

	void CWakeRibbonEffect::SetVertexSize( int nNums )
	{
		m_nSize = nNums;
	}

	void CWakeRibbonEffect::SetVisible( bool bVisible )
	{
		if (bVisible)
		{
			if(m_opTransMT.valid() && m_rpCallback.valid() && m_rpSelfMt.valid())
			{
				m_vecFirstPosition = m_opTransMT->getMatrix().getTrans();
				m_rpSelfMt->setMatrix(osg::Matrix::translate(m_vecFirstPosition));
				m_rpSelfMt->addUpdateCallback(m_rpCallback.get());
			}
		}
		else
		{
			if(m_rpSelfMt && m_rpCallback.valid())
			{
				m_rpSelfMt->removeUpdateCallback(m_rpCallback.get());
				ClearLine();
			}
		}
	}
}



namespace FeEffect
{
	CWakeRibbonCallback::CWakeRibbonCallback(CWakeRibbonEffect* pEffect)
		:osg::NodeCallback()
		,m_opRibbonEffect(pEffect)
	{

	}

	CWakeRibbonCallback::~CWakeRibbonCallback()
	{

	}

	void CWakeRibbonCallback::operator()( osg::Node *pNode,osg::NodeVisitor *pNv )
	{
		if (false == m_opRibbonEffect.valid())
		{
			osg::NodeCallback::traverse(pNode, pNv);
			return ;
		}

		if (false == m_opRibbonEffect->m_opTransMT.valid())
		{
			osg::NodeCallback::traverse(pNode, pNv);
			return ;
		}

		osg::Matrix transMatrix = m_opRibbonEffect->m_opTransMT->getMatrix();
		osg::Matrix rotateMatrix, scaleMatrix;
		if (m_opRibbonEffect->m_opRotateMt.valid())
		{
			rotateMatrix = m_opRibbonEffect->m_opRotateMt->getMatrix();
		}
		if (m_opRibbonEffect->m_opScaleMt.valid())
		{
			scaleMatrix = m_opRibbonEffect->m_opScaleMt->getMatrix();
		}


		osg::Vec3dArray* vertexs = dynamic_cast<osg::Vec3dArray*>(m_opRibbonEffect->m_rpGeometry->getVertexArray());
		osg::Vec4dArray* colorArray = dynamic_cast<osg::Vec4dArray*>(m_opRibbonEffect->m_rpGeometry->getColorArray());
		int nSize = vertexs->size();

		osg::Vec3d vecLeft = m_opRibbonEffect->m_vecCenter;
		osg::Vec3d vecRight = m_opRibbonEffect->m_vecCenter;
		vecLeft.x() -= m_opRibbonEffect->m_dWidth;
		vecRight.x() += m_opRibbonEffect->m_dWidth;

		vecLeft = vecLeft*scaleMatrix*rotateMatrix*transMatrix- m_opRibbonEffect->m_vecFirstPosition;
		vecRight = vecRight*scaleMatrix*rotateMatrix*transMatrix- m_opRibbonEffect->m_vecFirstPosition;

		if(nSize >= m_opRibbonEffect->m_nSize )
		{
			for(unsigned int i =0; i<nSize -3; i+= 2)
			{
				(*vertexs)[i] = (*vertexs)[i+2];
				(*vertexs)[i+1] = (*vertexs)[i+3];
			}

			(*vertexs)[nSize-2] = vecLeft;
			(*vertexs)[nSize-1] = vecRight;
		}
		else
		{
			float alpha = sinf(osg::PI * (float)nSize/(float)m_opRibbonEffect->m_nSize);
			osg::Vec3d vecColor(m_opRibbonEffect->m_vecColor.x(), m_opRibbonEffect->m_vecColor.y(), m_opRibbonEffect->m_vecColor.z());
			if(alpha >= 0.6)
			{
				alpha = 0.6;
			}
			colorArray->push_back(osg::Vec4(vecColor, alpha));
			colorArray->push_back(osg::Vec4(vecColor, alpha));

			vertexs->push_back(vecLeft);
			vertexs->push_back(vecRight);
		}

		m_opRibbonEffect->m_rpGeometry->setVertexArray(vertexs);
		m_opRibbonEffect->m_rpGeometry->setColorArray(colorArray);
		vertexs->dirty();

		if(m_opRibbonEffect->m_rpDrawArrays.valid())
		{
			m_opRibbonEffect->m_rpDrawArrays->set(GL_QUAD_STRIP, 0, vertexs->size());
		}
		m_opRibbonEffect->m_rpGeometry->dirtyDisplayList();
		m_opRibbonEffect->m_rpGeometry->dirtyBound();

		osg::NodeCallback::traverse(pNode,pNv);
	}
}


