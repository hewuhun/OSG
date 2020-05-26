/**************************************************************************************************
* @file HsiMeter.h
* @note 飞机的水平状况指示器，将0-360°的磁航向标在表盘上，并且以一个十字代表飞机，
*“飞机”机头所指即为飞机当前磁航向。还有航程与速度显示仪表。
* @author ty00007
* @data 2014-3-25
**************************************************************************************************/
#ifndef _HISMETER_H
#define _HISMETER_H

#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <iostream>
/**
 * @struct 速度拆分结构体，拆分成百、十、个位。 速度单位为：节
*/
typedef struct SVelocityValue
{
	double dVelo_below1000; //速度的百位数值
	double dVelo_below100;	//速度的十位数值
	double dVelo_below10;	//速度的个位数值
}SVelocityValue;

/**
 * @struct 航程距离拆分结构体，拆分成百、十、个位。 路程单位为：英里
*/
typedef struct SMileValue
{
	double dMile_below1000; //航程的百位数值
	double dMile_below100;  //航程的十位数值
	double dMile_below10;   //航程的个位数值
}SMileValue;

/**
 * @enum 几何节点序号
*/
typedef enum EHsiMeterGeomID
{
	VelocityBoard1       =   0, //速度的百位
	VelocityBoard2,				//速度的十位
	VelocityBoard3,				//速度的个位
	MileBoard1     =   3,		//航程的百位
	MileBoard2,					//航程的十位
	MileBoard3,					//航程的个位
	NodeValue,
}EHsiMeterGeomID;

//几何节点枚举的映射
typedef std::map<EHsiMeterGeomID, osg::Geometry*> mapHsiGeometryID;

/**
  * @class CHsiMeter
  * @brief 飞机水平指示器仪表类
  * @note 重建Hsi仪表，建立结合节点的枚举映射，包括更新仪表上的航程、航行速度、偏航角
  * @author ty00007
*/
class CHsiMeter : public osg::MatrixTransform
{
public:
	CHsiMeter(void);
	~CHsiMeter(void);
public:
	/**  
	  * @brief 更新速度指示数值
	  * @note 根据传入的速度改变仪表上显示速度指示值 
	  * @param dVelocity [in] 速度
	  * @return void
	*/
	void UpdateVelocity(double dVelocity);
	/**  
	  * @brief 更新航程显示
	  * @note 更具传入的航线值改变航程表上显示的数值
	  * @param dMile [in] 
	  * @return void
	*/
	void UpdateMiles(double dMile);
	/**  
	  * @brief 更新航向
	  * @note 更新罗盘上航向角度 
	  * @param dYaw [in] 航向
	  * @return void
	*/
	void UpdateHeading( double dYaw );
	/**  
	  * @brief 简要说明
	  * @note 详细说明  
	  * @param 参数 [in]
	  * @param 参数 [in]
	  * @return 返回值
	*/
	void UpdateCourseknod(double dCourseAngle);

	/**  
	  * @brief 获得速度
	  * @note 获得速度  
	  * @return 速度值
	*/
	double GetVelocity() { return m_dVelocity; };

	/**  
	  * @brief 获得航程
	  * @note 获得航程
	  * @return 返回航程值
	*/
	double GetMile(){ return m_dMile; };

	/**  
	  * @brief 获得速度显示的最大值
	  * @note 获得速度显示的最大值
	  * @return 返回速度显示的最大值
	*/
	double GetVeloMaxRange() { return m_dVeloMaxRange; };
	/**  
	  * @brief 获得速度显示的最小值
	  * @note 获得速度显示的最小值
	  * @return 返回速度显示的最小值
	*/
	double GetVeloMinRange() { return m_dVeloMinRange; };
	/**  
	  * @brief 获得航程显示的最大值
	  * @note 获得航程显示的最大值
	  * @return 返回航程显示的最大值
	*/
	double GetMileMaxRange() { return m_dMileMaxRange; };
	/**  
	  * @brief 获得航程显示的最小值
	  * @note 获得航程显示的最小值
	  * @return 返回航程显示的最小值
	*/
	double GetMileMinRange() { return m_dMileMinRange; };

protected:
	/**  
	  * @brief 重建仪表
	  * @note 分析仪表内部结构重新组合仪表，试用本类 
	  * @return osg::Group
	*/
	osg::Group* RebuildHsiMeter();
	/**  
	  * @brief 建立映射表
	  * @note 建立结合节点与其枚举的一一映射
	  * @return void
	*/
	void initializeHsiGeomID();

