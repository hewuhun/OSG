
#include <FeUtils/FreeEarthManipulatorBase.h>
#include <osgEarthDrivers/engine_mp/TerrainNode>
#include <osgEarth/GeoData>
#include <osgEarth/GeoCommon>
#include <osgViewer/Renderer>
#include <FeEarth/FreeEarth.h>
#include <osgEarth/DPLineSegmentIntersector>
#include <FeUtils/CoordConverter.h>
#include <osgEarth/NodeUtils>

namespace FeUtil
{
	FreeEarthManipulatorBase::EarthProjMatCallback::EarthProjMatCallback(FreeEarthManipulatorBase* f)
		:m_pFreeEarthManipulatorBase(f)
	{
		if(m_pFreeEarthManipulatorBase)
		{
			if(m_pFreeEarthManipulatorBase->m_opViewer.valid())
			{
				osg::Camera* cam = m_pFreeEarthManipulatorBase->m_opViewer->getCamera();
				if(cam)
				{
					osgViewer::Renderer* r = dynamic_cast<osgViewer::Renderer*>(cam->getRenderer());
					if(r)
					{
						for(int i = 0;i < 2;i++)
						{
							osgUtil::SceneView* sv = r->getSceneView(i);
							if(sv)
							{
								osgUtil::CullVisitor* cv = sv->getCullVisitor();
								if(cv)
								{
									cv->setClampProjectionMatrixCallback(this);
								}
							}
						}
					}
				}	
			}			
		}	
	}

