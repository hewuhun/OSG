/**************************************************************************************************
* @file ScreenKits.h
* @note 抬头相机的基类定义，定义了平常使用的抬头相机
* @author l00008
* @data 2014-03-13
**************************************************************************************************/

#ifndef HG_SCREEN_KIT_CAMERA_H
#define HG_SCREEN_KIT_CAMERA_H

#include <osg/Camera>

#include <FeKits/Export.h>
#include <FeKits/KitsCtrl.h>

namespace FeKit
{
	/**
	* @class CPlaceStrategy
	* @brief 位置策略
	* @note 不同的系统级别需要不同的工具位置，所以预订了两种不同的位置策略，一种是
	*       绝对的大小和绝对的起始位置，一种是相对于屏幕发生变化的。
	* @author l00008
	*/
	class CPlaceStrategy : public osg::Referenced
	{
	public:
		/**
		* @class KIT_PLACE
		* @brief 位置枚举
		* @note 位置枚举，目前有上下左右组合的四个位置,这四个位置是相对屏幕而言，主要是
		*	为了将HUD相机放置到一个固定的位置，并能够随着窗口大小的改变而保持固定的位置。
		* @author c00005
		*/
		enum KIT_PLACE
		{
			LEFT_TOP = 0, //左上位置
			LEFT_BOTTOM,  //左下位置
			RIGHT_TOP,    //右上位置
			RIGHT_BOTTOM, //右下位置
		};

		CPlaceStrategy():osg::Referenced(){}

		virtual ~CPlaceStrategy(){}

	public:
		/**  
		* @brief 获得绝对的起始位置
		* @note 通过指定新的屏幕的大小来获得当前绝对的起始点，起始点和相应的KIT_PLACE是
		*	      对应的，都是以左下角为标准点计算。
		* @param nScreenW [in] 渲染屏幕宽度
		* @param nScreenH [in] 渲染屏幕高度
		* @param nX [in] 渲染屏幕绝对的起始位置X
		* @param nY [in] 渲染屏幕绝对的起始位置Y
		* @return 返回值
		*/
		virtual void GetXY(int nScreenW, int nScreenH, int& nX, int& nY) = 0;
	};


	/**
	* @class CAbsoluteStrategy
	* @brief 绝对的大小和位置的策略
	* @note 绝对的大小和位置的策略，此策略下的工具本身的大小不会随着视口的大小而变化，
	*	      但是其会按照设定的位置方案调整合适的位置，比如一直在右下角处宽100，高50.
	* @author l00008
	*/
	class CAbsoluteStrategy : public CPlaceStrategy
	{
	public:
		/**  
		* @brief 构造函数
		* @note 通过XY位置的偏移来计算出当前工具的起始位置，这个偏移是相对于KIT_PLACE
		*		  设置的，不同的位置策略和偏移值共同决定了工具的位置。
		* @param nXOffset [in] 对于屏幕边角的X偏移，比如右上是上屏幕边界偏移
		* @param nYOffset [in] 对于屏幕边角的Y偏移，比如右上是右屏幕边界偏移
		* @param nWidth [in] 工具的宽度
		* @param nHeight [in] 工具的高度
		* @param nPlace [in] 位置枚举
		* @return 返回值
		*/
		CAbsoluteStrategy(int nXOffset, int nYOffset, int nWidth, int nHeight, KIT_PLACE ePlace);

		~CAbsoluteStrategy();

	public:
		/**  
		  * @note 根据屏幕的宽和高，以及当前的策略，获取到位置信息X和Y
		  * @param nScreenW [in] 屏幕的宽
		  * @param nScreenH [in] 屏幕的高
		  * @param nX [out] X位置
		  * @param nY [out] Y位置
		*/
		virtual void GetXY(int nScreenW, int nScreenH, int& nX, int& nY);

	private:
		///X偏移
		int     m_nXOffset; 

		///Y偏移 
		int     m_nYOffset;  

		///工具宽度
		int     m_nWidth;

		///工具高度
		int     m_nHeight;  

		///位置策略
		KIT_PLACE   m_ePlace; 
	};
}

namespace FeKit
{
	/**
	* @class CScreenKits
	* @brief 屏幕显示的工具部件
	* @note 屏幕显示的工具部件
	* @author l00008
	*/
	class FEKIT_EXPORT CScreenKits : public CKitsCtrl, public osg::Camera
	{
	public:
		/**  
		* @brief 构造一个HUD相机的父类
		* @note 通过指定位置以及相对于其位置的偏移值
		* @param nXOffset [in] X偏移
		* @param nYOffset [in] Y偏移
		* @param nWidth [in] 宽度
		* @param nHeight [in] 高度
		* @param ePlace [in] 位置
		* @return 返回值
		*/
		CScreenKits(FeUtil::CRenderContext* pContext,int nXOffset, int nYOffset, int nWidth, int nHeight, CPlaceStrategy::KIT_PLACE ePlace);

		virtual ~CScreenKits();

	public:
		/**
		*@note: 显示
		*/
		virtual void Show();
		
		/**
		*@note: 隐藏
		*/
		virtual void Hide();

		/**
		*@note: 使用屏幕的宽度和高度来更新工具的大小和起始位置，这个函数必须在
		*		创建完成或者视口变化是调用，否则工具的位置无法更新。
		*/
		virtual void ResizeKit(int nW, int nH);

	public:
		/**
		*@note: 设置相机的位置和宽高
		*/
		virtual void SetPosition(int nX, int nY, int nWidth, int nHeight);

		/**
		*@note: 获取相机的位置和宽高
		*/
		virtual void GetPosition(int& nX, int& nY, int& nWidth, int& nHeight);

		/**
		*@note: 获得相机的宽高比率，此比率会随着宽高发生变化而变化
		*/
		virtual void GetOrthoRate(double& dLeft, double& dRight, double& dBottom, double& dTop) const;

		/**
		*@note: 获取相机显示的深度
		*/
		virtual double GetScreeDepth() const;

		/**
		*@note: 设置相机显示的深度，默认为-1.0
		*/
		virtual void SetScreeDepth(double dDepth = -1.0);

		/**
		*@note: 判断是否在HUD相机范围之内
		*/
		virtual bool WithMe(int unX, int unY);

	protected:    
		/**
		*@note: 更新投影的宽高比
		*/
		void UpdteProjection();

		/**
		*@note: 设置HUD相机的初始属性
		*/
		void InitHUD();

	protected:
		///起始位置X坐标
		int             m_nX;      

		///起始位置Y坐标
		int             m_nY;       

		///相机宽度
		int             m_nWidth;       

		///相机高度
		int             m_nHeight;      

		///投影左边
		double          m_dRightRate;   

		///投影右边
		double          m_dLeftRate;    

		///投影下边
		double          m_dBottomRate;  

		///投影上边
		double          m_dTopRate;     

		///投影宽度
		double          m_dWidthRate;   

		///投影高度
		double          m_dHeightRate;  

		///渲染次序，默认为10
		int				m_nOrderNum;    

		///相机显示深度，为了多层渲染
		double          m_dDepth;       

		osg::ref_ptr<CPlaceStrategy>    m_rpPlaceStrgy;
	};
}

#endif //HG_SCREEN_KIT_CAMERA_H
