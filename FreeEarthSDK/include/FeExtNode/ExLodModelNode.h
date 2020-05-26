/**************************************************************************************************
* @file ExLodModelNode.h
* @note 拥有LOD功能的模型节点，分级显示不同模型
* @author g00034
* @data 2016-9-12
**************************************************************************************************/

#include <FeExtNode/Export.h>
#include <FeExtNode/ExternNode.h>
#include <FeExtNode/ExLodNode.h>
#include <FeUtils/RenderContext.h>

#ifndef FE_EXTERN_MODEL_LOD_NODE_H
#define FE_EXTERN_MODEL_LOD_NODE_H


namespace FeExtNode
{
	
	/**
	  * @class CExLodModelGroupOption
	  * @note LOD 模型组节点配置选项
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExLodModelGroupOption : public CExLodNodeOption
	{
	public:
		CExLodModelGroupOption(){}
		~CExLodModelGroupOption(){}

		osg::Vec3d& PositionLLHDegree() { return m_vecLLHDegree; }
		osg::Vec3d& ScaleDegree() { return m_vecScaleDegree; }
		osg::Vec3d& PostureDegree() { return m_vecPostureDegree; }

	private:
		/// 模型位置，经纬高
		osg::Vec3d   m_vecLLHDegree;

		/// 模型缩放
		osg::Vec3d   m_vecScaleDegree;

		/// 模型姿态
		osg::Vec3d   m_vecPostureDegree;
	};


	/**
	  * @class CExLodModelGroup
	  * @note LOD 模型组节点，可以添加多个 LOD 模型
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExLodModelGroup : public CExLodNode
	{
	public:
		/// 枚举子节点受父节点变换矩阵的影响 
		enum E_CHILD_AFFECT_BY_TRANSFORM{
			TRANS_EFFECT = 0,           // 子节点只受父节点的平移矩阵影响
			TRANS_ROTATE_EFFECT,        // 子节点同时受父节点的平移和旋转矩阵影响
			TRANS_ROTATE_SCALE_EFFECT,  // 子节点同时受父节点的平移、缩放和旋转矩阵影响
		};

		/// LOD节点信息
		typedef struct LodNodeInfo
		{
			osg::ref_ptr<CExLodNode>    node;         // 模型节点
			std::string                 nodePath;     // 模型路径
			E_CHILD_AFFECT_BY_TRANSFORM transEffect;  // 子节点受父节点变换矩阵的影响

			/**  
			  * @note 构造函数
			*/
			LodNodeInfo(){}
			LodNodeInfo(CExLodNode* pNode, E_CHILD_AFFECT_BY_TRANSFORM effect=TRANS_ROTATE_SCALE_EFFECT, const std::string& str="")
			{
				node = pNode; nodePath = str; transEffect = effect;
			}

