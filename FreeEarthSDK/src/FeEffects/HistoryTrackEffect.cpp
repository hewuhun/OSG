
#include <osg/MatrixTransform>
#include <osg/Stencil>

#include <osg/LineWidth>
#include <osg/Multisample>

#include <FeUtils/CoordConverter.h>
#include <FeEffects/HistoryTrackEffect.h>

#include <iostream>

namespace FeEffect
{

	CHistoryTrackCallback::CHistoryTrackCallback( osg::Geometry *pGeom ,osg::DrawArrays *pDrawArrays,CHistoryTrackEffect *pTrack,osg::Vec3d vecFirstPos)
		:osg::NodeCallback()
		,m_rpGeom(pGeom)
		,m_vecColor(1.0,0.0,0.0,1.0)
		,m_vecPos(0.0,0.0,0.0)
		,m_dWide(10.0)
		,m_opDrawArrays(pDrawArrays)
		,m_opTrack(pTrack)
		,m_vecFirstPos(vecFirstPos)
	{

	}

	CHistoryTrackCallback::CHistoryTrackCallback(osg::Geometry *pGeom,osg::DrawArrays *pDrawArrays,CHistoryTrackEffect *pTrack,osg::Vec3d vecFirstPos,osg::Vec4d vecColor,osg::Vec3d vecPos,double dWide)
		:osg::NodeCallback()
		,m_rpGeom(pGeom)
		,m_vecColor(vecColor)
		,m_vecPos(vecPos)
		,m_dWide(dWide)
		,m_opDrawArrays(pDrawArrays)
		,m_opTrack(pTrack)
		,m_vecFirstPos(vecFirstPos)
	{

	}

	CHistoryTrackCallback::~CHistoryTrackCallback()
	{

	}

	void CHistoryTrackCallback::SetColor( osg::Vec4d vecColor )
	{
		m_vecColor = vecColor;
	}

	void CHistoryTrackCallback::SetCenter(osg::Vec3d vecCenter)
	{
		m_vecPos = vecCenter;
	}

	void CHistoryTrackCallback::SetRibbonWide(double dWide)
	{
		m_dWide = dWide;
	}

	void CHistoryTrackCallback::operator()( osg::Node *pNode,osg::NodeVisitor *pNv )
	{
		osg::MatrixTransform * pMt = dynamic_cast<osg::MatrixTransform *>(pNode);
		static int sNum = 0;
		sNum++;
		if(sNum ==10)
		{
			sNum = 0;
		}
		else
		{
			traverse(pNode,pNv);
		}
		if(pMt)
		{
			osg::Matrix matrix = pMt->getMatrix();
			osg::Vec3dArray* vertexs = dynamic_cast<osg::Vec3dArray*>(m_rpGeom->getVertexArray());
			int nSize = vertexs->size();
			osg::Vec3d newPos = m_vecPos*matrix;

			newPos = newPos - m_vecFirstPos;

			if(nSize > 1)
			{
				if((*vertexs)[nSize-1] != newPos)
				{
					(*vertexs)[nSize-1] = newPos;
				}
			}
			else
			{
				vertexs->push_back(newPos);
			}

			
			vertexs->dirty();
			
			if(m_opDrawArrays.valid())
			{
				m_opDrawArrays->set(GL_LINE_STRIP,0,vertexs->size());
			}
			m_rpGeom->dirtyDisplayList();
			m_rpGeom->dirtyBound();
			
		}
		osg::NodeCallback::traverse(pNode,pNv);
	}

}

namespace FeEffect
{
	CHistoryTrackEffect::CHistoryTrackEffect(osg::MatrixTransform *pMt)
		:CFreeEffect()
		,m_rpGeometry(NULL)
		,m_rpVertexArray(NULL)
		,m_rpColorArray(NULL)
		,m_fLineWidth(1.0)
		,m_pMt(pMt)
		,m_pTrackCallbck(NULL)
		,m_rpTransMt(NULL)
	{
		m_vecHistoryVertex.clear();
		m_rpTransMt = new osg::MatrixTransform;
		addChild(m_rpTransMt);
	}

	CHistoryTrackEffect::~CHistoryTrackEffect()
	{
		m_pTrackCallbck = NULL;
	}

