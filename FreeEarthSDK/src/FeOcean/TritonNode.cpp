#include <FeOcean/TritonDrawable.h>
#include <FeOcean/TritonContext.h>
#include <FeOcean/TritonNode.h>
#include <osgEarth/VirtualProgram>
#include <FeOcean/OceanNode.h>
#include <FeTriton/Configuration.h>
#include <FeUtils/CoordConverter.h>
#include <osg/Texture>
#include <osg/Stencil>
#include <osgDB/WriteFile>
#include <FeTriton/GodRays.h>
#include <FeTriton/ProjectedGrid.h>
#include <FeUtils/IntersectUtil.h>
#include <osgViewer/CompositeViewer>

using namespace FeOcean;



TritonNode::CTileNodeCollector::CTileNodeCollector(TritonNode* on)
	:m_opOceanNode(on)
{
}

void FeOcean::TritonNode::CTileNodeCollector::collectTileNodeImplementation( osg::Drawable& dr, osgUtil::CullVisitor* cv )
{
	if(cv && m_opOceanNode.valid())
	{
		if(dr.getNumParents() == 1)
		{
			osg::Geode* gn = dynamic_cast<osg::Geode*>(dr.getParent(0));
			if(gn && gn->getNumChildren() == 1)
			{
				osgEarth::Drivers::MPTerrainEngine::TileNode* tn = dynamic_cast<osgEarth::Drivers::MPTerrainEngine::TileNode*>(gn->getParent(0));
				if(tn)
				{
					osgEarth::Drivers::MPTerrainEngine::MPGeometry* mpg = dynamic_cast<osgEarth::Drivers::MPTerrainEngine::MPGeometry*>(&dr);
					if(mpg)
					{
						std::pair< MPGeometryMap::iterator,bool > flag = m_opOceanNode->m_mpGeometryMap.insert(MPGeometryMap::value_type(mpg,0));
						if(flag.second)
							cv->getTileNodeList().push_back(tn);
					}
				}
			}
		}
	}
}


TritonNode::TritonNode(
    FeUtil::CRenderContext* rc, 
    const osgEarth::SpatialReference * srs,
    const std::string& strTritonPath)
	:COceanNode(rc)
	,m_pTritonInitThread(NULL)
	,m_opRenderContext(rc)
	,m_isReflectionCamTraverse(false)
	,m_pTileNodeCallback(NULL)
	,m_rpOriginalSceneCam(NULL)
    ,m_rpOriginalSceneTex(NULL)
    ,m_rpPostRenderNode(NULL)
	,m_rpGodRayTex(NULL)
	,m_rpGodRayCamera(NULL)
{
	m_pTritonInitThread = new TritonInitThread(this);

	m_opTritonContext = new TritonContext(strTritonPath);
	m_opTritonContext->SetSRS(srs);
	SetOceanContext(m_opTritonContext.get());

	//m_rpOriginalSceneCam = new MyOriginalSceneCamera(this);
	//m_rpOriginalSceneTex = new osg::Texture2D();
	m_rpPostRenderNode = new MyPostRenderNode();
	m_rpGodRayTex = new osg::Texture2D();
	m_rpGodRayCamera = new MyGodRayNode();

	m_pTileNodeCallback = new CTileNodeCollector(this);

	m_rpReflectionCullCallback = new MyReflectionCullCallback(this);
	SetupReflectionCamera();

	if(rc)
	{
		_drawable = new TritonDrawable(rc->GetView(), this);
		osg::Geode* geode = new osg::Geode();
		geode->addDrawable( _drawable );
		this->addChild( geode );

		_drawable->getOrCreateStateSet()->setRenderBinDetails(1,"DepthSortedBin");
	
		rc->GetManipulatorIntersectionList().push_back(this);
	}
	
}

TritonNode::~TritonNode()
{
	if(m_pTritonInitThread != NULL)
	{
		if(m_pTritonInitThread->isRunning())
		{
			m_pTritonInitThread->cancel();
			m_pTritonInitThread->YieldCurrentThread();

			delete m_pTritonInitThread;
			m_pTritonInitThread = NULL;
		}
	}

	m_pTileNodeCallback = NULL;
}

void TritonNode::OnSetSeaLevel()
{
	//m_mutex.lock();

	if ( GetTritonContext()->IsReady() )
	{
		float d = GetSeaLevel();
		GetTritonContext()->GetEnvironment()->SetSeaLevel( GetSeaLevel() );
	}
	dirtyBound();

	//m_mutex.unlock();
}

osg::BoundingSphere
	TritonNode::computeBound() const
{
	return osg::BoundingSphere();
}

TritonContext* TritonNode::GetTritonContext() const
{
	return m_opTritonContext.valid() ? m_opTritonContext.get() : 0;
}

void TritonNode::SetSeaLevel( double dValue )
{
	//m_mutex.lock();

    if (GetTritonContext()->GetEnvironment() != NULL)
    {
        GetTritonContext()->GetEnvironment()->SetSeaLevel(dValue);
    }

	//m_mutex.unlock();
}

double TritonNode::GetSeaLevel() const
{
	//m_mutex.lock();
	
	double level = 0.0;

    if (GetTritonContext()->GetEnvironment() != NULL)
    {
		level = GetTritonContext()->GetEnvironment()->GetSeaLevel();
    }

	//m_mutex.unlock();

	return level;
}


void TritonNode::SetWindSpeed( double dSpeed )
{
	//m_mutex.lock();

	if (GetTritonContext()->GetEnvironment() != NULL)
	{
		Triton::Vector3 pos;
		double dTemp, dLength, dDirect;
		GetTritonContext()->GetEnvironment()->GetWind(pos, dTemp, dDirect, dLength);

		Triton::WindFetch wf;
		wf.SetWindDirection(dDirect);
		wf.SetWindLength(dLength);
		wf.SetWindSpeed(dSpeed);

		GetTritonContext()->GetEnvironment()->ClearWindFetches();
		GetTritonContext()->GetEnvironment()->AddWindFetch(wf);
	}

	//m_mutex.unlock();
}

double TritonNode::GetWindSpeed()  const
{
	//m_mutex.lock();
	double dSpeed = 0;

	if (GetTritonContext()->GetEnvironment() != NULL)
	{
		double dDirection = 0.0;
		double dFetchLenght = 0.0;
		Triton::Vector3 vecPos;

		GetTritonContext()->GetEnvironment()->GetWind(vecPos, dSpeed, dDirection, dFetchLenght);
	}

	return dSpeed;

	//m_mutex.unlock();
}
	
