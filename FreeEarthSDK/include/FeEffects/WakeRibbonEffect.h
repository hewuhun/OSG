#ifndef FE_WAKERIBBON_EFFECT_H_
#define FE_WAKERIBBON_EFFECT_H_
/**************************************************************************************************
* @file WakeRibbonEffect.h
* @note 彩带尾迹效果
* @author z00013
* @data 2015-12-24
**************************************************************************************************/
#include <osg/Geometry>
#include <osg/Array>
#include <osg/Group>
#include <osg/Geode>

#include <FeEffects/FreeEffect.h>


namespace FeEffect
{
	class CWakeRibbonCallback;


	class FEEFFECTS_EXPORT CWakeRibbonEffect : public FeEffect::CFreeEffect
	{
		friend CWakeRibbonCallback;

	public:
		CWakeRibbonEffect();

		virtual ~CWakeRibbonEffect();

	public:
		virtual bool CreateEffect();

		virtual void SetVisible(bool bVisible);

		void ClearLine();

	public:
		void SetMatrixTransform(osg::MatrixTransform *pTransMT);

		void SetColor(osg::Vec4d vecColor);

		void SetCenter(osg::Vec3d vecCenter);

		void SetWidth( double dWidth );

		void SetVertexSize(int nNums);

	protected:
		osg::ref_ptr<osg::Geometry>							m_rpGeometry;
		osg::ref_ptr<osg::DrawArrays>						m_rpDrawArrays;
		osg::ref_ptr<osg::Vec3dArray>						m_rpVertexArray;
		osg::ref_ptr<osg::Vec4dArray>						m_rpColorArray;

		osg::ref_ptr<CWakeRibbonCallback>					m_rpCallback;

		osg::ref_ptr<osg::MatrixTransform>					m_rpSelfMt;

		osg::observer_ptr<osg::MatrixTransform>				m_opTransMT;
		osg::observer_ptr<osg::MatrixTransform>				m_opRotateMt;
		osg::observer_ptr<osg::MatrixTransform>				m_opScaleMt;

		int													m_nSize;
		double												m_dWidth;
		osg::Vec3d											m_vecCenter;
		osg::Vec4d											m_vecColor;
		osg::Vec3d										    m_vecFirstPosition;
	};

}


namespace FeEffect
{
	class CWakeRibbonCallback : public osg::NodeCallback
	{
	public:
		CWakeRibbonCallback(CWakeRibbonEffect* pEffect);

		~CWakeRibbonCallback();

	protected:
		virtual void operator()(osg::Node *pNode,osg::NodeVisitor *pNv);

	protected:
		osg::observer_ptr<CWakeRibbonEffect>					m_opRibbonEffect;
	};
}


#endif //FE_WAKERIBBON_EFFECT_H
