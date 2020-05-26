#include <FeKits/manipulator/FreeEarthManipulatorBase.h>

#include <osgViewer/Renderer>
#include <osgEarth/GeoData>
#include <osgEarth/GeoCommon>
#include <osgEarth/NodeUtils>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarth/DPLineSegmentIntersector>
#include <osgEarthDrivers/engine_mp/TerrainNode>

#include <FeUtils/CoordConverter.h>

namespace FeKit
{
	CCameraAltCallback::CCameraAltCallback( FeUtil::CRenderContext* pRenderContext )
		:m_opRenderContext(pRenderContext)
	{

	}

	void CCameraAltCallback::customOperator( const osgViewer::ViewerBase* pViewerBase )
	{
		if (pViewerBase)
		{
			if (m_opRenderContext.valid())
			{
				osg::Camera* cam = m_opRenderContext->GetCamera();
				osgEarth::MapNode* mapNode = m_opRenderContext->GetMapNode();
				if (cam && mapNode)
				{
					osg::Vec3d camPos, tmp;
					cam->getViewMatrix().getLookAt(camPos, tmp, tmp);

					osg::Vec3d start(0, 0, 0);
					osg::Vec3d rayDir = camPos;
					rayDir.normalize();

					double dis = 6378137.0 * 3.0;
					osg::Vec3d end = rayDir * dis;

					osg::NodePath nodePath;
					nodePath.push_back(mapNode->getTerrain()->getGraph());

					osg::ref_ptr<osgUtil::LineSegmentIntersector> picker = NULL;

					picker = new osgEarth::DPLineSegmentIntersector(osgUtil::Intersector::MODEL, start, end);
					picker->setIntersectionLimit(osgUtil::Intersector::LIMIT_NEAREST);

					osgUtil::IntersectionVisitor iv(picker.get());
					nodePath.back()->accept(iv);

					double alt = 0;

					if (picker->containsIntersections())
					{
						osgUtil::LineSegmentIntersector::Intersections& results = picker->getIntersections();
						osg::Vec3d hitPoint = results.begin()->getWorldIntersectPoint();

						alt = camPos.length() - hitPoint.length();
					}
					else
					{
						osg::Vec3d camPosLLH;
						FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(), camPos, camPosLLH);

						alt = camPosLLH.z();
					}

					m_opRenderContext->setCamAltFromTerrain(alt);
				}
			}
		}
	}


	bool getCurNear( double dCurrentAlt,double& dOutNear,double dMinNearClip )
	{
		float dAltNearRatio = 0.001;
		dOutNear = dCurrentAlt * dAltNearRatio;

		if(dOutNear < dMinNearClip)
		{
			dOutNear = dMinNearClip;
		}

		return true;
	}

	void modifyNearFar( double& dInputNear,double& dInputFar,FeUtil::CRenderContext* pRenderContext,double dMinNearClip )
	{
		if(pRenderContext)
		{
			osg::Camera* pCamera = pRenderContext->GetCamera();
			if(pCamera)
			{
				double dCurNear;
				double dCurAlt = abs(pRenderContext->getCamAltFromTerrain());

				///获取当前的近裁切面
				bool flag = getCurNear(dCurAlt,dCurNear,dMinNearClip);
				if(flag)
				{
					dInputNear = dCurNear;
				}
			}			
		}			
	}

	template<class matrix_type>
	bool clampProjectionMatrix( matrix_type& projection, double& znear, double& zfar,CEarthClampProjectionMatrixCallback* pc )
	{
		if(!pc)
			return false;

		FeUtil::CRenderContext* pRenderContext = pc->GetRenderContext();
		if(!pRenderContext)
			return false;

		osg::Camera* cam = pRenderContext->GetCamera();
		if(cam == NULL)
			return false;

		double nearFarRatio = cam->getNearFarRatio();

		double epsilon = 1e-6;
		if (zfar<znear-epsilon)
		{
			if (zfar != -FLT_MAX || znear != FLT_MAX)
			{
				OSG_INFO<<"_clampProjectionMatrix not applied, invalid depth range, znear = "<<znear<<"  zfar = "<<zfar<<std::endl;
			}
			return false;
		}

		if (zfar<znear+epsilon)
		{
			double average = (znear+zfar)*0.5;
			znear = average-epsilon;
			zfar = average+epsilon;

			if(pc->GetUseCustomNearFar())
			{
				znear = pc->GetCustomNear();
				zfar  = pc->GetCustomFar();
			}
			else
			{
				modifyNearFar(znear,zfar,pRenderContext,pc->GetMinNearClip());
			}	
		}

		if (fabs(projection(0,3))<epsilon  && fabs(projection(1,3))<epsilon  && fabs(projection(2,3))<epsilon )
		{
			return false;
		}
		else
		{
			double zfarPushRatio = 1.02;
			double znearPullRatio = 0.98;

			double desired_znear = znear * znearPullRatio;
			double desired_zfar = zfar * zfarPushRatio;

			if(pc->GetUseCustomNearFar())
			{
				desired_znear = pc->GetCustomNear();
				desired_zfar = pc->GetCustomFar();
			}
			else
			{
				modifyNearFar(desired_znear,desired_zfar,pRenderContext,pc->GetMinNearClip());
			}

			znear = desired_znear;
			zfar = desired_zfar;

			double trans_near_plane = (-desired_znear*projection(2,2)+projection(3,2))/(-desired_znear*projection(2,3)+projection(3,3));
			double trans_far_plane = (-desired_zfar*projection(2,2)+projection(3,2))/(-desired_zfar*projection(2,3)+projection(3,3));

			double ratio = fabs(2.0/(trans_near_plane-trans_far_plane));
			double center = -(trans_near_plane+trans_far_plane)/2.0;

			projection.postMult(osg::Matrix(1.0f,0.0f,0.0f,0.0f,
				0.0f,1.0f,0.0f,0.0f,
				0.0f,0.0f,ratio,0.0f,
				0.0f,0.0f,center*ratio,1.0f));
		}

		return true;
	}

	CEarthClampProjectionMatrixCallback::CEarthClampProjectionMatrixCallback( FeUtil::CRenderContext* pRenderContext )
		:m_opRenderContext(pRenderContext)
		,m_bUserCustomNearFar(false)
		,m_dCustomNear(1.0)
		,m_dCustomFar(100000.0)
		,m_dMinNearClip(10)
	{

	}

	bool CEarthClampProjectionMatrixCallback::clampProjectionMatrixImplementation( osg::Matrixf& projection, double& znear, double& zfar ) const
	{
		CEarthClampProjectionMatrixCallback* pc = const_cast<CEarthClampProjectionMatrixCallback*>(this);

		return clampProjectionMatrix(projection,znear,zfar,pc);
	}

	bool CEarthClampProjectionMatrixCallback::clampProjectionMatrixImplementation( osg::Matrixd& projection, double& znear, double& zfar ) const
	{
		CEarthClampProjectionMatrixCallback* pc = const_cast<CEarthClampProjectionMatrixCallback*>(this);

		return clampProjectionMatrix(projection,znear,zfar,pc);
	}

	void CEarthClampProjectionMatrixCallback::SetUseCustomNearFar( bool bFlag )
	{
		m_bUserCustomNearFar = bFlag;
	}

	bool CEarthClampProjectionMatrixCallback::GetUseCustomNearFar()
	{
		return m_bUserCustomNearFar;
	}

	void CEarthClampProjectionMatrixCallback::SetCustomNear( double dNear )
	{
		m_dCustomNear = dNear;
	}

	double CEarthClampProjectionMatrixCallback::GetCustomNear()
	{
		return m_dCustomNear;
	}

	void CEarthClampProjectionMatrixCallback::SetCustomFar( double dFar )
	{
		m_dCustomFar = dFar;
	}

	double CEarthClampProjectionMatrixCallback::GetCustomFar()
	{
		return m_dCustomFar;
	}

	void CEarthClampProjectionMatrixCallback::SetMinNearClip( double dMinNearClip )
	{
		m_dMinNearClip = dMinNearClip;
	}

	double CEarthClampProjectionMatrixCallback::GetMinNearClip()
	{
		return m_dMinNearClip;
	}

	FeUtil::CRenderContext* CEarthClampProjectionMatrixCallback::GetRenderContext()
	{
		return m_opRenderContext.get();
	}
}


