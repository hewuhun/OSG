/**************************************************************************************************
* @file ExternModelNode.h
* @note 场景实体模型
* @author g00034
* @data 2016-12-19
**************************************************************************************************/
#ifndef FE_MODEL_ELEMENT_H
#define FE_MODEL_ELEMENT_H

#include <osg/AutoTransform>
#include <FeExtNode/Export.h>
#include <FeExtNode/ExternNode.h>
#include <FeUtils/RenderContext.h>

namespace FeExtNode
{
	/**
	  * @class CExModelNodeOption
	  * @brief 模型的配置信息
	  * @author g00034
	*/
	struct FEEXTNODE_EXPORT CExModelNodeOption : public CExternNodeOption
	{
	public:
		CExModelNodeOption()
			: m_vecPosition(0,0,0)
			, m_vecPosture(0,0,0)
			, m_vecScale(1,1,1)
		{
		}
		~CExModelNodeOption(){}
	public:
		osg::Vec3d& Position() { return m_vecPosition; }
		osg::Vec3d& Posture() { return m_vecPosture; }
		osg::Vec3d& Scale() { return m_vecScale; }
		std::string& ModelPath() { return m_strModelPath; }

	private:
		/// 位置（单位：经纬度）
		osg::Vec3d									m_vecPosition;
		/// 姿态（单位：角度）
		osg::Vec3d									m_vecPosture;
		/// 缩放
		osg::Vec3d									m_vecScale;
		/// 模型路径
		std::string									m_strModelPath;
	};
}

namespace FeExtNode
{
	/**
	  * @class CExModelNode
	  * @note 场景中的模型类
	  * @author z00013
	*/
	class FEEXTNODE_EXPORT CExModelNode : public CExternNode
	{
	public:
		CExModelNode(FeUtil::CRenderContext* pRender, CExModelNodeOption* opt);

		virtual ~CExModelNode();

	public:
		/**  
		  * @note 设置模型位置以及获得模型的位置
		  * @param 参数 [in] vecPos 位置值（单位：经纬度）
		*/
		void SetPosition(const osg::Vec3d& vecPos);
		osg::Vec3d& GetPosition();

		/**  
		  * @note 设置模型方位角以及获得模型的方位角
		  * @param 参数 [in] vecPosture 方位角值（单位：角度）
		*/
		void SetPosture(const osg::Vec3d& vecPosture);
		osg::Vec3d& GetPosture();

		/**  
		  * @note 设置模型缩放比例以及获得模型的缩放比例
		  * @param 参数 [in] vecScale 缩放比例
		*/
		void SetScale(const osg::Vec3d& vecScale);
		osg::Vec3d& GetScale();

		/**  
		  * @note 模型资源路径设置 
		*/
		void SetModelPath(const std::string& strModelPath);
		const std::string& GetModelPath();

		/**  
		  * @note 获取模型节点
		*/
		osg::Node* GetModelNode();

		/**
		*@note 获得matrixtransform
		*/
        osg::MatrixTransform*  GetTransMT();

		/**  
		  * @note 获取场景节点
		*/
		osg::Group* GetSceneNode();

		/**  
		  * @note 设置模型颜色
		*/
		void SetModelColor( osg::Vec4d vecColor );
		osg::Vec4d GetModelColor() const { return m_vecColor; }

		/**  
		  * @note 设置模型选中颜色
		*/
		void SetSelectedColor( osg::Vec4d vecColor ) { m_vecColorSeleced = vecColor; }
		osg::Vec4d GetSelectedColor() const { return m_vecColorSeleced; }

		/**  
		  * @note 设置模型选中状态
		*/
		void SetSelectedState(bool bSelected);
		bool GetSelectedState() const;

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		//获取旋转姿态指针
		osg::MatrixTransform* GetRotateMT() const;

		//获取缩放姿态指针
		osg::MatrixTransform* GetScaleMT() const;
		virtual void Accept(CExternNodeVisitor& nv);


	protected:
		/**  
		  * @note 构建内部元素
		*/
		virtual bool CreateElement();
		

	protected:
		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExModelNodeOption* GetOption();
		
	protected:
		/// 模型颜色
		osg::Vec4d                                  m_vecColor;
		osg::Vec4d                                  m_vecColorSeleced;

		/// 模型节点
		osg::ref_ptr<osg::Node>						m_pModelNode;
		osg::ref_ptr<osg::Group>					m_pSceneNode;
		
		/// 位置变换矩阵节点
		osg::ref_ptr<osg::MatrixTransform>          m_rpMtPosition;
		osg::ref_ptr<osg::MatrixTransform>			m_rpRotateMT;
		osg::ref_ptr<osg::MatrixTransform>			m_rpScaleMT;
		osg::ref_ptr<osg::AutoTransform>			m_rpAutoScreenAT;
		
		osg::Matrix			                        m_mxTrans;
		osg::Matrix	                        		m_mxRotate;
		osg::Matrix                          		m_mxScale;
		bool                                        m_bSelected;
		bool                                        m_bSetColorBeforSelected;//模型选中后，再恢复，会造成在选中操作之前给模型设置的颜色丢失，因此以此标记

		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>   m_opRenderContext;
	};
}


#endif //FE_MODEL_ELEMENT_H