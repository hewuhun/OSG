/**************************************************************************************************
* @file DropScreen.h
* @note 截屏功能
* @author c00005
* @data 2015-7-8
**************************************************************************************************/
#ifndef _DROP_SCREEN_H
#define _DROP_SCREEN_H

#include <osg/Camera>
#include <osg/Image>

#include <osgViewer/GraphicsWindow>

#include <osg/BufferObject>

#include <osgDB/WriteFile>

#include <FeKits/Export.h>

#include <FeUtils/PathRegistry.h>

namespace FeKit
{
	/**
	* @class CCaptureDrawCallback
	* @brief 截屏功能类
	* @note 本类实现了截取屏幕的功能
	* @author 00008
	*/
	class FEKIT_EXPORT CCaptureDrawCallback : public osg::Camera::DrawCallback 
	{
	public:
		CCaptureDrawCallback();

		~CCaptureDrawCallback();

		virtual void operator ()(const osg::Camera& camera) const;
		///保存图像
		bool WriteImage(const std::string& strImageNmae);
	protected:
		mutable osg::ref_ptr<osg::Image>		m_rpImage;
		std::string								m_strPath;

		std::string                             m_strFileName;
		std::string                             m_strLatterName;
	};

}

#endif