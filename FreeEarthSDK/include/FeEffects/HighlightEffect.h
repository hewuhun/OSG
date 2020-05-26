#ifndef _FE_HIGHLIGHT_EFFECT_H_
#define _FE_HIGHLIGHT_EFFECT_H_

/**************************************************************************************************
* @file HighlightEffect.h
* @note 高亮效果
* @author z00013
* @data 2015-12-24
**************************************************************************************************/

#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ShapeDrawable>

#include <FeEffects/Export.h>
#include <FeEffects/FreeEffect.h>

namespace FeEffect
{
	class FEEFFECTS_EXPORT CHighlightEffect : public CFreeEffect
	{
	public:
		CHighlightEffect(double radius, osg::Vec4d color, bool grid=true, float radio = 0.8);
		~CHighlightEffect(void);

		virtual bool CreateEffect();

		void updateRadius(double radius);

		void SetColor(osg::Vec4d &vecColor);

		osg::Geometry* genCircle(double radius, osg::Vec4d color = osg::Vec4d(1.0,1.0,1.0,1.0));
		
	protected:
		osg::ref_ptr<osg::ShapeDrawable> m_pSphereDrawable;
		osg::ref_ptr<osg::Sphere> m_pSphere;
		double			m_dRadius;
		osg::Vec4d	m_vecColor;
		bool		m_blGrid;
	};

	class FEEFFECTS_EXPORT CHighlightCallback : public osg::NodeCallback
	{
	public:
		CHighlightCallback(osg::ShapeDrawable*);
		~CHighlightCallback();

	protected:
		virtual void operator()(osg::Node *pNode,osg::NodeVisitor *pNv);


	protected:
		osg::ref_ptr<osg::ShapeDrawable>								m_rpGeom;
		osg::Vec4d												m_vecColor;

	};
}


#endif //_FE_HIGHLIGHT_EFFECT_H_