			/**  
			  * @note 重载比较运算符
			*/
			bool operator == (const LodNodeInfo& info)
			{
				if(&info == this) return true;
				return (info.node == node && info.nodePath == nodePath && info.transEffect == transEffect);
			}
		} LodNodeInfo;
		typedef std::list<LodNodeInfo> LOD_NODE_INFO_LIST;


	public:
		/**  
		  * @note 构造函数
		  * @param opt [in] 节点的属性配置集合
		  * @param pRender [in] 当前渲染上下文
		*/
		CExLodModelGroup(CExLodModelGroupOption* opt, FeUtil::CRenderContext* pRender);
		~CExLodModelGroup();

		/**  
		  * @note 添加 Lod 节点
		  * @param osg::Node* [in] 模型节点
		  * @param E_CHILD_AFFECT_BY_TRANSFORM [in] 如何受父节点矩阵变化的影响
		  * @return 返回生成的 lod 节点
		*/
		CExLodNode* AddLodNode(osg::Node*, E_CHILD_AFFECT_BY_TRANSFORM);

		/**  
		  * @note 添加 Lod 节点
		  * @param osg::Node* [in] 模型节点路径
		  * @param E_CHILD_AFFECT_BY_TRANSFORM [in] 如何受父节点矩阵变化的影响
		  * @return 返回生成的 lod 节点
		*/
		CExLodNode* AddLodNode(const std::string&, E_CHILD_AFFECT_BY_TRANSFORM);

		/**  
		  * @note 移除 Lod 节点
		*/
		bool RemoveLodNode(CExLodNode*);

		/**  
		  * @note 设置模型位置
		*/
		void SetPositionByLLH(const osg::Vec3d& vecLLHDegree);

		/**  
		  * @note 获取模型位置
		*/
		const osg::Vec3d& GetPositionLLH(); 
		
		/**  
		  * @note 设置缩放
		  * @param vecScale [in] 坐标轴缩放比例
		*/
		void SetScale(const osg::Vec3d& vecScale);

		/**  
		  * @note 获取缩放值
		*/
		osg::Vec3d GetScale();

		/**  
		  * @note 设置旋转角度
		  * @param vecAngle [in] 各坐标轴旋转角度
		*/
		void SetRotate( const osg::Vec3d& vecAngle );
		
		/**  
		  * @note 获取旋转角度
		*/
		osg::Vec3d GetRotate();

		/**  
		  * @note 继承自osg::Group,用于在每一帧中遍历节点，
		  *		  进而判断节点是否在可视视距范围内，从而控制显示或者隐藏
		*/
		virtual void traverse(osg::NodeVisitor& nv);

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

	
		/** 编辑器相关接口 */
	protected:
		/**
		  * @note 创建编辑工具
		*/
		virtual FeNodeEditor::CFeNodeEditTool* CreateEditorTool();

		/**
		  * @note 开始拖拽编辑
		*/
		virtual void DraggerStart();

		/**
		  * @note 拖拽编辑器矩阵变化量
		*/
		virtual void DraggerUpdateMatrix(const osg::Matrix& matrix);

	public:
		/**
		  * @note 获取编辑点旋转角度（单位：角度）
		*/
		virtual osg::Vec3d GetEditPointRotateDegree();
	
		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExLodModelGroupOption* GetOption();

	protected:
		/**  
		  * @note 添加节点信息到 lod 节点列表中
		*/
		void AddLodNode2List(const LodNodeInfo& info) { m_listLodNode.push_back(info); }

		/**  
		  * @note 从 lod 节点列表中移除节点信息
		*/
		void RemoveLodNodeFromList(const LodNodeInfo& info) { m_listLodNode.remove(info); }

		/**  
		  * @note 添加节点到场景中
		*/
		bool AddLodNode2Scene(const LodNodeInfo& info);

		/**  
		  * @note 从场景中移除节点
		*/
		bool RemoveLodNodeFromScene(const LodNodeInfo& info);

		/**  
		  * @note 从 lod 列表中获取指定节点信息
		*/
		LodNodeInfo* GetLodNodeInfo(const CExLodNode* pNode);

		/**  
		  * @note 从 lod 列表中获取指定节点信息
		*/
		LodNodeInfo* GetLodNodeInfo(unsigned int nIndex);

		/**  
		  * @note 为指定节点，创建 lod 节点并返回
		*/
		CExLodNode* CreateLodNode(osg::Node* pNode, const double& dMinRange=-FLT_MAX, const double& dMaxRange=FLT_MAX);

		/**  
		  * @note 根据指定节点路径，创建 lod 节点并返回
		*/
		CExLodNode* CreateLodNode(const std::string& nodePath, const double& dMinRange=-FLT_MAX, const double& dMaxRange=FLT_MAX)
		{
			return CreateLodNode(osgDB::readNodeFile(nodePath), dMinRange, dMaxRange);
		}

	private:
		/// LOD 节点列表，保存多个 lod 分级模型的信息
		LOD_NODE_INFO_LIST                         m_listLodNode;

		/// 位置变换矩阵节点
		osg::ref_ptr<osg::MatrixTransform>         m_rpMtTrans;

		/// 缩放变换矩阵节点
		osg::ref_ptr<osg::MatrixTransform>         m_rpMtScale;

		/// 旋转变换矩阵节点
		osg::ref_ptr<osg::MatrixTransform>         m_rpMtRotate;

		/// 模型根节点
		osg::ref_ptr<osg::Group>                   m_rpRootNode;

		/// 位置变换矩阵
		osg::Matrix								   m_mxTrans;

		/// 缩放变换矩阵
		osg::Matrix								   m_mxScale;

		/// 旋转变换矩阵
		osg::Matrix                                m_mxRotate;

		/// 记录编辑器拖拽开始矩阵
		osg::Matrix                                m_mxTransStart;
		osg::Matrix                                m_mxScaleStart;
		osg::Matrix                                m_mxRotateStart;

		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>  m_opRenderContext;
	};

}



