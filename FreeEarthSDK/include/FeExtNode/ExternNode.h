/**************************************************************************************************
* @file CExternNode.h
* @note 外部扩展节点的基类，主要保存了名称，ID，是否可见等属性
* @author c00005
* @data 2016-7-4
**************************************************************************************************/
#ifndef FE_EXTERN_NODE_H
#define FE_EXTERN_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExternNodeVisitor.h>
#include <FeExtNode/IFeNodeEditable.h>

#include <FeUtils/GeneratorID.h>
#include <FeUtils/FreeViewPoint.h>

#include <osg/Group>
#include <osgEarth/optional>



using namespace FeUtil;
using namespace osgEarth;

///显示的掩码
const int VISIBLE_MASK = 0x000000FF;
///隐藏的掩码
const int UN_VISIBLE_EVENT_MASK = 0x00000000;
///拣选的掩码
const int ACCEPT_EVENT_MASK = 0x0000FF00;

namespace FeExtNode
{
	/**
	* @class CExternNodeOption
	* @note 扩展节点的配置选项,其中包含了名称、描述、视点以及是否可见
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExternNodeOption : public osg::Referenced
	{
	public:
		/**  
		  * @note 构造函数，无参数，使用默认值
		*/
		CExternNodeOption();
		
		/**  
		  * @note 构造函数，无参数，使用默认值
		  * @param name [in] 节点的名称
		*/
		CExternNodeOption( const std::string& name );

		/**  
		  * @note 析构函数
		*/
		virtual ~CExternNodeOption();

	public:
		/**  
		  * @note 获取节点的名称
		*/
		std::string& name() { return m_strName; }

		/**  
		  * @note 获取节点的视点位置
		*/
		CFreeViewPoint& viewPoint() { return m_vpViewPoint; }

		/**  
		  * @note 获取节点是否可见
		*/
		bool& visible() { return m_bVisible; }

		/**  
		  * @note 获取节点是否接受事件处理
		*/
		bool& acceptEvent() { return m_bAcceptEvent; }

		/**  
		  * @note 获取节点的描述信息
		*/
		std::string& description() { return m_strDesc; }

		
	protected:
		///扩展节点的名称
		std::string                 m_strName;

		///扩展节点是否可见
		bool			            m_bVisible;

		///扩展节点是否接受事件处理
		bool						m_bAcceptEvent;

		///扩展节点的描述信息
		std::string					m_strDesc;

		///扩展节点的视点信息
		CFreeViewPoint				m_vpViewPoint;

	};
}


namespace FeExtNode
{
	class CExComposeNode;
	
	/**
	* @class CExternNode
	* @note 扩展节点的基类，所有的用户自定义的节点都需要从它继承
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExternNode : public osg::Group
		,public FeNodeEditor::IFeNodeEditable // 支持编辑器接口
	{
	public:
		/**  
		  * @note 构造函数
		  * @param opt [in] 节点的属性配置集合
		*/
		CExternNode(CExternNodeOption* opt);

		/**  
		  * @note 析构函数
		*/
		virtual ~CExternNode();

	
		/** 编辑器相关接口 */
	protected:
		/**
		  * @note 编辑工具开始拖拽
		*/
		virtual void DraggerStart() {}

		/**
		  * @note 拖拽编辑工具时矩阵变化量
				注意： 目前，旋转和缩放的变化量使用局部坐标系，平移的变化量使用全局坐标系
		*/
		virtual void DraggerUpdateMatrix(const osg::Matrix& matrix){}

		/**
		  * @note 编辑工具结束拖拽
		*/
		virtual void DraggerEnd(){ UpdateEditor(); }
		
		/**
		  * @note 创建编辑工具
		*/
		virtual FeNodeEditor::CFeNodeEditTool* CreateEditorTool(){ return NULL;}

	public:
		/**
		  * @note 获取编辑点位置
		*/
		virtual osg::Vec3d GetEditPointPositionXYZ(){return getBound().center();}

		/**
		  * @note 获取编辑点旋转角度（单位：角度）
		*/
		virtual osg::Vec3d GetEditPointRotateDegree(){return osg::Vec3d(0,0,0);}

	public:
		/**  
		  * @note 获取扩展节点的ID值
		*/
		virtual FeUtil::FEID GetID();

		/**  
		  * @note 设置扩展节点的名称
		*/
		virtual void SetName(const std::string& strName);
			
		/**  
		  * @note 获取扩展节点的名称
		*/
		virtual std::string GetName();

		/**  
		  * @note 设置扩展节点的描述信息
		*/
		virtual void SetDescription(const std::string& strDesc);
		
		/**  
		  * @note 获取扩展节点的描述信息
		*/
		virtual std::string GetDescription();

		/**  
		  * @note 设置扩展节点是否可见
		*/
		virtual void SetVisible(const bool& bVisible);

		/**  
		  * @note 获取扩展节点是否可见
		*/
		virtual bool GetVisible();
		
		/**  
		  * @note 设置扩展节点是否接受事件处理
		*/
		virtual void SetAcceptEvent(const bool& bEvent);

		/**  
		  * @note 获取扩展节点是否接受事件处理
		*/
		virtual bool GetAcceptEvent();

		/**  
		  * @note 设置扩展节点的视点位置信息
		*/
		virtual void SetViewPoint(const CFreeViewPoint& viewPoint);
		
		/**  
		  * @note 获取扩展节点的视点位置信息
		*/
		virtual CFreeViewPoint GetViewPoint();

		/**  
		  * @note 获取扩展节点的Options信息，子类对该方法进行重写返回子类的Options
		*/
		virtual CExternNodeOption* GetOption();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

		/**
		* @note 是否可以作为一个节点组
		*/
		virtual CExComposeNode* AsComposeNode();

	protected:
		/**  
		  * @note 更新节点的掩码，用于显示或者接受事件状态改变时调用
		*/
		virtual void UpdateVisibleAndEvent();

	protected:
		///节点的配置信息
		osg::ref_ptr<CExternNodeOption>		m_rpOptions;

		///节点的ID值，该ID值为系统自动生成，用户只能获取，不能设置
		FEID								m_unID;
	};
}

#endif //FE_EXTERN_NODE_H