namespace FeKit
{
	//FreeEarthManipulatorBase::TileNodeVisitor::TileNodeVisitor(osg::Group* pGroupNode,double dWest,double dEast,double dNorth,double dSouth)
	//	: NodeVisitor(osg::NodeVisitor::NODE_VISITOR,osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
	//	, m_pGroupNode(pGroupNode)
	//	, m_dWest(dWest)
	//	, m_dEast(dEast)
	//	, m_dNorth(dNorth)
	//	, m_dSouth(dSouth)
	//{

	//}

	//void FreeEarthManipulatorBase::TileNodeVisitor::apply(osg::PagedLOD& node)
	//{
	//	if(node.getNumChildren() > 2)
	//	{
	//		traverse(node);
	//	}
	//	else if(node.getNumChildren() > 0)
	//	{
	//		osgEarth::Drivers::MPTerrainEngine::TileNode* tNode = dynamic_cast<osgEarth::Drivers::MPTerrainEngine::TileNode*>(node.getChild(0));
	//		if(tNode)
	//		{
	//			const osgEarth::Drivers::MPTerrainEngine::TileModel* tm = tNode->getTileModel();
	//			if(tm)
	//			{
	//				osgEarth::GeoExtent extent = tm->_tileKey.getExtent();
	//				osgEarth::Bounds bb(m_dWest,m_dSouth,m_dEast,m_dNorth);
	//				osgEarth::GeoExtent myExtent(extent.getSRS(),bb);

