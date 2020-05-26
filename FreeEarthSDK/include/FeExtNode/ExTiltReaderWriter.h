/**************************************************************************************************
* @file ExTiltReaderWriter.h
* @note 倾斜摄影数据
* @author c00005
* @data 2016-7-4
**************************************************************************************************/
#ifndef FE_EXTERN_TILT_READERWRITER_H
#define FE_EXTERN_TILT_READERWRITER_H

#include <FeExtNode/Export.h>

#include <FeUtils/tinyXml/tinyxml.h>
#include <FeUtils/tinyXml/tinyutil.h>

#include <vector>

#include <osg/Vec3d>

using namespace FeUtil;

namespace FeExtNode
{
	/**
	  * @class CExTiltConfig
	  * @note 倾斜摄影数据的配置文件
	  * @author c00005
	*/
	class FEEXTNODE_EXPORT CExTiltConfig
	{
	public:
		//倾斜摄影加载类型
		enum E_TILT_TYPE
		{
			E_TILT_TYPE_FILEPATH = 0,
			E_TILT_TYPE_DTATPATH,
			E_TILT_TYPE_SERVICE 
		};

		//倾斜摄影解析文件的文件名 
		typedef std::vector<std::string> TiltList;

	public:
		CExTiltConfig();

		~CExTiltConfig();

	public:
		//名称
		std::string& name(){return m_strName;}

		//数据放置的坐标位置
		osg::Vec3d& position(){return m_vecPosition;}

		//数据偏移量，以米为单位
		osg::Vec3d& offset(){return m_vecOffset;}

		//旋转量
		osg::Vec3d& rotate(){return m_vecRotate;}

		//缩放量
		osg::Vec3d& scale(){return m_vecScale;}

		//获取节点值
		TiltList& tiltList(){return m_listTiltNode;}

		///获取倾斜摄影数据类型
		int&		tileType(){return m_type;};

	protected:
		///倾斜摄影模型的名称
		std::string              m_strName;

		///倾斜摄影数据放置位置（经纬高）
		osg::Vec3d               m_vecPosition; 

		///倾斜摄影数据位置偏移量
		osg::Vec3d               m_vecOffset;

		///倾斜摄影数据旋转角度
		osg::Vec3d               m_vecRotate;	

		///倾斜摄影数据缩放系数
		osg::Vec3d               m_vecScale;    

		///倾斜摄影数据节点集合
		TiltList                 m_listTiltNode;

		///倾斜摄影加载数据类型
		int			m_type;
	};
}

namespace FeExtNode
{
	/**
	  * @class CExTiltReaderWriter
	  * @note 倾斜摄影数据配置文件的读写
	  * @author c00005
	*/
	class FEEXTNODE_EXPORT CExTiltReaderWriter : public TiXmlVisitor
	{
	public:
		CExTiltReaderWriter();

		CExTiltReaderWriter(bool bService);

		~CExTiltReaderWriter();

	public:
		/**  
		  * @note 获得指定的配置文件中的配置信息  
		  * @param std::string [in] 配置文件的路径
		  * @param CExTiltConfig [out] 配置文件的信息
		  * @return bool 是否读取成功
		*/
		bool Read(const std::string& strFilePath, CExTiltConfig& config);

				/**  
		  * @note 获得指定的服务配置文件中的配置信息  
		  * @param std::string [in] 配置文件的路径
		  * @param CExTiltConfig [out] 配置文件的信息
		  * @return bool 是否读取成功
		*/
		bool ReadURl( const std::string& strFilePath, CExTiltConfig& config );

		/**  
		  * @note 获得指定的配置文件中的配置信息  
		  * @param std::string [in] 配置文件的路径
		  * @param CExTiltConfig [int] 配置文件的信息
		  * @return bool 是否写入成功
		*/
		bool Write(const std::string& strFilePath, CExTiltConfig& config);

	public:
		virtual bool VisitEnter(const TiXmlElement& ele , const TiXmlAttribute* attr);

	protected:
		/**  
		* @note 获取各格式倾斜摄影模型名称
		*/
		bool AcceptHgtName(const TiXmlElement& ele, const TiXmlAttribute* attr);
		bool AcceptLfpName(const TiXmlElement& ele, const TiXmlAttribute* attr);

		/**  
		* @note 获取各格式倾斜摄影数据放置位置
		*/
		bool AcceptHgtPosition(const TiXmlElement& ele, const TiXmlAttribute* attr);
		bool AcceptLfpPosition(const TiXmlElement& ele, const TiXmlAttribute* attr);
		bool AcceptScpPosition(const TiXmlElement& ele, const TiXmlAttribute* attr);

		/**  
		* @note 获取各格式倾斜摄影数据位置偏移量
		*/
		bool AcceptHgtOffset(const TiXmlElement& ele, const TiXmlAttribute* attr);
		bool AcceptLfpOffset(const TiXmlElement& ele, const TiXmlAttribute* attr);

		/**  
		* @note 获取各格式倾斜摄影数据旋转角度
		*/
		bool AcceptHgtRotate(const TiXmlElement& ele, const TiXmlAttribute* attr);
		bool AcceptLfpRotate(const TiXmlElement& ele, const TiXmlAttribute* attr);

		/**  
		* @note 获取各格式倾斜摄影数据缩放系数
		*/
		bool AcceptHgtScale(const TiXmlElement& ele, const TiXmlAttribute* attr);
		bool AcceptLfpScale(const TiXmlElement& ele, const TiXmlAttribute* attr);

		/**  
		* @note 获取各格式倾斜摄影数据节点集合
		*/
		bool AcceptHgtNodeList(const TiXmlElement& ele, const TiXmlAttribute* attr);
		bool AcceptLfpNodeList(const TiXmlElement& ele, const TiXmlAttribute* attr);
		bool AcceptScpNodeList(const TiXmlElement& ele, const TiXmlAttribute* attr);

		/**  
		  * @note 获取倾斜摄影加载数据类型
		*/
        bool AcceptTypeName( const TiXmlElement& ele, const TiXmlAttribute* attr );
		
	protected:
		///倾斜摄影数据属性配置集合
		CExTiltConfig			m_tempConfig;

		///倾斜摄影数据属性配置文件路径
		std::string				m_strTiltPath;

		//是否是服务
		bool						m_bService;
	};
}

#endif //FE_EXTERN_TILT_READERWRITER_H
