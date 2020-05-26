#include <strstream>

#include <osgDB/FileUtils>
#include <FeKits/dropScreen/DropScreen.h>

namespace FeKit
{
	CCaptureDrawCallback::CCaptureDrawCallback()
		:m_rpImage(NULL)
		,m_strPath("")
		,m_strFileName("")
		,m_strLatterName(".bmp")
	{
		m_strPath = FeFileReg->GetFullPath("dropScreen");
	}

	CCaptureDrawCallback::~CCaptureDrawCallback()
	{

	}

	void CCaptureDrawCallback::operator()( const osg::Camera& camera ) const
	{
		m_rpImage = NULL;

		if(false == m_rpImage.valid()) 
		{
			m_rpImage = new osg::Image;

			unsigned int unWidth(0),unHeight(0);

			unWidth = camera.getViewport()->width();
			unHeight = camera.getViewport()->height();

			//分配一个image
			m_rpImage->allocateImage(unWidth,unHeight,1,GL_RGB,GL_UNSIGNED_BYTE);
		}

		{
			//读取像素信息抓图
			m_rpImage->readPixels(0, 0, m_rpImage->s(), m_rpImage->t(), GL_RGB,GL_UNSIGNED_BYTE);
		}
	}

	bool CCaptureDrawCallback::WriteImage(const std::string& strImageNmae)
	{
		//写入文件
		if (m_rpImage.valid())
		{
			osgDB::writeImageFile(*(m_rpImage.get()),strImageNmae);
			return true;
		}

		return false;
	}


}