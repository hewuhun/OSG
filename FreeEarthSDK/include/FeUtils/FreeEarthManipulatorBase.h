
#ifndef FREE_UTILS_EARTHMANIPULATORBASE
#define FREE_UTILS_EARTHMANIPULATORBASE

#include <FeUtils/Export.h>
#include <osgViewer/Viewer>
#include <osg/CullSettings>
#include <FeUtils/IntersectUtil.h>
#include <FeShell/SystemService.h>
#include <osgEarth/MapNode>

namespace FeUtil
{
	class FEUTIL_EXPORT FreeEarthManipulatorBase
	{
		struct  EarthProjMatCallback : public osg::CullSettings::ClampProjectionMatrixCallback
		{
			EarthProjMatCallback(FreeEarthManipulatorBase* f);
			virtual bool clampProjectionMatrixImplementation(osg::Matrixf& projection, double& znear, double& zfar) const;
			virtual bool clampProjectionMatrixImplementation(osg::Matrixd& projection, double& znear, double& zfar) const;

		private:
			FreeEarthManipulatorBase* m_pFreeEarthManipulatorBase;
		};

		class TileNodeVisitor : public osg::NodeVisitor
		{
		public:
			TileNodeVisitor(osg::Group* pGroupNode,double west,double east,double north,double south);

			virtual void apply(osg::MatrixTransform& node);
			virtual void apply(osg::PagedLOD& node);

			osg::Group* m_pGroupNode;
			double m_west,m_east,m_north,m_south; 
		};

	public:
		FreeEarthManipulatorBase(osgViewer::Viewer* v,FeUtil::CRenderContext *pRenderContext);
	public:
		osgViewer::Viewer* getViewer();
		void modifyNearFar(double& inputNear);
		void setMapNode(osgEarth::MapNode* mn);
		bool checkCollision(osg::Vec3d& start, osg::Vec3d& end, osg::Vec3d& safePoint, osg::Vec3d& upPoint,double checkDis = 5.0);	
	protected:
		bool getCurNear(double curAlt,double& outNear);
		double getAltFromTerrain(osg::Vec3d p);
		bool rayToWorld(osg::Vec3d start,osg::Vec3d end,osg::Vec3d& outPoint,osg::Vec3d& hitNormal);
		virtual osg::Matrixd getBaseMatrix() const;
		void getTileNode(osg::Group* pGroup,double west,double east,double north,double south);
		bool getIntersectPointWithTerrain(double radius,osg::Vec3d positon,osg::Group* tileGroup,osg::Vec3d& safePoint);
	protected:
		osg::ref_ptr<EarthProjMatCallback> m_rpProjCallback;
		osg::observer_ptr<osgViewer::Viewer> m_opViewer;
		osg::observer_ptr<osgEarth::MapNode> m_opMapNode;
		FeMath::COctreeTriangleSelector m_triangleSelector;
		FeMath::CSceneCollisionManager m_collisionManager;
		osg::observer_ptr<FeUtil::CRenderContext>	m_opRenderContext;;
	};
}

#endif // FREE_UTILS_EARTHMANIPULATOR
