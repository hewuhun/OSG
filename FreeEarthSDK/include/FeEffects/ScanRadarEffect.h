/**************************************************************************************************
* @file ScanRadarEffect.h
* @note 雷达效果(雷达扫描范围及内部扫描动画)
* @author z00013
* @data 2015-12-24
**************************************************************************************************/
#ifndef SCANRADAR_EFFECT_H
#define SCANRADAR_EFFECT_H

#include <osg/Group>

#include <FeEffects/Export.h>
#include <FeEffects/FreeEffect.h>
#include <FeUtils/MiniAnimation.h>
#include <FeUtils/RenderContext.h>


namespace FeEffect
{

	//雷达参数结构体
	struct SRadarParam
	{
		osg::Vec4d   vecColor;
		bool		 bDrawRange;
		bool		 bDrawScan;
		double		 dRadius;
		double		 dScanSpeed;
		double		 dAzimuthBegin;
		double       dAzimuthEnd;
		double		 dPitchBegin;
		double		 dPitchEnd;
	};

	//修改类型
	enum EnumRadarModify
	{
		RM_RADARPOSITION	= 0x0001,
		RM_RADARCOLOR		= 0x0002,
		RM_DRAWWAVE			= 0x0004,
		RM_RADIUS			= 0x0008,
		RM_AZIMUTHBEGIN		= 0x0010,
		RM_AZIMUTHEND		= 0x0020,
		RM_PITCHBEGIN		= 0x0040,
		RM_PITCHEND			= 0x0080,
		RM_DRAWSCAN			= 0x0100,
		RM_SCANSPEED		= 0x0200,
	};

	/**
    * @class ERadarScanType
    * @brief 扫描方式
    * @note 扫描方式：垂直扫描、水平扫描、自转（无内部扫面）；
    * @author z00013
    */
	enum ERadarScanType
	{
		VERTICAL_SCANTYPE,
		HORIZONTAL_SCANTYPE,
		ROTATION_SCANTYPE,
		STATIC_SCANTYPE
	};

	/**
    * @class CScanRadarEffect
    * @brief 内部扫描式雷达效果
    * @note 提供一个内部扫描式雷达效果的接口类，扫描范围是水平角度及俯仰角度；
    * @author z00013
    */
	class FEEFFECTS_EXPORT CScanRadarEffect : public CFreeEffect
	{
	public:
		CScanRadarEffect();
		/**
        * @brief 创建雷达效果
        * @note 通过提供雷达效果的中心点、扫描半径、水平角度、俯仰角度、颜色、扫描方式、扫描周期来创建雷达效果
        * @param center [in]（中心点，相对模型位置）
        * @param dRadius [in] 雷达半径
        * @param azMin [in] 水平角度起始角度（相对北极）
        * @param azMax [in] 水平角度结束角度
        * @param elevMin [in] 垂直角度起始角度
        * @param elevMax [in] 垂直角度结束角度
		* @param vecColor [in] 雷达颜色
		* @param eScanType [in] 扫描方式
		* @param dLoopTime [in] 扫描周期（秒）
        */
		CScanRadarEffect(FeUtil::CRenderContext* pContext,const osg::Vec3& center, double dRadius,
			float  azMin,  
			float  azMax,  
			float  elevMin,  
			float  elevMax,
			osg::Vec4d vecColor,
			ERadarScanType eScanType,
			double dLoopTime );
		virtual ~CScanRadarEffect();

	protected:
		virtual bool CreateEffect();

	public:
		/**
        * @brief 获得雷达效果中心点
        * @note 获得雷达效果中心点
        */
		virtual osg::Vec3d GetCenter() const;

		/**
        * @brief 设置雷达效果中心点
        * @note 
        * @param center [in]（中心点，相对模型位置）
        */
		virtual void SetCenter(const osg::Vec3d& center);

		/**
        * @brief 获得雷达效果半径
        * @note 
        */
		virtual double GetRadius() const;

		/**
        * @brief 设置雷达效果半径
        * @note 
        * @param dRadius [in]（半径）
        */
		virtual void SetRadius(double dRadius);

		/**
        * @brief 获得雷达效果扫描周期
        * @note 
        */
		virtual double GetLoopTime() const;