void TritonNode::SetWindDirection( double dDirection )
{
	//m_mutex.lock();

	if (GetTritonContext()->GetEnvironment() != NULL)
	{
		Triton::Vector3 pos;
		double dSpeed, dLength, dDirect;
		GetTritonContext()->GetEnvironment()->GetWind(pos, dSpeed, dDirect, dLength);

		Triton::WindFetch wf;
		wf.SetWindDirection(dDirection);
		wf.SetWindLength(dLength);
		wf.SetWindSpeed(dSpeed);
		GetTritonContext()->GetEnvironment()->ClearWindFetches();
		GetTritonContext()->GetEnvironment()->AddWindFetch(wf);
	}

	//m_mutex.unlock();
}

double TritonNode::GetWindDirection() const
{
	//m_mutex.lock();

	double dDirection = 0;

	if (GetTritonContext()->GetEnvironment() != NULL)
	{
		double dSpeed = 0.0;
		double dFetchLenght = 0.0;
		Triton::Vector3 vecPos;

		GetTritonContext()->GetEnvironment()->GetWind(vecPos, dSpeed, dDirection, dFetchLenght);
	}

	return dDirection;

	//m_mutex.unlock();
}

void TritonNode::SetWindLength( double dFetchLenght )
{
	//m_mutex.lock();

	if (GetTritonContext()->GetEnvironment() != NULL)
	{
		Triton::Vector3 pos;
		double dSpeed, dLength, dDirect;
		GetTritonContext()->GetEnvironment()->GetWind(pos, dSpeed, dDirect, dLength);

		Triton::WindFetch wf;
		wf.SetWindDirection(dDirect);
		wf.SetWindLength(dFetchLenght);
		wf.SetWindSpeed(dSpeed);

		GetTritonContext()->GetEnvironment()->ClearWindFetches();
		GetTritonContext()->GetEnvironment()->AddWindFetch(wf);
	}

	//m_mutex.unlock();
}

double TritonNode::GetWindLength() const
{
	//m_mutex.lock();
	double dFetchLenght = 0;

	if (GetTritonContext()->GetEnvironment() != NULL)
	{
		double dSpeed = 0.0;
		double dDirection = 0.0;
		Triton::Vector3 vecPos;

		GetTritonContext()->GetEnvironment()->GetWind(vecPos, dSpeed, dDirection, dFetchLenght);
	}

	return dFetchLenght;

	//m_mutex.unlock();
}

const char* vertexShader = 
	"#version " GLSL_VERSION_STR "\n" 
	GLSL_DEFAULT_PRECISION_FLOAT "\n" 

	"attribute vec4  osgearth_elevData; \n" // osgearth_elevData //oe_terrain_attr 
	"varying float height;\n" 

	"void setupContour(inout vec4 VertexModel) \n" 
	"{ \n" 
	"    height = osgearth_elevData[3]; \n" 
	"} \n"; 

const char* fragmentShader = 
	"#version " GLSL_VERSION_STR "\n" 
	GLSL_DEFAULT_PRECISION_FLOAT "\n" 

	"varying float height;\n" 

	"void colorContour( inout vec4 color ) \n" 
	"{ \n" 
	"   float nHeight = height;\n"
	"   color = vec4( nHeight, 0.0, 0.0, 1.0 ); \n" 
	"} \n"; 


void TritonNode::SetupHeightMap()
{ 
	if(m_opViewer.valid())
	{
		int textureUnit = 0; 
		int textureSize = 1024 * 4; 

		m_heightMap = new osg::Texture2D; 
		m_heightMap->setTextureSize(textureSize, textureSize); 
		m_heightMap->setInternalFormat(GL_LUMINANCE32F_ARB); 
		m_heightMap->setSourceFormat(GL_LUMINANCE); 
		m_heightMap->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		m_heightMap->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

		m_pHeightCamera = new HeightCamera(); 
		m_pHeightCamera->setName("ocean_height_camera");
		m_pHeightCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT); 
		m_pHeightCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); 
		m_pHeightCamera->setClearColor(osg::Vec4(-1000.0, -1000.0, -1000.0, 1.0f)); 
		m_pHeightCamera->setViewport(0, 0, textureSize, textureSize); 
		m_pHeightCamera->setRenderOrder(osg::Camera::PRE_RENDER); 
		m_pHeightCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
		m_pHeightCamera->attach(osg::Camera::COLOR_BUFFER, m_heightMap); 
		m_pHeightCamera->setAllowEventFocus(false);

		osg::ref_ptr<PassHeightMapToTritonCallback>  callback = new PassHeightMapToTritonCallback(GetTritonContext()->GetEnvironment(),this);
		callback->_opHeightMap = m_heightMap;
		m_pHeightCamera->setFinalDrawCallback(callback.get());

		osg::ref_ptr<osgEarth::VirtualProgram> heightProgram1 = new osgEarth::VirtualProgram(); 
		heightProgram1->setFunction( "setupContour", vertexShader,   osgEarth::ShaderComp::LOCATION_VERTEX_MODEL); 
		heightProgram1->setFunction( "colorContour", fragmentShader, osgEarth::ShaderComp::LOCATION_FRAGMENT_COLORING, -1.0); 
		
		osg::ref_ptr<osg::StateSet> stateSet1 = m_pHeightCamera->getOrCreateStateSet(); 
		stateSet1->setAttributeAndModes(heightProgram1, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE); 
		heightProgram1->addBindAttribLocation( "osgearth_elevData", osg::Drawable::ATTRIBUTE_6 ); 
		m_pHeightCamera->setViewMatrix(m_opViewer->getCamera()->getViewMatrix()); 
		m_pHeightCamera->setProjectionMatrix(m_opViewer->getCamera()->getProjectionMatrix()); 
	}
}  

void FeOcean::TritonNode::InitializeImpl()
{
	if(GetTritonContext() && GetTritonContext()->IsReady())
	{
		SetupHeightMap();
		if(m_pHeightCamera.valid())
		{
			this->addChild(m_pHeightCamera);
		}

		CreateOriginalSceneCam();
		initGodRayCamera();
		initPostRenderNode();
	}
}

