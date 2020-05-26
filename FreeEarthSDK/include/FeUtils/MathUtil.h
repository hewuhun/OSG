/**************************************************************************************************
* @file FloatUtil.h
* @note 定义了一些浮点型值的比较、溢出等方法
* @author l00008
* @data 2014-4-30
**************************************************************************************************/
#ifndef FE_MATH_UTIL_H
#define FE_MATH_UTIL_H

#include <list>
#include <vector>
#include <algorithm>
#include <osg/Vec3d>
#include <osg/Matrixd>
#include <osg/Geode>
#include <osg/TriangleFunctor>
#include <osgEarth/StringUtils>

#include <FeUtils/Export.h>
#include <FeUtils/RenderContext.h>

using namespace std;   

typedef std::vector<osg::Vec3d>    VecVertexs;


///拓扑几何算法，判断要素之间的几何关系
namespace FeMath
{
	/**  
	* @brief 判断一个点是否在多边形的内部
	* @note 判断一个点是否在多边形的内部，引射线法：从目标点出发引一条射线，看这条射线和多边形所有边的交点数目。
	* 如果有奇数个交点，则说明在内部，如果有偶数个交点，则说明在外部。
	*/
	extern FEUTIL_EXPORT bool ContainPointInPolygon(osg::Vec3d point, osg::Vec3dArray* pArray);

	/**  
	* @brief 计算多边形的面积
	* @note 计算多边形的面积，任意多边形的面积可由任意一点与多边形上依次两点连线构成的三角形矢量面积求和得出。
	* 矢量面积=三角形两边矢量的叉乘。
	*/
	extern FEUTIL_EXPORT double PolygonArea(osg::Vec3dArray* pArray);

	/**
	* @brief 计算多边形空间面积
	* @param pArray 点的世界坐标数组
	* @return 多边形空间面积
	*/
	extern FEUTIL_EXPORT double ComputePolygonSpaceArea(osg::Vec3dArray* pArray);

	/**
	* @brief 计算向量叉乘的模
	* @param vecA 向量A
	* @param vecB 向量B
	* @return 向量叉乘的模
	*/
	extern FEUTIL_EXPORT double ComputeCross(const osg::Vec3d &vecA, const osg::Vec3d &vecB);

	/** 
	* @brief 计算多边形投影面积 
	* @param[in]  const double *padX 经度坐标数组 
	* @param[in] const double* padY 纬度坐标数组 
	* @param[in] int nCount 点的个数 
	* @return 多边形投影面积
	*/
	extern FEUTIL_EXPORT double ComputePolygonShadowArea(const double *padX,const double* padY,int nCount);

	extern FEUTIL_EXPORT double GetQ(double x);  

	extern FEUTIL_EXPORT double GetQbar(double x);

	/**
	* @brief 角度转弧度
	*/
	extern FEUTIL_EXPORT double Deg2Rad(double x);
}


///插值算法，主要有基于地表上的插值和笛卡尔坐标系上的插值
namespace FeMath
{
	/**
	* @brief 坐标两点插值算法
	* @note 利用插值法获得两点之间插值后的点的数组，此方法在二维平面上在两点之间采用线性插值，
	*		高度上同样采用线性插值方法，插值的疏密程度由插值间隔指定，输出和输出的参数都是坐标，
	*		单位为度。
	* @param first [in] 第一个点位置，
	* @param last [in] 第二点位置
	* @param outArray [out] 输出的插值后的点的数组
	* @param delta [in] 插值间隔，默认0.02度
	* @return 无
	*/
	extern FEUTIL_EXPORT void Interpolation(const osg::Vec3d first, const osg::Vec3d last,osg::Vec3dArray * outArray,double delta = 0.02);


	/**
	* @brief 坐标多点插值
	* @note 利用插值发获得一个数组所有点之间插值后的点的数组，多点插值采用两点插值法做底层支持
	* @param vertexs [in] 需要插值的点的数组
	* @param outArray [out] 输出的插值后的点的数组
	* @param delta [in] 插值间隔，默认0.02
	* @return 无
	*/
	extern FEUTIL_EXPORT void Interpolation(const osg::Vec3dArray* vertexs,osg::Vec3dArray* outArray,double delta = 0.02);

