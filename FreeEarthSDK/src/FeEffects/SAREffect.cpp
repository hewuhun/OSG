#include <FeEffects/SAREffect.h>
#include <FeUtils/CoordConverter.h>
#include <osg/Depth>
#include <osgUtil/IntersectVisitor>
#include <osg/LineSegment>

namespace FeEffect
{

	CSAREffect::CSAREffect( FeUtil::CRenderContext* pRenderContext,osg::MatrixTransform *pMT )
		:CFreeEffect()
		,m_opMT(pMT)
		,m_opRenderContext(pRenderContext)
		,m_vecColor(1.0,0.0,0.0,0.5)
		,m_vecColorLine(0.0,1.0,0.0,1.0)
		,m_rpGeom(NULL)
		,m_rpGnode(NULL)
		,m_rpDrawArraysBL(NULL)
		,m_rpDrawElementsUShort(NULL)
		,m_rpGeomLine(NULL)
		,m_rpGnodeLine(NULL)
		,m_dMinAngle(30.0)
		,m_dMaxAngle(60.0)
		,m_dForwardAngle(40.0)
		,m_dAftAngle(40.0)
		,m_dLenght(31855000)
		,m_dHeight(0)
	{
		CreateEffect();
		this->addUpdateCallback(new CSAREffectCallback());
	}

	CSAREffect::~CSAREffect()
	{

	}

	bool CSAREffect::CreateEffect()
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

		//面
		m_rpForwardGeode = new osg::Geode;
		m_rpForwardGeom = new osg::Geometry;
		//m_rpForwardDraw = new osg::DrawElementsUShort(GL_TRIANGLE_FAN);
		m_rpForwardDraw = new osg::DrawElementsUShort(GL_LINE_STRIP);
		m_rpForwardGeom->addPrimitiveSet(m_rpForwardDraw);
		m_rpForwardGeom->setUseDisplayList(false);
		m_rpForwardGeom->setUseVertexBufferObjects(true);
		//面顶点序列
		osg::ref_ptr<osg::Vec3dArray> rpForwardVertex = new osg::Vec3dArray;
		m_rpForwardGeom->setVertexArray(rpForwardVertex);
		//面颜色序列
		osg::ref_ptr<osg::Vec4dArray> rpForwardColorArray = new osg::Vec4dArray;
		m_rpForwardGeom->setColorArray(rpForwardColorArray);
		m_rpForwardGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
		rpForwardColorArray->push_back(osg::Vec4d(0.0,0.0,1.0,0.5));
		
		//线
		m_rpGnodeLine = new osg::Geode;
		m_rpGeomLine = new osg::Geometry;
		m_rpDrawArraysBL = new osg::DrawArrays;
		m_rpDrawArraysSL = new osg::DrawArrays;
		m_rpGeomLine->addPrimitiveSet(m_rpDrawArraysBL);
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

		m_rpForwardGeode->addDrawable(m_rpForwardGeom);
		this->addChild(m_rpForwardGeode);

		m_rpGnodeLine->addDrawable(m_rpGeomLine);
		this->addChild(m_rpGnodeLine);

