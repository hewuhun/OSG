#ifndef FELOCKNODEMANIPULATOR_H
#define FELOCKNODEMANIPULATOR_H

#include <osgGA/CameraManipulator>
#include <FeKits/Export.h>
#include <FeUtils/RenderContext.h>
#include <FeUtils/logger/LoggerDef.h>
#include <FeKits/manipulator/FreeEarthManipulatorBase.h>

namespace FeKit
{
	class FEKIT_EXPORT FeLockNodeManipulator : public osgGA::CameraManipulator, FeKit::FreeEarthManipulatorBase
	{
	public:
		enum FEKIT_EXPORT LockType 
		{
			FIRSTPERSON    = 1<<0,
			THIRDPERSON    = 1<<1	
		};

		FeLockNodeManipulator(FeUtil::CRenderContext* rc);
		~FeLockNodeManipulator();

		virtual const char* className() const { return "FeLockNodeManipulator"; }

		/** set the position of the matrix manipulator using a 4x4 Matrix.*/
		virtual void setByMatrix(const osg::Matrixd& matrix){};

		/** set the position of the matrix manipulator using a 4x4 Matrix.*/
		virtual void setByInverseMatrix(const osg::Matrixd& matrix){};

		/** get the position of the manipulator as 4x4 Matrix.*/
		virtual osg::Matrixd getMatrix() const;

		/** get the position of the manipulator as a inverse matrix of the manipulator, typically used as a model view matrix.*/
		virtual osg::Matrixd getInverseMatrix() const;

		virtual void home(const osgGA::GUIEventAdapter& ,osgGA::GUIActionAdapter&);

		virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

		void updateCamera(osg::Camera& camera);

		void setThirdPersonHomeParam(double lockDis,float degreeHead = 0,float degreeTilt = 0);
		void setFirstPersonHomeParam(float degreeTilt = -90,float degreeHead = 0);

		virtual void setNode(osg::Node* pNode) { m_opLockNode = pNode; }

		virtual const osg::Node* getNode() const { return m_opLockNode.get(); }

		virtual osg::Node* getNode() { return m_opLockNode.get(); }

		void setLockNode(osg::Node* lockNode)
		{
			m_opLockNode = lockNode;
		}	

		osg::Node* getLockNode()
		{
			return m_opLockNode.get();
		}

		LockType getLockType()
		{
			return m_lockType;
		}
		void setLockType(LockType tp)
		{
			m_lockType = tp;
			resetCamera();
		}

		void resetCamera();

		void setFirstPersonCenterOffset(const osg::Vec3d& vecCenterOffset) { m_vecCenterOffset = vecCenterOffset; }

	protected:
		void computeViewMatrix();
		void computeLocalAxis(const osg::Vec3d& center,osg::Vec3d& northDir,osg::Vec3d& upDir,osg::Vec3d& rightDir);
		void checkClampProjectionMatrixCallback();
		void modifyNearFar(double& inputNear,double &inputFar);
		void modifyFirstPersonNearFar();
		osg::Node *RecursionNode(osg::Node *node, const std::string &strName); 

	protected:
		osg::observer_ptr<osg::Node> m_opLockNode;
		osg::Matrix m_viewMatrix;
		double m_minLockDis;
		double m_maxLockDis;
		LockType m_lockType;
		osg::Matrix m_worldTransMat;

		float m_homeHead;
		float m_homeTilt;
		double m_homeLockDis;

		float m_firstHomeHead;
		float m_firstHomeTilt;

		float m_curHead;
		float m_curTilt;
		double m_curLockDis;

		float m_lastHead;
		float m_lastTilt;
		double m_lastLockDis;

		osg::Vec3d m_lookDir;
		osg::Vec3d m_rightDir;
		osg::Vec3d m_upDir;

		float  m_lastX;
		float  m_lastY;
		float  m_dirRate;
		float  m_zoomRate;
		int    m_xDirParam;
		int    m_yDirParam;

		osg::Vec3d m_vecCenterOffset; 

	public:
		static double s_dRollRadianAngle;
	};

}

#endif //FELOCKNODEMANIPULATOR_H
