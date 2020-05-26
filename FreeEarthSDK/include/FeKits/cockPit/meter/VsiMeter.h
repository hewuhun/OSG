/**************************************************************************************************
* @file VsiMeter.h
* @note 攀升下降速度仪表
* @author ty00007
* @data 2014-3-22
**************************************************************************************************/
#ifndef _VSIMETER_H
#define _VSIMETER_H

#include <FeUtils/tinyXml/tinyxml.h>
#include <FeUtils/tinyXml/tinystr.h>

#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <iostream>

typedef std::map<double, double> mapClimbRate;
typedef std::vector<FeUtil::TiXmlElement*> vecXmlElements;
/**
  * @class CVsiMeter
  * @brief 攀升仪表类
  * @note 显示飞机飞行垂直方向上上升或者下降的速度
  * @author ty00007
*/
class CVsiMeter : public osg::MatrixTransform
{
public:
	CVsiMeter(void);
	~CVsiMeter(void);
public:
	/**  
	  * @brief 设置攀升幅度
	  * @note 设置飞机攀升幅度
	  * @param dClimbRate[in] 攀升幅度
	  * @return void
	*/
	void SetClimbRate( double dClimbRate)
	{
		m_dClimbRate = dClimbRate;
	};

	/**  
	  * @brief 获得攀升幅度
	  * @note 获得飞机的攀升幅度值
	  * @return 返回攀升幅度
	*/
	double GetClimbRate()
	{
		return m_dClimbRate;
	}

	/**  
	  * @brief 更新攀升或下降率
	  * @note 根据传入攀升或下降率来该表仪表显示数值
	  * @param dClimbRate [in] 攀升或下降率
	  * @return void
	*/
	void UpdateClimbRate( double dClimbRate );

protected:
	/**  
	  * @brief 重组仪表
	  * @note 重新构建攀升幅度显示仪表(Vsi) 
	  * @return void
	*/
	osg::Group* RebuildVsiMeter();
	
	/**  
	  * @brief 初始化表盘上转动角度与攀升幅度的映射
	  * @note 从XML文件中读取VSI配置，然后初始化表盘上转动角度与攀升幅度的映射
	  * @return 返回值
	*/
	void InitClimbRateMap();
	
	/**  
	  * @brief 递归查找特定XML节点
	  * @note 根据节点名称查找该节点，并把所有名字为strNodeModel查找出来加入vecXmlElem容器中
	  * @param 参数 strNodeName[in]  节点名字
	  * @param 参数 pParent[in] 根节点
	  * @param 参数 vecXmlElem[inout] 存放此类节点的容器
	  * @return void
	*/
	void FindXmlNode(std::string strNodeName, FeUtil::TiXmlElement* pParent, vecXmlElements& vecXmlElem);
	
	/**  
	  * @brief 获得节点的值
	  * @note 获得节点的值
	  * @param 参数 pTemp[in] 节点的指针
	  * @param 参数 cName[in] 节点名字
	  * @return 该节点存放的数值
	*/
	double GetNodeValue(FeUtil::TiXmlElement* pTemp, char* cName);

	/**  
	  * @brief 移动指针
	  * @note 根据传入的弧度值改变海拔表指针位子 
	  * @param 参数 dAngle[in] 弧度值
	  * @return void
	*/
	void RotateNeedle(double dAngle);

	/**  
	  * @brief 计算仪表指针需要转动的角度（单位：度）
	  * @note 详细说明  
	  * @param dClimbRate [in] 攀升幅度
	  * @return 仪表指针转动角度
	*/
	double CalcNeedleRotate(double dClimbRate);

protected:
	//攀升幅度值与仪表指针儿的映射表
	mapClimbRate                  m_mapClimbRate;
	//存放仪表指针的MT节点
	osg::MatrixTransform*         m_pVsiNeedleMt;
	//VSI表能表示的最大攀升幅度
	double                        m_dMaxClimbRatePerMin;
	//VSI表能表示的最大攀升幅度
	double                        m_dMinClimbRatePerMin;
	//攀升幅度值
	double                        m_dClimbRate;
};

#endif
