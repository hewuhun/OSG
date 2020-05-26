#include <FeEffects/FixedScanEffect.h>
#include <FeUtils/CoordConverter.h>
#include <osgUtil/IntersectVisitor>
#include <osg/ShapeDrawable>

namespace FeEffect
{
	CNewFixedScanEffect::CNewFixedScanEffect( FeUtil::CRenderContext* pRenderContext,osg::MatrixTransform *pMT )
		:CFreeEffect()
		,m_dLRAngle(5)
		,m_dFBAngle(5)
		,m_opGeode(NULL)
		,m_opOutLine(NULL)
		,m_opLeftFace(NULL)
		,m_opRightFace(NULL)
		,m_opFrontFace(NULL)
		,m_opBackFace(NULL)
		,m_vecFaceColor(1.0,0.0,0.0,0.2)
		,m_vecLineColor(1.0,0.0,0.0,0.6)
		,m_opMT(pMT)
		,m_opRenderContext(pRenderContext)
		,m_nNum(100)
		,m_dLenght(31855000)
		,m_dHeight(0)
		,m_dRectWidth(0)
		,m_dRectHeight(0)
		,m_vecLastXYZ(0,0,0)
	{
		osg::TessellationHints* hints = new osg::TessellationHints;
		hints->setDetailRatio(5.0f);
		osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0,0.0,0.0), osg::WGS_84_RADIUS_POLAR), hints);
		osg::Geode* geode = new osg::Geode;
		geode->addDrawable(sd);
		//std::string filename = osgDB::findDataFile("Images/land_shallow_topo_2048.jpg");
		//geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, new osg::Texture2D(osgDB::readImageFile(filename)));
		m_rpEarth = new osg::CoordinateSystemNode;
		m_rpEarth->setEllipsoidModel(new osg::EllipsoidModel());
		m_rpEarth->addChild(geode);
		CreateEffect();
	}

	bool CNewFixedScanEffect::CreateEffect()
	{
		m_rpMT = new osg::MatrixTransform;
		addChild(m_rpMT);
		UpdateEffect();	
		return true;
	}

	bool CNewFixedScanEffect::UpdateEffect()
	{
		createNode();
		m_rpUpdateCallback = new CUpdateScanCallback(this);
		m_rpMT->addCullCallback(m_rpUpdateCallback);
		return true;
	}

	void CNewFixedScanEffect::SetRotate( osg::Vec3d posture )
	{
		m_rpMT->setMatrix(osg::Matrix::rotate(osg::inDegrees(posture.x()),osg::X_AXIS, osg::inDegrees(posture.y()), osg::Y_AXIS,osg::inDegrees(posture.z()), osg::Z_AXIS));
	}

	void CNewFixedScanEffect::SetRotate( osg::Matrix mat )
	{
		m_rpMT->setMatrix(mat);
	}

	void CNewFixedScanEffect::createNode()
	{
		int binNum = 11;

		m_opLeftFace = new osg::Geometry();
		m_opLeftFace->setDataVariance(osg::Object::DYNAMIC);
		m_opLeftFace->setUseDisplayList(false);
		m_opLeftFace->setUseVertexBufferObjects(true);
		m_opLeftFace->getOrCreateStateSet()->setRenderBinDetails(binNum,"DepthSortedBin");

		m_opRightFace = new osg::Geometry();
		m_opRightFace->setDataVariance(osg::Object::DYNAMIC);
		m_opRightFace->setUseDisplayList(false);
		m_opRightFace->setUseVertexBufferObjects(true);
		m_opRightFace->getOrCreateStateSet()->setRenderBinDetails(binNum,"DepthSortedBin");

		m_opFrontFace = new osg::Geometry();
		m_opFrontFace->setDataVariance(osg::Object::DYNAMIC);
		m_opFrontFace->setUseDisplayList(false);
		m_opFrontFace->setUseVertexBufferObjects(true);
		m_opFrontFace->getOrCreateStateSet()->setRenderBinDetails(binNum,"DepthSortedBin");

		m_opBackFace = new osg::Geometry();
		m_opBackFace->setDataVariance(osg::Object::DYNAMIC);
		m_opBackFace->setUseDisplayList(false);
		m_opBackFace->setUseVertexBufferObjects(true);
		m_opBackFace->getOrCreateStateSet()->setRenderBinDetails(binNum,"DepthSortedBin");

		m_opFrontFace = new osg::Geometry();
		m_opFrontFace->setDataVariance(osg::Object::DYNAMIC);
		m_opFrontFace->setUseDisplayList(false);
		m_opFrontFace->setUseVertexBufferObjects(true);
		m_opFrontFace->getOrCreateStateSet()->setRenderBinDetails(binNum,"DepthSortedBin");

		m_opOutLine = new osg::Geometry();
		m_opOutLine->setDataVariance(osg::Object::DYNAMIC);
		m_opOutLine->setUseDisplayList(false);
		m_opOutLine->setUseVertexBufferObjects(true);

		osg::Vec3d vecPosition = m_opMT->getMatrix().getTrans();
		osg::Vec3d vecLLH;
		osg::Vec3d vecNewLLH;
		osg::Vec3d vecNewPosition;
		m_vecLastXYZ = vecPosition;
		FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(),vecPosition,vecLLH);
		vecNewLLH = vecLLH;
		vecNewLLH.z() = 0;
		FeUtil::DegreeLL2LLH(m_opRenderContext.get(),vecNewLLH);
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),vecLLH,vecNewPosition);
		m_dHeight = vecLLH.z() - vecNewLLH.z();
		if (m_dLenght<m_dHeight)
		{
			m_dHeight = m_dLenght;
		}
		double dLRWidth = m_dHeight*tan(osg::DegreesToRadians(m_dLRAngle));
		double dFBWidth = m_dHeight*tan(osg::DegreesToRadians(m_dFBAngle));
		m_dRectWidth = 2*dLRWidth;
		m_dRectHeight = 2*dFBWidth;
		osg::Vec3d vecLeftUp(-dLRWidth,dFBWidth,-m_dHeight);
		osg::Vec3d vecRightUp(dLRWidth,dFBWidth,-m_dHeight);
		osg::Vec3d vecRightDown(dLRWidth,-dFBWidth,-m_dHeight);
		osg::Vec3d vecLeftDown(-dLRWidth,-dFBWidth,-m_dHeight);

		m_vecLeftUp = GetHitLLHByLocalXYZ(vecLeftUp,vecPosition);
		m_vecRightUp = GetHitLLHByLocalXYZ(vecRightUp,vecPosition);
		m_vecRightDown = GetHitLLHByLocalXYZ(vecRightDown,vecPosition);
		m_vecLeftDown = GetHitLLHByLocalXYZ(vecLeftDown,vecPosition);
		m_pVertex = new osg::Vec3Array;
		PushBackPoint(m_opLeftFace.get(),vecLeftDown,vecLeftUp,vecPosition);
		PushBackPoint(m_opBackFace.get(),vecLeftUp,vecRightUp,vecPosition);
		PushBackPoint(m_opRightFace.get(),vecRightUp,vecRightDown,vecPosition);
		PushBackPoint(m_opFrontFace.get(),vecRightDown,vecLeftDown,vecPosition);

		osg::Vec4Array* vecColorArray = new osg::Vec4Array();
		vecColorArray->push_back(m_vecFaceColor);
		m_opLeftFace->setColorArray(vecColorArray,osg::Array::BIND_OVERALL);
		m_opBackFace->setColorArray(vecColorArray,osg::Array::BIND_OVERALL);
		m_opRightFace->setColorArray(vecColorArray,osg::Array::BIND_OVERALL);
		m_opFrontFace->setColorArray(vecColorArray,osg::Array::BIND_OVERALL);

		osg::Vec3d vecCenter(0,0,0);
		osg::Vec3Array* lineVa = new osg::Vec3Array();
		//osg::DrawElementsUByte* lineIa = new osg::DrawElementsUByte(osg::PrimitiveSet::LINES);
		osg::DrawElementsUInt*  lineIa = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES);
		osg::Vec4Array* lineCa = new osg::Vec4Array();	
		lineVa->push_back(vecCenter);
		PushBackOutLine(lineVa,vecLeftUp,vecLeftDown,vecPosition);
		PushBackOutLine(lineVa,vecLeftDown,vecRightDown,vecPosition);
		PushBackOutLine(lineVa,vecRightDown,vecRightUp,vecPosition);
		PushBackOutLine(lineVa,vecRightUp,vecLeftUp,vecPosition);
		lineIa->push_back(0);
		lineIa->push_back(1);
		lineIa->push_back(0);
		lineIa->push_back(m_nNum+1);
		lineIa->push_back(0);
		lineIa->push_back(2*m_nNum+1);
		lineIa->push_back(0);
		lineIa->push_back(3*m_nNum+1);
		for (int i=1;i<4*m_nNum;++i)
		{
			lineIa->push_back(i);
			lineIa->push_back(i+1);
		}
		lineIa->push_back(4*m_nNum);
		lineIa->push_back(1);	
		m_opOutLine->setVertexArray(lineVa);
		m_opOutLine->addPrimitiveSet(lineIa);
		lineCa->push_back(m_vecLineColor);
		m_opOutLine->setColorArray(lineCa,osg::Array::BIND_OVERALL);
		m_opOutLine->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

		m_opGeode = new osg::Geode();
		m_opGeode->addDrawable(m_opLeftFace.get());
		m_opGeode->addDrawable(m_opRightFace.get());
		m_opGeode->addDrawable(m_opBackFace.get());
		m_opGeode->addDrawable(m_opFrontFace.get());
		m_opGeode->addDrawable(m_opOutLine.get());

		m_opGeode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		m_opGeode->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		m_rpMT->addChild(m_opGeode.get());
	}
	void CNewFixedScanEffect::updataLength()
	{
		osg::Vec3d vecPosition = m_opMT->getMatrix().getTrans();
		if (m_vecLastXYZ != vecPosition)
		{
			osg::Vec3d vecLLH;
			osg::Vec3d vecNewLLH;
			osg::Vec3d vecNewPosition;
			FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(),vecPosition,vecLLH);
			vecNewLLH = vecLLH;
			vecNewLLH.z() = 0;
			FeUtil::DegreeLL2LLH(m_opRenderContext.get(),vecNewLLH);
			FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),vecLLH,vecNewPosition);
			m_dHeight = vecLLH.z() - vecNewLLH.z();
			if (m_dLenght<m_dHeight)
			{
				m_dHeight = m_dLenght;
			}
			double dLRWidth = m_dHeight*tan(osg::DegreesToRadians(m_dLRAngle));
			double dFBWidth = m_dHeight*tan(osg::DegreesToRadians(m_dFBAngle));
			m_dRectWidth = 2*dLRWidth;
			m_dRectHeight = 2*dFBWidth;
			osg::Vec3d vecLeftUp(-dLRWidth,dFBWidth,-m_dHeight);
			osg::Vec3d vecRightUp(dLRWidth,dFBWidth,-m_dHeight);
			osg::Vec3d vecRightDown(dLRWidth,-dFBWidth,-m_dHeight);
			osg::Vec3d vecLeftDown(-dLRWidth,-dFBWidth,-m_dHeight);

			m_vecLeftUp = GetHitLLHByLocalXYZ(vecLeftUp,vecPosition);
			m_vecRightUp = GetHitLLHByLocalXYZ(vecRightUp,vecPosition);
			m_vecRightDown = GetHitLLHByLocalXYZ(vecRightDown,vecPosition);
			m_vecLeftDown = GetHitLLHByLocalXYZ(vecLeftDown,vecPosition);
			if(m_pVertex->size()>0)
			{
				m_pVertex->clear();
			}
			UpdatePoint(m_opLeftFace.get(),vecLeftDown,vecLeftUp,vecPosition);
			UpdatePoint(m_opBackFace.get(),vecLeftUp,vecRightUp,vecPosition);
			UpdatePoint(m_opRightFace.get(),vecRightUp,vecRightDown,vecPosition);
			UpdatePoint(m_opFrontFace.get(),vecRightDown,vecLeftDown,vecPosition);
			osg::Vec3d vecCenter(0,0,0);
			osg::Vec3Array* lineVa = dynamic_cast<osg::Vec3Array*>(m_opOutLine->getVertexArray());
			lineVa->clear();
			lineVa->push_back(vecCenter);
			PushBackOutLine(lineVa,vecLeftUp,vecLeftDown,vecPosition);
			PushBackOutLine(lineVa,vecLeftDown,vecRightDown,vecPosition);
			PushBackOutLine(lineVa,vecRightDown,vecRightUp,vecPosition);
			PushBackOutLine(lineVa,vecRightUp,vecLeftUp,vecPosition);
			lineVa->dirty();
			m_opOutLine->setVertexArray(lineVa);
			m_opOutLine->dirtyDisplayList();
			m_opOutLine->dirtyBound();
		}
	}

	void CNewFixedScanEffect::SetLRAngle( double dAngle )
	{
		m_dLRAngle = dAngle;
	}

	void CNewFixedScanEffect::SetFBAngle( double dAngle )
	{
		m_dFBAngle = dAngle;
	}

	double CNewFixedScanEffect::GetLRAngle()
	{
		return m_dLRAngle;
	}

	double CNewFixedScanEffect::GetFBAngle()
	{
		return m_dFBAngle;
	}

	osg::Vec3d CNewFixedScanEffect::GetLLHByLL( osg::Vec3d vecLLH )
	{
		vecLLH.z() = 0;
		FeUtil::DegreeLL2LLH(m_opRenderContext.get(),vecLLH);
		return vecLLH;
	}

	osg::Vec3d CNewFixedScanEffect::GetWorldXYZByLLH( osg::Vec3d vecLLH )
	{
		osg::Vec3d vecXYZ;
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),vecLLH,vecXYZ);
		return vecXYZ ;
	}

	osg::Vec3d CNewFixedScanEffect::GetLLHByLocalXYZ( osg::Vec3d vecXYZ )
	{
		osg::Matrix mtLocal;
		mtLocal.setTrans(vecXYZ);
		osg::Matrix mtWorld = mtLocal * m_opMT->getMatrix();
		osg::Vec3d vecLLH;
		FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(),mtWorld.getTrans(),vecLLH);
		return  vecLLH;
	}

	osg::Vec3d CNewFixedScanEffect::GetWorldXYZByHit( osg::Vec3d vecXYZ,osg::Vec3d vecCenter )
	{
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
		if (0)
		{	
			m_rpEarth->accept(*(ivXY.get()));//结构交集检测
		}
		m_opRenderContext->GetMapNode()->accept(*(ivXY.get()));//结构交集检测
		if(ivXY->hits())//如果碰撞
		{
			vecXYZ = ivXY->getHitList(lineXY).back().getWorldIntersectPoint();
		}
		else
		{
			vecXYZ = vecNewXYZ;
		}
		return vecXYZ;
	}

	osg::Vec3d CNewFixedScanEffect::GetLocalXYZByWorldXYZ( osg::Vec3d vecXYZ )
	{
		osg::Matrix mtWorld;
		mtWorld.setTrans(vecXYZ);
		osg::Matrix mtLocal = mtWorld * m_opMT->getInverseMatrix();
		return mtLocal.getTrans();
	}

	osg::Vec3d CNewFixedScanEffect::GetWorldXYZByLocalXYZ( osg::Vec3d vecXYZ )
	{
		osg::Matrix mtLocal;
		mtLocal.setTrans(vecXYZ);
		osg::Matrix mtWorld = mtLocal * m_opMT->getMatrix();
		return mtWorld.getTrans();
	}

	osg::Vec3d CNewFixedScanEffect::GetHitLLHByLocalXYZ( osg::Vec3d vecXYZ,osg::Vec3d vecCenter )
	{
		osg::Vec3d vecPoint;
		vecPoint = GetWorldXYZByLocalXYZ(vecXYZ);
		vecPoint = GetWorldXYZByHit(vecPoint,vecCenter);
		vecPoint = GetLLHByWorldXYZ(vecPoint);
		return vecPoint;
	}

	osg::Vec3d CNewFixedScanEffect::GetLLHByWorldXYZ( osg::Vec3d vecXYZ )
	{
		osg::Vec3d vecPoint;
		FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(),vecXYZ,vecPoint);
		return vecPoint;
	}

	void CNewFixedScanEffect::PushBackPoint( osg::Geometry* pGeometry, osg::Vec3d vecStart, osg::Vec3d vecStop,osg::Vec3 vecCenter )
	{
		double dXLen = (vecStop.x() - vecStart.x())/m_nNum;
		double dYLen = (vecStop.y() - vecStart.y())/m_nNum;
		osg::Vec3Array* vecArray = new osg::Vec3Array();
		vecArray->push_back(osg::Vec3d(0,0,0));
		for (int i=0;i<=m_nNum;++i)
		{
			osg::Vec3d vecPoint = vecStart + osg::Vec3d(dXLen*i, dYLen*i, 0);
			if (m_dLenght>m_dHeight)
			{
				vecPoint = GetWorldXYZByLocalXYZ(vecPoint);
				vecPoint = GetWorldXYZByHit(vecPoint,vecCenter);

				m_pVertex->push_back(GetLLHByWorldXYZ(vecPoint));

				vecPoint = GetLocalXYZByWorldXYZ(vecPoint);
			}
			else
			{
				m_pVertex->push_back(GetLLHByWorldXYZ(GetWorldXYZByLocalXYZ(vecPoint)));
			}
			vecArray->push_back(vecPoint);
		}
		pGeometry->setVertexArray(vecArray);
		pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,0,m_nNum+2));
	}

	void CNewFixedScanEffect::UpdatePoint( osg::Geometry* pGeometry, osg::Vec3d vecStart, osg::Vec3d vecStop,osg::Vec3 vecCenter )
	{
		double dXLen = (vecStop.x() - vecStart.x())/m_nNum;
		double dYLen = (vecStop.y() - vecStart.y())/m_nNum;
		osg::Vec3Array* vecArray = dynamic_cast<osg::Vec3Array*>(pGeometry->getVertexArray());
		vecArray->clear();
		vecArray->push_back(osg::Vec3d(0,0,0));
		for (int i=0;i<=m_nNum;++i)
		{
			osg::Vec3d vecPoint = vecStart + osg::Vec3d(dXLen*i, dYLen*i, 0);
			if (m_dLenght>m_dHeight)
			{
				vecPoint = GetWorldXYZByLocalXYZ(vecPoint);
				/*vecPoint = GetLLHByLocalXYZ(vecPoint);
				vecPoint = GetWorldXYZByLLH(vecPoint);*/
				vecPoint = GetWorldXYZByHit(vecPoint,vecCenter);

				m_pVertex->push_back(GetLLHByWorldXYZ(vecPoint));

				vecPoint = GetLocalXYZByWorldXYZ(vecPoint);
			}
			else
			{
				m_pVertex->push_back(GetLLHByWorldXYZ(GetWorldXYZByLocalXYZ(vecPoint)));
			}
			vecArray->push_back(vecPoint);
		}
		vecArray->dirty();
		pGeometry->setVertexArray(vecArray);
		pGeometry->dirtyDisplayList();
		pGeometry->dirtyBound();
	}

	void CNewFixedScanEffect::PushBackOutLine( osg::Vec3Array* vecArray, osg::Vec3d vecStart, osg::Vec3d vecStop,osg::Vec3 vecCenter)
	{
		double dXLen = (vecStop.x() - vecStart.x())/m_nNum;
		double dYLen = (vecStop.y() - vecStart.y())/m_nNum;	
		for (int i=0;i<m_nNum;++i)
		{
			osg::Vec3d vecPoint = vecStart + osg::Vec3d(dXLen*i, dYLen*i, 0);
			if (m_dLenght>m_dHeight)
			{
				vecPoint = GetWorldXYZByLocalXYZ(vecPoint);
				/*vecPoint = GetLLHByLocalXYZ(vecPoint);
				vecPoint = GetWorldXYZByLLH(vecPoint);*/
				vecPoint = GetWorldXYZByHit(vecPoint,vecCenter);
				vecPoint = GetLocalXYZByWorldXYZ(vecPoint);
			}
			vecArray->push_back(vecPoint);
		}
	}

	void CNewFixedScanEffect::SetLenght( double dLenght )
	{
		m_dLenght = dLenght;
	}

	double CNewFixedScanEffect::GetLenght()
	{
		return m_dLenght;
	}

	void CNewFixedScanEffect::SetFaceColor( osg::Vec4d vecColor )
	{
		m_vecFaceColor = vecColor;
		osg::Vec4Array* vecLeftArray = dynamic_cast<osg::Vec4Array*>(m_opLeftFace->getColorArray());
		if (vecLeftArray)
		{
			vecLeftArray->clear();
			vecLeftArray->push_back(m_vecFaceColor);
			vecLeftArray->dirty();
			m_opLeftFace->setColorArray(vecLeftArray);
		}
		osg::Vec4Array* vecRightArray = dynamic_cast<osg::Vec4Array*>(m_opRightFace->getColorArray());
		if (vecRightArray)
		{
			vecRightArray->clear();
			vecRightArray->push_back(m_vecFaceColor);
			vecRightArray->dirty();
			m_opRightFace->setColorArray(vecRightArray);
		}
		osg::Vec4Array* vecFrontArray = dynamic_cast<osg::Vec4Array*>(m_opFrontFace->getColorArray());
		if (vecFrontArray)
		{
			vecFrontArray->clear();
			vecFrontArray->push_back(m_vecFaceColor);
			vecFrontArray->dirty();
			m_opFrontFace->setColorArray(vecFrontArray);
		}
		osg::Vec4Array* vecBackArray = dynamic_cast<osg::Vec4Array*>(m_opBackFace->getColorArray());
		if (vecBackArray)
		{
			vecBackArray->clear();
			vecBackArray->push_back(m_vecFaceColor);
			vecBackArray->dirty();
			m_opBackFace->setColorArray(vecBackArray);
		}
	}

	void CNewFixedScanEffect::SetLineColor( osg::Vec4d vecColor )
	{
		m_vecLineColor = vecColor;
		osg::Vec4Array* vecArray = dynamic_cast<osg::Vec4Array*>(m_opOutLine->getColorArray());
		if (vecArray)
		{
			vecArray->clear();
			vecArray->push_back(m_vecLineColor);
			vecArray->dirty();
			m_opOutLine->setColorArray(vecArray);
		}
	}

	osg::Vec4d CNewFixedScanEffect::GetFaceColor()
	{
		return m_vecFaceColor;
	}

	osg::Vec4d CNewFixedScanEffect::GetLineColor()
	{
		return m_vecLineColor;
	}

	double CNewFixedScanEffect::GetRectWidth()
	{
		return m_dRectWidth;
	}

	double CNewFixedScanEffect::GetRectHeight()
	{
		return m_dRectHeight;
	}

	osg::Vec3d CNewFixedScanEffect::GetLeftUp()
	{
		return m_vecLeftUp;
	}

	osg::Vec3d CNewFixedScanEffect::GetRightUp()
	{
		return m_vecRightUp;
	}

	osg::Vec3d CNewFixedScanEffect::GetRightDown()
	{
		return m_vecRightDown;
	}

	osg::Vec3d CNewFixedScanEffect::GetLeftDown()
	{
		return m_vecLeftDown;
	}

	osg::Vec3Array* CNewFixedScanEffect::GetVertex()
	{
		return m_pVertex;
	}

	CUpdateScanCallback::CUpdateScanCallback(CNewFixedScanEffect* pFixedScan )
		:osg::NodeCallback()
		,m_opFixedScan(pFixedScan)
		,m_nCount(0)
	{

	}

	void CUpdateScanCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		++m_nCount;
		if (m_nCount>10)
		{
			if(m_opFixedScan.valid())
			{
				m_opFixedScan->updataLength();
			}
			m_nCount = 0;
		}
		traverse(node,nv);
	}

}
