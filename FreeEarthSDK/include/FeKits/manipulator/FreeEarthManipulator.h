/**************************************************************************************************
* @file FreeEarthManipulator.h
* @note FreeEarth地球操作器
* @author 
* @data 2017-2-14
**************************************************************************************************/
#ifndef FREE_EARTH_MANIPULATOR_H
#define FREE_EARTH_MANIPULATOR_H

#include <FeKits/Export.h>
#include <FeUtils/RenderContext.h>

#include <osg/AutoTransform>
#include <osg/CullSettings>

#include <FeKits/manipulator/PreEarthManipulator.h>
#include <FeKits/manipulator/FreeEarthManipulatorBase.h>

namespace FeKit
{
	class FreeEarthManipulator;

	struct CameraUpdateCallback : public osg::NodeCallback
	{
		CameraUpdateCallback(FreeEarthManipulator* m);
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
		FreeEarthManipulator* m_pEarthManipulator;
	};

	struct FreeEarthManipTerrainCallback : public TerrainCallback
	{
		FreeEarthManipTerrainCallback(FreeEarthManipulator* manip) : m_pEarthManipulator(manip) { }
		void onTileAdded( const TileKey& key, osg::Node* tile, TerrainCallbackContext& context );
		FreeEarthManipulator* m_pEarthManipulator;
	};

	class FEKIT_EXPORT FreeEarthManipulator : public PreEarthManipulator, public FreeEarthManipulatorBase
	{	
		friend struct EarthProjMatCallback;
		friend struct CameraUpdateCallback;
		friend struct FreeEarthManipTerrainCallback;

	public:
		FreeEarthManipulator(FeUtil::CRenderContext* pContext);

		virtual ~FreeEarthManipulator();

		virtual const char* className() const { return "FreeEarthManipulator"; }

	private:
		bool				m_bGoogleModeEnable;     // 判断是否是google 模式

		osg::Vec3d			m_v3dLastCenter;	  // 记录中心点
		osg::Quat			m_qLastQuat;		  // 记录四元素

		//Pan 使用
		bool				m_bLastPanIntersected;
		osg::Vec3d			m_v3dLastPanIntersect;
		bool				m_bPanRelease;
		bool				m_bEnableThrowing;
		osg::ref_ptr<osg::Camera> m_rpCameraCopy;

		bool				m_bIsRotateChecked;
		bool				m_bLastPanSkyIntersected;
		bool				m_bLastZoomIntersected;
		bool				m_bLastRotateIntersected;

		osg::Vec3d			m_v3dLastZoomIntersect;
		osg::Vec3d			m_v3dLastRotateIntersect;
		mutable bool		m_bLastIntersectWithTerrain;
		osg::ref_ptr<Settings> m_settings;
		unsigned int		m_uRotateSensitivity;

		osg::Vec3d			m_v3dOldCenter;
		osg::Quat			m_qOldCenterRotation;
		osg::Quat			m_qOldRotation;
		double				m_dOldDistance;

		bool				m_bCurIntersectWithEarth;
		osg::Vec3d			m_v3dCurIntersectWithEarth;
		osg::Vec3d			m_v3dLastPanAxis;
		double				m_dLastPanAngle;
		double				m_dMaxZoomDis;
		double				m_dLastZoomDis;	
		bool				m_bIsRotating;
		bool				m_bIsZooming;

		double				m_billboardScale;

		osg::ref_ptr<FreeEarthManipTerrainCallback> m_rpTerrainCallback;
		osg::ref_ptr<osg::AutoTransform> m_billboardMT;
		osg::ref_ptr<osg::Billboard> m_billboard;

		// 单个函数中使用
		float				m_fLastX;
		float				m_fLastY;      
		double				m_dScrollDuration;
		double				m_dSkyPlaneDis;

	protected:
		virtual bool  handlePointAction (const Action &type, float mx, float my, osg::View *view);

		virtual void  handleMovementAction (const ActionType &type, double dx, double dy, osg::View *view);			