	bool CHistoryTrackEffect::CreateEffect()
	{
		m_rpTransMt->removeChildren(0,getNumChildren());

		osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
		pGeode->setName("TrackLine");
		if(!m_rpGeometry.valid())
		{
			m_rpGeometry = new osg::Geometry;
		}
		
		if(!m_rpVertexArray.valid())
		{
			m_rpVertexArray = new osg::Vec3dArray;
		}
		m_rpVertexArray->clear();
		
		if(!m_rpColorArray.valid())
		{
			m_rpColorArray = new osg::Vec4dArray;
		}
		m_rpColorArray->clear();
		m_rpColorArray->push_back(osg::Vec4d(0.0,1.0,0.0,1.0));

		m_rpGeometry->setDataVariance(osg::Object::DYNAMIC);
		m_rpGeometry->setUseDisplayList(false);
		m_rpGeometry->setUseVertexBufferObjects(true);

		m_rpGeometry->setColorArray(m_rpColorArray.get());
		//m_rpGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
		m_rpGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

		m_rpGeometry->setVertexArray(m_rpVertexArray.get());
		
		m_rpGeometry->setNormalArray(m_rpVertexArray.get());
		m_rpGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

		if(!m_rpDrawArrays.valid())
		{
			m_rpDrawArrays = new osg::DrawArrays(GL_LINES,0,m_rpVertexArray->size());
		}
		
		m_rpGeometry->addPrimitiveSet(m_rpDrawArrays);
		osg::StateSet *pStateSet = m_rpGeometry->getOrCreateStateSet() ;
		pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
		pStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		osg::ref_ptr<osg::LineWidth> rpLineWidth = new osg::LineWidth(1.0);
		pStateSet->setAttributeAndModes(rpLineWidth,osg::StateAttribute::ON);

		//osg::Multisample* pms= new osg::Multisample;
		//pms->setSampleCoverage(16,true);
		//pStateSet->setAttributeAndModes(pms,osg::StateAttribute::ON);

		pGeode->addDrawable(m_rpGeometry.get());
		pGeode->getOrCreateStateSet()->setMode(GL_LINE_SMOOTH,osg::StateAttribute::ON);
		pGeode->getOrCreateStateSet()->setMode(GL_POINT_SMOOTH,osg::StateAttribute::ON);

		osg::ref_ptr<osg::Stencil> lineStripStencil = new osg::Stencil;  
		lineStripStencil->setFunction(osg::Stencil::ALWAYS, 0x0, 0x0);  
		lineStripStencil->setOperation(osg::Stencil::INCR, osg::Stencil::INCR, osg::Stencil::INCR);  
		pGeode->getOrCreateStateSet()->setAttributeAndModes(lineStripStencil,osg::StateAttribute::ON);

		m_rpTransMt->addChild(pGeode.get());

		return true;
	}

	void CHistoryTrackEffect::AddTrackCallback()
	{
		if(m_pTrackCallbck.valid())
		{
			return;
		}
		if(m_pMt)
		{
			if(!m_pTrackCallbck.valid())
			{
				m_pTrackCallbck = new CHistoryTrackCallback(m_rpGeometry.get(),m_rpDrawArrays.get(),this,m_vecXYZPos,m_vecColor,m_vecCenter);
			}
			
			m_pMt->addUpdateCallback(m_pTrackCallbck.get());
		}
	}

	void CHistoryTrackEffect::RemoveTrackCallback()
	{
		if(m_pMt && m_pTrackCallbck.valid())
		{
			m_pMt->removeUpdateCallback(m_pTrackCallbck.get());
			m_pTrackCallbck =NULL;
		}
	}

	void CHistoryTrackEffect::ClearLine()
	{
		if(m_rpTransMt.valid())
		{
			m_rpTransMt->removeChildren(0,getNumChildren());
		}
		
		if(m_rpColorArray.valid())
		{
			m_rpColorArray->clear();
		}
		if(m_rpVertexArray.valid())
		{
			m_rpVertexArray->clear();
		}

		m_vecHistoryVertex.clear();
	}

	bool CHistoryTrackEffect::IsDrawed() const
	{

		return false;
	}

	bool CHistoryTrackEffect::UpdataEffect()
	{
		if(m_bVisible)
		{
			AddTrackCallback();
		}
		else
		{
			RemoveTrackCallback();
		}
		return true;
	}

	void CHistoryTrackEffect::SetMatrixTransform( osg::MatrixTransform *pMT )
	{
		m_pMt = pMT;
	}

