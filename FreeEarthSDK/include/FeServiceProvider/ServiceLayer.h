#ifndef ServiceLayer_h__
#define ServiceLayer_h__

#include <string>
#include "FeServiceProvider/Export.h"

namespace FeServiceProvider
{
	class ServiceProvider;

	class  FESERVICEPROVIDER_EXPORT ServiceLayer
	{
	public:
		//服务的图层类
		ServiceLayer();

		~ServiceLayer();

	public:
		//获取EPSG Code 
		std::string GetEPSGCode();

		//获取CRSCode
		std::string GetCRSCode();

		//获取名称
		std::string GetName();

		//获取底部坐标
		std::string GetLowerCorner();

		//获取顶部坐标
		std::string GetUpperCorner();

		//获取图层的服务
		ServiceProvider * GetServiceProvider();

		//设置EPSG Code
		void SetEPSGCode(const std::string & code);

		//设置CRS Code
		void SetCRSCode(const std::string & code);

		//设置名称
		void SetName(const std::string & name);

		//设置底部坐标
		void SetLowerCorner(const std::string & lower);

		//设置顶部坐标
		void SetUpperCorner(const std::string & upper);

		//设置服务
		void SetServiceProvider(ServiceProvider * serviceProvider);

	private:
		std::string			m_strEPSGCode;
		std::string			m_strCRSCode;
		std::string			m_strName;
		std::string			m_strLower;
		std::string			m_strUpper;
		ServiceProvider *   m_pServiceProvider;
	};
}



#endif //ServiceLayer_h__