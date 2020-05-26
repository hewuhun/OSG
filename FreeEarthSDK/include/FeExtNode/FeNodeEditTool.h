/**************************************************************************************************
* @file FeNodeEditorTool.h
* @note 节点编辑工具基类
* @author g00034
* @data 2016-11-3
**************************************************************************************************/

#ifndef FE_NODE_EDIT_TOOL_H
#define FE_NODE_EDIT_TOOL_H

#include <osg/Group>
#include <osgManipulator/Dragger>
#include <FeExtNode/FeTranslateDragger.h>
#include <FeExtNode/FeScaleDragger.h>
#include <FeExtNode/FeRotateDragger.h>

namespace FeExtNode
{
	class CFeEventListenMgr;
}

namespace FeNodeEditor
{
	class CExDraggerCallback;
	class IFeNodeEditable;

	/**
	* @class CFeNodeEditTool
	* @note 节点编辑工具基类，包含平移，缩放，旋转编辑工具
	* @author g00034
	*/
	class FEEXTNODE_EXPORT CFeNodeEditTool : public osg::Group
	{
		friend class CExDraggerCallback;
	public:
		/// 编辑器种类
		enum E_EDITOR_TYPE
		{
			EDITOR_TRANS = 1 << 0,  // 平移编辑器
			EDITOR_SCALE = 1 << 1,  // 缩放编辑器
			EDITOR_ROTATE = 1 << 2, // 旋转编辑器
		};

		/// 编辑器拖拽状态
		enum E_DRAG_STATE
		{
			DRAG_STOP = 0,  /// 停止拖拽
			DRAG_START,     /// 开始拖拽
			DRAG_DRAGGING,  /// 正在拖拽
		};

	protected:
		/**  
		  * @note 构造函数
		  * @param pEditNode [in] 编辑节点
		  * @param editorTypeSupport [in] 支持的编辑器种类
		*/
		CFeNodeEditTool(IFeNodeEditable* pEditNode, unsigned int editorTypeSupport);

		/**  
		  * @note 析构函数
		*/
		virtual ~CFeNodeEditTool();

	public:
		/**  
		  * @note 更新编辑器
		*/
		virtual void UpdateEditing();

		/**  
		  * @note 显示编辑器
		*/
		void ShowEditor();

		/**  
		  * @note 隐藏编辑器
		*/
		void HideEditor();

		/**  
		  * @note 编辑器是否显示
		*/
		bool IsEditorVisible();

		/**  
		  * @note 显示平移编辑器
		*/
		bool ShowTransEditor();

		/**  
		  * @note 显示缩放编辑器
		*/
		bool ShowScaleEditor();

		/**  
		  * @note 显示旋转编辑器
		*/
		bool ShowRotateEditor();

		/**  
		  * @note 切换编辑器
		*/
		void SwitchEditor();

		/**  
		  * @note 安装事件监听管理器
		*/
		void SetupEventListenMgr(FeExtNode::CFeEventListenMgr* pEventListenMgr);

	protected:
		/**  
		  * @note 创建编辑器
		  * @param editorTypeSupport [in] 支持的编辑器种类
		*/
		void CreateEditors(unsigned int eType);

		/**  
		  * @note 内部更新编辑器
		*/
		virtual void UpdateEditingInternal();

		/**  
		  * @note 计算编辑器缩放系数
		*/
		virtual float GetEditorScaleVal() = 0;

		/**  
		  * @note 计算转换到指定坐标点的平移矩阵
		*/
		virtual osg::Matrix ComputeTransMatrixByWorldXYZ(const osg::Vec3d& worldXYZ) = 0;

		/**
		  * @note 编辑工具开始拖拽
		*/
		virtual void DraggerStart();

		/**
		  * @note 拖拽编辑工具时矩阵变化量
				注意： 目前，旋转和缩放的变化量使用局部坐标系，平移的变化量使用全局坐标系
		*/
		virtual void DraggerUpdateMatrix(const osg::Matrix& matrix);

		/**
		  * @note 编辑工具结束拖拽
		*/
		virtual void DraggerEnd();

		/**
		  * @note 通知事件监听者，节点状态发生变化
		*/
		void NotifyListenerMgr();

		/**
		  * @note 获取编辑器拖拽状态
		*/
		E_DRAG_STATE GetDragState() const { return m_eDragState; }

	protected:
		/// 编辑节点
		IFeNodeEditable*	                m_pEditNode;

		/// 支持的编辑器种类
		unsigned int                        m_nEditorSupport;