	//				osgEarth::GeoPoint lowerLeft(extent.getSRS(), extent.xMin(), extent.yMin(), 0.0, osgEarth::ALTMODE_ABSOLUTE);
	//				osgEarth::GeoPoint upperRight(extent.getSRS(), extent.xMax(), extent.yMax(), 0.0, osgEarth::ALTMODE_ABSOLUTE);
	//				osg::Vec3d ll, ur;
	//				lowerLeft.toWorld( ll );
	//				upperRight.toWorld( ur );
	//				double radius = (ur - ll).length() / 2.0;

	//				if(extent.intersects(myExtent) && radius > 2.0)
	//				{
	//					traverse(node);
	//				}
	//			}
	//		}
	//		else
	//		{
	//			traverse(node);
	//		}
	//	}
	//}

	//void FreeEarthManipulatorBase::TileNodeVisitor::apply(osg::MatrixTransform& node)
	//{
	//	if(m_pGroupNode)
	//	{
	//		osg::Group* gp = dynamic_cast<osg::Group*>(node.getParent(0));
	//		if(gp)
	//		{
	//			if(gp->getNumChildren() < 2 )
	//			{
	//				m_pGroupNode->addChild(&node);
	//			}
	//		}
	//	}
	//}


	FreeEarthManipulatorBase::TileNodeVisitor::TileNodeVisitor
		(osg::Vec3d vecCamPos, osg::Group* pGroupNode, double dWest,double dEast,double dNorth,double dSouth)
		: NodeVisitor(osg::NodeVisitor::NODE_VISITOR,osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
		, m_pGroupNode(pGroupNode)
		, m_dWest(dWest)
		, m_dEast(dEast)
		, m_dNorth(dNorth)
		, m_dSouth(dSouth)
		, m_vecCamPos(vecCamPos)
	{

	}

	float FreeEarthManipulatorBase::TileNodeVisitor::getDistanceToViewPoint(const osg::Vec3& pos, bool useLODScale) const
	{
		osg::Vec3d lp = m_vecCamPos - pos;
		return lp.length();
	}

	void FreeEarthManipulatorBase::TileNodeVisitor::apply(osg::PagedLOD& node)
	{
		if(node.getNumChildren() > 2)
			traverse(node);
		else if(node.getNumChildren() > 0)
		{
			osgEarth::Drivers::MPTerrainEngine::TileNode* tNode = dynamic_cast<osgEarth::Drivers::MPTerrainEngine::TileNode*>(node.getChild(0));
			if(tNode)
			{
				const osgEarth::Drivers::MPTerrainEngine::TileModel* tm = tNode->getTileModel();
				if(tm)
				{
					osgEarth::GeoExtent extent = tm->_tileKey.getExtent();
					osgEarth::Bounds bb(m_dWest,m_dSouth,m_dEast,m_dNorth);
					osgEarth::GeoExtent myExtent(extent.getSRS(),bb);

					osgEarth::GeoPoint lowerLeft(extent.getSRS(), extent.xMin(), extent.yMin(), 0.0, osgEarth::ALTMODE_ABSOLUTE);
					osgEarth::GeoPoint upperRight(extent.getSRS(), extent.xMax(), extent.yMax(), 0.0, osgEarth::ALTMODE_ABSOLUTE);
					osg::Vec3d ll, ur;
					lowerLeft.toWorld( ll );
					upperRight.toWorld( ur );
					double radius = (ur - ll).length() / 2.0;

					if(extent.intersects(myExtent) && radius > 2.0)
					{
						traverse(node);
					}
				}
			}
			else
			{
				traverse(node);
			}
		}
	}
	void FreeEarthManipulatorBase::TileNodeVisitor::apply(osg::MatrixTransform& node)
	{
		if(m_pGroupNode)
		{
			osg::Group* gp = dynamic_cast<osg::Group*>(node.getParent(0));
			if(gp)
			{
				if(gp->getNumChildren() < 2 )
				{
					m_pGroupNode->addChild(&node);
				}
			}
		}
		/*
		if(m_pGroupNode)
		{
		if(node.getNumParents() > 0)
		{
		osgEarth::Drivers::MPTerrainEngine::TileNode* tNode = dynamic_cast<osgEarth::Drivers::MPTerrainEngine::TileNode*>(node.getParent(0));
		if(tNode)
		{
		const osgEarth::Drivers::MPTerrainEngine::TileModel* tm = tNode->getTileModel();
		if(tm)
		{
		osgEarth::GeoExtent extent = tm->_tileKey.getExtent();
		osgEarth::Bounds bb(m_dWest,m_dSouth,m_dEast,m_dNorth);
		osgEarth::GeoExtent myExtent(extent.getSRS(),bb);

		osgEarth::GeoPoint lowerLeft(extent.getSRS(), extent.xMin(), extent.yMin(), 0.0, osgEarth::ALTMODE_ABSOLUTE);
		osgEarth::GeoPoint upperRight(extent.getSRS(), extent.xMax(), extent.yMax(), 0.0, osgEarth::ALTMODE_ABSOLUTE);
		osg::Vec3d ll, ur;
		lowerLeft.toWorld( ll );
		upperRight.toWorld( ur );
		double radius = (ur - ll).length() / 2.0;

		if(extent.intersects(myExtent) && radius > 2.0)
		{
		m_pGroupNode->addChild(&node);
		}
		}
		}
		}
		}*/
	}

	FreeEarthManipulatorBase::FreeEarthManipulatorBase(FeUtil::CRenderContext *pRenderContext)
		:m_opRenderContext(pRenderContext)
		,m_rpMaskNodeGroup(NULL)
	{
		if(pRenderContext)
		{
			osgViewer::Viewer* vv = dynamic_cast<osgViewer::Viewer*>(pRenderContext->GetView());
			if(vv)
				m_opViewer = vv;

			m_opMapNode = pRenderContext->GetMapNode();
		}
	}

	void FreeEarthManipulatorBase::modifyNearFar( double& dInputNear,double &dInputFar)
	{

	}

	bool FreeEarthManipulatorBase::CheckCollision( osg::Vec3d& start, osg::Vec3d& end, osg::Vec3d& safePoint, osg::Vec3d& upPoint,double checkDis /*= 5.0*/ )
	{
		if(m_opViewer.valid())
		{
			osg::Camera* cam = m_opViewer->getCamera();
			if(cam == NULL)
				return false;

			osg::Matrixd projMat = cam->getProjectionMatrix();

			double fov,aspect,dCurNear,tmpNearFar;
			projMat.getPerspective(fov,aspect,dCurNear,tmpNearFar); 
			fov = osg::DegreesToRadians(fov);

			osg::Vec3d curLook,curLeft,curUp;
			osg::Matrixd curMat = GetBaseMatrix();
			curLook.set(-curMat(2,0),-curMat(2,1),-curMat(2,2));
			curLook.normalize();
			curLeft.set(curMat(0,0),curMat(0,1),curMat(0,2));
			curLeft.normalize();
			curUp.set(curMat(1,0),curMat(1,1),curMat(1,2));
			curUp.normalize();

			double curTop = dCurNear * tan(fov / 2.0);
			double curRight = curTop * aspect;

			osg::Vec3d curFrustumPoints[5];
			curFrustumPoints[0] = end;
			curFrustumPoints[1] = end + curLook * dCurNear - curLeft * curRight - curUp * curTop;
			curFrustumPoints[2] = end + curLook * dCurNear + curLeft * curRight - curUp * curTop;
			curFrustumPoints[3] = end + curLook * dCurNear + curLeft * curRight + curUp * curTop;
			curFrustumPoints[4] = end + curLook * dCurNear - curLeft * curRight + curUp * curTop;

			double radius = dCurNear + 1;//(curFrustumPoints[1] - end).length() + 0.5;

			double west = 180;
			double east = -180;
			double north = -90;
			double south = 90;
			for (int i = 0; i < 5; i++)
			{
				FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(),curFrustumPoints[i],curFrustumPoints[i]);
				if(curFrustumPoints[i].y() < south)
					south = curFrustumPoints[i].y();
				if(curFrustumPoints[i].y() > north)
					north = curFrustumPoints[i].y();
				if(curFrustumPoints[i].x() < west)
					west = curFrustumPoints[i].x();
				if(curFrustumPoints[i].x() > east)
					east = curFrustumPoints[i].x();
			}		

			osg::ref_ptr<osg::Group> rGroup = new osg::Group();
			GetTileNode(rGroup.get(),west,east,north,south);
			GetTerrainMaskLayer(rGroup.get());

			return GetIntersectPointWithTerrain(radius,end,rGroup.get(),safePoint);
		}

		return false;
	}

