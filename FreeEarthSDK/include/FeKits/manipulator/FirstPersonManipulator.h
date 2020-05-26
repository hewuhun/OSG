#ifndef FIRSTPERSONMANIPULATOR_UTIL_H
#define FIRSTPERSONMANIPULATOR_UTIL_H

#include <osgGA/CameraManipulator>
#include <FeKits/Export.h>
#include <FeUtils/RenderContext.h>
#include <FeUtils/IntersectUtil.h>
#include <FeUtils/logger/LoggerDef.h>

namespace FeKit
{

	class FEKIT_EXPORT FirstPersonManipulator : public osgGA::CameraManipulator
	{
		/*struct ModelProjMatCallback : public osg::CullSettings::ClampProjectionMatrixCallback
		{
		ModelProjMatCallback(FirstPersonManipulator* em);
		virtual bool clampProjectionMatrixImplementation(osg::Matrixf& projection, double& znear, double& zfar) const;
		virtual bool clampProjectionMatrixImplementation(osg::Matrixd& projection, double& znear, double& zfar) const;

		FirstPersonManipulator* m_pFirstPersonManipulator;
		};*/

		struct MyPathData
		{
			osg::Vec3 pathPosition;
			float     pathHead;
			float     pathTilt;
		};

	public:
		FirstPersonManipulator(FeUtil::CRenderContext* rc);
		~FirstPersonManipulator();

		/** set the position of the matrix manipulator using a 4x4 Matrix.*/
		virtual void setByMatrix(const osg::Matrixd& matrix);

		/** set the position of the matrix manipulator using a 4x4 Matrix.*/
		virtual void setByInverseMatrix(const osg::Matrixd& matrix);

		/** get the position of the manipulator as 4x4 Matrix.*/
		virtual osg::Matrixd getMatrix() const;

		/** get the position of the manipulator as a inverse matrix of the manipulator, typically used as a model view matrix.*/
		virtual osg::Matrixd getInverseMatrix() const;

		virtual void home(const osgGA::GUIEventAdapter& ,osgGA::GUIActionAdapter&);

		virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

		virtual void updateCamera(osg::Camera& camera);

		void setHomePosition(osg::Vec3d spherePos,float degreeHead = 0,float degreeTilt = 0);

		bool init(osg::Node* modelNode,std::string octFileName = "");

		FeUtil::CRenderContext* getContext()
		{
			return m_opRenderContext.get();
		}
		double getNearClip()
		{
			return m_nearClip;
		}
		double getFarClip()
		{
			return m_farClip;
		}
		void SetSavePath(std::string strPath)
		{
			m_strSavePath = strPath;
		}
	private:
		void setByLookAt(const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up);
		osg::Matrixd getRotation(const osg::Vec3d& center);
		osg::Vec3d getLocalNorth(osg::Vec3d c);
		osg::Vec3d getLocalSide(osg::Vec3d c);
		void updateCamera();
		void updateLookDir();
		void updatePosition();
		void checkClampProjectionMatrixCallback();
		void resetCamera();

		void startSavePath();
		bool savePathToFile(std::string fileName);
		bool readPathFromFile(std::string fileName);
		void playSavePath();
		void modifyNearFar();
		void getHomePositionFromCam();
	private:
		osg::Matrix             m_viewMat;
		osg::Vec3d              m_center;
		osg::Quat               m_rotation;
		osg::Quat               m_centerRotation;
		osg::observer_ptr<FeUtil::CRenderContext> m_opRenderContext; 
		FeMath::COctreeTriangleSelector m_triangleSelector;
		FeMath::CSceneCollisionManager m_collisionManager;
		bool                    m_initialized;
		bool                    m_initLocalCenter;
		double                  m_nearClip;
		double                  m_farClip;
		unsigned int            m_stepPerMove;
		unsigned int            m_curStep;
		osg::Vec3d              m_curMoveDir;
		bool                    m_needJump;
		unsigned int            m_jumpStep;

		bool                    m_left;
		bool                    m_right;
		bool                    m_up;
		bool                    m_down;

		float                   m_head;
		float                   m_tilt;

		float                   m_homeHead;
		float                   m_homeTilt;
		osg::Vec3d              m_homeCenter;
		osg::Vec3d              m_forwardDir;
		osg::Vec3d              m_sideDir;

		float                   m_lastX;
		float                   m_lastY;
		float                   m_dirRate;
		float                   m_speedRate;

		osg::Vec3d              m_radius;
		bool                    m_isFalling;
		float                   m_slidingSpeed;
		osg::Vec3d              m_gravity;

		osg::Matrix             m_transMat;
		osg::Vec3d              m_localNorth;
		osg::Vec3d              m_localSide;
		osg::Vec3d              m_localUp;
		osg::Vec3d              m_localCenter;
		osg::Vec3d              m_localInitUp;

		float                   m_heightOffset;
		osg::Vec3d              m_eyePos;
		bool                    m_pathSaving;
		bool                    m_pathPlaying;
		unsigned int            m_curSavePathStep;
		typedef std::vector<MyPathData> SavePathList;
		SavePathList            m_savePathList;
		unsigned int            m_maxSavePathSize;

		std::string				m_strSavePath;
	};

}

#endif //MANIPULATOR_UTIL_H
