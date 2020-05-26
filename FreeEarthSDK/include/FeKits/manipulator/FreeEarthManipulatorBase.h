
#ifndef FREE_KITS_EARTHMANIPULATORBASE
#define FREE_KITS_EARTHMANIPULATORBASE

#include <osgViewer/Viewer>
#include <osg/CullSettings>
#include <osgEarth/MapNode>

#include <FeKits/Export.h>
#include <FeUtils/IntersectUtil.h>
#include <FeUtils/RenderContext.h>

namespace FeKit
{
	/**
	  * @class CCameraAltCallback
	  * @brief 相机高度获取回调
	  * @note 通过相机的位置与地面的碰撞检测，计算出相机距离地面的高度值
	  * @author 
	*/
	struct FEKIT_EXPORT CCameraAltCallback : public osgViewer::ViewerBase::UpdateCallback
	{
	public:
		CCameraAltCallback(FeUtil::CRenderContext* pRenderContext);

	public:
		virtual void customOperator(const osgViewer::ViewerBase* pViewerBase);

	private:
		osg::observer_ptr<FeUtil::CRenderContext> m_opRenderContext;
	};

	/** 
	* Callback for overriding the CullVisitor's default clamping of the projection matrix to computed near and far values.
	* Note, both Matrixf and Matrixd versions of clampProjectionMatrixImplementation must be implemented as the CullVisitor
	* can target either Matrix data type, configured at compile time.
	*/
	struct FEKIT_EXPORT CEarthClampProjectionMatrixCallback : public osg::CullSettings::ClampProjectionMatrixCallback
	{
	public:
		CEarthClampProjectionMatrixCallback(FeUtil::CRenderContext* pRenderContext);

	public:
		virtual bool clampProjectionMatrixImplementation(osg::Matrixf& projection, double& znear, double& zfar) const;

		virtual bool clampProjectionMatrixImplementation(osg::Matrixd& projection, double& znear, double& zfar) const;

	public:
		void SetUseCustomNearFar(bool bFlag);

		bool GetUseCustomNearFar();

		void SetCustomNear(double dNear);

		double GetCustomNear();

		void SetCustomFar(double dFar);

		double GetCustomFar();

		void SetMinNearClip(double dMinNearClip);

		double GetMinNearClip();

		FeUtil::CRenderContext* GetRenderContext();

	private:
		osg::observer_ptr<FeUtil::CRenderContext> m_opRenderContext;

		bool                                      m_bUserCustomNearFar;
		double                                    m_dCustomNear;
		double                                    m_dCustomFar;
		double                                    m_dMinNearClip;
	};
}

namespace FeKit
{
	class FEKIT_EXPORT FreeEarthManipulatorBase
	{	
	public:
		FreeEarthManipulatorBase(FeUtil::CRenderContext *pRenderContext);

	public:
		virtual void modifyNearFar(double& dInputNear,double &dInputFar);

		bool CheckCollision(osg::Vec3d& start, osg::Vec3d& end, osg::Vec3d& safePoint, osg::Vec3d& upPoint,double checkDis = 5.0);	

		bool CheckCollisionWithCheckPoint(osg::Vec3d& start, osg::Vec3d& end, osg::Vec3d& safePoint, osg::Vec3d& checkPoint, double checkDis = 5.0);

	protected:
		double GetAltFromTerrain(osg::Vec3d vecPos);

		virtual osg::Matrixd GetBaseMatrix() const;

		void GetTerrainMaskLayer(osg::Group* pGroup);

		bool RayToWorld(osg::Vec3d start,osg::Vec3d end,osg::Vec3d& outPoint,osg::Vec3d& hitNormal);

		void GetTileNode(osg::Group* pGroup,double west,double east,double north,double south);

		bool GetIntersectPointWithTerrain(double radius,osg::Vec3d positon,osg::Group* tileGroup,osg::Vec3d& safePoint);

		bool GetIntersectPointWithTerrain(double radius, osg::Vec3d start, osg::Vec3d end, osg::Group* tileGroup, osg::Vec3d& safePoint);

	protected:
		//class TileNodeVisitor : public osg::NodeVisitor
		//{
		//public:
		//	TileNodeVisitor(osg::Group* pGroupNode,double west,double east,double north,double south);

		//public:
		//	virtual void apply(osg::MatrixTransform& node);

		//	virtual void apply(osg::PagedLOD& node);

		//protected:
		//	osg::Group*		m_pGroupNode;
		//	double			m_dWest;
		//	double			m_dEast;
		//	double			m_dNorth;
		//	double			m_dSouth; 
		//};

		class TileNodeVisitor : public osg::NodeVisitor
		{
		public:
			TileNodeVisitor(osg::Vec3d vecCamPos, osg::Group* pGroupNode, double dWest,double dEast,double dNorth,double dSouth);

		public:
			virtual void apply(osg::MatrixTransform& node);

			virtual void apply(osg::PagedLOD& node);

			virtual float getDistanceToViewPoint(const osg::Vec3& pos, bool useLODScale) const;

		protected:
			osg::Group*			m_pGroupNode;
			double				m_dWest;
			double				m_dEast;
			double				m_dNorth;
			double				m_dSouth;
			osg::Vec3d			m_vecCamPos;
		};

	protected:
		osg::ref_ptr<osg::Group>					m_rpMaskNodeGroup;

		osg::observer_ptr<osgViewer::Viewer>		m_opViewer;
		osg::observer_ptr<osgEarth::MapNode>		m_opMapNode;
		osg::observer_ptr<FeUtil::CRenderContext>	m_opRenderContext;

		FeMath::COctreeTriangleSelector				m_triangleSelector;
		FeMath::CSceneCollisionManager				m_collisionManager;
	};
}

#endif // FREE_KITS_EARTHMANIPULATORBASE