	bool FreeEarthManipulatorBase::CheckCollisionWithCheckPoint(osg::Vec3d& start, osg::Vec3d& end, osg::Vec3d& safePoint, osg::Vec3d& checkPoint, double checkDis /*= 5.0*/)
	{
		if (m_opViewer.valid())
		{
			osg::Camera* cam = m_opViewer->getCamera();
			if (cam == NULL)
				return false;
			osg::Matrixd projMat = cam->getProjectionMatrix();

			double fov, aspect, dCurNear, tmpNearFar;
			projMat.getPerspective(fov, aspect, dCurNear, tmpNearFar);
			fov = osg::DegreesToRadians(fov);

			osg::Vec3d curLook, curLeft, curUp;
			osg::Matrixd curMat = GetBaseMatrix();
			curLook.set(-curMat(2, 0), -curMat(2, 1), -curMat(2, 2));
			curLook.normalize();
			curLeft.set(curMat(0, 0), curMat(0, 1), curMat(0, 2));
			curLeft.normalize();
			curUp.set(curMat(1, 0), curMat(1, 1), curMat(1, 2));
			curUp.normalize();

			double curTop = dCurNear * tan(fov / 2.0);
			double curRight = curTop * aspect;

			osg::Vec3d curFrustumPoints[5];
			curFrustumPoints[0] = checkPoint;
			curFrustumPoints[1] = checkPoint + curLook * dCurNear - curLeft * curRight - curUp * curTop;
			curFrustumPoints[2] = checkPoint + curLook * dCurNear + curLeft * curRight - curUp * curTop;
			curFrustumPoints[3] = checkPoint + curLook * dCurNear + curLeft * curRight + curUp * curTop;
			curFrustumPoints[4] = checkPoint + curLook * dCurNear - curLeft * curRight + curUp * curTop;

			double radius = 2.0;//(curFrustumPoints[1] - end).length() + 0.5;

			double west = 180;
			double east = -180;
			double north = -90;
			double south = 90;
			for (int i = 0; i < 5; i++)
			{
				FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(), curFrustumPoints[i], curFrustumPoints[i]);
				if (curFrustumPoints[i].y() < south)
					south = curFrustumPoints[i].y();
				if (curFrustumPoints[i].y() > north)
					north = curFrustumPoints[i].y();
				if (curFrustumPoints[i].x() < west)
					west = curFrustumPoints[i].x();
				if (curFrustumPoints[i].x() > east)
					east = curFrustumPoints[i].x();
			}

			osg::ref_ptr<osg::Group> rGroup = new osg::Group();
			GetTileNode(rGroup.get(), west, east, north, south);

			return GetIntersectPointWithTerrain(radius, start, end, rGroup.get(), safePoint);
		}

