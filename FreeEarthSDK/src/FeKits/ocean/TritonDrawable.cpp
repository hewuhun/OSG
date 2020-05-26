#include <FeKits/ocean/TritonDrawable.h>
#include <FeKits/ocean/TritonContext.h>
#include <osg/Texture2D>
#include <osgEarth/VirtualProgram>
#include <osgEarth/NodeUtils>
#include <osgEarth/MapNode>
#include <OpenThreads/Mutex>

using namespace FeKit;

//仅内部使用，修改掩码图像回调
struct PassHeightMapToTritonCallback : public osg::Camera::DrawCallback 
{ 
	PassHeightMapToTritonCallback(Triton::Environment* tritonEnvironment) 
		: _tritonEnvironment(tritonEnvironment), 
		_enable(false), 
		_id(0) 
	{ 
		
	}; 

	virtual void operator()( osg::RenderInfo& renderInfo ) const 
	{ 
		//osgEarth::Threading::ScopedMutexLock lock1(_mutex1);
		if (_tritonEnvironment != NULL)
		{
			_tritonEnvironment->SetHeightMap((Triton::TextureHandle)_id,_heightMapMatrix); 

		}
		
	} 

	mutable bool _enable; 
	int _id; 
	Triton::Matrix4 _heightMapMatrix; 

private: 
	Triton::Environment* _tritonEnvironment; 
	mutable OpenThreads::Mutex _mutex1;	
}; 

//仅内部使用，地行更新回调，构造纹理矩阵
class OceanTerrainChangedCallback : public osgEarth::TerrainCallback 
{ 
public: 
	OceanTerrainChangedCallback(
		Triton::Environment* tritonEnvironment, 
		osgEarth::Map* map, 
		osgViewer::View* viewer, 
		osg::Camera* heightCam, 
		osg::Texture2D* heightMap ) 
		: osgEarth::TerrainCallback(),
		_tritonEnvironment(tritonEnvironment), 
		_map(map), 
		m_pViewer(viewer), 
		_heightCam(heightCam), 
		m_heightMap(heightMap) 
	{ 
	} 