	/**
	* @brief 插值
	* @note 利用插值发获得两点之间插值后的点的数组
	* @param first [in] 第一个点位置（经纬高信息）
	* @param last [in] 第二点位置（经纬高信息）
	* @param outArray [out] 输出的插值后的点的数组，差值后数组中存放的数据还是经纬高形式的数据不是世界坐标
	* @param delta [in] 插值间隔，默认0.02
	* @return 无返回值
	*/
	extern FEUTIL_EXPORT void Interpolation(const osg::Vec4d first,const osg::Vec4d last,osg::Vec4dArray* outArray,double delta = 0.02);

	/**
	* @brief 插值
	* @note 利用插值发获得一个数组所有点之间插值后的点的数组
	* @param vertexs [in] 需要插值的点的数组
	* @param outArray [out] 输出的插值后的点的数组
	* @param delta [in] 插值间隔，默认0.02
	* @return 无返回值
	*/
	extern FEUTIL_EXPORT void Interpolation(const osg::Vec4dArray* vertexs,osg::Vec4dArray* outArray,double delta = 0.02);


	extern FEUTIL_EXPORT bool LinearInterpolation(const osg::Vec3d& first, const osg::Vec3d& last, osg::Vec3dArray * pOutArray, double dDelta = 0.02);
	extern FEUTIL_EXPORT bool LinearInterpolation(const osg::Vec3dArray* pVertexs,osg::Vec3dArray* pOutArray,double delta = 0.02);
}

///浮点数学算法，主要针对于浮点数和零比较以及两个浮点数比较的算法
namespace FeMath
{
	/**  
	* @brief 比较两个浮点型值是否相等
	* @param dValue1 第一个比较值
	* @param dValue2 第二个比较值
	* @return 如果相等返回true，否则返回false
	*/
	extern FEUTIL_EXPORT bool Equal(double dValue1, double dValue2);
	extern FEUTIL_EXPORT bool Equal(float fValue1, float fValue2);

	/**  
	* @brief 比较浮点型值是否等于0
	* @param dPram
	* @return 如果相等返回true，否则返回false
	*/
	extern FEUTIL_EXPORT  bool EqualZero(double dValue);
	extern FEUTIL_EXPORT  bool EqualZero(float fValue);


	///其中通用数据类型和std::string的转换在osgEarth库中的StringUtils.h中实现，
	///本文件中主要针对osg以及osgEarth没有提供的算法做补充，其他的算法一概没有
	///做重新实现，其中StringUtils中主要有如下的接口

	///将string转化为通用类型
	//template<> inline bool as<bool>( const std::string& str, const bool& default_value );

	///将通用类型转换为string
	// template<typename T> inline std::string toString(const T& value)
}

///两点之间计算角度算法
namespace FeUtil
{
	/**  
	* @brief 两点之间计算角度算法
	* @param First 第一个点经纬度
	* @param Second 第二个点经纬度
	* @param YawAngle 俯仰角度
	* @param PitchAngle 偏航角度
	* @return 如果相等返回true，否则返回false
	*/
	extern FEUTIL_EXPORT bool GetAngle(CRenderContext *pRenderContext,osg::Vec3d First,osg::Vec3d Second,double &YawAngle,double &PitchAngle );

	/**  
	* @brief 计算两点向量之间的距离
	* @param vecFrom 点的世界坐标
	* @param vecTo 点的世界坐标
	* @return 两点之间的距离
	*/
	extern FEUTIL_EXPORT  double GetDis( osg::Vec3 vecFrom, osg::Vec3 vecTo );

