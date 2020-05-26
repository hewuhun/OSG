/**************************************************************************************************
* @file ExternNodeEditing.h
* @note 外部扩展节点编辑器的基类，主要编辑节点的位置信息
* @author c00005
* @data 2016-7-4
**************************************************************************************************/
#ifndef FE_EXTERN_NODE_EDITING_H
#define FE_EXTERN_NODE_EDITING_H

#include <osg/Group>
#include <osgEarthAnnotation/Draggers>

#include <FeExtNode/Export.h>
#include <FeExtNode/FeNodeEditTool.h>

namespace FeUtil
{
	class CRenderContext;
}

namespace FeExtNode
{
	class CExternNode;

	/**
	* @class CExternNodeEditing
	* @note 扩展节点编辑器的基类
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExternNodeEditing : public FeNodeEditor::CFeNodeEditTool
	{
	public:
		/**  
		  * @note 构造函数
		  * @param pRender [in] 当前渲染上下文
		  * @param pEditNode [in] 编辑节点
		  * @param editorTypeSupport [in] 支持的编辑器种类
		*/
		CExternNodeEditing(FeUtil::CRenderContext* pRender, CExternNode* pEditNode, 
			unsigned int editorTypeSupport);

		/**  
		  * @note 析构函数
		*/
		virtual ~CExternNodeEditing();

	protected:
		/**  
		  * @note 计算编辑器缩放系数
		*/
		virtual float GetEditorScaleVal();

		/**  
		  * @note 计算转换到指定坐标点的平移矩阵
		*/
		virtual osg::Matrix ComputeTransMatrixByWorldXYZ(const osg::Vec3d& worldXYZ);

		/**  
		  * @note 编辑器裁剪操作
		*/
		void traverse( osg::NodeVisitor& nv );
		bool IsNeedCull( const osg::Vec3d& XYZ, const osg::Vec3d& eye );

		/**  
		  * @note 内部更新编辑器
		*/
		virtual void UpdateEditingInternal();

	protected:
		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>  m_opRenderContext;

		float m_fScaleFactor;
		osg::Vec3 m_eye, m_center, m_up;

		/// 是否需要重绘制
		bool m_bNeedRedraw;
	};
}


namespace FeExtNode
{
	/**
	* @class CExEditor2D
	* @note 2D编辑器
	* @author g00034
	*/
	class FEEXTNODE_EXPORT CExEditor2D : public CExternNodeEditing
	{
	public:
		/// 编辑器所在平面
		enum E_EDITOR_PLANE
		{
			PLANE_XY = 0, // XY平面编辑器
			PLANE_XZ,     // XZ平面编辑器
			PLANE_YZ,     // YZ平面编辑器
		};

		/**  
		  * @note 构造函数
		  * @param pRender [in] 当前渲染上下文
		  * @param pEditNode [in] 编辑节点
		  * @param editorPlane [in] 2D编辑器所在平面
		  * @param editorTypeSupport [in] 支持的编辑器种类，默认支持平移
		*/
		CExEditor2D(FeUtil::CRenderContext* pRender, CExternNode* pEditNode, 
			E_EDITOR_PLANE editorPlane = PLANE_XY, unsigned int editorTypeSupport = EDITOR_TRANS);
	};
}



namespace FeExtNode
{
	/**
	* @class CExPointEditing
	* @note 点类型编辑器
	* @author g00034
	*/
	class FEEXTNODE_EXPORT CExPointEditing : public CExEditor2D
	{
	public:
		/**  
		  * @note 构造函数
		  * @param pRender [in] 当前渲染上下文
		  * @param pEditNode [in] 编辑节点
		  * @param editorPlane [in] 2D编辑器所在平面
		  * @param editorTypeSupport [in] 支持的编辑器种类，默认支持平移
		*/
		CExPointEditing(FeUtil::CRenderContext* pRender, CExternNode* pEditNode, 
			E_EDITOR_PLANE editorPlane = PLANE_XY, unsigned int editorTypeSupport = EDITOR_TRANS)
		: CExEditor2D(pRender, pEditNode, editorPlane, editorTypeSupport)
		{
		}
	};
}


namespace FeExtNode
{
	/**
	* @class CExLocalizedNodeEditing
	* @note Localized类图元编辑器
	* @author g00034
	*/
	class FEEXTNODE_EXPORT CExLocalizedNodeEditing : public CExternNodeEditing
	{
	public:
		/**  
		  * @note 构造函数
		  * @param pRender [in] 当前渲染上下文
		  * @param pEditNode [in] 编辑节点
		  * @param editorTypeSupport [in] 支持的编辑器种类，默认支持平移
		*/
		CExLocalizedNodeEditing(FeUtil::CRenderContext* pRender, CExternNode* pEditNode, 
			unsigned int editorTypeSupport = EDITOR_TRANS)
			:CExternNodeEditing(pRender, pEditNode, editorTypeSupport)
		{
			ShowTransEditor();

			if(m_rpTransDragger.valid())
			{
				m_rpTransDragger->ShowAxisZ(false);
			}
		}
	};
}

namespace FeExtNode
{
	/**
	* @class CExFeatureNodeEditing
	* @note FeatureNode类图元编辑器
	* @author g00034
	*/
	class FEEXTNODE_EXPORT CExFeatureNodeEditing : public CExternNodeEditing
	{
	public:
		/**  
		  * @note 构造函数
		  * @param pRender [in] 当前渲染上下文
		  * @param pEditNode [in] 编辑节点
		  * @param editorTypeSupport [in] 支持的编辑器种类，默认支持平移
		*/
		CExFeatureNodeEditing(FeUtil::CRenderContext* pRender, CExternNode* pEditNode, 
			unsigned int editorTypeSupport = EDITOR_TRANS)
			:CExternNodeEditing(pRender, pEditNode, editorTypeSupport)
		{
			ShowTransEditor();

			if(m_rpTransDragger.valid())
			{
				m_rpTransDragger->ShowAxisZ(false);
			}
		}
	};
}


namespace FeExtNode
{
	/**
	* @class CExModelEditing
	* @note 模型编辑器
	* @author g00034
	*/
	class FEEXTNODE_EXPORT CExModelEditing : public CExternNodeEditing
	{
	public:
		/**  
		  * @note 构造函数
		  * @param pRender [in] 当前渲染上下文
		  * @param pEditNode [in] 编辑节点
		  * @param editorTypeSupport [in] 支持的编辑器种类，默认支持所有编辑器
		*/
		CExModelEditing(FeUtil::CRenderContext* pRender, CExternNode* pEditNode, 
			unsigned int editorTypeSupport = EDITOR_TRANS|EDITOR_SCALE|EDITOR_ROTATE)
		: CExternNodeEditing(pRender, pEditNode, editorTypeSupport)
		{
			ShowTransEditor();
		}
	};
}


#endif //FE_EXTERN_NODE_EDITING_H
