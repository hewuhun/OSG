
#ifndef FE_PARALLELSPHERE_H
#define FE_PARALLELSPHERE_H

#include <osg/Node>
#include <osg/Geode>
#include <osg/Geometry>

#include <FeKits/KitsCtrl.h>
#include <FeKits/Export.h>

namespace FeKit
{
    /**
    * @class CParallelSphere
    * @brief 
    * @note 
    * @author c00005
    */
    class FEKIT_EXPORT CCelestialReference : public osg::Group, public CKitsCtrl
    {
    public:
        CCelestialReference(FeUtil::CRenderContext* pContext);

		CCelestialReference(FeUtil::CRenderContext* pContext,
			double dRadius,
			double dLineWidth = 1.0,
			osg::Vec4 colorFill = osg::Vec4(1.0, 1.0, 0.0, 0.1),
			osg::Vec4 colorLine = osg::Vec4(1.0, 1.0, 0.0, 0.9));

		virtual ~CCelestialReference(void);

    public:
        /**
        *@note: 显隐控制所有的工具
        */
        virtual void Show();

        virtual void Hide();

	public:
		///设置赤道状态
		void SetEquatorVisible(bool bVisible);

		///设置赤道状态
		void SetEclipticVisible(bool bVisible);

		///设置赤道状态
		void SetAxisVisible(bool bVisible);

    protected:

		void CreateUnit();

		osg::Node* CreateEquator();

		osg::Node* CreateEcliptic();

		osg::Node* CreateAxis();

	private:
		double			m_dRadius;
		double			m_dLineWidth;
		osg::Vec4       m_colorFill;
		osg::Vec4		m_colorLine;

		osg::observer_ptr<osg::Node>      m_opEquator;  //赤道平面
		osg::observer_ptr<osg::Node>      m_opEcliptic;  //黄道平面
		osg::observer_ptr<osg::Node>      m_opAxis;  //地轴

    };
}

#endif //FE_MINILOGO_H
