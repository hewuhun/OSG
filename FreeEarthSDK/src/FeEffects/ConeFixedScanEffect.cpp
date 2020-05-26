#include <FeEffects/ConeFixedScanEffect.h>
#include <FeUtils/CoordConverter.h>
#include <osg/Depth>
#include <osgUtil/IntersectVisitor>
#include <osg/LineSegment>

namespace FeEffect
{

	CConeFixedScanEffect::CConeFixedScanEffect( FeUtil::CRenderContext* pRenderContext,osg::MatrixTransform *pMT )
		:CFreeEffect()
		,m_opMT(pMT)
		,m_opRenderContext(pRenderContext)
		,m_vecColor(1.0,0.0,0.0,0.15)
		,m_vecColorLine(0.0,1.0,0.0,1.0)
		,m_rpGeom(NULL)
		,m_rpGnode(NULL)
		,m_rpDrawArraysSL(NULL)
		,m_rpDrawElementsUShort(NULL)
		,m_rpGeomLine(NULL)
		,m_rpGnodeLine(NULL)
		,m_dAngle(10.0)
		,m_dLenght(31855000)
		,m_dHeight(0)
	{
		CreateEffect();
		this->addUpdateCallback(new CConeEffectCallback());
	}

	CConeFixedScanEffect::~CConeFixedScanEffect()
	{

	}

	bool CConeFixedScanEffect::CreateEffect()
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

		m_rpDrawElementsUShort = new osg::DrawElementsUShort(GL_TRIANGLE_FAN);
		m_rpGeom->addPrimitiveSet(m_rpDrawElementsUShort);
		m_rpGeom->setUseDisplayList(false);
		m_rpGeom->setUseVertexBufferObjects(true);

		//面顶点序列
		osg::ref_ptr<osg::Vec3dArray> rpVertex = new osg::Vec3dArray;
		m_rpGeom->setVertexArray(rpVertex);

		//面颜色序列
		osg::ref_ptr<osg::Vec4dArray> rpColorArray = new osg::Vec4dArray;
		m_rpGeom->setColorArray(rpColorArray);
		m_rpGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

		rpColorArray->push_back(m_vecColor);

		//线
		m_rpGnodeLine = new osg::Geode;
		m_rpGeomLine = new osg::Geometry;
		m_rpDrawArraysSL = new osg::DrawArrays;
		m_rpGeomLine->addPrimitiveSet(m_rpDrawArraysSL);
		m_rpGeomLine->setUseDisplayList(false);
		m_rpGeomLine->setUseVertexBufferObjects(true);
		//线顶点序列
		osg::ref_ptr<osg::Vec3dArray> rpVertexLine = new osg::Vec3dArray;
		m_rpGeomLine->setVertexArray(rpVertexLine);
		//线颜色序列
		osg::ref_ptr<osg::Vec4dArray> rpColorArrayLine = new osg::Vec4dArray;
		m_rpGeomLine->setColorArray(rpColorArrayLine);
		m_rpGeomLine->setColorBinding(osg::Geometry::BIND_OVERALL);
		rpColorArrayLine->push_back(m_vecColorLine);

		UpdateEffect();

		m_rpGnode->addDrawable(m_rpGeom);
		this->addChild(m_rpGnode);

		m_rpGnodeLine->addDrawable(m_rpGeomLine);
		this->addChild(m_rpGnodeLine);

