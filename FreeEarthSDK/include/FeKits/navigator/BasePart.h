/**************************************************************************************************
* @file CComponentBase.h
* @note 组成指北针的所有组件的基类，本基类主要定义了组件的基础功能和框架。
* @author l00008
* @data 2014-02-20
**************************************************************************************************/
#ifndef FE_NAVIGATOR_COMPONENT_BASE_H
#define FE_NAVIGATOR_COMPONENT_BASE_H

#include <osg/Geode>

#include <osg/NodeVisitor>
#include <osg/MatrixTransform>

#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>

#include <osg/BlendColor>
#include <osg/BlendFunc>

#include <FeKits/navigator/NavigatorCtrl.h>

const double PI__2 = 6.28318530717958647692;

namespace FeKit
{
	/**
	* @class CBasePart
	* @brief 组件基类
	* @note 本组件主要有如下的四方面：
	*       1、组件的创建过程；
	*       2、组件的绝对大小和相对大小；
	*       3、组件的事件处理过程；
	*       4、组件的显隐和拣选过程。本组件可以添加子组件，构成复杂的组件。
	* @author l00008
	*/
	class CBasePart : public osg::MatrixTransform
	{
	public:
		/**
		* @brief 组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CBasePart(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

	public:
		/**
		*@note 判断屏幕坐标点是否在空间的可拣选范围中
		*/
		virtual bool WithinMe(int nX, int nY) = 0;

		/**
		*@note 设置组件对应的处理控制器，每个空间都保存一个处理器的引用用来更新场景
		*/
		virtual void SetCtrl(CNavigatorCtrl* pCtrl) = 0;

		/**
		* @brief 休眠或者唤醒组件
		* @note 休眠或者唤醒组件
		* @param bShow [in]（true,激活；false,休眠）
		*/
		virtual void Active(bool bActive) = 0;

		/**
		* @brief 选中或者未选中组件
		* @note 选中或者未选中组件
		* @param bShow [in]（true,选中；false,未选中）
		*/
		virtual void Select(bool bSelect) = 0;

		/**
		*@note 更新位置，此处的开始坐标和宽高都是HUD相机的屏幕起始点和宽高
		*/
		virtual void UpdatePos(int nX, int nY, int nWidth, int nHeight, double dWR, double dHR);

		/**
		*@note: 更新组件需要的值，目前组件采用单值更新
		*/
		virtual void UpdateValue(double dValue){};

	protected:
		virtual ~CBasePart(void);

		/**
		*@note: 用于激活事件处理器
		*/
		virtual void traverse(osg::NodeVisitor& nv);

		/**
		*@note: 处理鼠标按下事件
		*/
		virtual bool PushHandle(int nX, int nY) {return false;}

		/**
		*@note: 处理鼠标拖拽事件
		*/
		virtual bool DragHandle(int nX, int nY) {return false;}

		/**
		*@note: 处理鼠标移动事件
		*/
		virtual bool MoveHandle(int nX, int nY) {return false;}

		/**
		*@note: 处理鼠标释放事件
		*/
		virtual bool ReleaseHandle(int nX, int nY) {return false;}

		/**
		*@note: 处理鼠标双击事件
		*/
		virtual bool DoubleClickHandle(int nX, int nY) {return false;}

		/**
		*@note: 事件处理器
		*/
		virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	protected:
		///开始坐标X(屏幕)
		int                                m_nPosX;      
		///开始坐标Y(屏幕)
		int                                m_nPosY;      
		///宽度(屏幕)
		int                                m_nWidth;     
		///高度(屏幕)
		int                                m_nHeight;    

		///中心坐标X
		int                                m_nCenterX;   
		///中心坐标Y
		int                                m_nCenterY;   
		///半径
		double                             m_dRadius;    

		///投影左边比例
		double                             m_dLeft;      
		///投影右边比例
		double                             m_dRight;     
		///投影下边比例
		double                             m_dBottom;    
		///投影上边比例
		double                             m_dTop;       
		///投影宽度比例
		double                             m_dWidth;     
		///投影高度比例
		double                             m_dHeight;    

		///是否休眠
		bool                               m_bActive;    
		///是否被选中
		bool                               m_bSelect;    
	};

	/**
	* @class CCompositPart
	* @brief 组合组件，可以保存组合多个组件
	* @author c00005
	*/
	class CCompositPart : public CBasePart
	{
	public:
		/**
		* @brief 组合组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CCompositPart(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		virtual ~CCompositPart(){};

	public:
		/**
		*@note 更新位置，此处的开始坐标和宽高都是HUD相机的屏幕起始点和宽高
		*/
		virtual void UpdatePos(int nX, int nY, int nWidth, int nHeight, double dWR, double dHR);

		/**
		*@note 判断屏幕坐标点是否在空间的可拣选范围中
		*/
		virtual bool WithinMe(int nX, int nY);

		/**
		*@note 设置组件对应的处理控制器，每个空间都保存一个处理器的引用用来更新场景
		*/
		virtual void SetCtrl(CNavigatorCtrl* pCtrl);

		/**
		*@note 设置激活或者休眠
		*/
		virtual void Active(bool bActive);

		/**
		*@note 设置是否选中
		*/
		virtual void Select(bool bSelect){}

	protected:
		///子组件的保存向量
		typedef std::vector<osg::observer_ptr<CBasePart> >	ChildPart;
		ChildPart											m_vecParts;
	};

	/**
	* @class CLeafPart
	* @brief 叶子组件，最终的功能实现组件
	* @author c00005
	*/
	class CLeafPart : public CBasePart
	{
	public:
		/**
		* @brief 叶子组件构造函数
		* @param strRes [in] 组件的资源路径
		*/
		CLeafPart(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes);

		virtual ~CLeafPart(){};

	public:
		/**
		* @brief 创建组件
		* @note 通过偏移位置、宽度比例高度比例创建组件
		* @param bAspectRatio [in](如果为真则会将平面的宽高相等并且是安装最小计量，并且中心也随之改变)
		*/
		virtual void Create(bool bAspectRatio = true);

		/**
		*@note 判断屏幕坐标点是否在空间的可拣选范围中
		*/
		virtual bool WithinMe(int nX, int nY);

		/**
		*@note 设置组件对应的处理控制器，每个空间都保存一个处理器的引用用来更新场景
		*/
		virtual void SetCtrl(CNavigatorCtrl* pCtrl);

		/**
		*@note 设置激活或者休眠
		*/
		virtual void Active(bool bActive);

		/**
		*@note 设置是否选中
		*/
		virtual void Select(bool bSelect);

	protected:
		///贴图名称
		std::string                         m_strImageName;     
		///实际的场景节点
		osg::observer_ptr<osg::Geode>       m_opGeodeNode;      

		///休眠半透
		double                              m_dNormalTran;    
		///active之后的半透
		double                              m_dActiveTran;     
		///选中之后的半透
		double                              m_dSelectTran;     

		///控制器，控制场景
		osg::observer_ptr<CNavigatorCtrl>   m_opCtrl;         
	};
}

#endif //FE_NAVIGATOR_COMPONENT_BASE_H