	/**  
	  * @brief 显示隐藏隔板
	  * @note 当航程到达最大值时，设置隔板表示已超过航程最大表示范围
	  * @param bflag [in] 是否显示隔板
	  * @return void
	*/
	void ShowNagetiveFlag( bool bflag );

	/**  
	  * @brief 拆解速度值
	  * @note 拆解速度值成百、十、个位，存入结构体中  
	  * @param dVelocity [in] 速度
	  * @return SVelocityValue结构体类型
	*/
	SVelocityValue SpiteVelocity(double dVelocity);

	/**  
	  * @brief 拆解航程
	  * @note 拆解航程值成百、十、个位，存入结构体中  
	  * @param dMile [in] 航程
	  * @return SMileValue结构体类型
	*/
	SMileValue SpiteMile(double dMile);

	/**  
	  * @brief 移动纹理
	  * @note 移动纹理  
	  * @param nID [in] 枚举值转成int
	  * @param dTransformTex [in] 移动纹理距离
	  * @return 返回值
	*/
	void TranslateTexDrum(int nID, double dTransformTex);

	/**  
	  * @brief 旋转罗盘
	  * @note 根据传入角度旋转罗盘，显示出偏航角度  
	  * @param dAnlge [in] 偏航角
	  * @return double
	*/
	void AircraftYaw(double dAnlge);

	/**  
	  * @brief 记录速度
	  * @note 记录速度  
	  * @param dVelocity [in] 速度
	  * @return void
	*/
	void RecordVelocity(double dVelocity) {  m_dVelocity = dVelocity; };

	/**  
	  * @brief 记录航程
	  * @note 记录航程
	  * @param dVelocity [in] 航程
	  * @return void
	*/
	void RecordMile(double dMile){ m_dMile = dMile; };

protected:
	//表示航行速度个位的结合节点
    osg::Geometry* m_pDrum_course1;
	//表示航行速度十位的结合节点
	osg::Geometry* m_pDrum_course2;
	//表示航行速度百位的结合节点
	osg::Geometry* m_pDrum_course3;
	//表示航行速度个位的结合节点
	osg::Geometry* m_pDrum_mile1;
	//表示航行速度十位的结合节点
	osg::Geometry* m_pDrum_mile2;
	//表示航行速度百位的结合节点
	osg::Geometry* m_pDrum_mile3;
	//仪表上的罗盘MT
	osg::MatrixTransform* m_pHsi_compass_cardMT;
	//仪表指针MT
	osg::MatrixTransform* m_pHsi_NeedlesMT;
	//仪表上HGD旋钮
	osg::MatrixTransform* m_pHsi_HDG_knodsMT;
	//仪表上CRS旋钮
	osg::MatrixTransform* m_pHsi_CRS_knodsMT;
	
	// HSI提供一个“俯视图”，你的飞机就在HSI的中心，
	//黄色的CDI指针（双针）和 
	//白色的三角形To/From/Off标志表明了VOR台站与飞机的水平相对位置，
	//当CDI回中、两端与OBS选定角度上的短双黄线重合时，飞机就在该VOR的选定边（Radial）上。
	osg::MatrixTransform* m_pHsi_cdiMT;
	//超出范围后显示的隔板
	osg::Group* m_pNegativeflag;
	//运动速度
	double m_dVelocity;
	//运动的路程
	double m_dMile;
	//几何节点和枚举的映射
	mapHsiGeometryID m_mapGeomID;
	//仪表能表示的最大范围
	double m_dVeloMaxRange;
	//仪表能表示的最小范围
	double m_dVeloMinRange;
	//仪表能表示的最大范围
	double m_dMileMaxRange;
	//仪表能表示的最小范围
	double m_dMileMinRange;

};

#endif