	/**  
	* @brief 计算两点向量之间的距离
	* @param vecFrom 点的世界坐标
	* @param vecTo 点的世界坐标
	* @return 两点之间的距离
	*/
	extern FEUTIL_EXPORT double GetDis(osg::Vec3d vecFrom, osg::Vec3d vecTo);

	/**
	* @brief 计算两点之间的运行时间
	* @param vecFrom, vecTo 两点的世界坐标
	* @param dSpeed 两点之间的飞行速度
	* @return 两点之间的运行时间
	*/
	extern FEUTIL_EXPORT  double GetRunTime(osg::Vec3d vecFrom, osg::Vec3d vecTo, double dSpeed);

	/**
	* @brief 计算贝塞尔曲线, 生成的点集不包含起始点、终止点
	* @param start 开始绘制点
	* @param end 结束绘制点
	* @param control 绘制的控制点
	* @param output 生成的贝塞尔曲线点, 不包含start，end点。
	* @return 空
	*/
	extern FEUTIL_EXPORT  void ComputeBLine(osg::Vec3d start, osg::Vec3d end, osg::Vec3d control, std::vector<osg::Vec3d>& output, int numInter = 50);

	/**
	* @brief 球面插值
	* @param keyPoints 需要进行插值的点集
	* @param output    插值后的点集
	* @param deltaDistance 插值的距离。
	* @param BSEInter 拐角处是否进行贝塞尔插值
	* @param numBSEInter 贝塞尔曲线插入点的个数
	* @param disStartInsert 贝塞尔曲线开始平滑点距离控制点的距离。必须小于deltaDistance
	* @return 空
	*/
	extern FEUTIL_EXPORT void InterpolationSphere(const std::vector<osg::Vec3d>& keyPionts, std::vector<osg::Vec3d>& output, 
		double deltaDistance = 1000, bool BSEInter = true, int numBSEInter = 50, double disStartInsert = 50.0);
	
	/**
	* @brief 两点之间进行球面插值，输出的点集不包含起始点和结束点
	* @param start 开始插入的点
	* @param end 插入的结束点
	* @param output 插入后的点集，不包含start、end点
	* @param deltaDistance 插入点之间的距离。
	* @return 空
	*/
	extern FEUTIL_EXPORT void InterpolationSphere(const osg::Vec3d start, const osg::Vec3d end, std::vector<osg::Vec3d>& output, double deltaDistance);

	/**
	* @brief  获取start, end之间第一个球面插值点
	* @param start 开始插入的点
	* @param end 插入的结束点
	* @param output 第一个球面插值点
	* @param deltaDistance 插入点之间的距离。
	* @return 成功失败标志
	*/
	extern FEUTIL_EXPORT bool GetFirstInterpolation(const osg::Vec3d start, const osg::Vec3d end, osg::Vec3d& output, double deltaDistance);

	/**
	* @brief  获取source点集合的球面插值点
	* @param source 输入点集
	* @param depedent 输出点集
	*/
	extern FEUTIL_EXPORT void GetInterpolation( const VecVertexs& source, VecVertexs& depedent );

	//获取三大角度
	/**
	* @brief  根据输入的矩阵计算三大角度（俯仰、偏航、滚转）
	* @param matrix [in]矩阵
	* @param dPitch [out]俯仰角度
	* @param dRoll [out]滚转角度
	* @param dHeading [out]偏航角度
	*/
	extern FEUTIL_EXPORT   void GetDegreesAngle(FeUtil::CRenderContext *pRenderContext, osg::Matrix matrix,double& dPitch,double& dRoll,double& dHeading );

	//贴地运行时的三个角度
	/**
	* @brief  根据输入的矩阵,以及模型前、后、左、右4点位置，计算模型在地面的姿态角
	* @param curMatrix [in]矩阵
	* @param front [in]模型底部前方中点
	* @param back [in]模型底部正后方中点
	* @param left [in]模型底部正左侧中点
	* @param right [in]模型底部正右侧中点
	* @param vecPosture [out]角度
	*/
	extern FEUTIL_EXPORT  bool CalculateSlopeAndPitchRadian(FeUtil::CRenderContext *pRenderContext, osg::Matrix &curMatrix, osg::Vec3d front, osg::Vec3d back, osg::Vec3d left, osg::Vec3d right,osg::Vec3d &vecPosture);