	virtual void onTileAdded(const osgEarth::TileKey& tileKey, osg::Node* terrain, osgEarth::TerrainCallbackContext& context) 
	{ 
		//osgEarth::Threading::ScopedMutexLock lock1(_mutex1);
		osg::Vec3d eye, center, up; 
		m_pViewer->getCamera()->getViewMatrixAsLookAt(eye, center, up); 
		double fovyDEG=0.0, aspectRatio=0.0, zNear=0.0, zFar=0.0; 
		m_pViewer->getCamera()->getProjectionMatrixAsPerspective(fovyDEG, aspectRatio,zNear,zFar); 

		double fovxDEG = osg::RadiansToDegrees( 2.0 * atan( tan(osg::DegreesToRadians(fovyDEG))/2.0 * aspectRatio )); 

		//Eye LLH
		double eyeLat=0.0, eyeLon=0.0, eyeHeight=0.0; 
		_map->getSRS()->getEllipsoid()->convertXYZToLatLongHeight(eye.x(), eye.y(), eye.z(), eyeLat, eyeLon, eyeHeight); 

		//Eye XYZ
		double clampedEyeX=0.0, clampedEyeY=0.0,clampedEyeZ=0.0; 
		_map->getSRS()->getEllipsoid()->convertLatLongHeightToXYZ(eyeLat, eyeLon, 0.0, clampedEyeX, clampedEyeY, clampedEyeZ); 

		//LookAt LLH
		osg::Vec3 mslEye(clampedEyeX,clampedEyeY,clampedEyeZ); 
		double lookAtLat=0.0, lookAtLon=0.0, lookAtHeight=0.0; 
		_map->getSRS()->getEllipsoid()->convertXYZToLatLongHeight(center.x(), center.y(), center.z(), lookAtLat, lookAtLon, lookAtHeight); 

		// Calculate the distance to the horizon from the eyepoint 
		double eyeLen = eye.length(); 
		double radE = mslEye.length(); 
		double hmax = radE + 8848.0;  //最高高不过珠峰
		double hmin = radE - 12262.0; //最深深不过××海沟

		double hasl = osg::maximum(0.1, eyeLen - radE); 
		double radius = eyeLen - hasl; 
		double horizonDistance = osg::minimum(radE, sqrt( 2.0*radius*hasl + hasl*hasl )); 

		osg::Vec3d heightCamEye(eye); 

		double near = osg::maximum(1.0, heightCamEye.length() - hmax); 
		double far = osg::maximum(10.0, heightCamEye.length() - hmin + radE); 

		
		_heightCam->setProjectionMatrix(osg::Matrix::ortho(-horizonDistance,horizonDistance,-horizonDistance,horizonDistance,near,far) ); 
		_heightCam->setViewMatrixAsLookAt( heightCamEye, osg::Vec3d(0.0,0.0,0.0), osg::Vec3d(0.0,0.0,1.0)); 

		const osg::Matrixd bias(0.5, 0.0, 0.0, 0.0, 
			0.0, 0.5, 0.0, 0.0, 
			0.0, 0.0, 0.5, 0.0, 
			0.5, 0.5, 0.5, 1.0); 

		osg::Matrix hMM = _heightCam->getViewMatrix() * _heightCam->getProjectionMatrix() * bias; 
		Triton::Matrix4 heightMapMatrix(hMM(0,0),hMM(0,1),hMM(0,2),hMM(0,3), 
			hMM(1,0),hMM(1,1),hMM(1,2),hMM(1,3), 
			hMM(2,0),hMM(2,1),hMM(2,2),hMM(2,3), 
			hMM(3,0),hMM(3,1),hMM(3,2),hMM(3,3)); 

		unsigned int contextID = m_pViewer->getCamera()->getGraphicsContext()->getState()->getContextID(); 
		_texObj = m_heightMap->getTextureObject(contextID); 

		if(_texObj) 
		{ 
			PassHeightMapToTritonCallback* cb = dynamic_cast<PassHeightMapToTritonCallback*>(_heightCam->getFinalDrawCallback()); 
			if(cb)
			{

			cb->_enable = true; 
			cb->_id = _texObj->id(); 
			cb->_heightMapMatrix = heightMapMatrix; 

			}
		} 

	} 

private: 
	OpenThreads::Mutex         _mutex1;
	Triton::Environment*       _tritonEnvironment; 
	osg::observer_ptr<osgEarth::Map> _map; 
	osgViewer::View* m_pViewer; 
	osg::Camera* _heightCam; 
	osg::Texture2D* m_heightMap; 
	osg::Texture::TextureObject* _texObj; 
	
};  



TritonDrawable::TritonDrawable(osgViewer::View * viewer, TritonContext * TRITON)
	:_TRITON(TRITON)
	,m_pViewer(viewer)
	,m_heightMap(0)
	,m_pHeightCamera(0)
	,m_pTexObj(0)
	,_first(true)
{
	setUseDisplayList(false);
	setUseVertexBufferObjects(false);
	setDataVariance(osg::Object::DYNAMIC);
}


osg::BoundingBox TritonDrawable::computeBound() const
{
	return osg::BoundingBox();
}



void TritonDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
	//osgEarth::Threading::ScopedMutexLock lock1(_mutex1);
	osg::State& state = *renderInfo.getState();

	state.disableAllVertexArrays();

	if (_first == true)
	{
		_first = false;
		_TRITON->Initialize( renderInfo );
		SetupHeightMap();
	}
	
	if ( !_TRITON->ready() )
		return;

	::Triton::Environment* environment = _TRITON->GetEnvironment();

	if ( environment )
	{
		environment->SetCameraMatrix( state.getModelViewMatrix().ptr() );
		environment->SetProjectionMatrix( state.getProjectionMatrix().ptr() );
	}

	state.dirtyAllVertexArrays();

	if ( environment )
	{
		osg::Light* light = renderInfo.getView() ? renderInfo.getView()->getLight() : NULL;

		osg::Matrix lightLocalToWorldMatrix = osg::Matrix::identity();


		if ( light && light->getPosition().w() == 0 )
		{
			osg::Vec4 ambient = light->getAmbient();
			osg::Vec4 diffuse = light->getDiffuse();
			osg::Vec4 position = light->getPosition();

			position = position * lightLocalToWorldMatrix;

			environment->SetDirectionalLight(
				::Triton::Vector3( position[0], position[1], position[2] ),
				::Triton::Vector3( diffuse[0],  diffuse[1],  diffuse[2] ) );

			environment->SetAmbientLight(
				::Triton::Vector3( ambient[0], ambient[1], ambient[2] ) );
			
		}


		if ( _TRITON->GetOcean() )
		{
			_TRITON->GetOcean()->Draw( renderInfo.getView()->getFrameStamp()->getSimulationTime() );
		}
	

	}

	state.dirtyAllVertexArrays();

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



