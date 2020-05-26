/**************************************************************************************************
* @file LanSerializer.h
* @note 系统语言序列化以及反序列化
* @author c00005
* @data 2017-4-1
**************************************************************************************************/
#ifndef SHELL_LAN_CONFIG_SERIALIZER_H
#define SHELL_LAN_CONFIG_SERIALIZER_H

#include <FeUtils/tinyXml/tinyxml.h>
#include <FeUtils/tinyXml/tinyutil.h>

#include <string>
#include <map>

#include <FeShell/Export.h>

namespace FeShell
{
	/**
	* @class CLanConfigModule
	* @brief 系统语言配置文件模型
	* @note 定义了系统语言配置文件的模型，保存和系统语言配置文件的数据信息和获取数据信息的接口
	* @author c00005
	*/
	class FESHELL_EXPORT CLanConfig
	{
	public:
		//语言的Key以及对应的语言解析文件的文件名 
		typedef std::map<std::string, std::string> LanMap;

	public:
		CLanConfig();

		~CLanConfig();

	public:
		/**  
		* @brief 是否有效
		*/
		bool Valid();

		/**  
		* @brief 设置是否有效
		*/
		void SetValid(bool bValue);

		/**  
		* @brief 设置语言列表
		*/
		void SetLanguageList(std::string strKey, std::string strValue);

		/**  
		* @brief 获取语言列表
		*/
		LanMap GetLanguageList();

		/**  
		* @brief 获取默认语言
		*/
		std::string GetDefLanguage();

		/**  
		* @brief 设置默认语言
		*/
		void SetDefLanguage(std::string strLan);

	private:
		///是否合法
		bool                        m_bValid;           

		///默认语言
		std::string					m_strDefLan;

		///语言与翻译文件的map对
		LanMap						m_mapLan;
	};
}

namespace FeShell
{
	/**
	* @class CLanConfigReader
	* @brief 语言配置的反序列化
	* @note 序列化系统语言配置的信息，从语言配置文件中获得系统所需要的语言信息
	* @author c00005
	*/
	class FESHELL_EXPORT CLanConfigReader :public FeUtil::TiXmlVisitor
	{
	public:
		CLanConfigReader();
		~CLanConfigReader();

	public:
		/**  
		* @brief 获得指定的配置文件配置信息
		*/
		CLanConfig Execute(const std::string& strFileName);

		/**  
		* @brief 获得默认的配置文件下的配置信息
		*/
		CLanConfig ExecuteDefault();

	public:
		virtual bool VisitEnter(const FeUtil::TiXmlElement& ele , const FeUtil::TiXmlAttribute* attr);

	protected:
		bool AcceptLan(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr);

	protected:
		///系统语言配置数据模型
		CLanConfig      m_configModule; 
	};

	/**
	* @class CLanConfigReader
	* @brief 语言配置的序列化
	* @note 序列化系统语言配置的信息，从语言配置文件中获得系统所需要的语言信息
	* @author c00005
	*/
	class FESHELL_EXPORT CLanConfigWriter : public FeUtil::TiXmlVisitor
	{
	public:
		CLanConfigWriter();

		virtual ~CLanConfigWriter();

	public:
		bool Execute(CLanConfig pLanConfig);

		bool Execute(CLanConfig pLanConfig, const std::string& strFileName);
	};
}

#endif//SHELL_LAN_CONFIG_SERIALIZER_H
