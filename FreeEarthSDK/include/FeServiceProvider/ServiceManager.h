#ifndef ServiceManager_h__
#define ServiceManager_h__

#include "FeServiceProvider/Export.h"
#include <vector>
#include "FeUtils/tinyXml/tinyxml.h"
#include <iostream>
namespace FeServiceProvider
{
	class ServiceProvider;

	class FESERVICEPROVIDER_EXPORT ServiceManager
	{
	public:
		ServiceManager();
		~ServiceManager();
	public:

		//序列化到文件
		void Serialize(const std::string & fileName);

		//反序列化
		void DeSerialize(const std::string & fileName);

		//序列化到文档根节点
		void Serialize(FeUtil::TiXmlDocument * doc);

		//通过文档根节点反序列化
		void DeSerialize(FeUtil::TiXmlDocument * doc);

		//添加服务
		void AddService(ServiceProvider * serviceProvider);

		//移除服务
		void RemoveService(ServiceProvider * serviceProvider);

		//清空服务
		void CleanService();

		//获取所有服务
		std::vector<ServiceProvider *> GetServices();

	private:
		//序列化到元素
		void Seria(FeUtil::TiXmlElement * servicesEle);
		//根据元素反序列化
		void DeSeria(FeUtil::TiXmlElement * servicesEle);
	private:
		std::vector<ServiceProvider *> m_vecServices;
	};
}


#endif //ServiceManager_h__