		return true;
	}

	bool CSAREffect::UpdateEffect()
	{
		osg::Vec3dArray* pVertex = dynamic_cast<osg::Vec3dArray*>(m_rpGeom->getVertexArray());
		osg::Vec3dArray* pVertexLine = dynamic_cast<osg::Vec3dArray*>(m_rpGeomLine->getVertexArray());
		osg::Vec3dArray* pForwardVertex = dynamic_cast<osg::Vec3dArray*>(m_rpForwardGeom->getVertexArray());
		pVertex->clear();
		pVertexLine->clear();
		pForwardVertex->clear();

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
		double dInnerRadius = tan(osg::DegreesToRadians(m_dMinAngle)) * m_dHeight;
		double dExternalRadius = tan(osg::DegreesToRadians(m_dMaxAngle)) * m_dHeight;
		double dForwardRadius = tan(osg::DegreesToRadians(m_dForwardAngle)) * m_dHeight;
		osg::Vec3d vecCenter(0.0,0.0,-m_dHeight),vecForwardCenter(0.0,-m_dHeight,0.0);

		for (double m=0; m<=360; m+=10)
		{
			double alp = osg::DegreesToRadians(m);
			osg::Vec3d vecPoint1(vecCenter+osg::Vec3d(dInnerRadius*sinf(alp), dInnerRadius*cosf(alp), 0.0));
			osg::Vec3d vecPoint2(vecCenter+osg::Vec3d(dExternalRadius*sinf(alp), dExternalRadius*cosf(alp), 0.0));
			if (m_dLenght>m_dHeight)
			{
				vecPoint1 = GetPointXYZByHit(vecPoint1,vecXYZ);
				vecPoint2 = GetPointXYZByHit(vecPoint2,vecXYZ);
			}
			pVertex->push_back(vecPoint1);
			pVertex->push_back(vecPoint2);

			osg::Vec3d vecForwardPoint(vecForwardCenter+osg::Vec3d(dForwardRadius*sinf(alp), 0.0, dForwardRadius*cosf(alp)));
			if (GetPointXYZifHit(vecForwardPoint,vecXYZ))
			{
				pForwardVertex->push_back(vecForwardPoint);
			}		
		}

		int nSize = pVertex->size();
		osg::Vec3d vecInnerStart = pVertex->at(0);
		osg::Vec3d vecExternalStart = pVertex->at(1);
		osg::Vec3d vecInnerEnd = pVertex->at(nSize-2);
		osg::Vec3d vecExternalEnd = pVertex->at(nSize-1);
		osg::Vec3d vec000(0.0,0.0,0.0);

		pVertex->push_back(vec000);

		pVertex->dirty();

		m_rpDrawElementsUShort->clear();
		m_rpDrawElementsUShort->addElement(nSize);
		for(int  n =0; n < nSize/2; n++ )
		{
			m_rpDrawElementsUShort->addElement(n*2);

			pVertexLine->push_back(pVertex->at(n*2));
		}
		for(int  n = nSize/2; n > 0; --n )
		{
			m_rpDrawElementsUShort->addElement(n*2-1);
			pVertexLine->push_back(pVertex->at(n*2-1));
		}
		//m_rpDrawElementsUShort->addElement(0);	

		int nForwardSize = pForwardVertex->size();
		//pForwardVertex->push_back(vec000);
		pForwardVertex->dirty();
		m_rpForwardDraw->clear();
		//m_rpForwardDraw->addElement(nForwardSize);
		for(int  n =0; n < nForwardSize; ++n )
		{
			m_rpForwardDraw->addElement(n);
		}
		//m_rpForwardDraw->addElement(0);	
		m_rpForwardGeom->dirtyDisplayList();
		m_rpForwardGeom->dirtyBound();

		//线7点
		/*pVertexLine->push_back(vecInnerStart);
		pVertexLine->push_back(vec000);
		pVertexLine->push_back(vecExternalStart);
		pVertexLine->push_back(vec000);
		pVertexLine->push_back(vecExternalEnd);
		pVertexLine->push_back(vec000);
		pVertexLine->push_back(vecInnerEnd);*/
		
		pVertexLine->dirty();

		//线
		
		if(m_rpDrawArraysBL.valid())
		{
			m_rpDrawArraysBL->set(GL_LINE_STRIP, 0, nSize/2 );
		}
		if(m_rpDrawArraysSL.valid())
		{
			m_rpDrawArraysSL->set(GL_LINE_STRIP, nSize/2, nSize/2 );
		}
		m_rpGeom->dirtyDisplayList();
		m_rpGeom->dirtyBound();

		m_rpGeomLine->dirtyDisplayList();
		m_rpGeomLine->dirtyBound();

		return true;
	}

	osg::Vec3d CSAREffect::GetPointXYZByHit( osg::Vec3d vecXYZ,osg::Vec3d vecCenter )
	{
		osg::Matrix mtLocal;
		mtLocal.setTrans(vecXYZ);
		vecXYZ  = (mtLocal * m_opMT->getMatrix()).getTrans();

		osg::Vec3d vecLen = vecXYZ-vecCenter;
		while (sqrt(pow(vecLen.x(),2)+pow(vecLen.y(),2)+pow(vecLen.z(),2))<m_dLenght)
		{
			vecLen.x() += vecLen.x();
			vecLen.y() += vecLen.y();
			vecLen.z() += vecLen.z();
		}	
		osg::Vec3d vecNewXYZ = vecXYZ + vecLen;//得到新的位置

		//osg::Vec3d vecLen = vecXYZ-vecCenter;
		//vecLen.x() *= 10;
		//vecLen.y() *= 10;
		//vecLen.z() *= 10;
		//osg::Vec3d vecNewXYZ = vecXYZ + vecLen;//得到新的位置
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

	bool CSAREffect::GetPointXYZifHit( osg::Vec3d &vecXYZ,osg::Vec3d vecCenter )
	{
		osg::Matrix mtLocal;
		mtLocal.setTrans(vecXYZ);
		vecXYZ  = (mtLocal * m_opMT->getMatrix()).getTrans();

		osg::Vec3d vecLen = vecXYZ-vecCenter;
		while (sqrt(pow(vecLen.x(),2)+pow(vecLen.y(),2)+pow(vecLen.z(),2))<m_dLenght)
		{
			vecLen.x() += vecLen.x();
			vecLen.y() += vecLen.y();
			vecLen.z() += vecLen.z();
		}	
		osg::Vec3d vecNewXYZ = vecXYZ + vecLen;//得到新的位置

		osg::ref_ptr<osgUtil::IntersectVisitor> ivXY = new osgUtil::IntersectVisitor();
		osg::ref_ptr<osg::LineSegment> lineXY = new osg::LineSegment(vecNewXYZ, vecCenter);//根据新的位置得到线段检测
		ivXY->addLineSegment(lineXY.get()) ;
		m_opRenderContext->GetMapNode()->accept(*(ivXY.get()));//结构交集检测

		if(ivXY->hits())//如果碰撞
		{
			vecXYZ = ivXY->getHitList(lineXY).back().getWorldIntersectPoint();
			osg::Matrix mtWorld;
			mtWorld.setTrans(vecXYZ);
			vecXYZ = (mtWorld * m_opMT->getInverseMatrix()).getTrans();
			return true;
		}
		return false;
	}
	void CSAREffect::SetMinAngle( double dAngle )
	{
		m_dMinAngle = dAngle;
	}

	double CSAREffect::GetMinAngle()
	{
		return m_dMinAngle;
	}

	void CSAREffect::SetMaxAngle( double dAngle )
	{
		m_dMaxAngle = dAngle;
	}

	double CSAREffect::GetMaxAngle()
	{
		return m_dMaxAngle;
	}

	void CSAREffect::SetForwardAngle( double dAngle )
	{
		m_dForwardAngle = dAngle;
	}

	double CSAREffect::GetForwardAngle()
	{
		return m_dForwardAngle;
	}

	void CSAREffect::SetAftAngle( double dAngle )
	{
		m_dAftAngle = dAngle;
	}

	double CSAREffect::GetAftAngle()
	{
		return m_dAftAngle;
	}

	void CSAREffect::SetLenght( double dLenght )
	{
		m_dLenght = dLenght;
	}

	double CSAREffect::GetLenght()
	{
		return m_dLenght;
	}

}

namespace FeEffect
{

	CSAREffectCallback::CSAREffectCallback()
	{

	}

	CSAREffectCallback::~CSAREffectCallback()
	{

	}

	void CSAREffectCallback::operator()( osg::Node *pNode,osg::NodeVisitor *pNv )
	{
		CSAREffect * pSAREffect = dynamic_cast<CSAREffect *>(pNode);
		if(pSAREffect)
		{
			pSAREffect->UpdateEffect();
		}

		osg::NodeCallback::traverse(pNode,pNv);
	}


}
