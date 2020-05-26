/**************************************************************************************************
* @file ExTiltModelNode.h
* @note 倾斜摄影数据节点
* @author c00005
* @data 2016-7-4
**************************************************************************************************/
#ifndef FE_EXTERN_TILT_MODEL_NODE_H
#define FE_EXTERN_TILT_MODEL_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExternNode.h>
#include <FeExtNode/ExTiltReaderWriter.h>

#include <FeUtils/RenderContext.h>

#include <osg/MatrixTransform>
#include <osgEarth/MapNode>

using namespace FeUtil;
using namespace osgEarth;

namespace FeExtNode
{
	/**
	* @class CExTiltModelNodeOption
	* @note 倾斜摄影数据节点的配置选项
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExTiltModelNodeOption : public CExternNodeOption
	{
	public:
		///倾斜摄影加载类型
		enum E_TILT_TYPE
		{
			E_TILT_TYPE_FILEPATH = 0,
			E_TILT_TYPE_DTATPATH,
			E_TILT_TYPE_SERVICE 
		};
		/**  
		* @note 构造函数
		*/
		CExTiltModelNodeOption();

		/**  
		* @note 析构函数
		*/
		virtual ~CExTiltModelNodeOption();

	public:
		///倾斜摄影数据的配置文件路径
		std::string& tiltPath(){return m_strTiltPath;}

		///倾斜摄影数据的配置信息
		CExTiltConfig& tiltConfig(){return m_configTilt;}

		///获取倾斜摄影数据类型
		int&		tileType(){return m_nType;}

	protected:
		///倾斜摄影数据的配置文件路径
		std::string		m_strTiltPath;

		///倾斜摄影数据的配置信息
		CExTiltConfig		m_configTilt;

		///倾斜摄影加载数据类型
		int						   m_nType;
	};
}

namespace FeExtNode
{
	/**
	* @class CExTiltModelNodeOption
	* @note 倾斜摄影数据节点
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExTiltModelNode : public CExternNode
	{
	public:
		/**  
		* @note 倾斜摄影数据节点
		* @param opt [in] 倾斜摄影数据节点的配置属性集合
		*/
		CExTiltModelNode(FeUtil::CRenderContext* pRenderContext, CExTiltModelNodeOption* opt);

		/**  
		* @note 析构函数
		*/
		~CExTiltModelNode();

	public:
		/**  
		* @note 设置倾斜摄影数据的配置文件路径
		*/
		virtual void SetTiltPath(const std::string& strPath);

		/**  
		* @note 设置服务倾斜摄影数据的配置文件路径
		*/
		virtual void SetServiceTiltPath(const std::string& strPath);

		/**  
		* @note 获取倾斜摄影数据的配置文件路径
		*/
		virtual std::string GetTiltPath();

		/**  
		* @note 设置倾斜摄影数据的缩放系数
		*/
		virtual void SetScale(const osg::Vec3d& vecScale);

		/**  
		* @note 获取倾斜摄影数据的缩放系数
		*/
		virtual osg::Vec3d GetScale();

		/**  
		* @note 设置倾斜摄影数据的旋转系数
		*/
		virtual void SetRotate(const osg::Vec3d& vecRotate);

		/**  
		* @note 获取倾斜摄影数据的旋转系数
		*/
		virtual osg::Vec3d GetRotate();

		/**  
		* @note 设置倾斜摄影数据放置的位置（经纬高，角度值）
		*/
		virtual void SetPosition(const osg::Vec3d& vecLLH);

		/**  
		* @note 获取倾斜摄影数据的位置（经纬高，角度值）
		*/
		virtual osg::Vec3d GetPosition();

		/**  
		* @note 设置倾斜摄影数据位置偏移量（单位米）
		*/
		virtual void SetOffset(const osg::Vec3d& vecOffset);
		
		/**  
		* @note 获取倾斜摄影数据位置偏移量（单位米）
		*/
		virtual osg::Vec3d GetOffset();

		/**  
		  * @brief 遍历数据目录，解析数据信息，并进行加载
		*/
		void GenerateData(std::string strPath);

		/**  
		  * @note 设置倾斜摄影的数据加载类型
		*/
		void SetTiltType(int type);

		/**  
		  * @note 获取倾斜摄影的数据加载类型
		*/
        int GetTiltType();

	protected:
		/**  
		* @note 初始化节点
		*/
		void InitNode();

		/**  
		* @note 解析本地倾斜摄影数据的配置文件，并将数据进行加载
		 * @parma[in] bool bService  true为服务 false为本地
		*/
		void ParseTiltConfig(bool bService = false);

	public:
		/**  
		* @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExTiltModelNodeOption* GetOption();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

	protected:
		///渲染的图形上下文，保存了MapNode、Viewer等信息
		osg::observer_ptr<FeUtil::CRenderContext>	m_opRenderContext;

		///位置矩阵
		osg::ref_ptr<osg::MatrixTransform>			m_rpTransMt;

		///位置偏移矩阵
		osg::ref_ptr<osg::MatrixTransform>			m_rpOffsetMt;

		///旋转矩阵
		osg::ref_ptr<osg::MatrixTransform>			m_rpRotateMt;

		///缩放矩阵
		osg::ref_ptr<osg::MatrixTransform>			m_rpScaleMt;
	};
}

#endif //FE_EXTERN_TILT_MODEL_NODE_H
