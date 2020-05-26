#ifndef CORRIDER_EFFECT_H
#define CORRIDER_EFFECT_H

/**************************************************************************************************
* @file OsgIdealChannel.h
* @note 本文件定义负责绘制理想航道的类
* @author w00009 
* @data 
**************************************************************************************************/


#include <osg/MatrixTransform>
#include <osg/Geode> 
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Group>
#include <osg/AutoTransform>
#include <osg/LineWidth>
#include <FeEffects/Export.h>
#include <FeUtils/RenderContext.h>

namespace FeEffect
{

	/**
	* @class COsgIdealChannel
	* @brief 本类负责绘制理想通道
	* @note 理想通道有三种模式，本类绘制三种模式的理想通道并控制三种模式的通道
	* @author w00009
	*/
	class FEEFFECTS_EXPORT COsgIdealChannel : public osg::Group, public FeUtil::CRenderContextObserver
	{
	public:
		/**
		* @brief 构造函数
		* @note 该类的构造函数，用于构造对象
		* @return 无返回值
		*/
		COsgIdealChannel(FeUtil::CRenderContext* pContext);
		/**
		* @brief 析构函数
		* @note 该类的默认析构函数，对该类的对象进行析构
		* @return 无返回值
		*/
		~COsgIdealChannel(void);
	public:
		/**
		* @brief 初始化
		* @note 初始化函数
		* @return 无返回值
		*/
		bool Init();
		/**
		* @brief 绘制原点的线框
		* @note 绘制原点的线框（进近模式一)，即理想通道的第一种模式
		* @return 绘制成功则返回true，否则返回值false
		*/
		bool DrawWireframe();
		/**
		* @brief 绘制原点的锁定框
		* @note 绘制原点的锁定框（进近模式二）及通道的第二种模式
		* @return 绘制成功返回true，否则返回false
		*/
		bool DrawLockFrame();
		/**
		* @brief 得到角度
		* @note 得到原点画的线框移到地球上需要旋转的偏航角度和俯仰角度
		* @param First [in] 第一个点的经纬高
		* @param Second [in] 第二个点的经纬高
		* @param YawAngle [out] 需要旋转的航向角
		* @param PitchAngle [out] 需要旋转的俯仰角
		* @return 得到相关角度返回true，否则返回false
		*/
		bool GetAngle(osg::Vec3d First,osg::Vec3d Second,double &YawAngle,double &PitchAngle);
		/**
		* @brief 获取姿态
		* @note 本函数是根据相邻的两个点得线框的态势（俯仰角，偏航角，横滚角）
		* @param first [in] 第一个点的位置
		* @param second [in] 第二点的位置
		* @param xAngleFuYang [out] 俯仰角
		* @param yAngleHengGun [out] 横滚角
		* @param zAngleHangXiang [out] 航向角
		* @return 无返回值
		*/	
		void getFlyPosture(osg::Vec3d first,osg::Vec3d second,double& xAngleFuYang,double& yAngleHengGun,double& zAngleHangXiang);
		/**
		* @brief 打开理想航道模式一
		* @note 根据UDP传来的控制消息，打开理想航道的模式一
		* @return 打开成功返回true，否则返回false
		*/
		bool OpenIdealChannelMode1();
		/**
		* @brief 打开理想航道模式二
		* @note 根据UDP传来的控制消息，打开理想航道的模式二
		* @return 打开成功返回true，否则返回false
		*/
		bool OpenIdealChannelMode2();
		/**
		* @brief 打开理想航道模式三
		* @note 根据UDP传来的控制消息，打开理想航道的模式三
		* @return 打开成功返回true，否则返回false
		*/
		bool OpenIdealChannelMode3();
		/**
		* @brief 关闭理想通道
		* @note 根据传来的UDP消息关闭理想航道
		* @return 关闭成功返回true
		*/
		bool CloseIdealChannel();
	public:
		/**
		* @brief 创建或者重绘理想通道
		* @note 根据传来的理想通道的关键点的数组（osg::Vec4dArray），绘制理想通道
		* @param pKeysVertex [in] 理像通道的关键点的数组
		* @param dKeysRotateAngle [in] 关键点处理想通道垂直方向的旋转角度(弧度)
		* @return 返回值
		*/
		bool RedrawIdealChannel(osg::Vec4dArray* pKeysVertex,double dKeysRotateAngle=0.3);
		/**
		* @brief 创建或者重绘理想通道
		* @note 根据传来的理想通道的关键点的数组（osg::Vec4dArray），绘制理想通道,此函数转为飞机降落是用的进近通道做准备
		* @param pKeysVertex [in] 理像通道的关键点的数组
		* @return 返回值
		*/
		bool ReDrawIdealChannelForApproach(osg::Vec4dArray* pKeysVertex);
		/**
		* @brief 计算B样条的顶点数组
		* @note 计算B样条的顶点数组
		* @param startPoint [in] B样条开始点
		* @param endPoint [in] B样条结束点
		* @param controlPoint [in] B样条控制点
		* @param outArray [out] 输出的计算好的顶点数组
		* @param insertPointNum [in] 要插入的顶点的个数
		* @return 无返回值
		*/	
		void ComputerBLine(osg::Vec4d startPoint, osg::Vec4d endPoint, 
			osg::Vec4d controlPoint,osg::Vec4dArray *outArray,int insertPointNum);
		/**
		* @brief 得到理想航道所有点
		* @note 根据理想航道关键点数组，经过向数组相邻点之间插值（先线性插值然后再做B样条插值）
		*       得到最总组成理想航道的所有点
		* @param pKeysVertex [in] 理像通道的关键点的数组
		* @param dInsertCount [in] B样条插值法插值时要向两个点之间插入点的个数
		* @param _pStartSmooth [in] 存放理想航道中开始进入平滑过渡阶段的起始点
		* @param _pEndSmooth [in] 存放理想航道中平滑过渡阶段结束的点
		* @return 返回值
		*/
		osg::Vec4dArray* getFinalLinePoint(osg::Vec4dArray * pKeysVertex,double dInsertCount
			,osg::Vec4dArray *_pStartSmooth,osg::Vec4dArray *_pEndSmooth);
		/**
		* @brief 绘制组成理想航道的线框之间的连线
		* @note 此函数负责绘制组成理想航道线框之间的连线（左下角连线、左上角连线、右下角连线、右上角连线）
		* @param _pLeftBottomLine [in] 绘制左下角连线所需点的数组
		* @param _pLeftTopLine [in] 绘制左上角连线所需点的数组
		* @param _pRightBottomLine [in] 绘制右下角连线所需点的数组
		* @param _pRightTopLine [in] 绘制右上角连线所需点的数组
		* @param PointCount [in] 绘制连线所用点的个数
		* @return 返回值
		*/
		osg::Geode * DrawWireframeLigature(osg::Vec3dArray * _pLeftBottomLine,osg::Vec3dArray * _pLeftTopLine,
			osg::Vec3dArray * _pRightBottomLine,osg::Vec3dArray * _pRightTopLine,int PointCount);
		/**
		* @brief 绘制组成理想航道的四个面
		* @note 理想航道有一种模式是由四个面组成的，此函数负责绘制这四个面
		* @param _pVertexLeftFace [in] 绘制左面用到的点的数组
		* @param _pVertexRightFace [in] 绘制右面用到的点的数组
		* @param _pVertexTopFace [in] 绘制上面用到的点的数组
		* @param _pVertexBottomFace [in] 绘制下面用到的点的数组
		* @param PointCount [in] 绘制面用到点的个数
		* @return 返回值
		*/
		osg::Geode * DrawChannelFace(osg::Vec3dArray * _pVertexLeftFace,osg::Vec3dArray * _pVertexRightFace
			,osg::Vec3dArray * _pVertexTopFace,osg::Vec3dArray * _pVertexBottomFace,int PointCount);
	public:

