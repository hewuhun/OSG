#include <FeExtNode/ExTiltModelNode.h>

#include <FeUtils/CoordConverter.h>

#include <osgDB/ReadFile>

#include <iostream>
#include <string>

#ifdef WIN32
#include <io.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace FeExtNode
{
	using namespace osgEarth;

	CExTiltModelNodeOption::CExTiltModelNodeOption()
		:CExternNodeOption()
	{

	}

	CExTiltModelNodeOption::~CExTiltModelNodeOption()
	{

	}
}

namespace FeExtNode
{
	CExTiltModelNode::CExTiltModelNode( FeUtil::CRenderContext* pRenderContext, CExTiltModelNodeOption* opt )
		:CExternNode(opt)
		,m_opRenderContext(pRenderContext)
		,m_rpTransMt(NULL)
		,m_rpOffsetMt(NULL)
		,m_rpRotateMt(NULL)
		,m_rpScaleMt(NULL)
	{
		InitNode();
		
		switch (opt->tileType())
		{
		case FeExtNode::CExTiltModelNodeOption::E_TILT_TYPE_FILEPATH:
			{
				ParseTiltConfig();
			}
			break;
		case FeExtNode::CExTiltModelNodeOption::E_TILT_TYPE_DTATPATH:
			{
				//读取数据文件夹
				GenerateData(opt->tiltPath());
				ParseTiltConfig();
			}
			break;
		case FeExtNode::CExTiltModelNodeOption::E_TILT_TYPE_SERVICE:
			{
				ParseTiltConfig(true);
			}
			break;
		}
	}

	CExTiltModelNode::~CExTiltModelNode()
	{

	}

	CExTiltModelNodeOption* CExTiltModelNode::GetOption()
	{
		return dynamic_cast<CExTiltModelNodeOption*>(m_rpOptions.get());
	}

	void CExTiltModelNode::InitNode()
	{
		//初始化
		if (false == m_rpTransMt.valid())
		{
			m_rpTransMt = new osg::MatrixTransform;
		}
		addChild(m_rpTransMt);

		if (false == m_rpOffsetMt.valid())
		{
			m_rpOffsetMt = new osg::MatrixTransform;
		}
		m_rpTransMt->addChild(m_rpOffsetMt);

		if (false == m_rpRotateMt.valid())
		{
			m_rpRotateMt = new osg::MatrixTransform;
		}
		m_rpOffsetMt->addChild(m_rpRotateMt);

		if (false == m_rpScaleMt.valid())
		{
			m_rpScaleMt = new osg::MatrixTransform;
		}
		m_rpRotateMt->addChild(m_rpScaleMt);
	}

	void CExTiltModelNode::SetScale( const osg::Vec3d& vecScale )
	{
		if (m_rpScaleMt.valid())
		{
			m_rpScaleMt->setMatrix(osg::Matrix::scale(vecScale));

			GetOption()->tiltConfig().scale() = vecScale;
		}
	}

	osg::Vec3d CExTiltModelNode::GetScale()
	{
		return GetOption()->tiltConfig().scale();
	}

	void CExTiltModelNode::SetRotate( const osg::Vec3d& vecRotate )
	{
		if (m_rpRotateMt.valid())
		{
			m_rpScaleMt->setMatrix(osg::Matrix::rotate(
				osg::DegreesToRadians(vecRotate.x()), osg::X_AXIS,
				osg::DegreesToRadians(vecRotate.y()), osg::Y_AXIS,
				osg::DegreesToRadians(vecRotate.z()), osg::Z_AXIS));

			GetOption()->tiltConfig().rotate() = vecRotate;
		}
	}

	osg::Vec3d CExTiltModelNode::GetRotate()
	{
		return GetOption()->tiltConfig().rotate();
	}

	void CExTiltModelNode::SetPosition( const osg::Vec3d& vecLLH )
	{
		if (m_rpTransMt.valid())
		{
			osg::Matrix mat;
			FeUtil::DegreeLLH2Matrix(m_opRenderContext.get(), vecLLH, mat);
			m_rpTransMt->setMatrix(mat);

			GetOption()->tiltConfig().position() = vecLLH;
		}
		
	}

	osg::Vec3d CExTiltModelNode::GetPosition()
	{
		return GetOption()->tiltConfig().position();
	}

	void CExTiltModelNode::SetOffset( const osg::Vec3d& vecOffset )
	{
		if (m_rpOffsetMt.valid())
		{
			m_rpOffsetMt->setMatrix(osg::Matrix::translate(vecOffset));

			GetOption()->tiltConfig().offset() = vecOffset;
		}
	}

	osg::Vec3d CExTiltModelNode::GetOffset()
	{
		return GetOption()->tiltConfig().offset();
	}

