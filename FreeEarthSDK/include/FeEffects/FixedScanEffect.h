/**************************************************************************************************
* @file FixedScanEffect.h
* @note 凝视相机
* @author g00037
* @data 2017-2-24
**************************************************************************************************/
#ifndef FIXED_SCAN_EFFECT_H
#define FIXED_SCAN_EFFECT_H
#include <iostream>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/NodeCallback>
#include <osg/Drawable>
#include <osg/MatrixTransform>
#include <FeEffects/Export.h>
#include <FeEffects/FreeEffect.h>


using namespace osg;

namespace FeEffect
{
	class CUpdateScanCallback;

#define MAXNUMLINEWAVE 10
	class FEEFFECTS_EXPORT CNewFixedScanEffect : public FeEffect::CFreeEffect
	{
		friend class CUpdateScanCallback;
	public:
		CNewFixedScanEffect(FeUtil::CRenderContext* pRenderContext,osg::MatrixTransform *pMT);
        bool CreateEffect();
		void SetLRAngle(double fAngle);
		void SetFBAngle(double fAngle);
		double GetLRAngle();
		double GetFBAngle();
		void SetLenght(double dLenght);
		double GetLenght();
		///通过设置凝视相机的扫描角和步进角，从而移动凝视扫描波
		void SetRotate(osg::Vec3d posture);
		void SetRotate(osg::Matrix mat);
		void updataLength();
		/**
        * @brief 更新凝视相机
        * @note  设置完成各种参数后，调用更新完成；
        */
		virtual bool UpdateEffect();

		void SetFaceColor(osg::Vec4d vecColor);

		void SetLineColor(osg::Vec4d vecColor);

		osg::Vec4d GetFaceColor();

		osg::Vec4d GetLineColor();

		double GetRectWidth();

		double GetRectHeight();

		osg::Vec3d GetLeftUp();

		osg::Vec3d GetRightUp();

		osg::Vec3d GetRightDown();

		osg::Vec3d GetLeftDown();

		osg::Vec3Array* GetVertex();
	protected:
		osg::Vec3d GetLLHByLL(osg::Vec3d vecLLH);
		osg::Vec3d GetWorldXYZByLLH(osg::Vec3d vecLLH);
		osg::Vec3d GetLLHByLocalXYZ(osg::Vec3d vecXYZ);
        osg::Vec3d GetWorldXYZByHit(osg::Vec3d vecXYZ,osg::Vec3d vecCenter);
		osg::Vec3d GetLocalXYZByWorldXYZ(osg::Vec3d vecXYZ);
		osg::Vec3d GetWorldXYZByLocalXYZ(osg::Vec3d vecXYZ);
		osg::Vec3d GetHitLLHByLocalXYZ(osg::Vec3d vecXYZ,osg::Vec3d vecCenter);
		osg::Vec3d GetLLHByWorldXYZ(osg::Vec3d vecXYZ);
		void createNode();
		void updateUniform(osg::NodeVisitor* nv);
		void PushBackPoint(osg::Geometry* pGeometry, osg::Vec3d vecStart, osg::Vec3d vecStop,osg::Vec3 vecCenter);
		void UpdatePoint(osg::Geometry* pGeometry, osg::Vec3d vecStart, osg::Vec3d vecStop,osg::Vec3 vecCenter);
		void PushBackOutLine(osg::Vec3Array* vecArray, osg::Vec3d vecStart, osg::Vec3d vecStop,osg::Vec3 vecCenter);
	private:
		int                                             m_nNum;
		double                                          m_dLenght;
		double                                          m_dHeight;
		double                                          m_dRectWidth;
		double                                          m_dRectHeight;
		double                                          m_dLRAngle;
		double                                          m_dFBAngle;
		osg::Vec3d                                      m_vecLastXYZ;
		osg::Vec4d                                      m_vecFaceColor;
		osg::Vec4d                                      m_vecLineColor;
		osg::observer_ptr<osg::Geode>                   m_opGeode;
		osg::observer_ptr<osg::Geometry>                m_opLeftFace;
		osg::observer_ptr<osg::Geometry>                m_opRightFace;
		osg::observer_ptr<osg::Geometry>                m_opFrontFace;
		osg::observer_ptr<osg::Geometry>                m_opBackFace;
		osg::observer_ptr<osg::Geometry>                m_opOutLine;
		osg::ref_ptr<CUpdateScanCallback>               m_rpUpdateCallback;
		osg::ref_ptr<osg::MatrixTransform>              m_rpMT;//自身矩阵
		osg::observer_ptr<osg::MatrixTransform>			m_opMT;//父矩阵
		osg::observer_ptr<FeUtil::CRenderContext>		m_opRenderContext;
		osg::Vec3d                                      m_vecLeftUp;
	    osg::Vec3d                                      m_vecRightUp;
     	osg::Vec3d                                      m_vecRightDown;
		osg::Vec3d                                      m_vecLeftDown;
		osg::ref_ptr<osg::CoordinateSystemNode>         m_rpEarth;
		osg::Vec3Array*                                 m_pVertex;
	};


	class CUpdateScanCallback : public osg::NodeCallback
	{
	public:
		CUpdateScanCallback(CNewFixedScanEffect* pFixedScan);
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
	protected:
		osg::observer_ptr<CNewFixedScanEffect>                  m_opFixedScan;
		int                                                     m_nCount;
	};
}
#endif //FIXED_SCAN_EFFECT_H
