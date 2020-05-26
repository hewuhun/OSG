/**************************************************************************************************
* @file EntityTrackEffect.h
* @note 实体尾迹类
* @author L00099
* @data 2017-3-9
**************************************************************************************************/
#ifndef EFFECT_TRACK_EFFECT_H
#define EFFECT_TRACK_EFFECT_H

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

#include <osg/ShapeDrawable>


namespace FeEffect
{
	class FEEFFECTS_EXPORT CEntityTrackEffect : public FeEffect::CFreeEffect
	{
	public:
		CEntityTrackEffect(osg::MatrixTransform * pMt);
		~CEntityTrackEffect();

	public:

		bool CreateEffect();

		void UpdateTrackLine();

	private:
		osg::observer_ptr<osg::Geometry>  m_trackLineGeometry;
		osg::Geode                       *m_trackLineGeode;
		osg::DrawArrays*  m_pLineDraw; 
		osg::observer_ptr<osg::MatrixTransform> m_opTrackMt;
		osg::observer_ptr<osg::MatrixTransform> m_opMt;
		int m_nCount;

	};

	class  CTrackEffectCallback : public osg::NodeCallback
	{
	public:
		CTrackEffectCallback();

		~CTrackEffectCallback();

	protected:
		virtual void operator()(osg::Node *pNode,osg::NodeVisitor *pNv);
	};

}
#endif