		//得到理想通道的关键点
		osg::Vec4dArray * getArrayVertex();

	public:
		///存放理想通道
		osg::ref_ptr<osg::MatrixTransform> m_pRoot;
		///存放构成理想通道的线框(进近模式一)
		osg::ref_ptr<osg::Geode> m_pWireframeGeode;
		///存放理想通道的锁定框（进近模式二）
		osg::ref_ptr<osg::Geode> m_pLockFrameGeode;
		///通道进近模式一(线框)的group节点
		osg::ref_ptr<osg::Group> m_pApproachMode1;
		///通道进近模式二（锁定框）的group节点
		osg::ref_ptr<osg::Group> m_pApproachMode2;
		///通道进近模式三（面框）的group节点
		osg::ref_ptr<osg::Group> m_pApproachMode3;
		///存放理想通道的关键点
		osg::Vec4dArray * m_pArrayVertex;
		///绘制原点线框所需的左下部点
		osg::Vec3d LeftBottom;
		///绘制原点线框所需的右下部点
		osg::Vec3d RightBottom;
		///绘制原点线框所需的左上部点
		osg::Vec3d LeftTop;
		///绘制原点线框所需的右上部点
		osg::Vec3d RightTop; 
		///判断理想通道是否已经创建
		bool m_bCreat;
		///主视口的宽
		int m_iWidth;
		///主视口的高
		int m_iHeight;
		//理想通道的宽
		int m_iChannelWidth;
		//理想通道的高
		int m_iChannelHeight;
		//理想航道线框的颜色及线框连线的颜色
		osg::Vec4d m_Vec4Color;
	};

}

#endif //CORRIDER_EFFECT_H

