/**************************************************************************************************
* @file FreeViewPoint.h
* @note 视点的位置信息封装
* @author c00005
* @data 2015-7-8
**************************************************************************************************/
#ifndef FREE_VIEW_POINT_H
#define FREE_VIEW_POINT_H

#include <osgViewer/View>
#include <osgEarth/Viewpoint>

#include <FeUtils/Export.h>

namespace FeUtil
{
	/**
	* @class CFreeViewPoint
	* @note 视点的位置信息封装
	* @author c00005
	*/
	class FEUTIL_EXPORT CFreeViewPoint
	{
	public:
		/**  
		* @brief 构造函数
		*/
		CFreeViewPoint();

		/**  
		* @brief 构造函数
		*/
		CFreeViewPoint(const osgEarth::Viewpoint& viewPoint);
		
		/**  
		* @brief 构造函数
		*/
		CFreeViewPoint(const CFreeViewPoint& viewPoint);

		/**  
		* @brief 构造函数
		* @note 构造函数，使用经纬高（角度值）以及俯仰角、偏航角、距离以及飞行时间初始化视点的位置
		*/
		CFreeViewPoint(double dLon, double dLat, double dHei, double dHeading, double dPitch, double dRange, double dTime);

		/**  
		* @brief 构造函数
		* @note 构造函数，使用经纬高（角度值）以及俯仰角、偏航角、距离以及飞行时间初始化视点的位置
		*/
		CFreeViewPoint(std::string strName, double dLon, double dLat, double dHei, double dHeading, double dPitch, double dRange, double dTime);
		
		/**  
		* @brief 析构函数
		*/
		virtual ~CFreeViewPoint();

	public:
		///获取经度（角度值）信息
		double GetLon() const;

		///获取纬度（角度值）信息
		double GetLat() const;

		///获取高度信息
		double GetHei() const;

		///获取俯仰角信息
		double GetPitch() const;

		///获取偏航角信息
		double GetHeading() const;

		///获取视点的距离信息
		double GetRange() const;

		///获取飞行时间
		double GetTime() const;

		///设置飞行时间
		void SetTime(double dTime);

		///获取视点名称
		std::string GetName() const;

		///设置名称
		void SetName(std::string strName);

		///获取osgEarth的ViewPoint信息，即将此类定义的信息转换成osgEarth::Viewpoint
		osgEarth::Viewpoint GetEarthVP() const;

		/**  
		* @brief 设置视点的位置
		* @param vecFocuse [in] 视点位置的经纬度信息（角度值）
		*/
		void SetViewPoint(const osg::Vec4d& vecFocuse);

		/**  
		* @brief 设置视点的位置
		* @param viewPoint [in] 视点位置在osgEarth中的表示方式，即将osgEarth::Viewpoint装换到本类
		*/
		void SetViewPoint(const osgEarth::Viewpoint& viewPoint);

		/**  
		* @brief 设置视点的位置
		* @note 通过经纬度（角度值）、俯仰角、偏航角、距离以及飞行时间设置视点
		*/
		void SetViewPoint(double dLon, double dLat, double dHei, double dHeading, double dPitch, double dRange, double dTime);

	protected:
		std::string				m_strName;
		double					m_dLon; //经度
		double					m_dLat; //纬度
		double					m_dHeigh; //高度
		double					m_dPitch; //俯仰角（垂直向下是-90°）
		double					m_dHeading; //滚转角
		double					m_dRange; //视点范围
		double					m_dTime;  //定位时间
	};


	class FEUTIL_EXPORT CViewPointObserver
	{
	public:
		CViewPointObserver();

		virtual ~CViewPointObserver();

	public:
		virtual CFreeViewPoint GetViewPoint();

		virtual  void SetViewPoint(const CFreeViewPoint& viewPoint );

	protected:
		CFreeViewPoint     m_viewPoint;
	};


	typedef std::list<CFreeViewPoint> FreeViewPoints;
}

#endif //FREE_VIEW_POINT_H