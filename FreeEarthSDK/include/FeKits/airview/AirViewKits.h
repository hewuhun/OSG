/**************************************************************************************************
* @file AirViewKits.h
* @note 鸟瞰图的边框、主平面以及标记等
* @author c00005
* @data 2017-4-1
**************************************************************************************************/
#ifndef FE_AIR_VIEW_KIT_H
#define FE_AIR_VIEW_KIT_H

#include <osgViewer/View>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Camera>
#include <osg/Switch>
#include <osg/MatrixTransform>

#include <FeKits/Export.h>

namespace AVGlobal
{
	///鸟瞰图最大的显示距离
	const double MAX_DISTANCE = 20000000.0;

	///鸟瞰图最小的显示距离
	const double MIN_DISTANCE = 1000000.0; 

	///最大显示距离和最小显示距离的差值
	const double DELTA_DISTANCE = MAX_DISTANCE - MIN_DISTANCE;

	///全球经度大小
	const double LONGITUDE = 360.0; 

	///全球维度大小
	const double LATITUDE = 180.0;  

	///显示四边形的焦点
	const int SWITCH_QUAD_NODE=0;

	///显示十字架形的焦点
	const int SWITCH_CORSS_NODE=1;
}

namespace FeKit
{
	/**
	* @class CAirViewBoeder
	* @brief 鸟瞰图的边框
	* @note 提供一个和主场景相互区别的边框
	* @author l00008
	*/
	class CAirViewBoeder : public osg::Geode
	{
	public:
		/**  
		* @brief 鸟瞰图的边框绘制类
		* @param dX [in] 边框的X位置
		* @param dY [in] 边框的Y位置
		* @param dWidth [in] 边框的宽度
		* @param dHeight [in] 边框的高度
		* @param vecColor [in] 边框的颜色
		*/
		CAirViewBoeder(double dX, double dY, double dWidth, double dHeight, const osg::Vec4d& vecColor);

		~CAirViewBoeder(void);

	protected:
		/**  
		* @brief 创建鸟瞰图的边框功能函数
		* @param dX [in] 边框的X位置
		* @param dY [in] 边框的Y位置
		* @param dWidth [in] 边框的宽度
		* @param dHeight [in] 边框的高度
		* @param vecColor [in] 边框的颜色
		*/
		void CreateBorder( double dX, double dY, double dWidth, double dHeight, const osg::Vec4d& vecColor );
	};


	/**
	* @class CAirViewFocus
	* @brief 鸟瞰图的焦点
	* @note 在鸟瞰图中指示当前的相机焦点
	* @author l00008
	*/
	class CAirViewFocus : public osg::MatrixTransform
	{
	public:
		/**  
		* @brief 鸟瞰图的焦点
		* @note 鸟瞰图的焦点绘制构造函数
		* @param dX [in] 焦点的X位置
		* @param dY [in] 焦点的Y位置
		* @param dWidth [in] 焦点的宽度
		* @param dHeight [in] 焦点的高度
		* @param vecColor [in] 焦点的颜色
		* @return 返回值
		*/
		CAirViewFocus( double dX, double dY, double dWidth, double dHeight, const osg::Vec4d& vecColor );

		~CAirViewFocus(void);

	public:
		/**  
		* @note 更新焦点的位置
		*/
		void UpdateFocus(const osg::Vec3d& vecWorldPos);

		/**  
		* @brief 创建鸟瞰图的焦点的功能函数
		* @param dX [in] 焦点的X位置
		* @param dY [in] 焦点的Y位置
		* @param dWidth [in] 焦点的宽度
		* @param dHeight [in] 焦点的高度
		* @param vecColor [in] 焦点的颜色
		* @return 返回值
		*/
		void CreateFocus(double dX, double dY, double dWidth, double dHeight, const osg::Vec4d& vecColor);

	protected:
		///焦点的X位置
		double          m_dX;

		///焦点的Y位置
		double          m_dY;

		///焦点的宽度
		double          m_dWidth;

		///焦点的高度
		double          m_dHeight;

		///焦点的颜色
		osg::Vec3d      m_vecWorldPos;

		///焦点的变换节点
		osg::ref_ptr<osg::Switch> m_rpSwitch;
	};


	/**
	* @class CAirViewGround
	* @brief 鸟瞰图背景，显示主场景的平面图
	* @note 显示主场景的平面图
	* @author l00008
	*/
	class CAirViewGround : public osg::Group
	{
	public:
		/**  
		* @note 主场景平面图的构造函数  
		* @param dX [in] 平面图的X位置
		* @param dY [in] 平面图的Y位置
		* @param dWidth [in] 平面图的宽度
		* @param dHeight [in] 平面图的高度
		* @param strGroundImage [in] 平面图的背景图
		*/
		CAirViewGround(
			double dX, 
			double dY, 
			double dWidth, 
			double dHeight,
			const std::string& strGroundImage
			);

		~CAirViewGround(void);

