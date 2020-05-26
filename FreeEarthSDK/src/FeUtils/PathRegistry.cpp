#include <stdio.h>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <PathRegistry.h>
#include <EnvironmentVariableReader.h>

namespace FeUtil
{
    CFileReg* CFileReg::m_pInstance = NULL;
	CFileReg::CDestoryInstance CFileReg::destroy;

    CFileReg::CFileReg()
		:m_strFilePath("")
    {

    }

    CFileReg::~CFileReg()
    {
    }

    CFileReg* CFileReg::Instance()
    {
        if(!m_pInstance)
        {
            m_pInstance = new CFileReg();
        }

        return m_pInstance;
    }


	//获取全路径
    std::string CFileReg::GetFullPath( const std::string& strFileName )
    {
        if(strFileName.empty() || osgDB::fileExists(strFileName))
        {
            return strFileName;
        }

		return m_strFilePath + "/" + strFileName;
    }




    std::string CFileReg::GetTransWord( const std::string& strKeyWords )
    {
        //可以使用map表或者其他的表记录映射翻译文件，暂时不实现
        return strKeyWords;
    }

	std::string CFileReg::GetDataPath() const
	{
		return m_strFilePath;
	}

}

namespace FeUtil
{
	CReadFileRegistry* CReadFileRegistry::m_pInstance = NULL;
	CReadFileRegistry::CDestoryInstance CReadFileRegistry::destroy;

	osg::Node* CReadFileRegistry::ReadNodeFileInstance( const std::string& strFilePath )
	{
		if(m_loadNodeInstance.count(strFilePath))
		{
			if(!m_loadNodeInstance[strFilePath].valid())
			{
				m_loadNodeInstance[strFilePath] = osgDB::readNodeFile(strFilePath);
			}
		}
		else
		{
			m_loadNodeInstance[strFilePath] = osgDB::readNodeFile(strFilePath);
		}

		return m_loadNodeInstance[strFilePath].get();
	}

	CReadFileRegistry* CReadFileRegistry::Instance()
	{
		if(!m_pInstance)
		{
			m_pInstance = new CReadFileRegistry();
		}

		return m_pInstance;
	}

	CReadFileRegistry::CReadFileRegistry()
	{

	}

	CReadFileRegistry::~CReadFileRegistry()
	{

	}

	osg::Image* CReadFileRegistry::ReadImageFileInstance( const std::string& strPath )
	{
		if(m_loadImageInstance.count(strPath))
		{
			if(!m_loadImageInstance[strPath].valid())
			{
				m_loadImageInstance[strPath] = osgDB::readImageFile(strPath);
			}
		}
		else
		{
			m_loadImageInstance[strPath] = osgDB::readImageFile(strPath);
		}

		return m_loadImageInstance[strPath].get();
	}

}