		return false;
	}

	double FreeEarthManipulatorBase::GetAltFromTerrain(osg::Vec3d vecPos)
	{
		osg::Vec3d vecStart(0,0,0);
		osg::Vec3d vecRayDir = vecPos;
		vecRayDir.normalize();

		double dDis = 6378137.0 * 3.0;
		osg::Vec3d vecEnd = vecRayDir * dDis;

		osg::Vec3d vecHitPoint, vecHitNormal;
		bool flag = RayToWorld(vecStart, vecEnd, vecHitPoint, vecHitNormal);

		double dAlt = 0;

		if(flag)
		{
			dAlt = vecPos.length() - vecHitPoint.length();
		}
		else
		{
			osg::Vec3d vecCamPosLLH;
			FeUtil::XYZ2DegreeLLH(m_opRenderContext.get() , vecPos, vecCamPosLLH);

			dAlt = vecCamPosLLH.z();
		}

		return dAlt;
	}

	bool FreeEarthManipulatorBase::RayToWorld( osg::Vec3d start,osg::Vec3d end,osg::Vec3d& outPoint,osg::Vec3d& hitNormal )
	{
		if(m_opMapNode.valid())
		{
			osgEarth::Terrain* te = m_opMapNode->getTerrain();
			if(!te)
				return false;

			osg::NodePath nodePath;
			nodePath.push_back(te->getGraph());
			if ( nodePath.empty() )
				return false;

			osg::ref_ptr<osgUtil::LineSegmentIntersector> picker = NULL;

			picker = new osgEarth::DPLineSegmentIntersector(osgUtil::Intersector::MODEL, start, end);	

			osgUtil::IntersectionVisitor iv(picker.get());
			nodePath.back()->accept(iv);

			if ( picker->containsIntersections() )
			{
				osgUtil::LineSegmentIntersector::Intersections& results = picker->getIntersections();
				outPoint = results.begin()->getWorldIntersectPoint();
				hitNormal = results.begin()->getWorldIntersectNormal();

				return true;
			}
		}

		return false;
	}


	osg::Matrixd FreeEarthManipulatorBase::GetBaseMatrix() const
	{
		osg::Matrixd m;

		return m;
	}


	void FreeEarthManipulatorBase::GetTileNode( osg::Group* pGroup,double west,double east,double north,double south )
	{
		if(m_opViewer == NULL)
			return;

		osg::Camera* cam = m_opViewer->getCamera();
		if(cam == NULL)
			return;

		osgEarth::Drivers::MPTerrainEngine::TerrainNode* tNode = osgEarth::findRelativeNodeOfType<osgEarth::Drivers::MPTerrainEngine::TerrainNode>( cam );
		if(tNode && tNode->getNumChildren() > 0 && tNode->getChild(0))
		{
			osg::Vec3d camPos,tmp;
			cam->getViewMatrix().getLookAt(camPos,tmp,tmp);
			TileNodeVisitor nv(camPos,pGroup,west,east,north,south);
			tNode->getChild(0)->accept(nv);
		}
	}

	bool FreeEarthManipulatorBase::GetIntersectPointWithTerrain( double radius,osg::Vec3d positon,osg::Group* tileGroup,osg::Vec3d& safePoint )
	{
		bool hit = false;

		if(tileGroup)
		{
			m_triangleSelector.ConstructTriangleOctree(*tileGroup);

			osg::Vec3d radVec(radius,radius,radius);
			osg::Vec3d newStart = positon;
			newStart.normalize();
			newStart = newStart * osg::WGS_84_RADIUS_EQUATOR * 3.0;
			osg::Vec3d direction = positon - newStart;
			FeMath::triangle3d outTriangle;
			osg::Vec3d hitPosition;
			bool isFalling;
			double slidingSpeed = 0.01;
			osg::Vec3d gravity(0,0,0);

			safePoint = m_collisionManager.getCollisionResultPosition(&m_triangleSelector,newStart,radVec,
				direction,outTriangle,hitPosition,
				isFalling,slidingSpeed,gravity,hit);
		}

		return hit;
	}

	bool FreeEarthManipulatorBase::GetIntersectPointWithTerrain(double radius, osg::Vec3d start, osg::Vec3d end, osg::Group* tileGroup, osg::Vec3d& safePoint)
	{
		bool hit = false;

		if (tileGroup)
		{
			m_triangleSelector.ConstructTriangleOctree(*tileGroup);

			osg::Vec3d radVec(radius, radius, radius);
			osg::Vec3d direction = end - start;
			FeMath::triangle3d outTriangle;
			osg::Vec3d hitPosition;
			bool isFalling;
			double slidingSpeed = 0.01;
			osg::Vec3d gravity(0, 0, 0);

			safePoint = m_collisionManager.getCollisionResultPosition(&m_triangleSelector, start, radVec,
				direction, outTriangle, hitPosition,
				isFalling, slidingSpeed, gravity, hit);
		}

		return hit;
	}

	void FreeEarthManipulatorBase::GetTerrainMaskLayer( osg::Group* pGroup )
	{
		if(m_opRenderContext.valid() && m_opMapNode.valid() && pGroup)
		{
			osgEarth::Map* map = m_opMapNode->getMap();
			if(map)
			{
				osgEarth::MaskLayerVector mv;
				map->getTerrainMaskLayers(mv);

				if(mv.empty())
					return;

				if(!m_rpMaskNodeGroup.valid())
				{
					m_rpMaskNodeGroup = new osg::Group();

					for (osgEarth::MaskLayerVector::iterator it = mv.begin(); it != mv.end(); it++)
					{
						if((*it).valid())
						{
							osg::Vec3dArray* va = (*it)->getOrCreateMaskBoundary(1.0,m_opMapNode->getMapSRS(),NULL);
							if(va)
							{
								osg::ref_ptr<osgEarth::Symbology::Polygon> pg = new osgEarth::Symbology::Polygon(&(va->asVector()));
								osg::ref_ptr<osgEarth::Features::Feature> ft = new osgEarth::Features::Feature(pg,m_opMapNode->getMapSRS());
								osg::ref_ptr<osgEarth::Annotation::FeatureNode> fn = new osgEarth::Annotation::FeatureNode(m_opMapNode.get(),ft);

								m_rpMaskNodeGroup->addChild(fn);
							}					
						}
					}
				}
				else
				{	
					osg::Camera* cam = m_opRenderContext->GetCamera();
					if(cam)
					{
						osg::Vec3d vecCameraPos,tmp;
						cam->getViewMatrixAsLookAt(vecCameraPos,tmp,tmp);

						osg::Matrixd projMat = cam->getProjectionMatrix();
						double fov, aspect, dCurNear, tmpNearFar;
						projMat.getPerspective(fov, aspect, dCurNear, tmpNearFar);

						osg::BoundingSphere camBS(vecCameraPos,dCurNear + 1);

						for (int i = 0;i < m_rpMaskNodeGroup->getNumChildren();i++)
						{
							osg::Node* cn = m_rpMaskNodeGroup->getChild(i);
							if(cn && camBS.intersects(cn->getBound()))
							{
								pGroup->addChild(cn);
							}
						}
					}
				}
			}
		}
	}
}
