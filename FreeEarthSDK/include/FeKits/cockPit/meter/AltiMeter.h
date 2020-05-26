/**************************************************************************************************
* @file AltiMeter.h
* @note 重组海拔仪表，并提供指示数值改变功能
* @author ty00007
* @data 2014-3-20
**************************************************************************************************/
#ifndef _ALTIMETER_H
#define _ALTIMETER_H

#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/Geode>
#include <iostream>
/**
 * @struct 海拔拆分结构体， 海拔的单位是英尺
*/
typedef struct SAltiValue
{
	///小数值
	double dDelta;
	///指针指示数值，[0-99]
	double dAltiNeedle;
	///海拔百位指示数值，[0, 9] 
	double dAltiBoard_below_100;
	///海拔千位指示数值，[0, 9]
	double dAltiBoard_below_1000;
	///海拔万位指示数值，[0, 9]
	double dAltiBoard_below_10000;

	///气压数值的个位，[0, 9] 
	double dAirpressure_below_10;
	///气压数值的十位，[0, 9] 
	double dAirpressure_below_100;
	///气压数值的百位，[0, 9] 
	double dAirpressure_below_1000;
	///气压数值的千位，[0, 9] 
	double dAirpressure_below_10000;
}SAltiValue;

/**
 * @enum 几何节点序号
*/
typedef enum EAltiMeterGeomID
{
	AltiBoard1       =   0,     //海拔高度的百位
	AltiBoard2,					//海拔高度的千位
	AltiBoard3,					//海拔高度的万位
	Alti_NoneValue,
	Airpressure1     =   4,     //气压的个位
	Airpressure2,				//气压的十位
	Airpressure3,				//气压的百位
	Airpressure4,				//气压的千位
	Airpressure_NodeValue,
}EAltiMeterGeomID;

typedef std::map<EAltiMeterGeomID, osg::Geometry*> mapGeometryID;

/**
  * @class CAltiMeter
  * @brief 海拔仪表
  * @note 拆分海拔仪表部件重新组建
  * @author ty00007
*/
class CAltiMeter: public osg::MatrixTransform
{
public:
	CAltiMeter(void);
	~CAltiMeter(void);
public:
	/**  
	  * @brief 更新海拔
	  * @note 更新海拔高度 
	  * @param 参数 dAltitude[in]  海拔
	  * @return void
	*/
	void UpdateAltitude(double dHeight);

	/**  
	  * @brief 指针节点
	  * @note 获得海拔仪表的指针节点 
	  * @return 返回指针节点
	*/
	osg::MatrixTransform* GetAceelNeedle(){ return m_pAceelNeedle; };

	/**  
	  * @brief 仪表指示万位数值的几何节点
	  * @note 获得海拔仪表指示万位数值的几何节点
	  * @return 返回仪表指示万位数值的几何节点
	*/
	osg::Geometry* GetDrum1(){  return m_pdrum1; };

	/**  
	  * @brief 仪表指示千位数值的几何节点
	  * @note 获得仪表指示千位数值的几何节点
	  * @return 返回仪表指示千位数值的几何节点
	*/
	osg::Geometry* GetDrum2(){  return m_pdrum2; };
	
	/**  
	  * @brief 仪表指示百位数值的几何节点
	  * @note 获得仪表指示百位数值的几何节点
	  * @return 返回仪表指示百位数值的几何节点
	*/
	osg::Geometry* GetDrum3(){  return m_pdrum3; };
	
	/**  
	  * @brief 仪表指示气压个位数值的几何节点 （注：这里气压单位是mmHg）
	  * @note 获得仪表指示气压个位数值的几何节点
	  * @return 返回仪表指示气压个位数值的几何节点
	*/
	osg::Geometry* GetDrum_Ingh_1(){ return m_pdrum_ingh_1; };
	
	/**  
	  * @brief 仪表指示气压十位数值的几何节点
	  * @note 获得仪表指示气压十位数值的几何节点
	  * @return 返回仪表指示气压十位数值的几何节点
	*/
	osg::Geometry* GetDrum_Ingh_2(){ return m_pdrum_ingh_2; };
	
