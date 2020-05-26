/**************************************************************************************************
* @file IFeNodeEditable.h
* @note 节点编辑接口类
* @author g00034
* @data 2016-11-3
**************************************************************************************************/

#ifndef I_FE_NODE_EDITOR_H
#define I_FE_NODE_EDITOR_H

#include <osg/Matrix>
#include <FeExtNode/FeNodeEditTool.h>

namespace FeNodeEditor
{
	/**
	* @class IFeNodeEditable
	* @brief 节点编辑接口类
	* @note 需要支持编辑操作的节点需要继承该接口
	* @author g00034
	*/
	class IFeNodeEditable
	{
		friend class CFeNodeEditTool;

	protected:
		IFeNodeEditable(void):m_bEditting(false){}
		virtual ~IFeNodeEditable(void){}

	public:
		/**
		  * @note 开始编辑状态
		*/
		bool StartEdit() { return m_bEditting = StartEditImplement(); }

		/**
		  * @note 结束编辑状态
		*/
		bool StopEdit() { m_bEditting = !StopEditImplement(); return !m_bEditting; }

		/**
		  * @note 是否开启了编辑状态
		*/
		bool IsEditting(){ return m_bEditting; }

		/**
		  * @note 获取编辑工具
		*/
		FeNodeEditor::CFeNodeEditTool* GetOrCreateEditorTool() {return (m_rpEditTool.valid())?m_rpEditTool:(m_rpEditTool = CreateEditorTool());}

		/**
		  * @note 获取编辑点位置
		*/
		virtual osg::Vec3d GetEditPointPositionXYZ() = 0;

		/**
		  * @note 获取编辑点旋转角度（单位：角度）
		*/
		virtual osg::Vec3d GetEditPointRotateDegree() = 0;

	protected:
		/**
		  * @note 开始编辑状态
		*/
		virtual bool StartEditImplement() {return true;}

		/**
		  * @note 结束编辑状态
		*/
		virtual bool StopEditImplement() {return true;}
		
		/**
		  * @note 创建编辑工具
		*/
		virtual CFeNodeEditTool* CreateEditorTool() = 0;
		
		/**
		  * @note 编辑工具开始拖拽
		*/
		virtual void DraggerStart() = 0;

		/**
		  * @note 拖拽编辑工具时矩阵变化量
				注意： 目前，旋转和缩放的变化量使用局部坐标系，平移的变化量使用全局坐标系
		*/
		virtual void DraggerUpdateMatrix(const osg::Matrix& matrix) = 0;

		/**
		  * @note 编辑工具结束拖拽
		*/
		virtual void DraggerEnd() = 0;

		/**  
		  * @note 更新编辑器
		*/
		virtual void UpdateEditor(){ if(m_rpEditTool.valid()) m_rpEditTool->UpdateEditing(); }
	
	private:
		///扩展节点编辑状态
		bool					            m_bEditting;

		///扩展节点编辑工具
		osg::ref_ptr<CFeNodeEditTool>  	    m_rpEditTool;
	};

}


namespace FeNodeEditor
{
	/**
	  * @class IFeEditPointEventReceiver
	  * @brief 编辑点事件接收接口
	  * @note 某些多顶点图元需要在每一个顶点位置创建编辑点以支持编辑，该类作为这些编辑点的事件接收类
	  * @author g00034
	*/
	class IFeEditPointEventReceiver
	{
	public:
		/**
		  * @note 激活编辑点
		  * @param editPoint [in] 被激活的编辑点
		*/
		virtual void ActiveEditPoint(const CFeEditPoint& editPoint) = 0;
	};
}

#endif // I_FE_NODE_EDITOR_H