void FeOcean::TritonNode::traverse( osg::NodeVisitor& nv )
{
	if ( nv.getVisitorType() == nv.CULL_VISITOR &&  GetSRS() != NULL)
	{
		osgUtil::CullVisitor* cv = osgEarth::Culling::asCullVisitor(nv);

		if(m_isReflectionCamTraverse)
			return;

		if ( cv->getCurrentCamera() )
		{
			if (!m_bIsInVisibleRange)
			{
				cv->setCollectTileNodeCallback(NULL);
				m_mpGeometryMap.clear();
				return;
			}
			else if(IsInitialized())
			{		
				if(cv->getCollectTileNodeCallback() == NULL && m_pTileNodeCallback.valid())
				{
					cv->setCollectTileNodeCallback(m_pTileNodeCallback.get());
					return;
				}

				if(m_pHeightCamera && m_opRenderContext.valid())
				{
					osgUtil::CullVisitor::TileNodeList l = cv->getTileNodeList();

					osgEarth::MapNode* mn = m_opRenderContext->GetMapNode();
					if(mn)
					{
						osgEarth::TerrainEngineNode* tn = mn->getTerrainEngine();
						if(tn)
						{
							TileNodeVisitor tv(this,l);
							tv.initBoundBox();
							tn->accept(tv);
						}
					}

					m_pHeightCamera->removeChild(0,m_pHeightCamera->getNumChildren());
					m_pHeightCamera->m_boundSphere.init();
					for (osgUtil::CullVisitor::TileNodeList::iterator it = l.begin();it != l.end();it++)
					{
						m_pHeightCamera->m_boundSphere.expandBy((*it)->getBound());
						m_pHeightCamera->addChild((*it).get());
					}
					m_pHeightCamera->setMatrix();

					if(IsEnableReflection() && m_rpReflectionCam.valid())
					{
						setReflectionMatrix(m_bCanReflecting);
						if(!this->containsNode(m_rpReflectionCam))
							this->addChild(m_rpReflectionCam);
					}
					else
					{
						if(GetTritonContext())
						{
							Triton::Environment* en = GetTritonContext()->GetEnvironment();
							if(en)
							{
								Triton::Matrix3 tm;
								en->SetPlanarReflectionMap(NULL,tm); 
							}
						}
					}

					Group::traverse(nv);

					if(m_rpReflectionCam.valid() && this->containsNode(m_rpReflectionCam))
						this->removeChild(m_rpReflectionCam);

					m_bCanReflecting = false;

					m_pHeightCamera->removeChild(0,m_pHeightCamera->getNumChildren());

					return;
				}	
			}
		}
	}
	osg::Group::traverse( nv );
}


void FeOcean::TritonNode::SetupReflectionCamera()
{
	if(!m_rpReflectionCam.valid())
	{
		int textureSize = 1024 * 2; 

		m_rpReflectionTexture = new osg::Texture2D; 
		m_rpReflectionTexture->setInternalFormat(GL_RGBA);
		m_rpReflectionTexture->setTextureSize(textureSize, textureSize); 
		m_rpReflectionTexture->setSourceFormat(GL_LUMINANCE); 
		m_rpReflectionTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		m_rpReflectionTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		m_rpReflectionTexture->setWrap(osg::Texture::WRAP_S,osg::Texture::MIRROR);
		m_rpReflectionTexture->setWrap(osg::Texture::WRAP_T,osg::Texture::MIRROR);

		m_rpReflectionCam = new ReflectionCamera(this);
		m_rpReflectionCam->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK),
			osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		m_rpReflectionCam->setName("ocean_reflection_cam");
		m_rpReflectionCam->setReferenceFrame(osg::Transform::ABSOLUTE_RF); 
		m_rpReflectionCam->setViewport(0, 0, textureSize, textureSize); 
		m_rpReflectionCam->setRenderOrder(osg::Camera::PRE_RENDER); 
		m_rpReflectionCam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
		m_rpReflectionCam->attach(osg::Camera::COLOR_BUFFER, m_rpReflectionTexture); 
		m_rpReflectionCam->setAllowEventFocus(false);

		osg::ref_ptr<FinalReflectionMapDrawCallback>  callback = new FinalReflectionMapDrawCallback(this);
		m_rpReflectionCam->setFinalDrawCallback(callback.get());

	}
}

void FeOcean::TritonNode::setReflectionMatrix(bool isReflecting)
{
	if(m_rpReflectionCam.valid() && m_opRenderContext.valid() && GetTritonContext())
	{
		osg::Camera* cam = m_opRenderContext->GetCamera();
		Triton::Environment* en = GetTritonContext()->GetEnvironment();
		Triton::Ocean* oc = GetTritonContext()->GetOcean();
		if(cam && en && oc)
		{
			en->SetCameraMatrix(cam->getViewMatrix().ptr());
			en->SetProjectionMatrix(cam->getProjectionMatrix().ptr());

			Triton::Matrix4 vMat;
			oc->ComputeReflectionMatrices(vMat,m_reflectionTextureMat);

			osg::Matrix reflectMat = osg::Matrix(vMat(0,0),vMat(0,1),vMat(0,2),vMat(0,3), 
				vMat(1,0),vMat(1,1),vMat(1,2),vMat(1,3), 
				vMat(2,0),vMat(2,1),vMat(2,2),vMat(2,3), 
				vMat(3,0),vMat(3,1),vMat(3,2),vMat(3,3)); 

			osg::Matrix newViewMat = cam->getViewMatrix();
			if(isReflecting)
				newViewMat = reflectMat * newViewMat;

			m_rpReflectionCam->setViewMatrix(newViewMat);

			float fov,asp,nc,fc;
			cam->getProjectionMatrix().getPerspective(fov,asp,nc,fc);
			m_rpReflectionCam->setProjectionMatrixAsPerspective(fov,asp,1.0, 10000.0);
		}
	}
}

