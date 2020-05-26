#include <osg/Geometry>

#include <FeKits/airview/AirViewKits.h>

#include <FeUtils/UtilityGeom.h>

namespace FeKit
{

	CAirViewBoeder::CAirViewBoeder
		( double dX, double dY, double dWidth, double dHeight, const osg::Vec4d& vecColor)
		:osg::Geode()
	{
		CreateBorder(dX, dY, dWidth, dHeight, vecColor);
	}

	CAirViewBoeder::~CAirViewBoeder( void )
	{

	}

	void CAirViewBoeder::CreateBorder
		(double dX, double dY, double dWidth, double dHeight, const osg::Vec4d& vecColor)
	{
		osg::ref_ptr<osg::Geometry> pGeome = FeUtil::CreateRectangle(dX, dY, dWidth, dHeight, vecColor, 3.0);

		if(pGeome)
		{
			addDrawable(pGeome);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////

	CAirViewFocus::CAirViewFocus
		( double dX, double dY, double dWidth, double dHeight, const osg::Vec4d& vecColor )
		:osg::MatrixTransform()
		,m_vecWorldPos(0.0, 0.0, 0.0)
		,m_dX(dX)
		,m_dY(dY)
		,m_dWidth(dWidth)
		,m_dHeight(dHeight)
	{
		CreateFocus(m_dX, m_dY, m_dWidth, m_dHeight, vecColor);
	}

	CAirViewFocus::~CAirViewFocus( void )
	{

	}

	void CAirViewFocus::CreateFocus
		( double dX, double dY, double dWidth, double dHeight, const osg::Vec4d& vecColor )
	{
		osg::ref_ptr<osg::Geometry> pGeome = 
			FeUtil::CreateRectangle(dX, dY, dWidth, dHeight, vecColor, 1.2);

			osg::Geode* pGeode = new osg::Geode;
			pGeode->addDrawable(pGeome);

            //添加，红色十字    
            osg::Geode* pGeode1 = new osg::Geode;
            osg::Geometry* pGeom = new osg::Geometry;
            osg::Vec3dArray* pVertex = new osg::Vec3dArray;
            osg::Vec4dArray* pColor = new osg::Vec4dArray;
            osg::Vec3d centre = osg::Vec3d(dX+dWidth/2.0, dY+dHeight/2.0, 0.0);
            pVertex->push_back(centre + osg::Vec3d(1, 0, 0.1));
            pVertex->push_back(centre + osg::Vec3d(-1, 0, 0.1));
            pVertex->push_back(centre + osg::Vec3d(0, 1, 0.1));
            pVertex->push_back(centre + osg::Vec3d(0, -1, 0.1));
            pGeom->setVertexArray(pVertex);
            pColor->push_back(osg::Vec4d(1,0,0,1));
            pGeom->setColorArray(pColor);
            pGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
            pGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, pVertex->size()));
            pGeode1->addDrawable(pGeom);
            pGeode1->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF
                | osg::StateAttribute::PROTECTED 
                | osg::StateAttribute::OVERRIDE); 
            pGeode1->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF
                | osg::StateAttribute::PROTECTED 
                | osg::StateAttribute::OVERRIDE); 
            pGeode1->getOrCreateStateSet()->setRenderBinDetails(102, "RenderBin");
            //////////////////////////////////////////////////////////////////////////结束

            m_rpSwitch = new osg::Switch;
            m_rpSwitch->addChild(pGeode);
            m_rpSwitch->addChild(pGeode1);
            m_rpSwitch->setSingleChildOn(AVGlobal::SWITCH_QUAD_NODE);

            addChild(m_rpSwitch);
	}

	//////////////////////////////////////////////////////////////////////////
	CAirViewGround::CAirViewGround(
		double dX, 
		double dY, 
		double dWidth, 
		double dHeight,
		const std::string& strGroundImage)
		:osg::Group()
	{
		CreateGround(dX, dY, dWidth, dHeight, strGroundImage);
	}

	CAirViewGround::~CAirViewGround( void )
	{

	}

	void CAirViewGround::CreateGround( 
		double dX, 
		double dY, 
		double dWidth, 
		double dHeight,
		const std::string& strGroundImage)
	{
		//使用背景创建一个全球图像
		osg::Vec3d vecCorner(dX, dY, -1.0);

       
		osg::Geode* pGeode = 
			FeUtil::CreateBasePlate(strGroundImage, vecCorner, dWidth, dHeight, osg::Vec4d(1.0, 1.0, 1.0, 0.95));


        addChild(pGeode);
	}

	void CAirViewFocus::UpdateFocus( const osg::Vec3d& vecWorldPos)
	{
		/*
		根据相机提供的视点位置、视点的距离为参考设置鸟瞰图中指示焦点的大小和位置，如果超过了可见范围
		会显示最小或者最大的焦点，此框显示的信息是相机的位置在地面的投影而不是真正意义上的地面位置，
		如果以后算法有修改的需求可以再做处理。 LPF
		*/
		if(vecWorldPos != m_vecWorldPos)
		{
			m_vecWorldPos = vecWorldPos;

			//将坐标偏移到0-360度之间
			osg::Vec3d vecWorldPosReset = 
				vecWorldPos - osg::Vec3d(AVGlobal::LONGITUDE * -0.5, AVGlobal::LATITUDE * -0.5, 0.0);

			double dLonRate = vecWorldPosReset.x() / AVGlobal::LONGITUDE;
			double dLatRate = vecWorldPosReset.y() / AVGlobal::LATITUDE;

			double dScaleRate = (m_vecWorldPos.z() - AVGlobal::MIN_DISTANCE) / AVGlobal::DELTA_DISTANCE;

            osg::Vec3d vecTrans;

            if (m_rpSwitch.valid())
            {
                m_rpSwitch->setSingleChildOn(AVGlobal::SWITCH_QUAD_NODE);
            }
        
			if(dScaleRate > 1.0)
			{
				dScaleRate = 1.0;
                vecTrans = osg::Vec3d(m_dX + dLonRate * m_dWidth, m_dY + dLatRate * m_dHeight, -1.0);
			}
			else if(dScaleRate < 0.05)
			{
                if (m_rpSwitch.valid())
                {
                    m_rpSwitch->setSingleChildOn(AVGlobal::SWITCH_CORSS_NODE);
                }
				dScaleRate = 0.1;
			}

            vecTrans = osg::Vec3d(m_dX + dLonRate * m_dWidth, m_dY + dLatRate * m_dHeight, -1.0);
            osg::Matrix scaleM = osg::Matrix::scale(dScaleRate, dScaleRate, 1.0);
            osg::Matrix transM = osg::Matrix::translate(vecTrans);
            setMatrix(scaleM * transM);

		}
	}

	CAirViewMark::CAirViewMark( const osg::Vec3d& vecLLH, double dLineWidth, const osg::Vec4d& vecColor, bool bFull )
		:osg::MatrixTransform()
		,m_vecLLH(vecLLH)
		,m_dLineWidth(dLineWidth)
		,m_vecColor(vecColor)
		,m_bFull(bFull)
		,m_bShow(true)
		,m_dAirViewHeight(1.0)
		,m_dAirViewWidth(1.0)
	{
		
	}

	void CAirViewMark::UpdateMark( const osg::Vec3d& vecLLH )
	{

		/*
		根据相机提供的视点位置、视点的距离为参考设置鸟瞰图中指示焦点的大小和位置，如果超过了可见范围
		会显示最小或者最大的焦点，此框显示的信息是相机的位置在地面的投影而不是真正意义上的地面位置，
		如果以后算法有修改的需求可以再做处理。 LPF
		*/
		if(vecLLH != m_vecLLH)
		{
			m_vecLLH = vecLLH;

			double dLonRate = m_vecLLH.x() / AVGlobal::LONGITUDE;
			double dLatRate = m_vecLLH.y() / AVGlobal::LATITUDE;

			osg::Vec3d vecTrans(dLonRate * m_dAirViewWidth,  dLatRate * m_dAirViewHeight , -1.0);
			osg::Matrix transM = osg::Matrix::translate(vecTrans);
			setMatrix(transM);
		}
	}

	void CAirViewMark::Show()
	{
		setNodeMask(0xFFFFFFFF);
		m_bShow = true;
	}

	void CAirViewMark::Hide()
	{
		setNodeMask(0x00000000);
		m_bShow = false;
	}

	bool CAirViewMark::IsHide()
	{
		return m_bShow;
	}

	void CAirViewMark::AttachAirViewSize( double dWidth, double dHeight )
	{
		m_dAirViewHeight = dHeight;
		m_dAirViewWidth = dWidth;

		CreateMark();
	}

	CCircleMark::CCircleMark( const osg::Vec3d& vecLLH, double dLineWidth, const osg::Vec4d& vecColor, bool bFull )
		:CAirViewMark(vecLLH, dLineWidth, vecColor, bFull)
	{
		CreateMark();
	}

	CCircleMark::~CCircleMark()
	{

	}

	void CCircleMark::CreateMark()
	{
		removeChildren(0, getNumChildren());

		double dRadius = 0.05;
		double dX = (m_vecLLH.x()/AVGlobal::LONGITUDE) * m_dAirViewWidth;
		double dY = (m_vecLLH.y()/AVGlobal::LATITUDE ) * m_dAirViewHeight;

		osg::ref_ptr<osg::Geometry> rpGeom = FeUtil::CreateCircle(dX, dY, dRadius, m_vecColor, m_dLineWidth);
		if(rpGeom.valid())
		{
			osg::ref_ptr<osg::Geode> rpGeode = new osg::Geode;
			rpGeode->addDrawable(rpGeom);
			addChild(rpGeode);
		}
	}

	CTrilateralMark::CTrilateralMark( const osg::Vec3d& vecLLH, double dLineWidth, const osg::Vec4d& vecColor, bool bFull)
		:CAirViewMark(vecLLH, dLineWidth, vecColor, bFull)
	{
		CreateMark();
	}


	CTrilateralMark::~CTrilateralMark()
	{

	}

	void CTrilateralMark::CreateMark()
	{
		removeChildren(0, getNumChildren());

		double dWidth = 0.1;
		double dHeight = 0.1;
		double dX = (m_vecLLH.x()/AVGlobal::LONGITUDE - dWidth * 0.5) * m_dAirViewWidth;
		double dY = (m_vecLLH.y()/AVGlobal::LATITUDE - dHeight * 0.5) * m_dAirViewHeight;

		osg::ref_ptr<osg::Geometry> rpGeom = FeUtil::CreateTrilateral(dX, dY, dWidth, dHeight, m_vecColor, m_dLineWidth, m_bFull);
		if(rpGeom.valid())
		{
			osg::ref_ptr<osg::Geode> rpGeode = new osg::Geode;
			rpGeode->addDrawable(rpGeom);
			addChild(rpGeode);
		}
	}



	CRectangleMark::CRectangleMark( const osg::Vec3d& vecLLH, double dLineWidth, const osg::Vec4d& vecColor, bool bFull)
		:CAirViewMark(vecLLH, dLineWidth, vecColor, bFull)
	{
		CreateMark();
	}

	CRectangleMark::~CRectangleMark()
	{

	}

	void CRectangleMark::CreateMark()
	{
		removeChildren(0, getNumChildren());

		double dWidth = 0.1;
		double dHeight = 0.1;
		double dX = (m_vecLLH.x()/AVGlobal::LONGITUDE - dWidth * 0.5) * m_dAirViewWidth;
		double dY = (m_vecLLH.y()/AVGlobal::LATITUDE - dHeight * 0.5) * m_dAirViewHeight;

		osg::ref_ptr<osg::Geometry> rpGeom = FeUtil::CreateRectangle(dX, dY, dWidth, dHeight, m_vecColor, m_dLineWidth, m_bFull);
		if(rpGeom.valid())
		{
			osg::ref_ptr<osg::Geode> rpGeode = new osg::Geode;
			rpGeode->addDrawable(rpGeom);
			addChild(rpGeode);
		}
	}

}