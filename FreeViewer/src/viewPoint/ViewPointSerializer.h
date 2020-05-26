#ifndef VIEW_POINT_SERIALIZER_H
#define VIEW_POINT_SERIALIZER_H

#include <osgEarth/Viewpoint>

#include <FeUtils/tinyXml/tinyxml.h>
#include <FeUtils/tinyXml/tinyutil.h>

#include <FeUtils/FreeViewPoint.h>

#include <list>

using namespace FeUtil;

namespace FreeViewer
{
	/**
	  * @class CViewPointReaderWriter
	  * @brief 视点位置序列化以及反序列化
	  * @note 序列化视点位置配置信息，从配置文件中获取视点位置，或者将视点位置写入到配置文件中
	  * @author c00005
	*/
 	class CViewPointReaderWriter
	{
	public:
		/**
		*@note: 视点位置列表的序列化反序列化的构造函数
		*/
		CViewPointReaderWriter();
		
		/**
		*@note: 析构函数
		*/
		~CViewPointReaderWriter();

	public:
		/**
		*@note: 从指定的路径中读取用户自定义图层的配置
		*/
		FreeViewPoints Load(std::string strPath);

		/**
		*@note: 将用户自定义的图层保存到指定的配置文件中
		*/
		bool Save(std::string strPath);

		/**
		*@note: 向配置中添加视点位置配置项
		*/
		void AddViewPoint(CFreeViewPoint point);

		/**
		*@note: 获取视点位置列表
		*/
		FreeViewPoints GetViewPoints();

		/**
		*@note: 设置视点位置列表，并将以前存在的清空
		*/
		void SetViewPoints(FreeViewPoints viewPoints);

	protected:
		//视点位置列表
		FreeViewPoints						m_viewPoints;
	};
}

#endif//VIEW_POINT_SERIALIZER_H