void FeOcean::TritonNode::UpdateImpl()
{
	if(IsInitialized() && m_opRenderContext.valid())
	{
		removeOriginalSceneCamFromRoot();

		if(!m_bIsInVisibleRange)
		{
			return;
		}

		updateOriginalSceneTexSize();

		osg::ref_ptr<osg::Camera> cam = new osg::Camera();
		if(m_opViewer->getCameraManipulator())
		{
			m_opViewer->getCameraManipulator()->updateCamera(*cam.get());
		}

		if(_drawable)
		{
			osg::Vec3d eye,tmp;
			cam->getViewMatrix().getLookAt(eye,tmp,tmp);
			
			_drawable->setTritonBound(eye);
		}

		if(GetTritonContext() && GetTritonContext()->GetOcean() && !GetTritonContext()->GetOcean()->getIsCamAboveWater())
		{
			insertOriginalSceneCamToRoot();
			updateGodRay(cam);
			updatePostRenderNode();	
		}
	}
	
}
void FeOcean::TritonNode::CreateOriginalSceneCam( int width /*= 1024*/,int height /*= 1024*/ )
{
	if(m_rpOriginalSceneCam == NULL && m_rpOriginalSceneTex == NULL)
	{
		m_rpOriginalSceneTex = new osg::Texture2D();
		m_rpOriginalSceneTex->setTextureSize(width, height); 
		m_rpOriginalSceneTex->setInternalFormat(GL_RGB16F_ARB); 
		m_rpOriginalSceneTex->setSourceFormat(GL_LUMINANCE); 
		m_rpOriginalSceneTex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		m_rpOriginalSceneTex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		m_rpOriginalSceneTex->setWrap(osg::Texture::WRAP_S,osg::Texture::MIRROR);
		m_rpOriginalSceneTex->setWrap(osg::Texture::WRAP_T,osg::Texture::MIRROR);

		if(m_rpPostRenderNode.valid())
		{
			if(m_rpPostRenderNode->getNumChildren() > 0)
			{
				osg::Geode* gn = dynamic_cast<osg::Geode*>(m_rpPostRenderNode->getChild(0));
				if(gn && gn->getNumDrawables() > 0)
				{
					osg::Geometry* gm = dynamic_cast<osg::Geometry*>(gn->getDrawable(0));
					if(gm)
					{
						osg::StateSet* ss = gm->getStateSet();
						if(ss)
						{
							int textUnit = 0;
							ss->setTextureAttributeAndModes(textUnit,m_rpOriginalSceneTex,osg::StateAttribute::ON);
						}
					}
				}
			}
		}

		m_rpOriginalSceneCam = new MyOriginalSceneCamera(this);
		m_rpOriginalSceneCam->setName("ocean_originalscene_camera");
		m_rpOriginalSceneCam->setReferenceFrame(osg::Transform::RELATIVE_RF); 
		m_rpOriginalSceneCam->setViewport(0, 0, width, height); 
		m_rpOriginalSceneCam->setRenderOrder(osg::Camera::POST_RENDER); 
		m_rpOriginalSceneCam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
		m_rpOriginalSceneCam->attach(osg::Camera::COLOR_BUFFER, m_rpOriginalSceneTex); 
		m_rpOriginalSceneCam->setAllowEventFocus(false);	
		m_rpOriginalSceneCam->setInheritanceMask(osg::CullSettings::COMPUTE_NEAR_FAR_MODE |
			osg::CullSettings::CULLING_MODE                            |
			osg::CullSettings::LOD_SCALE                               |
			osg::CullSettings::SMALL_FEATURE_CULLING_PIXEL_SIZE        |
			osg::CullSettings::CLAMP_PROJECTION_MATRIX_CALLBACK        |
			osg::CullSettings::NEAR_FAR_RATIO                          |
			osg::CullSettings::IMPOSTOR_ACTIVE                         |
			osg::CullSettings::DEPTH_SORT_IMPOSTOR_SPRITES             |
			osg::CullSettings::IMPOSTOR_PIXEL_ERROR_THRESHOLD          |
			osg::CullSettings::NUM_FRAMES_TO_KEEP_IMPOSTORS_SPRITES    |
			osg::CullSettings::CULL_MASK                               |
			osg::CullSettings::CULL_MASK_LEFT                          |
			osg::CullSettings::CULL_MASK_RIGHT                         |
			osg::CullSettings::CLEAR_COLOR                             |
			osg::CullSettings::CLEAR_MASK                              |
			osg::CullSettings::LIGHTING_MODE                           |
			osg::CullSettings::LIGHT                                   );

	}
}

static const char *PostRenderVertSource = {
	"varying vec3 godRayPos;\n"
	"varying vec2 uv;\n"
	"uniform vec3 uCorner0;\n"
	"uniform vec3 uCorner01;\n" 
	"uniform vec3 uCorner02;\n"
	"void main(void)\n"
	"{\n"
	"    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"    gl_Position = gl_Vertex;\n"
	"    uv = (vec2(gl_Vertex.x, -gl_Vertex.y) + 1.0) * 0.5;\n"
	"    vec3 vCorner01 = uCorner01 * uv.x;\n"
	"    vec3 vCorner02 = uCorner02 * uv.y;\n"
	"    godRayPos = uCorner0 + vCorner01 + vCorner02;\n"
	"}\n"
};

static const char *PostRenderFragSource = {
	"uniform sampler2D originalMap;\n"
	"uniform sampler2D distortionMap;\n"
	"uniform sampler2D godRayMap;\n"
	"uniform float uTime;\n"
	"uniform vec3 uLightDirection;\n"
	"uniform vec3 uHGg;\n"
	"uniform float uIntensity;\n"
	"uniform vec3  uSunColor;\n"
	"uniform vec3  uCamUp;\n"
	"uniform float uHeightFade;\n"
	"varying vec3 godRayPos;\n"
	"varying vec2 uv;\n"
	"void main(void)\n"
	"{\n"
	"vec2 tc = uv;\n"
	"tc.y = 1 - tc.y;\n"
	"vec2 distortUV = (texture2D(distortionMap, vec2(tc.x + uTime, tc.y - uTime)).xy - 0.5) * 0.03;\n"
	"vec3 view_vector = normalize(godRayPos);\n"
	"float dot_product = dot(view_vector, uLightDirection);\n"
	"float angleCam = acos(dot(uCamUp,uLightDirection));\n"
	"float num = uHGg.x;\n"
	"float den = (uHGg.y - uHGg.z * dot_product);\n"
	"den = inversesqrt(den);\n"
	"float phase = num * pow(den, 3.0);\n"
	"gl_FragColor = texture2D(originalMap,tc + distortUV);\n"
	"if(angleCam < (3.1415926 / 3.0) && angleCam > 0.0)\n"
	"{\n"
	"gl_FragColor.xyz += (0.15 + uIntensity * texture2D(godRayMap, tc).x) * phase * uHeightFade * uSunColor;"
	"}\n"

	//"gl_FragColor.xyz = vec3(1.0,0.0,0.0);\n"

	"}\n"
};