	/**  
	  * @brief 仪表指示气压百位数值的几何节点
	  * @note 获得仪表指示气压百位数值的几何节点
	  * @return 返回仪表指示气压百位数值的几何节点
	*/
	osg::Geometry* GetDrum_Ingh_3(){ return m_pdrum_ingh_3; };
	
	/**  
	  * @brief 仪表指示气压万位数值的几何节点
	  * @note 获得仪表指示气压万位数值的几何节点
	  * @return 返回仪表指示气压万位数值的几何节点
	*/
	osg::Geometry* GetDrum_Ingh_4(){ return m_pdrum_ingh_4; };

	/**  
	  * @brief 海拔高度赋值
	  * @note 设置仪表需要显示的海拔高度值
	  * @param 参数 dAltitude[in] 海拔高度
	  * @return 返回void
	*/
	void SetAltitudeValue(double dAltitude) { m_dAltitude = dAltitude; };

	/**  
	  * @brief 获取海拔高度
	  * @note 获得海拔
	  * @return 返回海拔高度值
	*/
	double GetAltitudeValue() { return m_dAltitude; };

	/**  
	  * @brief 获得能表示的最大海拔值
	  * @note 获得该海拔仪表能表示最大的海拔高度
	  * @return 返回double m_dMaxAltiValue
	*/
	double GetMaxAltiValue() { return m_dMaxAltiValue; };

	/**  
	  * @brief 获得能表示的最小海拔值
	  * @note 获得该海拔仪表能表示最小的海拔高度
	  * @return 返回double m_dMinAltiValue
	*/
	double GetMinAltiValue() { return m_dMinAltiValue; };

protected:
	/**  
	  * @brief 显示隐藏挡板
	  * @note 当海拔高度处于不合法的范围时候，挡板显示提示这个一个不合理的高度值无法显示
	  * @param 参数 bflag[in] 是否显示
	  * @return void
	*/
	void ShowNagetiveFlag(bool bflag);

	/**  
	  * @brief 拆分海拔
	  * @note 把传入的海拔值万位、千位、百位和指针指示数值拆分开
	  * @param 参数 dAltitude[in] 海拔高度值
	  * @return 返回拆分后的海拔
	*/
	SAltiValue SpiteAltitude(double dAltitude);

	/**  
	  * @brief 移动海拔表的万、千、百位数值
	  * @note 传入需要移动几何节点ID和需要移动的距离来改变海拔表万千百位数值
	  * @param 参数 nID[in] 几何节点ID
	  * @param 参数 dTransformTex[in] 移动距离
	  * @return void
	*/
	void TranslateDrum(int nID, double dTransformTex);

	/**  
	  * @brief 移动指针
	  * @note 根据传入的弧度值改变海拔表指针位子 
	  * @param 参数 dAngle[in] 弧度值
	  * @return 返回值
	*/
	void RotateNeedle(double dAngle);

	/**  
	  * @brief 构建仪表
	  * @note 拆分仪表部件重新组合 
	  * @return osg::Group*
	*/
	osg::Group* RebuildAltiMeter();
	
	/**  
	  * @brief 初始化几何节点域ID映射
	  * @note 初始化几何节点域起ID的一一映射
	  * @return void
	*/
	void initializeGeomID();

protected:
	//指针节点
	osg::MatrixTransform* m_pAceelNeedle;
	//海拔高度百位的几何节点
	osg::Geometry* m_pdrum1;
	//海拔高度千位的几何节点
	osg::Geometry* m_pdrum2;
	//海拔高度万位的几何节点
	osg::Geometry* m_pdrum3;
	//气压数值个位的结合节点
	osg::Geometry* m_pdrum_ingh_1;
	//气压数值个位的结合节点
	osg::Geometry* m_pdrum_ingh_2;
	//气压数值个位的结合节点
	osg::Geometry* m_pdrum_ingh_3;
	//气压数值个位的结合节点
	osg::Geometry* m_pdrum_ingh_4;
	//无效值指示标示几何节点
	osg::Node* m_pNagetive_flag;
	//海拔高度值
	double m_dAltitude;
	//节点指针与节点编号映射
	mapGeometryID m_mapGeomID;
	//海拔表所能标示最大的海拔高度值
	double m_dMaxAltiValue;
	//海拔表所能标示最小的海拔高度值
	double m_dMinAltiValue;

};

#endif
