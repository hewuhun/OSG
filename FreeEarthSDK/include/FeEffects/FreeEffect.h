#ifndef FREE_EFFECT_H
#define FREE_EFFECT_H

#include <osg/Group>

#include <FeEffects/Export.h>
#include <FeUtils/MiniAnimation.h>
#include <FeUtils/RenderContext.h>

namespace FeEffect
{
	class FEEFFECTS_EXPORT CFreeEffect : public osg::Group
	{
	public:
		CFreeEffect();
		virtual ~CFreeEffect();

		virtual bool CreateEffect() = 0;

		virtual bool UpdataEffect();

		virtual bool ClearEffect();
	public:
		virtual bool GetVisible() const;

		virtual void SetVisible(bool bVisible);

		virtual std::string GetKey() const;

		virtual void SetKey(const std::string& strKey);

		void SetShowOrHide(bool blShow);

		virtual bool StartEffect();

		virtual void SetType(unsigned int unType);

		virtual unsigned int GetType() const;

		virtual void SetRadius(double dRadius);

		virtual double GetRadius() const;

		virtual void SetCenter(osg::Vec3d vecCenter);

		virtual osg::Vec3d GetCenter() const;

		virtual void SetColor(osg::Vec4d vecColor);

		virtual osg::Vec4d GetColor() const;
	protected:
		bool            m_bVisible;
		std::string     m_strKey;
		unsigned int    m_unType;
		double			m_dRadius;
		osg::Vec3d		m_vecCenter;
		osg::Vec4d		m_vecColor;
	};
}

namespace FeEffect
{
	class FEEFFECTS_EXPORT CShootEffect : public CFreeEffect
	{
	public:
		CShootEffect();
		virtual ~CShootEffect();

		virtual bool CreateEffect();
	};
}

namespace FeEffect
{
	class FEEFFECTS_EXPORT CExplodeEffect : public CFreeEffect
	{
	public:
		CExplodeEffect();
		virtual ~CExplodeEffect();

		virtual bool CreateEffect();

		virtual void SetPos(osg::Vec3d vecPos);

	protected:
		osg::Vec3d    m_vecPos;
	};
}

namespace FeEffect
{
	class FEEFFECTS_EXPORT CShootRangeEffect : public CFreeEffect
	{
	public:
		CShootRangeEffect();
		virtual ~CShootRangeEffect();

		virtual bool CreateEffect();

	};
}

namespace FeEffect
{
	class FEEFFECTS_EXPORT CFreeRadarEffect : public CFreeEffect
	{
	public:
		CFreeRadarEffect();
		CFreeRadarEffect(const osg::Vec3& center, double dRadius, double dLoopTime ,
			float  azMin,  
			float  azMax,  
			float  elevMin,  
			float  elevMax);
		virtual ~CFreeRadarEffect();

		virtual bool CreateEffect();

	public:
		virtual osg::Vec3d GetCenter() const;
		virtual void SetCenter(const osg::Vec3d& center);

		virtual double GetRadius() const;
		virtual void SetRadius(double dRadius);

		virtual double GetLoopTime() const;
		virtual void SetLoopTime(double dLoopTime);

		virtual float GetAzMin() const;
		virtual void SetAzMin(float fAzMin);

		virtual float GetAzMax() const;
		virtual void SetAzMax(float fAzMax);

		virtual float GetElevMin() const;
		virtual void SetElevMin(float fElevMin);

		virtual float GetElevMax() const;
		virtual void SetElevMax(float fElevMax);

		virtual bool UpdataEffect();

	protected:
		FeUtil::AnimationPath* CreateRadarAnimationPath();
		
	protected:
		osg::Vec3    m_vecCenter;
		double        m_dRadius;
		double        m_dLoopTime;
		float         m_fAzMin;  //最小方位角
		float         m_fAzMax;  //最大方位角
		float         m_fElevMin;//俯角
		float         m_fElevMax;//仰角
		osg::ref_ptr<osg::Group> m_rpRadarWaveGroup;

	};
}

namespace FeEffect
{
	class FEEFFECTS_EXPORT CSelementEffect : public CFreeEffect
	{
	public:
		CSelementEffect();
		virtual ~CSelementEffect();

		virtual bool CreateEffect();

	};
}

namespace FeEffect
{
	class FEEFFECTS_EXPORT CFreeRadarTEffect : public CFreeEffect
	{
	public:
		CFreeRadarTEffect();
		CFreeRadarTEffect(FeUtil::CRenderContext* pContext,const osg::Vec3& center, double dRadius, double dLoopTime ,
			float  azMin,  
			float  azMax,  
			float  elevMin,  
			float  elevMax,
			osg::Vec4d color);
		virtual ~CFreeRadarTEffect();

		virtual bool CreateEffect();

	public:
		virtual osg::Vec3d GetCenter() const;
		virtual void SetCenter(const osg::Vec3d& center);

		virtual double GetRadius() const;
		virtual void SetRadius(double dRadius);

		virtual double GetLoopTime() const;
		virtual void SetLoopTime(double dLoopTime);

		virtual float GetAzMin() const;
		virtual void SetAzMin(float fAzMin);

		virtual float GetAzMax() const;
		virtual void SetAzMax(float fAzMax);

		virtual float GetElevMin() const;
		virtual void SetElevMin(float fElevMin);

		virtual float GetElevMax() const;
		virtual void SetElevMax(float fElevMax);

		virtual void SetColor(osg::Vec4d vecColor);
		virtual osg::Vec4d GetColor() const;

		virtual bool UpdataEffect();

	protected:
		FeUtil::AnimationPath* CreateRadarAnimationPath();

	protected:
		osg::Vec3    m_vecCenter;
		double        m_dRadius;
		double        m_dLoopTime;
		float         m_fAzMin;  //最小方位角
		float         m_fAzMax;  //最大方位角
		float         m_fElevMin;//俯角
		float         m_fElevMax;//仰角
		osg::Vec4d    m_vecColor;
		osg::ref_ptr<osg::Group> m_rpRadarWaveGroup;

		osg::ref_ptr<osg::MatrixTransform> m_pMT;
		osg::observer_ptr<FeUtil::CRenderContext> m_opContext;

	};
}

#endif //FREE_EFFECT_H