void FeOcean::TritonNode::initPostRenderNode()
{
	if(m_rpPostRenderNode.valid() && GetTritonContext())
	{
		osg::Geometry* geom = new osg::Geometry;

		double nearDepth = 0.0;
		osg::Vec3dArray* coords = new osg::Vec3dArray(4);
		(*coords)[0] = osg::Vec3d(-1,1,nearDepth);
		(*coords)[1] = osg::Vec3d(1,1,nearDepth);
		(*coords)[2] = osg::Vec3d(1,-1,nearDepth);
		(*coords)[3] = osg::Vec3d(-1,-1,nearDepth);
		geom->setVertexArray(coords);

		osg::Vec2dArray* tcoords = new osg::Vec2dArray(4);
		(*tcoords)[0].set(0.0f,0.0f);
		(*tcoords)[1].set(1.0f,0.0f);
		(*tcoords)[2].set(1.0f,1.0f);
		(*tcoords)[3].set(0.0f,1.0f);
		geom->setTexCoordArray(0,tcoords);
		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

		osg::StateSet* ss = new osg::StateSet();

		int textUnit = 0;
		ss->setTextureAttributeAndModes(textUnit,m_rpOriginalSceneTex,osg::StateAttribute::ON);

		osg::Program* program = new osg::Program;
		program->setName( "ocean_postRender" );
		program->addShader( new osg::Shader( osg::Shader::VERTEX, PostRenderVertSource ) );
		program->addShader( new osg::Shader( osg::Shader::FRAGMENT, PostRenderFragSource ) );
		ss->setAttributeAndModes( program, osg::StateAttribute::ON );
		ss->addUniform( new osg::Uniform("originalMap", textUnit) );

		int textUnit1 = 1;
		osg::ref_ptr<osg::Texture2D> distortionTex = new osg::Texture2D();
		Triton::Environment* en = GetTritonContext()->GetEnvironment();
		if(en)
		{
			Triton::ResourceLoader* rl = en->GetResourceLoader();
			if(rl)
			{
				std::string ts = rl->GetFilePath("UnderwaterDistortion.jpg");
				osg::Image* img = osgDB::readImageFile(ts);
				distortionTex->setImage(img);
			}
		}
		distortionTex->setWrap(osg::Texture::WRAP_S,osg::Texture::REPEAT);
		distortionTex->setWrap(osg::Texture::WRAP_T,osg::Texture::REPEAT);
		ss->setTextureAttributeAndModes(textUnit1,distortionTex,osg::StateAttribute::ON);
		ss->addUniform( new osg::Uniform("distortionMap", textUnit1) );

		int textUnit2 = 2;
		ss->setTextureAttributeAndModes(textUnit2,m_rpGodRayTex,osg::StateAttribute::ON);
		ss->addUniform( new osg::Uniform("godRayMap", textUnit2) );

		ss->addUniform( new osg::Uniform("uTime", 0.1f) );
		ss->addUniform( new osg::Uniform("uCorner0",osg::Vec3(0,0,0)));
		ss->addUniform( new osg::Uniform("uCorner01",osg::Vec3(0,0,0)));
		ss->addUniform( new osg::Uniform("uCorner02",osg::Vec3(0,0,0)));
		ss->addUniform( new osg::Uniform("uLightDirection",osg::Vec3(0,0,0)));
		ss->addUniform( new osg::Uniform("uHGg",osg::Vec3(0,0,0)));
		ss->addUniform( new osg::Uniform("uIntensity",0.0f));
		ss->addUniform( new osg::Uniform("uSunColor",osg::Vec3(0,0,0)));
		ss->addUniform( new osg::Uniform("uCamUp",osg::Vec3(0,0,0)));
		ss->addUniform( new osg::Uniform("uHeightFade", 1.0f) );
		
		geom->setStateSet(ss);

		geom->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

		geom->setCullingActive(false);

		osg::Geode* gn = new osg::Geode();
		gn->addChild(geom);

		m_rpPostRenderNode->addChild( gn );
		m_rpPostRenderNode->setDrawBuffer(GL_FRONT_AND_BACK);
	}
}

void FeOcean::TritonNode::removeOriginalSceneCamFromRoot()
{
	if(m_opRenderContext.valid() && m_rpOriginalSceneCam.valid())
	{
		osg::Group* root = m_opRenderContext->GetRoot();
		if(root)
		{
			if(root->containsNode(m_rpOriginalSceneCam))
			{
				for (unsigned int i = 0;i < m_rpOriginalSceneCam->getNumChildren();i++)
				{
					if(m_rpOriginalSceneCam->getChild(i) == m_rpPostRenderNode)
						continue;

					if(!root->containsNode(m_rpOriginalSceneCam->getChild(i)))
						root->addChild(m_rpOriginalSceneCam->getChild(i));
				}

				m_rpOriginalSceneCam->removeChildren(0,m_rpOriginalSceneCam->getNumChildren());

				root->removeChild(m_rpOriginalSceneCam);
			}

			/*if(root->containsNode(m_rpPostRenderNode))
			{
				root->removeChild(m_rpPostRenderNode);
			}*/
			if(root->containsNode(m_rpGodRayCamera))
			{
				root->removeChild(m_rpGodRayCamera);
			}
		}
	}
}

void FeOcean::TritonNode::insertOriginalSceneCamToRoot()
{
	if(m_opRenderContext.valid() && m_rpOriginalSceneCam.valid())
	{
		osg::Group* root = m_opRenderContext->GetRoot();
		if(root)
		{
			if(!root->containsNode(m_rpOriginalSceneCam))
			{
				m_rpOriginalSceneCam->removeChildren(0,m_rpOriginalSceneCam->getNumChildren());

				for (unsigned int i = 0;i < root->getNumChildren();i++)
				{	
					m_rpOriginalSceneCam->addChild(root->getChild(i));
				}

				if(m_rpPostRenderNode.valid())
				{
					m_rpOriginalSceneCam->addChild(m_rpPostRenderNode);
				}

				root->removeChildren(0,root->getNumChildren());

				root->addChild(m_rpOriginalSceneCam);
			}
			else
			{
			}

			if(!root->containsNode(m_rpGodRayCamera) && m_rpGodRayCamera.valid())
			{
				root->addChild(m_rpGodRayCamera);
			}
			/*if(!root->containsNode(m_rpPostRenderNode) && m_rpPostRenderNode.valid())
			{
			root->addChild(m_rpPostRenderNode);
			}
			else
			{
			}*/
		}
	}
}