void TritonDrawable::SetupHeightMap() const
{ 
	int textureUnit = 0; 
	int textureSize = 2028; 

	m_heightMap = new osg::Texture2D; 
	m_heightMap->setTextureSize(textureSize, textureSize); 
	m_heightMap->setInternalFormat(GL_LUMINANCE32F_ARB); 
	m_heightMap->setSourceFormat(GL_LUMINANCE); 
	//m_heightMap->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR); 
	//m_heightMap->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR); 


	m_pHeightCamera = new osg::Camera; 
	m_pHeightCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF); 
	m_pHeightCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); 
	m_pHeightCamera->setClearColor(osg::Vec4(-1000.0, -1000.0, -1000.0, 1.0f)); 
	m_pHeightCamera->setViewport(0, 0, textureSize, textureSize); 
	m_pHeightCamera->setRenderOrder(osg::Camera::PRE_RENDER); 
	m_pHeightCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
	m_pHeightCamera->attach(osg::Camera::COLOR_BUFFER, m_heightMap); 
	m_pHeightCamera->setCullMask( 0xff ); 
	m_pHeightCamera->setAllowEventFocus(false); 
	osg::ref_ptr<PassHeightMapToTritonCallback>  callback = new PassHeightMapToTritonCallback(_TRITON->GetEnvironment());
	m_pHeightCamera->setFinalDrawCallback(callback.get());
	
	osg::ref_ptr<osgEarth::VirtualProgram> heightProgram = new osgEarth::VirtualProgram(); 
	heightProgram->setFunction( "setupContour", vertexShader,   osgEarth::ShaderComp::LOCATION_VERTEX_MODEL); 
	heightProgram->setFunction( "colorContour", fragmentShader, osgEarth::ShaderComp::LOCATION_FRAGMENT_COLORING, -1.0 ); 

	osg::ref_ptr<osg::StateSet> stateSet = m_pHeightCamera->getOrCreateStateSet(); 
	stateSet->setAttributeAndModes(heightProgram, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE); 
	stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE); 

	heightProgram->addBindAttribLocation( "osgearth_elevData", osg::Drawable::ATTRIBUTE_6 ); 

	m_pHeightCamera->setViewMatrix(m_pViewer->getCamera()->getViewMatrix()); 
	m_pHeightCamera->setProjectionMatrix(m_pViewer->getCamera()->getProjectionMatrix()); 

	osgEarth::MapNode* mapNode = osgEarth::findTopMostNodeOfType<osgEarth::MapNode>(m_pViewer->getSceneData());
	//mapNode->getOrCreateStateSet()->setAttributeAndModes(heightProgram, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    //mapNode->getTerrainEngine()->getOrCreateStateSet()->setAttributeAndModes(heightProgram, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
	if(mapNode) 
	{ 
		m_pHeightCamera->addChild( mapNode->getTerrainEngine() ); 
		osg::ref_ptr<OceanTerrainChangedCallback> terrainChangedCallback 
			= new OceanTerrainChangedCallback(  
			_TRITON->GetEnvironment(), 
			mapNode->getMap(), 
			m_pViewer, 
			m_pHeightCamera.get(), 
			m_heightMap.get()); 

		mapNode->getTerrain()->addTerrainCallback( terrainChangedCallback ); 
	} 
    //mapNode->getTerrainEngine()->getParent(0)->getParent(0)->addChild(m_pHeightCamera);
	m_pViewer->getSceneData()->asGroup()->addChild(m_pHeightCamera); 
}  