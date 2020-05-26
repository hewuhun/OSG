/**************************************************************************************************
* @file DropScreen.h
* @note 实现截屏输出
* @author w00040
* @data 2017-2-27
**************************************************************************************************/
#ifndef _DROP_SCREEN_
#define _DROP_SCREEN_

#include <FeKits/dropScreen/DropScreen.h>
#include <FeShell/SystemService.h>

namespace FreeViewer
{
	/**
	  * @class CDropScreen
	  * @brief 截屏
	  * @note 实现截屏输出
	  * @author w00040
	*/
	class CDropScreen
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDropScreen(FeShell::CSystemService* pSystemService);

		/**  
		  * @brief 析构函数
		*/
		~CDropScreen();

	public:
		/**  
		  * @brief 截屏
		*/
		void DropScrren( const std::string& strPath);

	private:
		///截图使用回调
		osg::ref_ptr<FeKit::CCaptureDrawCallback>		m_rpCaptureCallback;

		///setPostDrawCallback之前，保存上一个callback
		osg::observer_ptr<osg::Camera::DrawCallback>	m_opLastDrawCallback;

		///三维渲染服务
		osg::observer_ptr<FeShell::CSystemService>		m_opSystemService;

	};
}

#endif //_DROP_SCREEN_