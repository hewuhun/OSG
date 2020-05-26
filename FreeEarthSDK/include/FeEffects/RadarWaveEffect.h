#ifndef _FE_RADAR_WAVE_H__
#define _FE_RADAR_WAVE_H__
/**************************************************************************************************
* @file RadarWaveEffect.h
* @note 雷达波效果
* @author z00013
* @data 2015-12-24
**************************************************************************************************/
#include <FeEffects/SphereSegment.h>

#include <FeEffects/Export.h>

namespace FeEffect
{

	class WaveDrawableDrawCallback : public osg::Drawable::DrawCallback
	{
	public:
		WaveDrawableDrawCallback(osg::Vec4d vecColor,int density);
		~WaveDrawableDrawCallback();

	public:
		virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const;

		void SetChangeColor(bool bChange);
		void SetColor(osg::Vec4d &vecColor);
	private:

		bool m_bChangeColor;

		mutable int m_frameTiaoGuo;

		osg::Vec4d   m_vecColor;

		int			m_density;

	public:
		double		m_dTime;
	};
}

namespace FeEffect
{
	class FEEFFECTS_EXPORT CRadarWaveEffect : public CSphereSegment
	{
	public:
		CRadarWaveEffect(void);
		CRadarWaveEffect (const osg::Vec3 &centre, float radius, float azMin, float azMax, float elevMin, float elevMax, int density);
		CRadarWaveEffect (const osg::Vec3 &centre, float radius, const osg::Vec3 &vec, float azRange, float elevRange, int density);
		CRadarWaveEffect (const SphereSegment &rhs, const osg::CopyOp &co);

		~CRadarWaveEffect(void);

	protected:

		void Init();

	private:
		osg::ref_ptr<osg::Geometry>   m_geom;
		osg::Vec4d					  m_vecWaveColor;

	};

}
#endif //_FE_RADAR_WAVE_H__