	template<class matrix_type>
	bool clampProjectionMatrix(matrix_type& projection, double& znear, double& zfar,FreeEarthManipulatorBase* f)
	{
		if(f == NULL)
			return false;

		if(f->getViewer() == NULL)
			return false;

		osg::Camera* cam = f->getViewer()->getCamera();
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

			f->modifyNearFar(znear);
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

			double min_near_plane = zfar*nearFarRatio;
			if (desired_znear<min_near_plane) desired_znear=min_near_plane;

			f->modifyNearFar(desired_znear);
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

	bool FreeEarthManipulatorBase::EarthProjMatCallback::clampProjectionMatrixImplementation(osg::Matrixf& projection, double& znear, double& zfar) const
	{
		return clampProjectionMatrix( projection, znear, zfar,m_pFreeEarthManipulatorBase);
	}
	bool FreeEarthManipulatorBase::EarthProjMatCallback::clampProjectionMatrixImplementation(osg::Matrixd& projection, double& znear, double& zfar) const
	{
		return clampProjectionMatrix( projection, znear, zfar,m_pFreeEarthManipulatorBase);
	}

	FreeEarthManipulatorBase::TileNodeVisitor::TileNodeVisitor(osg::Group* pGroupNode,double west,double east,double north,double south)
		: NodeVisitor(osg::NodeVisitor::NODE_VISITOR,osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
		, m_pGroupNode(pGroupNode)
		, m_west(west)
		, m_east(east)
		, m_north(north)
		, m_south(south)
	{
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
					osgEarth::Bounds bb(m_west,m_south,m_east,m_north);
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
	}

	
	FreeEarthManipulatorBase::FreeEarthManipulatorBase(osgViewer::Viewer* v,FeUtil::CRenderContext *pRenderContext)
		:m_opViewer(v)
		,m_opRenderContext(pRenderContext)
	{
		m_rpProjCallback = new EarthProjMatCallback(this);
	}


	osgViewer::Viewer* FreeEarthManipulatorBase::getViewer()
	{
		if(m_opViewer.valid())
			return m_opViewer.get();
		else
			return NULL;
	}

	bool FreeEarthManipulatorBase::getCurNear( double curAlt,double& outNear )
	{
		double alt = curAlt;
		double minNear = 1.0;
		double maxNear = 1000.0;
		double maxHeight = 100000.0;

		if(alt < maxHeight)
		{
			if(alt < 1000)
				outNear = 1.0;
			else
				outNear = alt * (maxNear - minNear)/maxHeight + minNear;

			if(outNear <= 1.0)
				outNear = 1.0;

			return true;
		}

		return false;
	}


	void FreeEarthManipulatorBase::modifyNearFar( double& inputNear )
	{
		if(m_opViewer.valid())
		{
			osg::Camera* cam = m_opViewer->getCamera();
			if(cam)
			{
				osg::Vec3d cp,tmp;
				cam->getViewMatrixAsLookAt(cp,tmp,tmp);
				double curNear;
				double curAlt = abs(getAltFromTerrain(cp));
				bool flag = getCurNear(curAlt,curNear);
				if(flag)
				{
					inputNear = curNear;
				}
			}		
		}		
	}


	void FreeEarthManipulatorBase::setMapNode( osgEarth::MapNode* mn )
	{
		m_opMapNode = mn;
	}


	bool FreeEarthManipulatorBase::checkCollision( osg::Vec3d& start, osg::Vec3d& end, osg::Vec3d& safePoint, osg::Vec3d& upPoint,double checkDis /*= 5.0*/ )
	{
		if(m_opViewer.valid())
		{
			double neaxAlt = getAltFromTerrain(end);
			double curNear;
			bool flag = getCurNear(neaxAlt,curNear);
			if(!flag)
				return false;

			osg::Camera* cam = m_opViewer->getCamera();
			if(cam == NULL)
				return false;
			osg::Matrixd projMat = cam->getProjectionMatrix();

			double fov,aspect,tmpNearFar;
			projMat.getPerspective(fov,aspect,tmpNearFar,tmpNearFar); 
			fov = osg::DegreesToRadians(fov);

			osg::Vec3d curLook,curLeft,curUp;
			osg::Matrixd curMat = getBaseMatrix();
			curLook.set(-curMat(2,0),-curMat(2,1),-curMat(2,2));
			curLook.normalize();
			curLeft.set(curMat(0,0),curMat(0,1),curMat(0,2));
			curLeft.normalize();
			curUp.set(curMat(1,0),curMat(1,1),curMat(1,2));
			curUp.normalize();

			double curTop = curNear * tan(fov / 2.0);
			double curRight = curTop * aspect;

			osg::Vec3d curFrustumPoints[5];
			curFrustumPoints[0] = end;
			curFrustumPoints[1] = end + curLook * curNear - curLeft * curRight - curUp * curTop;
			curFrustumPoints[2] = end + curLook * curNear + curLeft * curRight - curUp * curTop;
			curFrustumPoints[3] = end + curLook * curNear + curLeft * curRight + curUp * curTop;
			curFrustumPoints[4] = end + curLook * curNear - curLeft * curRight + curUp * curTop;

			double radius = 2.0;//(curFrustumPoints[1] - end).length() + 0.5;

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
			getTileNode(rGroup.get(),west,east,north,south);

			return getIntersectPointWithTerrain(radius,end,rGroup.get(),safePoint);
		}
		return false;
	}

	double FreeEarthManipulatorBase::getAltFromTerrain( osg::Vec3d p )
	{
		osg::Vec3d start(0,0,0);
		osg::Vec3d rayDir = p;
		rayDir.normalize();

		double dis = 6378137.0 * 3.0;
		osg::Vec3d end = rayDir * dis;

		osg::Vec3d hitPoint,hitNormal;
		bool flag = rayToWorld(start,end,hitPoint,hitNormal);

		double alt = 0;

		if(flag)
		{
			alt = p.length() - hitPoint.length();
		}
		else
		{
			osg::Vec3d camPosLLH;
			FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(),p, camPosLLH);

			alt = camPosLLH.z();
		}

		return alt;
	}

	bool FreeEarthManipulatorBase::rayToWorld( osg::Vec3d start,osg::Vec3d end,osg::Vec3d& outPoint,osg::Vec3d& hitNormal )
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


	osg::Matrixd FreeEarthManipulatorBase::getBaseMatrix() const
	{
		osg::Matrixd m;
		return m;
	}


	void FreeEarthManipulatorBase::getTileNode( osg::Group* pGroup,double west,double east,double north,double south )
	{
		if(m_opViewer == NULL)
			return;

		osg::Camera* cam = m_opViewer->getCamera();
		if(cam == NULL)
			return;

		osgEarth::Drivers::MPTerrainEngine::TerrainNode* tNode = osgEarth::findRelativeNodeOfType<osgEarth::Drivers::MPTerrainEngine::TerrainNode>( cam );
        if(tNode && tNode->getNumChildren() > 0 && tNode->getChild(0))
		{
			TileNodeVisitor nv(pGroup,west,east,north,south);
			tNode->getChild(0)->accept(nv);
		}
	}


	bool FreeEarthManipulatorBase::getIntersectPointWithTerrain( double radius,osg::Vec3d positon,osg::Group* tileGroup,osg::Vec3d& safePoint )
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

}