	/**  
	  * @brief 计算两点的中心点
	*/
	extern FEUTIL_EXPORT bool GetCenterPosition(osg::Vec3d start, osg::Vec3d end, osg::Vec3d &center, CRenderContext *pRenderContext);
}

// 军标相关算法
namespace FeUtil
{
	/**  
	  * @brief 计算和基准向量v夹角为a、长度为d的目标向量
	  * @param v 基准向量
	  * @param a 目标向量和基准向量的夹角，默认为90度，这里的单位使用弧度
	  * @param d 目标向量的长度，即模，默认为1，即单位向量
	  * @return 返回目标向量数组
	*/
	extern FEUTIL_EXPORT osg::Vec3dArray* CalculateVector(osg::Vec3d v, double a=osg::PI_2, double d=1);

	/**  
	  * @brief 计算两个向量的角平分线向量  
	  * @param v1 向量1
	  * @param v2 向量2
	  * @return 返回角平分线向量
	*/
	extern FEUTIL_EXPORT osg::Vec3d CalculateAngularBisector(osg::Vec3d v1, osg::Vec3d v2);

	/**  
	  * @brief 计算两条直线的交点 
	  * @param v_1 直线1的向量
	  * @param v_2 直线2的向量
	  * @param point1 直线1上的任意一点
	  * @param point2 直线2上的任意一点
	  * @return 返回交点
	*/
	extern FEUTIL_EXPORT osg::Vec3d CalculateIntersection(osg::Vec3d v_1, osg::Vec3d v_2, osg::Vec3d point1, osg::Vec3d point2);
	/**  
	  * @brief 创建2次贝塞尔曲线
	  * @note  利用贝塞尔曲线算法，根据传入的控制点数组和平滑度，来算出2次贝塞尔曲线上的所有点
	  * @param vecCPoints [in] 控制点数组
	  * @param part [in] 曲线平滑度
	  * @return 返回包含曲线上的所有点的数组
	*/
	extern FEUTIL_EXPORT osg::Vec3dArray* CreateBezier2(osg::Vec3dArray* vecCPoints,int part=20);
	/**  
	  * @brief 创建3次贝塞尔曲线
	  * @note  利用贝塞尔曲线算法，根据传入的控制点数组和平滑度，来算出3次贝塞尔曲线上的所有点
	  * @param vecCPoints [in] 控制点数组
	  * @param part [in] 曲线平滑度
	  * @return 返回包含曲线上的所有点的数组
	*/
	extern FEUTIL_EXPORT osg::Vec3dArray* CreateBezier3(osg::Vec3dArray* vecCPoints,int part=20);
	/**  
	  * @brief 创建N次贝塞尔曲线
	  * @note  利用贝塞尔曲线算法，根据传入的控制点数组和平滑度，来算出N次贝塞尔曲线上的所有点
	  * @param vecCPoints [in] 控制点数组
	  * @param part [in] 曲线平滑度
	  * @return 返回包含曲线上的所有点的数组
	*/
	extern FEUTIL_EXPORT osg::Vec3dArray* CreateBezierN(osg::Vec3dArray* vecCPoints,int part);
	/**  
	  * @brief 计算阶乘(即1*2*3*4*...*n的值)
	  * @return 阶乘的值
	*/
	extern FEUTIL_EXPORT double Factorial( int n);
	/**  
	  * @brief 组合排序
	  * @note  计算从1*2*...*n/(1*2*...*k*1*2*...*(n-k))的值 
	  * @param n [in] 次数n
	  * @param k [in] 小于N的一个数k
	  * @return 组合排序的值
	*/
	extern FEUTIL_EXPORT double CombSort( int n,int k);
	/**  
	  * @brief Cardinal曲线公式中的基函数
	  * @param n [in] 使用的插值函数的次数
	  * @param k [in] 小于n的值	
	  * @param t [in] 张力系数 0-1之间的一个数
	  * @return 基函数的值
	*/
	extern FEUTIL_EXPORT double BEZ  (int n, int k, double t);
	/**  
	  * @brief 创建Cardinal控制点
	  * @note 利用输入的点数组计算出相应的Cardinal控制点，再使用贝塞尔曲线3创建经过所有Cardinal控制点的圆滑曲线 
	  * @param vecCPoints [in] 控制点数组
	  * @return Cardinal曲线的所有点数组
	*/
	extern FEUTIL_EXPORT osg::Vec3dArray* CreateCardinalCurve(osg::Vec3dArray* vecCPoints);