namespace FeExtNode
{
	
	/**
	  * @class CExLodModelNodeOption
	  * @note LOD模型节点配置选项
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExLodModelNodeOption : public CExLodModelGroupOption
	{
	public:
		enum E_MODEL_TYPE
		{
			E_MODEL_TYPE_LOCAL = 0,			//本地模型
			E_MODEL_TYPE_SERVICE				//服务模型
		};

		int&	ModelType(){return m_nModelType; }

		CExLodModelNodeOption(){}
		~CExLodModelNodeOption(){}

		///模型数据类型
		int					m_nModelType;
	};


	/**
	  * @class CExLodModelNode
	  * @note LOD模型节点类，主要用于分级显示不同模型
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExLodModelNode : public CExLodModelGroup
	{
	public:
		/// LOD分级枚举
		enum E_LOD_LEVEL
		{
			LOD_MODEL_BEST = 0,   // 精细模型
			LOD_MODEL_NORMAL,     // 普通模型
			LOD_MODEL_SIMPLE,     // 简单模型

			LOD_MODEL_COUNT       // 分级数
		};

		/**  
		  * @note 构造函数
		  * @param opt [in] 节点的属性配置集合
		  * @param pRender [in] 当前渲染上下文
		*/
		CExLodModelNode(CExLodModelNodeOption* opt, FeUtil::CRenderContext* pRender);
		~CExLodModelNode();

		/**  
		  * @note 设置LOD模型，如果不存在则创建
		  * @param node_level [in] 模型级别
		  * @param pNode [in] 模型路径
		  * @param dMinRange [in] 最小可视视距
		  * @param dMaxRange [in] 最大可视视距
		  * @return 返回是否成功
		*/
		bool SetLodModelNode(E_LOD_LEVEL node_level, const std::string& nodePath, const double& dMinRange=-FLT_MAX, const double& dMaxRange=FLT_MAX);
		
		/**  
		  * @note 设置LOD模型，如果不存在则创建
		  * @param node_level [in] 模型级别
		  * @param pNode [in] 模型路径
		  * @param dMinRange [in] 最小可视视距
		  * @param dMaxRange [in] 最大可视视距
		  * @return 返回是否成功
		*/
		bool SetLodModelNode(E_LOD_LEVEL node_level, osg::Node* pNode, const double& dMinRange=-FLT_MAX, const double& dMaxRange=FLT_MAX);

		/**  
		  * @note 获取LOD模型信息
		  * @param node_level [in] 模型级别
		 * @return 返回模型信息
		*/
		LodNodeInfo GetLodModelNodeInfo(E_LOD_LEVEL node_level);

		/**  
		  * @note 删除LOD模型
		  * @param node_level [in] 模型级别
		  * @return 返回是否删除成功
		*/
		bool RemoveLodModel(E_LOD_LEVEL node_level);

		/**  
		  * @note 设置模型可视视距  
		  * @param node_level [in] 模型级别
		  * @param dMinRange [in] 最小可视视距
		  * @param dMaxRange [in] 最大可视视距
		  * @return 是否设置成功
		*/
		bool SetLodModelRange(E_LOD_LEVEL node_level, const double& dMinRange, const double& dMaxRange);

		/**  
		  * @note 获取模型可视视距  
		  * @param node_level [in] 模型级别
		  * @param dMinRange [out] 最小可视视距
		  * @param dMaxRange [out] 最大可视视距
		  * @return 是否获取成功
		*/
		bool GetLodModelRange(E_LOD_LEVEL node_level, double& dMinRange, double& dMaxRange);

		/**  
		  * @note 设置当前模型数据类型(服务 本地)
		*/
		void SetModelType(int);

		/**  
		  * @note 设置当前模型数据类型(服务 本地)
		*/
		int GetModelType();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);
	
		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExLodModelNodeOption* GetOption();
	};
}


#endif // FE_EXTERN_MODEL_LOD_NODE_H


