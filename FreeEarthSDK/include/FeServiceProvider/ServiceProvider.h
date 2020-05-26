#ifndef ServiceProvider_h__
#define ServiceProvider_h__

#include <string>
#include <vector>
#include "FeUtils/tinyXml/tinyxml.h"
#include <stack>
#include "FeServiceProvider/Export.h"

using namespace FeUtil;

TiXmlElement * ReadXmlByStack(TiXmlElement * pRootElement, char * pName, bool & bFind);

namespace FeServiceProvider
{
	class ServiceLayer;
	class CurlHelper;
	class FESERVICEPROVIDER_EXPORT ServiceProvider
	{
	public:
		ServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName = "", const std::string & strPassword = "");

		virtual ~ServiceProvider();
	public:
		virtual std::string GetUserName();

		virtual std::string GetPassword();

		virtual std::string GetName();

		virtual std::string GetUrl();

		virtual std::string GetPluginType();

		virtual void SetUserName(const std::string & strUserName);

		virtual void SetPassword(const std::string & strPassword);

		virtual void SetName(const std::string & strName);

		virtual void SetUrl(const std::string & strUrl);

		virtual void SetPluginType(const std::string & strPluginType);

		virtual std::string GetServiceUrl();

		virtual bool TestNet();

		virtual int GetLayerCount();

		virtual std::string GetServiceType();

		virtual std::vector<ServiceLayer *> GetLayers();

		virtual void GetAttr();

		virtual std::string GetCurrentVersion();

		virtual void SetVersion(const std::string & strVersion);

		virtual std::string GetXml();
	protected:
		std::string					m_strUserName; //用户名
		std::string					m_strPassword; //密码
		std::string					m_strName; //自定义名称
		std::string					m_strUrl; //基础URL
		std::string					m_strFullUrl; //带服务类型和版本的URL路径
		std::string					m_strPlugin; //插件类型
		std::string                 m_strVersion;
		int                         m_nLayerCount;
		std::vector<ServiceLayer*>  m_pLayers;
		CurlHelper                * m_pCurlHelper;
	};
}

#endif //ServiceProvider