	void CHistoryTrackEffect::SetColor( osg::Vec4d vecColor )
	{
		CFreeEffect::SetColor(vecColor);
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

		if(m_pTrackCallbck.valid())
		{
			m_pTrackCallbck->SetColor(vecColor);
		}
	}

	void CHistoryTrackEffect::SetCenter( osg::Vec3d vecCenter )
	{
		m_vecCenter = vecCenter;
		if(m_pTrackCallbck.valid())
		{
			m_pTrackCallbck->SetCenter(m_vecCenter);
		}
	}

	void CHistoryTrackEffect::SetWide( double dWide )
	{
		CFreeEffect::SetRadius(dWide);
		if(m_pTrackCallbck.valid())
		{
			m_pTrackCallbck->SetRibbonWide(dWide);
		}
	} 

	void CHistoryTrackEffect::ContinueDrawLine(std::vector<osg::Vec3d> vecHistoryVertex)
	{
		osg::ref_ptr<osg::Geode> pNewLine = new osg::Geode;
		osg::ref_ptr<osg::Geometry> pNewGeom = new osg::Geometry;
		osg::ref_ptr<osg::Vec3dArray> pVertex = new osg::Vec3dArray;
		osg::ref_ptr<osg::Vec4dArray> pColorArray = new osg::Vec4dArray;
		osg::ref_ptr<osg::DrawArrays> pDrawArrays = new osg::DrawArrays;

		pNewGeom->setColorArray(pColorArray.get());
		pColorArray->push_back(m_vecColor);
		pNewGeom->setVertexArray(pVertex);
		pNewGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

		int nSize = vecHistoryVertex.size();
		for(int n =0;n<nSize;n++)
		{
			pVertex->push_back(vecHistoryVertex.at(n));
		}

		pDrawArrays->set(GL_LINE_STRIP,0,pVertex->size());
		pNewGeom->addPrimitiveSet(pDrawArrays);
		pNewLine->addDrawable(pNewGeom.get());

		osg::ref_ptr<osg::Stencil> lineStripStencil = new osg::Stencil;  
		lineStripStencil->setFunction(osg::Stencil::ALWAYS, 0x0, 0x0);  
		lineStripStencil->setOperation(osg::Stencil::INCR, osg::Stencil::INCR, osg::Stencil::INCR);  
		pNewGeom->getOrCreateStateSet()->setAttributeAndModes(lineStripStencil,osg::StateAttribute::ON);

		m_rpTransMt->addChild(pNewLine);

	}

	void  CHistoryTrackEffect::SetFirstPostion(osg::Vec3d vecPostion)
	{
		m_vecXYZPos = vecPostion;
		m_rpTransMt->setMatrix(osg::Matrix::translate(m_vecXYZPos));
	}

	bool CHistoryTrackEffect::ClearEffect()
	{
		RemoveTrackCallback();
		ClearLine();
		return false;
	}

	void CHistoryTrackEffect::AddKeyPos( osg::Vec3d newPos )
	{
		int nSize = m_vecHistoryVertex.size();

		newPos = newPos- m_vecXYZPos;

		osg::Vec3d oldPos = m_vecOldKey;
		m_vecOldKey = newPos;
		newPos = oldPos;

		m_rpVertexArray->push_back(newPos);

		if(nSize>4)
		{
			osg::Vec3d vecpos1 = m_vecHistoryVertex[nSize-1];
			osg::Vec3d vecpos2 = m_vecHistoryVertex[nSize-2];

			osg::Vec3d L = vecpos2-vecpos1;
			osg::Vec3d R = vecpos1-newPos;

			L.normalize();
			R.normalize();

			double dLength = (L^R).length();
			if(dLength > 0.0004)
			{
				m_vecHistoryVertex.push_back(newPos);
			}
			else
			{
				m_vecHistoryVertex[nSize-1] = newPos;
			}
		}
		else if(nSize <= 0)
		{
			newPos = osg::Vec3d(0.0,0.0,0.0);
			m_vecHistoryVertex.push_back(newPos);
		}
		else
		{
			m_vecHistoryVertex.push_back(newPos);
		}

		nSize = m_vecHistoryVertex.size();

		if(nSize > 100)
		{
			ContinueDrawLine(m_vecHistoryVertex);
			m_vecHistoryVertex.clear();
			m_vecHistoryVertex.push_back(newPos);
			m_rpVertexArray->clear();
			m_rpVertexArray->push_back(newPos);
		}

		
	}

}