	void CExTiltModelNode::SetTiltPath( const std::string& strPath )
	{
		if (0 == GetOption()->tiltPath().compare(strPath))
		{
			return;
		}

		GetOption()->tiltPath() = strPath;

		ParseTiltConfig();
	}

	void CExTiltModelNode::SetTiltType( int type )
	{
		GetOption()->tileType() = type;
	}

	int CExTiltModelNode::GetTiltType()
	{
		return GetOption()->tileType();
	}

	void CExTiltModelNode::SetServiceTiltPath( const std::string& strPath )
	{
		if (0 == GetOption()->tiltPath().compare(strPath))
		{
			return;
		}

		GetOption()->tiltPath() = strPath;

		ParseTiltConfig(true);
	}

	std::string CExTiltModelNode::GetTiltPath()
	{
		return GetOption()->tiltPath();
	}

	void CExTiltModelNode::ParseTiltConfig(bool bService /*= false*/)
	{
		//解析倾斜摄影数据的配置信息
		CExTiltConfig config;
		CExTiltReaderWriter tiltReader(bService);

		if (m_rpScaleMt->getNumChildren() > 0)
		{
			m_rpScaleMt->removeChild(0, m_rpScaleMt->getNumChildren());
		}

		if(false == GetOption()->tiltPath().empty())
		{
			if (bService)
			{
				if (!tiltReader.ReadURl(GetOption()->tiltPath(), config))
				{
					return;
				}
			}
			else
			{
				tiltReader.Read(GetOption()->tiltPath(), config);
			}
			
			GetOption()->tiltConfig() = config;

			if (m_rpScaleMt.valid())
			{
				//读取并加载倾斜摄影数据模型
				m_rpScaleMt->addChild(osgDB::readNodeFiles(config.tiltList()));
			}
		}
		
		//设置位置、姿态、缩放等信息
		SetScale(config.scale());
		SetRotate(config.rotate());
		SetOffset(config.offset());
		SetPosition(config.position());
	}

	void CExTiltModelNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

	void CExTiltModelNode::GenerateData( std::string strPath )
	{
        //解析倾斜摄影数据的配置信息
        CExTiltConfig config;
        GetOption()->tiltConfig() = config;
        config.tiltList().clear();

		GetOption()->tiltPath() = strPath;

        if (strPath.empty())
        {
            return;
        }

#ifdef WIN32
        // 遍历目录
        _finddata_t file_info;
        std::string current_path = strPath + "/*";
        int handle = _findfirst(current_path.c_str(), &file_info);
        if (-1 == handle)
        {
            return;
        }

        do
        {
            // 判断是否是子目录
            if (file_info.attrib == _A_SUBDIR) //是目录
            {
                // 忽略“.”和“..”目录
                if (strcmp(file_info.name, ".") !=0 && strcmp(file_info.name, "..")!=0)
                {
                    std::string nodePath = strPath;
                    nodePath += "/";
                    nodePath += file_info.name;
                    nodePath += "/";
                    nodePath += file_info.name;
                    nodePath += ".osgb";

                    config.tiltList().push_back(nodePath);
                }
            }
        }
        while (!_findnext(handle, &file_info));
        _findclose(handle);
#else
        struct stat s_buf;

        // 获取文件信息，把信息存储到s_buf中
        stat(strPath.c_str(), &s_buf);

        // 判断文件路径是否是目录
        if ( !S_ISDIR(s_buf.st_mode) )
        {
            return;
        }

        struct dirent *fileName;
        DIR *dir;
        dir = opendir(strPath.c_str());
        if ( NULL == dir )
        {
            return;
        }

        while ( (fileName = readdir(dir)) != NULL )
        {
            if ( strcmp(fileName->d_name, ".") == 0 || strcmp(fileName->d_name, "..") == 0 )
            {
                continue;
            }

            char cFilePath[200];
            bzero(cFilePath, 200);
            strcat(cFilePath, strPath.c_str());
            strcat(cFilePath, "/");
            strcat(cFilePath, fileName->d_name);

            // 获取文件信息，把信息存储到s_buf中
            stat(cFilePath, &s_buf);

            // 判断文件是否是目录
            if (S_ISDIR(s_buf.st_mode))
            {
                std::string nodePath = strPath;
                nodePath += "/";
                nodePath += fileName->d_name;
                nodePath += "/";
                nodePath += fileName->d_name;
                nodePath += ".osgb";

                config.tiltList().push_back(nodePath);
            }
        }
#endif

        if (m_rpScaleMt.valid())
        {
            //读取并加载倾斜摄影数据模型
            m_rpScaleMt->addChild(osgDB::readNodeFiles(config.tiltList()));
        }

		//设置位置、姿态、缩放等信息
		SetScale(config.scale());
		SetRotate(config.rotate());
		SetOffset(config.offset());
		SetPosition(config.position());
	}
}
