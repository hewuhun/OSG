/**************************************************************************************************
* @file FreeEarth.h
* @note FreeEarth三维渲染管理器
* @author c00005
* @data 2017-2-24
**************************************************************************************************/

#ifndef FREE_EARTH_H
#define FREE_EARTH_H

#include <osgViewer/Viewer>
#include <osgGA/KeySwitchMatrixManipulator>

#include <FeEarth/Export.h>
#include <FeShell/SystemManager.h>


namespace FeEarth
{
	/**
	* @class CFreeEarth
	* @brief FreeEarth三维渲染管理接口
	* @author c00005
	*/
    class FEEARTH_EXPORT CFreeEarth : public FeShell::CSystemManager
    {
	public:
		/**  
		  * @brief 构造函数
		  * @param strResorcePath [in] 三维场景资源路径
		*/
		CFreeEarth( const std::string& strResorcePath = "" );

        virtual ~CFreeEarth(void);

    public:
		/**
		*@note: 初始化渲染系统，指定系统默认的配置资源路径
		*/
		virtual bool Initialize( );

		/**
		*@note: 反初始化场景，将场景中的要素归位，并完成销毁
		*/
		virtual bool UnInitialize();

    protected:
		/**
		*@note: 初始化完成之后需要个性化的收尾工作
		*/
		virtual bool InitEnd(){return true;};

		/**
		*@note: 初始化完成之后需要个性化开始的准备工作;
		*/
		virtual bool InitBegin(){return true;};

		/**
        *@note: 一帧开始前的操作，此函数在一帧开始之前调用，有系统自动调用，用户不能够自行调用
        */
		virtual void PreFrame();

		/**
        *@note: 一帧结束之后的操作，此函数在一帧结束之后调用，有系统自动调用，用户不能够自行调用
        */
		virtual void PostFrame();

		/**
        *@note: 一帧结束开始之前，清空视图矩阵回调
        */
		void resetClampProjectionMatrixCallback();

		/**
        *@note: 一帧结束开始之前，设置使用自定义远近裁剪面为false
        */
		void resetUseCustomNearFar();

    };


}

#endif//SINGLE_VIEWER_IMPLEMENT_H

