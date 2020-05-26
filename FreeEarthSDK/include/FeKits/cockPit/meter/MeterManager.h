/**************************************************************************************************
* @file MeterManager.h
* @note 管理仪表配置文件，并且根据配置文件内容生成仪表显示在场景中
* @author ty00007
* @data 2014-3-27
**************************************************************************************************/
#ifndef METERMANAGER_H
#define METERMANAGER_H

#include <FeUtils/tinyXml/tinyxml.h>
#include <FeUtils/tinyXml/tinystr.h>

#include <FeKits/cockPit/meter/AltiMeter.h>
#include <FeKits/cockPit/meter/TimeMeter.h>
#include <FeKits/cockPit/meter/VsiMeter.h>
#include <FeKits/cockPit/meter/HsiMeter.h>
#include <FeKits/cockPit/meter/AdiMeter.h>
#include <FeKits/cockPit/meter/AoaMeter.h>

#include <stdarg.h>
#include <iostream>
#include <set>
#include <vector>

/**
 * @enum 仪表类型
*/
typedef enum MeterType
{
	AdiMeter     = 1, //adi仪表
	AltiMeter       , //alti仪表
	AoaMeter        , //aoa仪表
	HsiMeter        , //Hsi仪表
	TimeMeter       , //时间仪表
	VsiMeter        , //Vsi仪表
	UnValueMeter = 0  //无效值
}MeterType;

/**
 * @struct 结构体类型的x,y,z集合
*/
typedef struct SXYZ
{
	double x;
	double y;
	double z;
}SXYZ;
/**
 * @struct 存放仪表配置文件中节点的属性值
*/
typedef struct SMeterAttribute
{
	std::string strMeterName;   //仪表名称
	SXYZ position;              //放置位子
	SXYZ scale;                 //放缩比率
}SMeterAttribute;
//仪表的配置容器
typedef     std::vector< FeUtil::TiXmlElement* >       vecMeterXmlNode;
//存放需要加载的仪表ID的集合
typedef     std::set< MeterType >              setLoadMeterID;
//存放仪表节点属性的容器
typedef     std::vector< SMeterAttribute >     vecMeterAttributeList;

/**
  * @class CMeterManager
  * @brief 管理仪表配置文件
  * @note 管理仪表配置文件，并且根据配置文件内容生成仪表显示在场景中，
  *控制仪表的显示位子、比率、是否加载。
  * @author ty00007
*/
class CMeterManager : public FeUtil::TiXmlDocument
{
public:
	CMeterManager(void);
	CMeterManager(const char* cDocumentName);
	~CMeterManager(void);

protected:
	/**  
	  * @brief 读取xml文件，生成节点属性容器、需要加载节点ID容器和仪表节点容器
	  * @note 读取xml文件，生成节点属性容器、需要加载节点ID容器和仪表节点容器
	  * @return void
	*/
	void InitializeMeters();
	/**  
	  * @brief 生成仪表节点容器
	  * @note 生成仪表节点容器
	  * @param pRoot [in] 配置文件根节点
	  * @return void
	*/
	void collectMeterXmlNode(FeUtil::TiXmlElement* pRoot);
	/**  
	  * @brief 生成需要加载的仪表的ID容器
	  * @note 生成需要加载的仪表的ID容器
	  * @param pRoot [in] 配置文件根节点
	  * @return void
	*/
	void collectLoadingMeterID(FeUtil::TiXmlElement* pRoot);
	/**  
	  * @brief 生成仪表属性容器
	  * @note 生成仪表属性容器
	  * @param ID [in] 仪表枚举值
	  * @return void
	*/
	bool createMeterAttibuteList(MeterType ID);
	/**  
	  * @brief 生成仪表属性容器
	  * @note 生成仪表属性容器
	  * @param ID [in] 仪表枚举值
	  * @return void
	*/
	void RecordMeterAttribute(FeUtil::TiXmlElement* pTixmlElement);
	//仪表节点容器
	vecMeterXmlNode       m_vecMeterXmlNode;
	//需要加载的仪表的ID容器
	setLoadMeterID        m_setLoadMeterID;
	//仪表属性容器
	vecMeterAttributeList m_vecMeterAttributeList;

public:
	
	bool LoadMeterNodeToScene(bool bflag, MeterType ID, ...);
	/**  
	  * @brief 获得仪表属性容器
	  * @note 获得仪表属性容器
	  * @return 返回仪表属性容器
	*/
	vecMeterAttributeList GetMeterAttributeList() { return m_vecMeterAttributeList; };
	/**  
	  * @brief 获得需要加载的仪表的ID容器
	  * @note 获得需要加载的仪表的ID容器
	  * @return 返回需要加载的仪表的ID容器
	*/
	setLoadMeterID GetLoadMeterIDList() { return m_setLoadMeterID; };
	
	/**  
	  * @brief 返回名称为strMeterName仪表的序列号
	  * @note 返回名称为strMeterName仪表的序列号
	  * @param strMeterName[in]  仪表名称
	  * @return 仪表的序列号
	*/
	int ReturnMeterIndex(std::string strMeterName);
};

#endif
