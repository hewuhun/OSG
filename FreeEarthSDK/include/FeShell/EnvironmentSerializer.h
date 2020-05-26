/**************************************************************************************************
* @file EnvironmentSerializer.h
* @note 环境配置文件解析器
* @author c00005
* @data 2017-4-1
**************************************************************************************************/
#ifndef SHELL_ENVOIRMENT_CONFIG_SERIALIZER_H
#define SHELL_ENVOIRMENT_CONFIG_SERIALIZER_H

#include <FeShell/Export.h>

#include <FeUtils/tinyXml/tinyxml.h>
#include <FeUtils/tinyXml/tinyutil.h>

#include <string>
#include <map>

namespace FeShell
{
	typedef std::map<std::string, std::string> Configs;

	///太阳、月亮、星云等配置信息
	struct SEnvLightSetData
	{
		bool bSun;
		bool bMoon;
		bool bStar;
		bool bNebula;
		bool bAtmosphere;

		float fDensity;

		bool ParseConfig(std::string strName, std::string strValue);

		void GetConfig(Configs& configs);
	};

	///雨效果配置信息
	struct SEnvRainSetData 
	{
		bool bRain;
		bool bGlobal;

		double dDensity;
		float fheigth;

		float fLatitude;
		float fLongitude;
		float fRadius;

		bool ParseConfig(std::string strName, std::string strValue);

		void GetConfig(Configs& configs);
	};

	///雪效果配置信息
	struct SEnvSnowSetData 
	{
		bool bSnow;
		bool bGlobal;

		double dDensity;
		float fheigth;

		float fLatitude;
		float fLongitude;
		float fRadius;

		bool ParseConfig(std::string strName, std::string strValue);

		void GetConfig(Configs& configs);
	};

	///雾效果配置信息
	struct SEnvFogSetData 
	{
		bool bFog;
		bool bGlobal;

		double dDensity;
		double dHight;
		float fLatitude;
		float fLongitude;
		float fRadius;

		bool ParseConfig(std::string strName, std::string strValue);

		void GetConfig(Configs& configs);
	};

	///云效果配置信息
	struct SEnvCloudSetData 
	{
		bool  bCloud;

		int nAltitude;
		int nThickness;
		int nDensity;
		int nAlpha;
		int nSpeed;
		int nDirection;

		int nCloudType;

		double dDensity;
		double dAlpha;

		bool ParseConfig(std::string strName, std::string strValue);

		void GetConfig(Configs& configs);
	};

	///海洋效果配置信息
	struct SEnvOceanSetData 
	{
		bool bOcean;

		float fOceanLevel;
		float fViewPointLevel;
		float fAboveWeatherVisible;
		float fBelowWeatherVisible;
		float fWindSpeed;
		float fWindDirection;
		float fWindLength;

		bool ParseConfig(std::string strName, std::string strValue);

		void GetConfig(Configs& configs);
	};

	/**
	* @class CEnvSetData
	* @note 环境配置信息存储类
	* @author x00028
	*/
	class FESHELL_EXPORT CEnvSetData
	{
	public:
		CEnvSetData(){}

		~CEnvSetData(){}

	public:
		SEnvLightSetData        m_sEnvLightSetData;
		SEnvRainSetData			m_sEnvRainSetData;
		SEnvSnowSetData			m_sEnvSnowSetData;
		SEnvCloudSetData		m_sEnvCloudSetData;
		SEnvOceanSetData		m_sEnvOceanSetData;
		SEnvFogSetData			m_sEnvFogSetData;
	};
}

namespace FeShell
{
	/**
	* @class CEnvConfigReader
	* @brief 语言配置的反序列化
	* @note 序列化系统语言配置的信息，将配置文件读入map
	* @author x00028
	*/
	class FESHELL_EXPORT CEnvConfigReader :public FeUtil::TiXmlVisitor
	{
	public:
		CEnvConfigReader();

		~CEnvConfigReader();

	public:
		/**  
		* @brief 获得指定的配置文件配置信息
		*/
		CEnvSetData Execute(const std::string& strFileName);

	public:
		virtual bool VisitEnter(const FeUtil::TiXmlElement& ele , const FeUtil::TiXmlAttribute* attr);

	protected:
		bool AcceptEnv(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr);

	protected:
		///环境配置信息
		CEnvSetData      m_sEnvSetDate; 
	};

	/**
	* @class CEnvConfigWriter
	* @brief 语言配置的序列化
	* @note 序列化系统语言配置的信息，从map写入配置文件
	* @author x00028
	*/
	class FESHELL_EXPORT CEnvConfigWriter : public FeUtil::TiXmlVisitor
	{
	public:
		CEnvConfigWriter();

		virtual ~CEnvConfigWriter();

	public:
		bool Execute(CEnvSetData sEnvSetData);

		bool Execute(CEnvSetData sEnvSetData, const std::string& strFileName);

	protected:
		///环境配置信息
		CEnvSetData      m_sEnvSetDate; 
	};
}

#endif//SHELL_LAN_CONFIG_SERIALIZER_H