		virtual bool  serviceTask(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

		virtual void  updateSetViewpoint();

		virtual void  mousePushEvent(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, const Action& action);

		virtual void  mouseThrowingEvent(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

		virtual void  mouseReleaseEvent(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

		void  configureDefaultSettings();

		virtual void recalculateCenter( const osg::CoordinateFrame& frame, osg::Node* tile = NULL);

		// 放大缩小
		void zoom( double dx, double dy, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

		void zoomToPoint(double dx, double dy);

		// 拖动
		void panPush(float x, float y);

		void panAngleAixs(osg::Vec3d lp, osg::Vec3d cp,bool checkPan = false);

		void panThrown(double rate);

		// 天空
		bool getIntersectPointFromSky(float x,float y,osg::Camera* cam,osg::Vec3d& outPoint);

		// onTileAdded调用
		bool RayToWorld(osg::Vec3d start,osg::Vec3d end,osg::Vec3d& outPoint,osg::Vec3d& hitNormal);

		void storeParameter();
		void restoreParameter();

		// 设置视点调用 setViewpoint
		void setGoogleModeViewPoint(osgEarth::Viewpoint& v);
		bool getOldModeViewPoint(osgEarth::Viewpoint& v); // handlePointAction 调用

		//handlePointAction 调用 
		void updateEarthIntersectPoint(double radius);
		void rotate( double dx, double dy ,const osg::Vec3d& center);

		// 标牌
		osg::Drawable* createSquare(const osg::Vec3d& corner,const osg::Vec3d& width,const osg::Vec3d& height, osg::Image* image=NULL);
		void updateBillboard(osg::Vec3d corner);

		///实现osgEarth定位功能
	protected:
		optional<Viewpoint>     _setVP0;                    // Starting viewpoint
		optional<Viewpoint>     _setVP1;                    // Final viewpoint
		Duration                _setVPDuration;             // Transition time for setViewpoint
		double                  _setVPAccel, _setVPAccel2;  // Acceleration factors for setViewpoint
		double                  _setVPArcHeight;            // Altitude arcing height for setViewpoint

		osg::Quat               _tetherRotationVP0;         // saves _tetherRotation at the start of a transition
		osg::Quat               _tetherRotationVP1;         // target _tetherRotation if not tethered

		virtual bool isSettingViewpoint()
		{ 
			return _setVP0.isSet() && _setVP1.isSet() && !_setVP1->nodeIsSet();
		}

		virtual double setViewpointFrame(double time_s);

	public:
		virtual void setViewpoint( const osgEarth::Viewpoint& vp, double duration_s =0.0,bool testFlag = false );
		virtual void setByMatrix(const osg::Matrixd& matrix);
		virtual void setByLookAt(const osg::Vec3d& eye, const osg::Vec3d& lv, const osg::Vec3d& up);
		virtual void pan( double dx, double dy );
		virtual void zoom( double dx, double dy );
		virtual void rotate( double dx, double dy );

		// 导航条调用
		void rotateWithCamCenter( double dx, double dy );
		void panThroughCtrl(double dx, double dy);
		void getPanAxis(osg::Vec3d& up,osg::Vec3d& north,osg::Vec3d& right);

		// 鸟瞰图定位调用
		void flyToViewpoint(const Viewpoint& vp, double dTime = 2.0);

		FeUtil::CRenderContext* GetRenderContext(){return m_opRenderContext.get();}

	protected:
		virtual bool  screenToWorldHitTerrian(float x, float y, osg::View* view, osg::ref_ptr<osg::Camera>& out_cam, osg::Vec3d& out_coords,bool& hitTerrain) const;
		virtual bool  screenToWorld(float x, float y, osg::View* view, osg::Vec3d& out_coords ,bool& hitTerrain) const;	
		bool screenToWorldWithIntersector(float x, float y, osg::View* theView,osg::ref_ptr<osg::Camera>& out_cam,  osg::Vec3d& out_coords) const;
		bool screenToRay(float x, float y, osg::ref_ptr<osg::Camera>& cam, osg::Vec3d& start,osg::Vec3d& end) const;
		bool screenToMaskNodeIntersector(float x, float y, osg::View* theView, osg::Vec3d& out_coords) const;

		virtual void modifyFirstPersonNearFar();
	};
}

#endif // FREE_UTILS_EARTHMANIPULATOR