void FeOcean::TritonNode::updatePostRenderNode()
{
	if(m_rpPostRenderNode.valid() && m_opRenderContext.valid())
	{
		osg::Geode* gn = dynamic_cast<osg::Geode*>(m_rpPostRenderNode->getChild(0));
		if(gn)
		{
			osg::Geometry* gm = dynamic_cast<osg::Geometry*>(gn->getChild(0));
			if(gm)
			{
				osg::StateSet* ss = gm->getStateSet();
				if(ss)
				{
					osg::Uniform* uTime = ss->getUniform("uTime");
					if(uTime)
					{
						osgViewer::View* vv = m_opRenderContext->GetView();
						if(vv)
						{
							double totalSeconds = vv->getFrameStamp()->getReferenceTime();
							uTime->set((float)(totalSeconds * 0.1));
						}
					}

					osg::Uniform* uLightDir = ss->getUniform("uLightDirection");
					if(uLightDir)
					{
						if(GetTritonContext())
						{
							Triton::Environment* en = GetTritonContext()->GetEnvironment();
							if(en)
							{
								Triton::Vector3 ld = en->GetLightDirection();
								osg::Vec3 old(ld.x,ld.y,ld.z);
								old.normalize();
								uLightDir->set(old);
							}	
						}
					}

					if(GetTritonContext())
					{
						Triton::Ocean* oc = GetTritonContext()->GetOcean();
						if(oc)
						{
							Triton::ProjectedGrid* pg = oc->GetProjectedGrid();
							if(pg)
							{
								osg::Uniform* uSunColor = ss->getUniform("uSunColor");
								if(uSunColor)
								{
									Triton::Vector3 sc = pg->GetDoubleRefractionColor();
									uSunColor->set(osg::Vec3(sc.x,sc.y,sc.z));
								}
							}
							Triton::GodRays* gr = oc->GetGodRays();
							if(gr)
							{
								osg::Uniform* uHGg = ss->getUniform("uHGg");
								if(uHGg)
								{
									Triton::Vector3 gde = gr->getGodRaysExposure();
									uHGg->set(osg::Vec3(gde.x,gde.y,gde.z));
								}

								osg::Uniform* uIntensity = ss->getUniform("uIntensity");
								if(uIntensity)
								{
									uIntensity->set(gr->getGodRaysIntensity());
								}

								osg::Uniform* uHeightFade = ss->getUniform("uHeightFade");
								if(uHeightFade)
								{
									Triton::Environment* en = GetTritonContext()->GetEnvironment();
									if(en)
									{
										const double* cp = en->GetCameraPosition();
										osg::Vec3d ocp(cp[0],cp[1],cp[2]);
										Triton::Vector3 gp = gr->getGodRayPosition();
										osg::Vec3d ogp(gp.x,gp.y,gp.z);

										double alt = (ocp - ogp).length();
										float fade = 1.0;
										double ba = 200;
										double ea = 400;
										if(alt < ba)
											fade = 1.0;
										else if(alt > ba && alt < ea)
										{
											fade = (alt - ea) / (ba - ea);
										}
										else
											fade = 0.0;

										uHeightFade->set(fade);
									}
									
								}
							}
						}
					}

					osg::Uniform* uc0 = ss->getUniform("uCorner0");
					osg::Uniform* uc1 = ss->getUniform("uCorner01");
					osg::Uniform* uc2 = ss->getUniform("uCorner02");
					if(uc0 && uc1 && uc2)
					{	
						osg::Camera* cam = m_opRenderContext->GetCamera();
						if(cam)
						{
							float nearCilp,farClip,aspect,fov;
							cam->getProjectionMatrix().getPerspective(fov,aspect,nearCilp,farClip);
					
							osg::Vec3d camPos1,center,upVector1;
							cam->getViewMatrixAsLookAt(camPos1,center,upVector1);
							osg::Vec3d lookVector1 = center - camPos1;
							lookVector1.normalize();
							upVector1.normalize();
							osg::Vec3d rightVector1 = lookVector1 ^ upVector1;
							rightVector1.normalize();

							osg::Vec3d camPos(0,0,0);
							osg::Vec3d farCenter = camPos + lookVector1 * farClip;

							float halfAngle = fov / 2.0;
							double centerUpDis = farClip * std::tan(osg::DegreesToRadians(halfAngle));
							double leftCenterDis = centerUpDis * aspect;
						    
                            osg::Vec3d farLeftTop = farCenter + upVector1 * centerUpDis - rightVector1 * leftCenterDis;

							uc0->set((osg::Vec3)farLeftTop);
							uc1->set((osg::Vec3)(rightVector1 * leftCenterDis * 2.0));
							uc2->set((osg::Vec3)(-upVector1 * centerUpDis * 2.0));

							osg::Uniform* uCamUp = ss->getUniform("uCamUp");
							if(uCamUp)
							{
								osg::Vec3d camDir = camPos1;
								camDir.normalize();
								uCamUp->set((osg::Vec3)camDir);
							}
						}
					}
				}
			}
		}
	}
}

void FeOcean::TritonNode::updateGodRay(osg::Camera* cam)
{
	if(GetTritonContext() && m_opRenderContext.valid())
	{
		if(cam)
		{
			::Triton::Environment* environment = GetTritonContext()->GetEnvironment();
			if ( environment)
			{
				environment->SetCameraMatrix( cam->getViewMatrix().ptr() );
				environment->SetProjectionMatrix( cam->getProjectionMatrix().ptr() );			
			}

			Triton::Ocean* oc = GetTritonContext()->GetOcean();
			if(oc)
			{
				oc->updateGodRay();
			}
		}
	}
}

void FeOcean::TritonNode::initGodRayCamera()
{
	if(m_rpGodRayCamera.valid() && m_rpGodRayTex.valid())
	{
		int texWidth = 0;
		int texHeight = 0;
		if(m_opRenderContext.valid())
		{
			osg::Camera* cam = m_opRenderContext->GetCamera();
			if(cam)
			{
				osg::Viewport* vp = cam->getViewport();
				if(vp)
				{
					texWidth = vp->width();
					texHeight = vp->height();
				}
			}
		}

		m_rpGodRayTex->setTextureSize(texWidth, texHeight); 
		m_rpGodRayTex->setInternalFormat(GL_LUMINANCE32F_ARB); 
		m_rpGodRayTex->setSourceFormat(GL_LUMINANCE); 
		m_rpGodRayTex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		m_rpGodRayTex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

		m_rpGodRayCamera->setName("ocean_godray_camera");
		m_rpGodRayCamera->setReferenceFrame(osg::Transform::RELATIVE_RF); 
		m_rpGodRayCamera->setViewport(0, 0, texWidth, texHeight); 
		m_rpGodRayCamera->setRenderOrder(osg::Camera::PRE_RENDER); 
		m_rpGodRayCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
		m_rpGodRayCamera->attach(osg::Camera::COLOR_BUFFER, m_rpGodRayTex); 
		m_rpGodRayCamera->setAllowEventFocus(false);	
		m_rpGodRayCamera->setInheritanceMask(osg::CullSettings::COMPUTE_NEAR_FAR_MODE |
			osg::CullSettings::CULLING_MODE                            |
			osg::CullSettings::LOD_SCALE                               |
			osg::CullSettings::SMALL_FEATURE_CULLING_PIXEL_SIZE        |
			osg::CullSettings::CLAMP_PROJECTION_MATRIX_CALLBACK        |
			osg::CullSettings::NEAR_FAR_RATIO                          |
			osg::CullSettings::IMPOSTOR_ACTIVE                         |
			osg::CullSettings::DEPTH_SORT_IMPOSTOR_SPRITES             |
			osg::CullSettings::IMPOSTOR_PIXEL_ERROR_THRESHOLD          |
			osg::CullSettings::NUM_FRAMES_TO_KEEP_IMPOSTORS_SPRITES    |
			osg::CullSettings::CULL_MASK                               |
			osg::CullSettings::CULL_MASK_LEFT                          |
			osg::CullSettings::CULL_MASK_RIGHT                         |
			osg::CullSettings::CLEAR_COLOR                             |
			osg::CullSettings::CLEAR_MASK                              |
			osg::CullSettings::LIGHTING_MODE                           |
			osg::CullSettings::LIGHT                                   );

		
		osg::ref_ptr<osg::Geode> gn = new osg::Geode();
		FeOcean::GodRayDrawable* gr = new FeOcean::GodRayDrawable(this);
		gn->addChild(gr);

		m_rpGodRayCamera->addChild(gn);
	}
}