		return true;
	}

	bool CConeFixedScanEffect::UpdateEffect()
	{
		osg::Vec3dArray* pVertex = dynamic_cast<osg::Vec3dArray*>(m_rpGeom->getVertexArray());
		osg::Vec3dArray* pVertexLine = dynamic_cast<osg::Vec3dArray*>(m_rpGeomLine->getVertexArray());
		pVertex->clear();
		pVertexLine->clear();

		osg::Vec3d vecStartLLH,vecEndLLH,vecEndXYZ;
		osg::Vec3d vecXYZ = m_opMT->getMatrix().getTrans();

		FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(),vecXYZ,vecStartLLH);
		vecEndLLH = vecStartLLH;
		vecEndLLH.z() = 0;
		FeUtil::DegreeLL2LLH(m_opRenderContext.get(),vecEndLLH);

		m_dHeight =  vecStartLLH.z() - vecEndLLH.z();
		if (m_dLenght<m_dHeight)
		{
			m_dHeight = m_dLenght;
		}
		double dRadius = tan(osg::DegreesToRadians(m_dAngle)) * m_dHeight;
		osg::Vec3d vecCenter(0.0,0.0,-m_dHeight);

		for (double m=0; m<=360; m+=10)
		{
			double alp = osg::DegreesToRadians(m);
			osg::Vec3d vecPoint(vecCenter+osg::Vec3d(dRadius*sinf(alp), dRadius*cosf(alp), 0.0));
			if (m_dLenght>m_dHeight)
			{
				vecPoint = GetPointXYZByHit(vecPoint,vecXYZ);
			}
			pVertex->push_back(vecPoint);
		}
		int nSize = pVertex->size();
		osg::Vec3d vecExternalStart = pVertex->at(0);
		osg::Vec3d vecExternalEnd = pVertex->at(nSize-1);
		osg::Vec3d vec000(0.0,0.0,0.0);

		pVertex->push_back(vec000);

		pVertex->dirty();

		m_rpDrawElementsUShort->clear();
		m_rpDrawElementsUShort->addElement(nSize);
		for(int  n =0; n < nSize; n++ )
		{
			m_rpDrawElementsUShort->addElement(n);

			pVertexLine->push_back(pVertex->at(n));
		}
		m_rpDrawElementsUShort->addElement(0);	

		pVertexLine->dirty();


		nSize = pVertexLine->size();
		if(m_rpDrawArraysSL.valid())
		{
			m_rpDrawArraysSL->set(GL_LINE_STRIP, 0, nSize );
		}

		m_rpGeom->dirtyDisplayList();
		m_rpGeom->dirtyBound();

		m_rpGeomLine->dirtyDisplayList();
		m_rpGeomLine->dirtyBound();

		return true;
	}

	osg::Vec3d CConeFixedScanEffect::GetPointXYZByHit( osg::Vec3d vecXYZ,osg::Vec3d vecCenter )
	{
		osg::Matrix mtLocal;
		mtLocal.setTrans(vecXYZ);
		vecXYZ  = (mtLocal * m_opMT->getMatrix()).getTrans();

		osg::Vec3d vecLen = vecXYZ-vecCenter;
		vecLen.x() *= 10;
		vecLen.y() *= 10;
		vecLen.z() *= 10;
		osg::Vec3d vecNewXYZ = vecXYZ + vecLen;//得到新的位置
		osg::ref_ptr<osgUtil::IntersectVisitor> ivXY = new osgUtil::IntersectVisitor();
		osg::ref_ptr<osg::LineSegment> lineXY = new osg::LineSegment(vecNewXYZ, vecCenter);//根据新的位置得到线段检测
		ivXY->addLineSegment(lineXY.get()) ;
		m_opRenderContext->GetMapNode()->accept(*(ivXY.get()));//结构交集检测

		if(ivXY->hits())//如果碰撞
		{
			vecXYZ = ivXY->getHitList(lineXY).back().getWorldIntersectPoint();
		}
		else
		{
			vecXYZ = vecNewXYZ;
		}
		osg::Matrix mtWorld;
		mtWorld.setTrans(vecXYZ);
		vecXYZ = (mtWorld * m_opMT->getInverseMatrix()).getTrans();
		return vecXYZ;
	}

	void CConeFixedScanEffect::SetAngle( double dAngle )
	{
		m_dAngle = dAngle;
	}

	double CConeFixedScanEffect::GetAngle()
	{
		return m_dAngle;
	}

	void CConeFixedScanEffect::SetLenght( double dLenght )
	{
		m_dLenght = dLenght;
	}

	double CConeFixedScanEffect::GetLenght()
	{
		return m_dLenght;
	}

}

namespace FeEffect
{

	CConeEffectCallback::CConeEffectCallback()
	{

	}

	CConeEffectCallback::~CConeEffectCallback()
	{

	}

	void CConeEffectCallback::operator()( osg::Node *pNode,osg::NodeVisitor *pNv )
	{
		CConeFixedScanEffect * pSAREffect = dynamic_cast<CConeFixedScanEffect *>(pNode);
		if(pSAREffect)
		{
			pSAREffect->UpdateEffect();
		}

		osg::NodeCallback::traverse(pNode,pNv);
	}


}