	protected:
		/**  
		* @note 主场景平面图构建的功能函数  
		* @param dX [in] 平面图的X位置
		* @param dY [in] 平面图的Y位置
		* @param dWidth [in] 平面图的宽度
		* @param dHeight [in] 平面图的高度
		* @param strGroundImage [in] 平面图的背景图
		*/
		void CreateGround(
			double dX, 
			double dY, 
			double dWidth, 
			double dHeight,
			const std::string& strGroundImage);
	};


	/**
	* @class CAirViewMark
	* @brief 鸟瞰图的标记点
	* @note 鸟瞰图的标记点，其中实现了三种标记点，用来标记
	* @author l00008
	*/
	class FEKIT_EXPORT CAirViewMark : public osg::MatrixTransform
	{
	public:
		/**  
		* @brief 构造函数
		* @note 使用标记的位置信息、线宽、颜色、是否填充来初始化标记 
		* @param vecLLH [in] 标记的位置使用经纬度表示
		* @param dLineWidth [in] 标记的线宽
		* @param vecColor [in] 标记的颜色值
		* @param bFull [in] 标记是否需要填充
		* @return 返回值
		*/
		CAirViewMark( const osg::Vec3d& vecLLH, double dLineWidth, const osg::Vec4d& vecColor, bool bFull);

		~CAirViewMark(void){};

	public:
		/**  
		* @note 更新标记的位置  
		*/
		virtual void UpdateMark(const osg::Vec3d& vecLLH);

		/**  
		* @note 显示标记  
		*/
		virtual void Show();

		/**  
		* @note 隐藏标记
		*/
		virtual void Hide();

		/**  
		* @note 判断标记是否隐藏
		*/
		virtual bool IsHide();

		/**  
		* @note 设置鸟瞰图的归一化坐标信息
		*/
		virtual void AttachAirViewSize(double dWidth, double dHeight);

	protected:
		/**  
		* @note 创建标记点
		*/
		virtual void CreateMark() = 0;

	protected:
		///标记是否需要填充
		bool			m_bFull;

		///线宽
		double          m_dLineWidth;

		///位置
		osg::Vec3d      m_vecLLH;

		///颜色
		osg::Vec4d      m_vecColor;

		///鸟瞰图的宽度归一化
		double     m_dAirViewWidth;   

		//鸟瞰图的高度归一化
		double	   m_dAirViewHeight;

		///是否可见
		bool			m_bShow;
	};

	/**
	* @class CCircleMark
	* @brief 圆形标记点，继承自标记点的基类
	* @author c00005
	*/
	class FEKIT_EXPORT CCircleMark : public CAirViewMark
	{
	public:
		/**  
		* @brief 圆形标记点构造函数
		* @param vecLLH [in] 标记的位置使用经纬度表示
		* @param dLineWidth [in] 标记的线宽
		* @param vecColor [in] 标记的颜色值
		* @param bFull [in] 标记是否需要填充
		* @return 返回值
		*/
		CCircleMark( const osg::Vec3d& vecLLH, double dLineWidth = 1.0, const osg::Vec4d& vecColor = osg::Vec4d(1.0, 0.0, 0.0, 1.0), bool bFull = false );

		virtual ~CCircleMark();

	protected:
		/**  
		* @note 创建标记点
		*/
		virtual void CreateMark();
	};

	/**
	* @class CTrilateralMark
	* @brief 三角形标记点，继承自标记点的基类
	* @author c00005
	*/
	class FEKIT_EXPORT CTrilateralMark : public CAirViewMark
	{
	public:
		/**  
		* @brief 三角形标记点构造函数
		* @param vecLLH [in] 标记的位置使用经纬度表示
		* @param dLineWidth [in] 标记的线宽
		* @param vecColor [in] 标记的颜色值
		* @param bFull [in] 标记是否需要填充
		* @return 返回值
		*/
		CTrilateralMark( const osg::Vec3d& vecLLH, double dLineWidth = 1.0, const osg::Vec4d& vecColor = osg::Vec4d(1.0, 0.0, 0.0, 1.0), bool bFull = false );

		virtual ~CTrilateralMark();

	protected:
		/**  
		* @note 创建标记点
		*/
		virtual void CreateMark();
	};

	/**
	* @class CRectangleMark
	* @brief 矩形标记点，继承自标记点的基类
	* @author c00005
	*/
	class FEKIT_EXPORT CRectangleMark : public CAirViewMark
	{
	public:
		/**  
		* @brief 矩形标记点构造函数
		* @param vecLLH [in] 标记的位置使用经纬度表示
		* @param dLineWidth [in] 标记的线宽
		* @param vecColor [in] 标记的颜色值
		* @param bFull [in] 标记是否需要填充
		* @return 返回值
		*/
		CRectangleMark( const osg::Vec3d& vecLLH, double dLineWidth = 1.0, const osg::Vec4d& vecColor = osg::Vec4d(1.0, 0.0, 0.0, 1.0), bool bFull = false );

		virtual ~CRectangleMark();

	protected:
		/**  
		* @note 创建标记点
		*/
		virtual void CreateMark();
	};

}

#endif //FE_AIR_VIEW_KIT_H