bool FeOcean::TritonNode::intersectWithRay( const osg::Vec3d& start,const osg::Vec3d& end,osg::Vec3d& hitPoint )
{
	bool hitOcean = false;

	if(m_opRenderContext.valid())
	{	
		osg::Vec3d intersectPoint1,intersectPoint2;
		double eqRad = osg::WGS_84_RADIUS_EQUATOR;
		double polRad = osg::WGS_84_RADIUS_POLAR;
		Triton::Configuration::GetDoubleValue("equatorial-earth-radius-meters", eqRad);
		Triton::Configuration::GetDoubleValue("polar-earth-radius-meters", polRad);

		osg::Vec3d s = start;
		osg::Vec3d e = end;

		hitOcean = FeMath::intersectWithEllipse(eqRad, eqRad ,polRad ,
			s,e,intersectPoint1,intersectPoint2);

		bool hit1 = false;
		bool hit2 = false;
		osg::Vec3d rdir = end - start;
		rdir.normalize();
		osg::Vec3d dir1 = intersectPoint1 - start;
		dir1.normalize();
		osg::Vec3d dir2 = intersectPoint2 - start;
		dir2.normalize();

		double dot1 = rdir * dir1;
		double dot2 = rdir * dir2;
		if(dot1 > 0.5)
			hit1 = true;
		if(dot2 > 0.5)
			hit2 = true;

		if(hitOcean)
		{
			if(hit1 && hit2)
			{
				double length1 = (intersectPoint1 - start).length();
				double length2 = (intersectPoint2 - start).length();

				hitPoint = length1 < length2 ? intersectPoint1 : intersectPoint2;
			}
			else if(hit1)
				hitPoint = intersectPoint1;
			else if(hit2)
				hitPoint = intersectPoint2;
			else
				return false;
		}			
	}

	return hitOcean;
}

void FeOcean::TritonNode::updateOriginalSceneTexSize()
{
	if(m_opRenderContext.valid() && m_rpOriginalSceneTex.valid() && m_rpOriginalSceneCam.valid())
	{
		int texWidth = 1024;
		int texHeight = 1024;

		osg::Camera* cam = m_opRenderContext->GetCamera();
		if(cam)
		{
			osg::Viewport* vp = cam->getViewport();
			if(vp)
			{
				texWidth = vp->width();
				texHeight = vp->height();
			}
		}

		if(m_rpOriginalSceneTex->getTextureHeight() != texHeight || m_rpOriginalSceneTex->getTextureWidth() != texWidth)
		{	
			DeleteOriginalSceneCam();
			CreateOriginalSceneCam(texWidth,texHeight);
		}	
	}
}

void FeOcean::TritonNode::DeleteOriginalSceneCam()
{
	m_rpOriginalSceneCam = NULL;
	if(m_rpPostRenderNode.valid())
	{
		if(m_rpPostRenderNode->getNumChildren() > 0)
		{
			osg::Geode* gn = dynamic_cast<osg::Geode*>(m_rpPostRenderNode->getChild(0));
			if(gn && gn->getNumDrawables() > 0)
			{
				osg::Geometry* gm = dynamic_cast<osg::Geometry*>(gn->getDrawable(0));
				if(gm)
				{
					osg::StateSet* ss = gm->getStateSet();
					if(ss)
					{
						int textUnit = 0;
						ss->removeTextureAttribute(textUnit,m_rpOriginalSceneTex);
					}
				}
			}
		}
	}
	m_rpOriginalSceneTex = NULL;
}


void FeOcean::TritonNode::HeightCamera::setMatrix()
{	
	if(getNumChildren() > 0)
	{
		osg::BoundingSphere bs = m_boundSphere;
		osg::Vec3d sc = bs.center();
		osg::Vec3d lookDir = sc;
		lookDir.normalize();
		double safeRadius = bs.radius() + 100;

		osg::Vec3d camPos = sc + lookDir * safeRadius;

		osg::Vec3d worldUp(0,0,1);
		double dot = osg::absolute(worldUp * lookDir);
		if (osg::equivalent(dot, 1.0))
		{
			worldUp = osg::Vec3d(0, 1, 0);
		}

		this->setViewMatrixAsLookAt( camPos, osg::Vec3d(0.0,0.0,0.0), worldUp); 
		this->setProjectionMatrix(osg::Matrix::ortho(-safeRadius,safeRadius,-safeRadius,safeRadius,1.0,safeRadius * 2.0)); 

		const osg::Matrixd bias(0.5, 0.0, 0.0, 0.0, 
			0.0, 0.5, 0.0, 0.0, 
			0.0, 0.0, 0.5, 0.0, 
			0.5, 0.5, 0.5, 1.0); 
		if (this->getViewMatrix().valid() && this->getProjectionMatrix().valid())
		{
			osg::Matrix hMM = this->getViewMatrix() * this->getProjectionMatrix() * bias; 
			Triton::Matrix4 heightMapMatrix = Triton::Matrix4(hMM(0,0),hMM(0,1),hMM(0,2),hMM(0,3), 
				hMM(1,0),hMM(1,1),hMM(1,2),hMM(1,3), 
				hMM(2,0),hMM(2,1),hMM(2,2),hMM(2,3), 
				hMM(3,0),hMM(3,1),hMM(3,2),hMM(3,3)); 

			PassHeightMapToTritonCallback* cb = dynamic_cast<PassHeightMapToTritonCallback*>(this->getFinalDrawCallback()); 
			if(cb)
			{
				cb->_enable = true; 
				cb->_heightMapMatrix = heightMapMatrix; 
			}
		}
	}
}