		/**
        * @brief 设置雷达效果扫描周期
        * @note 
        * @param dLoopTime [in]（扫描周期）
        */
		virtual void SetLoopTime(double dLoopTime);

		/**
        * @brief 获得雷达扫描范围的水平最小角度
        * @note 
        */
		virtual float GetAzMin() const;

		/**
        * @brief 设置雷达扫描范围的水平最小角度
        * @note 
        * @param fAzMin [in]（水平最小角度）
        */
		virtual void SetAzMin(float fAzMin);

		/**
        * @brief 获得雷达扫描范围的水平最大角度
        * @note 
        */
		virtual float GetAzMax() const;
		/**
        * @brief 设置雷达扫描范围的水平最大角度
        * @note 
        * @param fAzMax [in]（水平最大角度）
        */
		virtual void SetAzMax(float fAzMax);

		/**
        * @brief 获得雷达扫描范围的垂直最小角度
        * @note 
        */
		virtual float GetElevMin() const;
		/**
        * @brief 设置雷达扫描范围的垂直最小角度
        * @note 
        * @param fElevMin [in]（垂直最小角度）
        */
		virtual void SetElevMin(float fElevMin);

		/**
        * @brief 获得雷达扫描范围的垂直最大角度
        * @note 
        */
		virtual float GetElevMax() const;
		/**
        * @brief 设置雷达扫描范围的垂直最小角度
        * @note 
        * @param fElevMax [in]（垂直最小角度）
        */
		virtual void SetElevMax(float fElevMax);

		/**
        * @brief 设置雷达颜色
        * @note 
        * @param vecColor [in]（雷达效果颜色）
        */
		virtual void SetColor(osg::Vec4d vecColor);
		/**
        * @brief 获得雷达效果颜色
        * @note 
        */
		virtual osg::Vec4d GetColor() const;

		/**
        * @brief 更新雷达效果
        * @note  设置完成各种参数后，调用更新完成；
        */
		virtual bool UpdateEffect();

	public:
		//设置雷达波束范围
		void SetRange(float fAzMin, float fAzMax,float fElevMin,float fElevMax);

		/**
        * @brief 设置雷达波束内部扫描波束的角度范围扫面角度(完成设置后调用UpdateEffect())
        * @param fElfinAzMax [in]（雷达内部扫描精灵水平角度）
		* @param fElfinElevMax [in]（雷达内部扫描精灵垂直角度）
        */
		void SetScanElfinRange(float fElfinAzMax,float fElfinElevMax);

		
		/**
        * @brief 设置波束扫描类型(完成设置后调用UpdateEffect())
        * @param eScanType [in]（雷达扫描类型）
        */
		void SetScanType(ERadarScanType eScanType);

		/**
        * @brief 设置内部扫描波束颜色(完成设置后调用UpdateEffect())
        * @param vecColor [in]（雷达内部扫描精灵颜色）
        */
		void SetScanElfinColor(osg::Vec4d vecElfinColor);

		//设置角度偏差
		void SetRotateOffset(double dOffsetPitch,double dOffsetRoll,double dOffsetYaw);

	protected:
		FeUtil::AnimationPath* CreateRadarAnimationPath();
		FeUtil::AnimationPath* CreateRadarElfinScanPath();

		virtual void CreateScanElfin();
		virtual void CreateRadar();
		
	protected:
		osg::Vec3    m_vecCenter;
		double        m_dRadius;
		double        m_dLoopTime;

		//角度值；以正北方向为基准
		float         m_fAzMin;  //最小方位角
		float         m_fAzMax;  //最大方位角
		float         m_fElevMin;//俯角
		float         m_fElevMax;//仰角
		osg::Vec4d    m_vecColor;
		osg::ref_ptr<osg::Group> m_rpRadarWaveGroup;
		
		//角度值；以正北方向为基准

		float         m_fElfinAzMax;  //内部扫描精灵最大方位角

		float         m_fElfinElevMax;//内部扫描精灵仰角

		osg::ref_ptr<osg::MatrixTransform> m_pMT;
		osg::observer_ptr<FeUtil::CRenderContext> m_opContext;

		ERadarScanType			m_eScanType;
		osg::Vec4d    m_vecElfinColor;

		double					m_dOffsetYaw;
		double					m_dOffsetPitch;
		double					m_dOffsetRoll;

	};
}

#endif //SCANRADAR_EFFECT_H