		/// 当前激活的编辑器
		osg::observer_ptr<CFeCompositeDragger> m_opCurActiveDragger;

		/// 平移编辑器
		osg::ref_ptr<CFeTranslateDragger>   m_rpTransDragger;

		/// 缩放编辑器
		osg::ref_ptr<CFeScaleDragger>       m_rpScaleDragger;

		/// 旋转编辑器
		osg::ref_ptr<CFeRotateDragger>      m_rpRotateDragger;

		/// 编辑器操作矩阵
		osg::ref_ptr<osg::MatrixTransform>  m_rpMtNode;

		/// 相机根节点，实现最后渲染
		osg::ref_ptr<osg::Camera>           m_rpCameraRoot;

		/// 编辑器回调对象
		osg::ref_ptr<CExDraggerCallback>    m_rpTransDraggerCallback;
		osg::ref_ptr<CExDraggerCallback>    m_rpScaleDraggerCallback;
		osg::ref_ptr<CExDraggerCallback>    m_rpRotateDraggerCallback;

		/// 事件监听管理器
		osg::observer_ptr<FeExtNode::CFeEventListenMgr> m_opEventListenMgr;
		
		/// 编辑器拖拽状态
		E_DRAG_STATE                        m_eDragState;
	};

}

namespace FeNodeEditor
{
	/**
	  * @class CExDraggerCallback
	  * @brief 编辑器回调类
	  * @note 编辑器回调类，用于处理鼠标拖拽事件，并通知编辑节点进行相应变化
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExDraggerCallback : public osgManipulator::DraggerTransformCallback
	{
	public:
		/**  
		  * @note 构造函数  
		  * @param transform [in] 编辑器中的更新矩阵
		  * @param pEditTool [in] 编辑器
		  * @param handleCommandMask [in] 编辑操作的掩码
		*/
		CExDraggerCallback(osg::MatrixTransform* transform, CFeNodeEditTool* pEditTool,
			int handleCommandMask = osgManipulator::DraggerTransformCallback::HANDLE_ALL);
		
		/**  
		  * @note 接收编辑器命令流，并进行解析，从而更改编辑节点
		  * @param command [in] 编辑器命令流
		  * @return 是否执行成功
		*/
		virtual bool receive(const osgManipulator::MotionCommand& command);
		
	protected:
		/// 编辑节点
		//IFeNodeEditable*  m_pEditNode;
		osg::observer_ptr<CFeNodeEditTool> m_opEditTool;
	};
}



namespace osgEarth
{
	class MapNode;
	class GeoPoint;
}
namespace osg
{
	class ShapeDrawable;
}

namespace FeNodeEditor
{
	class IFeEditPointEventReceiver;

	/**
	  * @class CFeEditPoint
	  * @brief 编辑点
	  * @note 某些多顶点图元需要在每一个顶点位置创建编辑点以支持编辑，该类提供编辑点
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CFeEditPoint : public osg::MatrixTransform
	{
	public:
		/**  
		  * @note 构造编辑点 
		  * @param mapNode [in] 地球节点
		  * @param receiver [in] 事件接收对象
		  * @param pointIndex [in] 编辑点索引
		*/
		CFeEditPoint(osgEarth::MapNode* mapNode, IFeEditPointEventReceiver* receiver, int pointIndex);

		/**  
		  * @note 获取索引 
		*/
		int  GetPointIndex() const { return m_pointIndex; }

		/**  
		  * @note 设置颜色 
		*/
		void SetColor(const osg::Vec4 vecColor);

		/**  
		  * @note 设置位置 
		*/
		void SetPosition( const osgEarth::GeoPoint& position);

		/**  
		  * @note 遍历函数 
		*/
		void traverse(osg::NodeVisitor& nv);

		/**  
		  * @note 处理事件 
		*/
		bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	private:
		/**  
		  * @note 创建显示球体图形 
		*/
		osg::Node* CreateSperePoint();

	private:
		/// 编辑点事件接受对象
		IFeEditPointEventReceiver*            m_pEventReceiver;
		/// 编辑点索引
		int                                   m_pointIndex;
		/// 地球节点
		osg::observer_ptr<osgEarth::MapNode>  m_opMapNode;
		/// 绘制对象
		osg::observer_ptr<osg::ShapeDrawable> m_shapeDrawable;
		/// 记录鼠标按下位置
		osg::Vec2                             m_activePushPos;
		/// 是否激活编辑器
		bool                                  m_bActive;
	};
}

#endif // FE_NODE_EDIT_TOOL_H