FeOcean::TritonNode::HeightCamera::HeightCamera()
{
}

void FeOcean::TritonNode::PassHeightMapToTritonCallback::operator()( osg::RenderInfo& renderInfo ) const
{	
	osgEarth::Threading::ScopedMutexLock lock1(_mutex1);

	/// 将生成的地形高度纹理传递给海洋模块
	if (_tritonEnvironment != NULL && _opHeightMap.valid())
	{
		osg::Texture::TextureObject* to = _opHeightMap->getTextureObject(renderInfo.getContextID());
		if(to)
		{
			_tritonEnvironment->SetHeightMap((Triton::TextureHandle)to->id(),_heightMapMatrix); 
		}
	}

	/// 清空搜集的地球瓦片
	if(_opTritonNode.valid())
	{
		_opTritonNode->m_mpGeometryMap.clear();
	}
}


FeOcean::TritonNode::TileNodeVisitor::TileNodeVisitor( TritonNode* tn,osgUtil::CullVisitor::TileNodeList& tl)
	:m_opTritonNode(tn)
	,m_tileNodeList(tl)
{
	setTraversalMode(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
}

float FeOcean::TritonNode::TileNodeVisitor::getDistanceToViewPoint( const osg::Vec3& pos, bool useLODScale ) const
{
	if(m_opTritonNode.valid() && m_opTritonNode->m_opRenderContext.valid())
	{
		osg::Camera* cam = m_opTritonNode->m_opRenderContext->GetCamera();
		if(cam)
		{
			osg::Vec3d camPos,tmp;
			cam->getViewMatrixAsLookAt(camPos,tmp,tmp);
			return (pos - camPos).length();
		}
	}
	return 0.0;
}

void FeOcean::TritonNode::TileNodeVisitor::apply( osg::Node& node )
{
	if(m_box.intersects(node.getBound()))
		traverse(node);
}

void FeOcean::TritonNode::TileNodeVisitor::apply( osg::Geometry& dr )
{
	if(m_opTritonNode.valid())
	{
		if(dr.getNumParents() == 1)
		{
			osg::Geode* gn = dynamic_cast<osg::Geode*>(dr.getParent(0));
			if(gn && gn->getNumChildren() == 1)
			{
				osgEarth::Drivers::MPTerrainEngine::TileNode* tn = dynamic_cast<osgEarth::Drivers::MPTerrainEngine::TileNode*>(gn->getParent(0));
				if(tn)
				{
					osgEarth::Drivers::MPTerrainEngine::MPGeometry* mpg = dynamic_cast<osgEarth::Drivers::MPTerrainEngine::MPGeometry*>(&dr);
					if(mpg)
					{
						const osgEarth::Drivers::MPTerrainEngine::TileModel* tm = tn->getTileModel();
						if (tm && tm->_tileKey.getLevelOfDetail() > 8)
						{
							std::pair< MPGeometryMap::iterator, bool > flag = m_opTritonNode->m_mpGeometryMap.insert(MPGeometryMap::value_type(mpg, 0));
							if (flag.second)
							{
								m_tileNodeList.push_back(tn);
							}
						}
					}
				}
			}
		}
	}
}

void FeOcean::TritonNode::TileNodeVisitor::apply( osg::Geode& node )
{
	traverse(node);
}

void FeOcean::TritonNode::TileNodeVisitor::initBoundBox()
{
	m_box.init();
	if(m_opTritonNode.valid() && m_opTritonNode->m_opRenderContext.valid())
	{
		osg::Camera* cam = m_opTritonNode->m_opRenderContext->GetCamera();
		if(cam)
		{
			osg::Vec3d camPos,tmp;
			cam->getViewMatrixAsLookAt(camPos,tmp,tmp);
			camPos.normalize();
			camPos *= osg::WGS_84_RADIUS_EQUATOR;
			m_box.set(camPos,10000);
		}
	}
}

void FeOcean::TritonNode::FinalReflectionMapDrawCallback::operator()( osg::RenderInfo& renderInfo ) const
{
	if(_opTritonNode.valid())
	{
		if (_opTritonNode->GetTritonContext() && _opTritonNode->m_rpReflectionTexture.valid())
		{
			osg::Texture::TextureObject* to = _opTritonNode->m_rpReflectionTexture->getTextureObject(renderInfo.getContextID());
			Triton::Environment* en = _opTritonNode->GetTritonContext()->GetEnvironment();
			if(to && en)
			{
				en->SetPlanarReflectionMap((Triton::TextureHandle)to->id(),_opTritonNode->m_reflectionTextureMat); 
			}
		}
	}
	
}


void FeOcean::TritonNode::ReflectionCamera::traverse( osg::NodeVisitor& nv )
{
	if ( nv.getVisitorType() == nv.CULL_VISITOR )
	{
		osgUtil::CullVisitor* cv = osgEarth::Culling::asCullVisitor(nv);
		if(cv && m_opTritonNode.valid())
		{
			if(m_opTritonNode->m_opRenderContext.valid())
			{
				if(m_opTritonNode->m_rpReflectionCullCallback.valid())
				{
					if(m_opTritonNode->m_opAtmosphere.valid())
						this->addChild(m_opTritonNode->m_opAtmosphere.get());
					if(m_opTritonNode->m_opSun.valid())
						this->addChild(m_opTritonNode->m_opSun.get());
					if(m_opTritonNode->m_opMoon.valid())
						this->addChild(m_opTritonNode->m_opMoon.get());
					
					if(!m_opTritonNode->m_opRenderContext->GetOceanReflectionList().empty())
					{
						for (FeUtil::CRenderContext::OceanReflectionList::iterator it = m_opTritonNode->m_opRenderContext->GetOceanReflectionList().begin();
							it != m_opTritonNode->m_opRenderContext->GetOceanReflectionList().end();it++)
						{
							if((*it).valid())
								this->addChild((*it).get());
						}
					}
					
					cv->setReflecting(m_opTritonNode->m_bCanReflecting);
					cv->setReflectionCallback(m_opTritonNode->m_rpReflectionCullCallback);
					m_opTritonNode->m_isReflectionCamTraverse = true;

					osg::Group::traverse(nv);

					m_opTritonNode->m_isReflectionCamTraverse = false;
					cv->setReflectionCallback(NULL);
					cv->setReflecting(false);
					this->removeChild(0,this->getNumChildren());
				}	
			}	
		}
	}


}