	/**  
	  * @brief 计算两点间的距离 
	  * @param pointA [in] 第一个点
	  * @param pointB [in] 第二个点
	  * @return 两点间的距离值
	*/
    extern FEUTIL_EXPORT double CalculateDistance(osg::Vec3d pointA, osg::Vec3d pointB);
	/**  
	  * @brief 计算由两点构成的向量  
	  * @param pointA [in] 起点
	  * @param pointB [in] 终点
	  * @return 两点间的向量
	*/
    extern FEUTIL_EXPORT osg::Vec3d ToVector(osg::Vec3d pointA,osg::Vec3d pointB);
	/**  
	  * @brief 根据两点计算其所在向量的箭头
	  * @param startP [in] 向量的起点
	  * @param endP [in]   向量的终点
	  * @param ratio [in]  直线长度与箭头长度的比值，默认为10倍
	  * @param angle [in]  箭头所在直线与直线的夹角，范围为0到π，默认为PI/6。
	  * @return 点的数组
	*/
    extern FEUTIL_EXPORT osg::Vec3dArray*  CalculateArrowLines(osg::Vec3d startP, osg::Vec3d endP,double ratio=10, double angle=osg::PI/6);
	/**  
	  * @brief 通过三角形的底边两端点坐标以及底边两夹角，计算第三个点坐标
	  * @note 详细说明  
	  * @param pointS [in]  底边第一个点
	  * @param pointE [in]  底边第二个点
	  * @param a_S [in]  底边和第一个点所在的另一条边的夹角
	  * @param a_E [in]  底边和第二个点所在的另一条边的夹角
	  * @return 返回顶点（理论上存在两个值）
	*/
	extern FEUTIL_EXPORT osg::Vec3dArray* CalculateIntersectionFromTwoCorner(osg::Vec3d pointS, osg::Vec3d pointE, double a_S=osg::PI/4, double a_E=osg::PI/4);
	/**  
	  * @brief 根据圆心、半径，与X轴的起点角和终点角计算圆弧 
	  * @param center [in] 圆心
	  * @param radius [in] 半径
	  * @param startAngle [in] 起点角,范围为0到2π
	  * @param endAngle [in]   终点角,范围为0到2π
	  * @param direction [in] 从起点到终点的方向，其值为1：逆时针，其值为-1：顺时针。默认为-1，即顺时针。
	  * @param sides [in]     圆弧所在圆的点数，默认为360个，即每1°一个点
	  * @return 圆弧上的点数组
	*/
	extern FEUTIL_EXPORT osg::Vec3dArray* CalculateArc(osg::Vec3d center,double radius, double startAngle,double endAngle,int direction=-1,double sides=360);

	/**  
		* @brief 创建闭合Cardinal曲线的控制点
		* @note 利用输入的点数组计算出相应的Cardinal控制点，再使用贝塞尔曲线3创建经过所有Cardinal控制点的圆滑闭合曲线  
		* @param vecCPoints [in] 控制点数组
		* @return Cardinal闭合曲线的所有点数组
	*/
	extern FEUTIL_EXPORT osg::Vec3dArray* CreateCloseCardinal(osg::Vec3dArray* vecCPoints );
}

#endif //FE_MATH_UTIL_H
