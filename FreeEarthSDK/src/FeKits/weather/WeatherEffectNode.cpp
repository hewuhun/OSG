#include <FeKits/weather/WeatherEffectNode.h>

#include <stdlib.h>

#include<OpenThreads/ScopedLock>
#include<osg/Texture2D>
#include<osg/PointSprite>
#include<osgDB/FileUtils>
#include<osgUtil/CullVisitor>
#include<osgUtil/GLObjectsVisitor>
#include <osg/Notify>
#include <osg/io_utils>
#include <osg/Timer>
#include <osg/ImageUtils>

#include <FeUtils/CoordConverter.h>

namespace FeKit
{

#define USE_LOCAL_SHADERS

	static float random(float min,float max) { return min+(max-min)*(float)rand()/(float)RAND_MAX; }

	CWeatherEffectNode::CWeatherEffectNode()
		:CKitsCtrl(NULL)
		,_previousFrameTime(FLT_MAX)
	{
		setNumChildrenRequiringUpdateTraversal(1);

		setUpGeometries(1024);

		//新增参数
		_bIsWorld = true;
		_vecWeatherPosition = osg::Vec3(0.0, 0.0, FLT_MAX);
		_vec4WeatherRange = osg::Vec4(-180, 180, -90, 90);
		_fWeatherHeight = 6000;

		IsHide() ? Hide() : Show();
	}

	CWeatherEffectNode::CWeatherEffectNode( FeUtil::CRenderContext* pContext)
		:CKitsCtrl(pContext)
		,_previousFrameTime(FLT_MAX)
	{
		setNumChildrenRequiringUpdateTraversal(1);

		setUpGeometries(1024);

		//新增参数
		_bIsWorld = true;
		_vecWeatherPosition = osg::Vec3(0.0, 0.0, FLT_MAX);
		_vec4WeatherRange = osg::Vec4(-180, 180, -90, 90);
		_fWeatherHeight = 6000;

		IsHide() ? Hide() : Show();

	}

	CWeatherEffectNode::CWeatherEffectNode( 
		const CWeatherEffectNode& copy, const osg::CopyOp& copyop /*= osg::CopyOp::SHALLOW_COPY*/ )
		:osg::Node()
		,CKitsCtrl(NULL)
	{
		setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);

		_wind = copy._wind;
		_particleSpeed = copy._particleSpeed;
		_particleSize = copy._particleSize;
		_particleColor = copy._particleColor;
		_maximumParticleDensity = copy._maximumParticleDensity;
		_cellSize = copy._cellSize;
		_nearTransition = copy._nearTransition;
		_farTransition = copy._farTransition;

		//新增参数
		_bIsWorld = copy._bIsWorld;
		_vecWeatherPosition = copy._vecWeatherPosition;
		_vec4WeatherRange = copy._vec4WeatherRange;
		_fWeatherHeight = copy._fWeatherHeight;

		m_opRenderContext = copy.m_opRenderContext;

		_fog = copy._fog.valid() ? dynamic_cast<osg::Fog*>(copy._fog->clone(copyop)) : 0;

		_useFarLineSegments = copy._useFarLineSegments;

		_dirty = true;

		IsHide() ? Hide() : Show();

		update();
	}

	CWeatherEffectNode::~CWeatherEffectNode( void )
	{

	}

	void CWeatherEffectNode::accept( osg::NodeVisitor& nv )
	{
		if (nv.validNodeMask(*this))
		{
			nv.pushOntoNodePath(this); 
			nv.apply(*this);
			nv.popFromNodePath();
		}
	}

	void CWeatherEffectNode::rain( float intensity )
	{
		_wind.set(0.0f,0.0f,0.0f);

		_particleSpeed = -2.0f + -5.0f*intensity;
		//_particleSpeed = osg::Vec3(-2.0f, -2.0f, -2.0f) + osg::Vec3(-5.0, -5.0, -5.0)*intensity;

		_particleSize = 0.01 + 0.02*intensity;
		_particleColor = osg::Vec4(0.6, 0.6, 0.6, 1.0) -  osg::Vec4(0.1, 0.1, 0.1, 1.0)* intensity;
		_maximumParticleDensity = intensity * 8.5f;
		_cellSize.set(5.0f / (0.25f+intensity), 5.0f / (0.25f+intensity), 5.0f);
		//_nearTransition = 25.f;
		_nearTransition = 40.f;
		_farTransition = 100.0f - 60.0f*sqrtf(intensity);

		if (!_fog) _fog = new osg::Fog;
		_fog->setMode(osg::Fog::EXP);
		_fog->setDensity(0.005f*intensity);
		_fog->setColor(osg::Vec4(0.5, 0.5, 0.5, 1.0));

		_useFarLineSegments = true;

		_dirty = true;

		update();
	}

	void CWeatherEffectNode::snow( float intensity )
	{
		_wind.set(0.0f,0.0f,0.0f);

		_particleSpeed = -0.75f - 0.25f*intensity;
		//_particleSpeed = osg::Vec3(-0.75f, -0.75f, -0.75f) + osg::Vec3(-0.25, -0.25, -0.25)*intensity;

		_particleSize = 0.02f + 0.03f*intensity;
		_particleColor = osg::Vec4(0.85f, 0.85f, 0.85f, 1.0f) -  osg::Vec4(0.1f, 0.1f, 0.1f, 1.0f)* intensity;
		_maximumParticleDensity = intensity * 8.2f;
		_cellSize.set(5.0f / (0.25f+intensity), 5.0f / (0.25f+intensity), 5.0f);
		//_nearTransition = 25.f;
		_nearTransition = 40.f;
		_farTransition = 100.0f - 60.0f*sqrtf(intensity);

		if (!_fog) _fog = new osg::Fog;

		_fog->setMode(osg::Fog::EXP);
		_fog->setDensity(0.01f*intensity);
		_fog->setColor(osg::Vec4(0.6, 0.6, 0.6, 1.0));

		_useFarLineSegments = false;

		_dirty = true;

		update();
	}

	void CWeatherEffectNode::update()
	{
		_dirty = false;

		OSG_INFO<<"CPrecipitationEffect::update()"<<std::endl;

		float length_u = _cellSize.x();
		float length_v = _cellSize.y();
		float length_w = _cellSize.z();

		_period = fabsf(_cellSize.x() / _particleSpeed);

		_du.set(length_u, 0.0f, 0.0f);
		_dv.set(0.0f, length_v, 0.0f);
		_dw.set(0.0f, 0.0f, length_w);

		_inverse_du.set(1.0f/length_u, 0.0f, 0.0f);
		_inverse_dv.set(0.0f, 1.0f/length_v, 0.0f);
		_inverse_dw.set(0.0f, 0.0f, 1.0f/length_w);

		OSG_INFO<<"Cell size X="<<length_u<<std::endl;
		OSG_INFO<<"Cell size Y="<<length_v<<std::endl;
		OSG_INFO<<"Cell size Z="<<length_w<<std::endl;

		//新增--计算区域大小,如果mapNode为NULL，则默认将天气范围变为全局的   
		if (!_bIsWorld)	//if (_vecWeatherPosition.z() != FLT_MAX)
		{
			if (m_opRenderContext.valid())
			{
#if 0
				{
					float totalLonLen = m_opRenderContext->GetMapNode()->getMapSRS()->getEllipsoid()->getRadiusEquator();
					float totalLatLen = m_opRenderContext->GetMapNode()->getMapSRS()->getEllipsoid()->getRadiusPolar();

					float rangeLon = 360.0/totalLonLen*_vecWeatherPosition.z();
					float rangeLat = 180.0/totalLatLen*_vecWeatherPosition.z();

					_vec4WeatherRange = osg::Vec4(
						_vecWeatherPosition.x() - rangeLon,
						_vecWeatherPosition.x() + rangeLon,
						_vecWeatherPosition.y() - rangeLat,
						_vecWeatherPosition.y() + rangeLat);
				}
#else
				{
					//*************LGB****************
					osg::Vec3d worldXYZ;
					if (FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),osg::Vec3(_vecWeatherPosition.x(),(double)_vecWeatherPosition.y(), 0.0),worldXYZ))
					{
						float meanPerimeter_2 = osg::PI * worldXYZ.length();
						float meanRange = 360.0 * _vecWeatherPosition.z() /meanPerimeter_2;
						_vec4WeatherRange = osg::Vec4(
							_vecWeatherPosition.x() - meanRange,
							_vecWeatherPosition.x() + meanRange,
							_vecWeatherPosition.y() - meanRange,
							_vecWeatherPosition.y() + meanRange);

						baseHeight_Lgb = worldXYZ.z();
					}
					else
					{
						OSG_INFO<<" MapNode Is NULL! "<<std::endl;
						_bIsWorld = true;
					}
				}
#endif
			}
			else
			{
				OSG_INFO<<" MapNode Is NULL! "<<std::endl;
				_bIsWorld = true;
			}
		}

		{
			OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
			_viewDrawableMap.clear();
		}

		// set up state/
		{
			if (!_stateset)
			{
				_stateset = new osg::StateSet;
				_stateset->addUniform(new osg::Uniform("baseTexture",0));

				_stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
				_stateset->setMode(GL_BLEND, osg::StateAttribute::ON);

				osg::Texture2D* texture = new osg::Texture2D(createSpotLightImage(osg::Vec4(1.0f,1.0f,1.0f,1.0f),osg::Vec4(1.0f,1.0f,1.0f,0.0f),32,1.0));
				//osg::Texture2D* texture = new osg::Texture2D(createSpotLightImage(osg::Vec4(1.0f,1.0f,1.0f,1.0f),osg::Vec4(0.0f,0.0f,0.0f,0.0f),64,0.5));
				_stateset->setTextureAttribute(0, texture);
			}

			if (!_inversePeriodUniform)
			{
				_inversePeriodUniform = new osg::Uniform("inversePeriod",1.0f/_period);
				_stateset->addUniform(_inversePeriodUniform.get());
			}
			else _inversePeriodUniform->set(1.0f/_period);

			if (!_particleColorUniform)
			{
				_particleColorUniform = new osg::Uniform("particleColour", _particleColor);
				_stateset->addUniform(_particleColorUniform.get());
			}
			else _particleColorUniform->set(_particleColor);
			//else _particleColorUniform->set(osg::Vec4(1.0, 0.0, 0.0, 1.0));
			if (!_particleSizeUniform)
			{
				_particleSizeUniform = new osg::Uniform("particleSize", _particleSize);
				_stateset->addUniform(_particleSizeUniform.get());
			}
			else _particleSizeUniform->set(_particleSize);

		}
	}

	void CWeatherEffectNode::compileGLObjects( osg::RenderInfo& renderInfo ) const
	{
		if (_quadGeometry.valid())
		{
			_quadGeometry->compileGLObjects(renderInfo);
			if (_quadGeometry->getStateSet()) _quadGeometry->getStateSet()->compileGLObjects(*renderInfo.getState());
		}

		if (_lineGeometry.valid())
		{
			_lineGeometry->compileGLObjects(renderInfo);
			if (_lineGeometry->getStateSet()) _lineGeometry->getStateSet()->compileGLObjects(*renderInfo.getState());
		}

		if (_pointGeometry.valid())
		{
			_pointGeometry->compileGLObjects(renderInfo);
			if (_pointGeometry->getStateSet()) _pointGeometry->getStateSet()->compileGLObjects(*renderInfo.getState());
		}
	}

	void CWeatherEffectNode::createGeometry( unsigned int numParticles, osg::Geometry* quad_geometry, osg::Geometry* line_geometry, osg::Geometry* point_geometry )
	{
		// particle corner offsets
		osg::Vec2 offset00(0.0f,0.0f);
		osg::Vec2 offset10(1.0f,0.0f);
		osg::Vec2 offset01(0.0f,1.0f);
		osg::Vec2 offset11(1.0f,1.0f);

		osg::Vec2 offset0(0.5f,0.0f);
		osg::Vec2 offset1(0.5f,1.0f);

		osg::Vec2 offset(0.5f,0.5f);


		// configure quad_geometry;
		osg::Vec3Array* quad_vertices = 0;
		osg::Vec2Array* quad_offsets = 0;
		if (quad_geometry)
		{
			quad_geometry->setName("quad");

			quad_vertices = new osg::Vec3Array(numParticles*4);
			quad_offsets = new osg::Vec2Array(numParticles*4);

			quad_geometry->setVertexArray(quad_vertices);
			quad_geometry->setTexCoordArray(0, quad_offsets);
		}

		// configure line_geometry;
		osg::Vec3Array* line_vertices = 0;
		osg::Vec2Array* line_offsets = 0;
		if (line_geometry)
		{
			line_geometry->setName("line");

			line_vertices = new osg::Vec3Array(numParticles*2);
			line_offsets = new osg::Vec2Array(numParticles*2);

			line_geometry->setVertexArray(line_vertices);
			line_geometry->setTexCoordArray(0, line_offsets);
		}

		// configure point_geometry;
		osg::Vec3Array* point_vertices = 0;
		osg::Vec2Array* point_offsets = 0;
		if (point_geometry)
		{
			point_geometry->setName("point");

			point_vertices = new osg::Vec3Array(numParticles);
			point_offsets = new osg::Vec2Array(numParticles);

			point_geometry->setVertexArray(point_vertices);
			point_geometry->setTexCoordArray(0, point_offsets);
		}

		// set up vertex attribute data.
		for(unsigned int i=0; i< numParticles; ++i)
		{
			osg::Vec3 pos( random(0.0f, 1.0f), random(0.0f, 1.0f), random(0.0f, 1.0f));

			// quad particles
			if (quad_vertices)
			{
				(*quad_vertices)[i*4] = pos;
				(*quad_vertices)[i*4+1] = pos;
				(*quad_vertices)[i*4+2] =  pos;
				(*quad_vertices)[i*4+3] =  pos;
				(*quad_offsets)[i*4] = offset00;
				(*quad_offsets)[i*4+1] = offset01;
				(*quad_offsets)[i*4+2] = offset11;
				(*quad_offsets)[i*4+3] = offset10;
			}

			// line particles
			if (line_vertices)
			{
				(*line_vertices)[i*2] = pos;
				(*line_vertices)[i*2+1] = pos;
				(*line_offsets)[i*2] = offset0;
				(*line_offsets)[i*2+1] = offset1;
			}

			// point particles
			if (point_vertices)
			{
				(*point_vertices)[i] = pos;
				(*point_offsets)[i] = offset;
			}
		}
	}

	void CWeatherEffectNode::setUpGeometries( unsigned int numParticles )
	{
		unsigned int quadRenderBin = 13;
		unsigned int lineRenderBin = 12;
		unsigned int pointRenderBin = 11;


		OSG_INFO<<"CPrecipitationEffect::setUpGeometries("<<numParticles<<")"<<std::endl;

		bool needGeometryRebuild = false;

		if (!_quadGeometry || _quadGeometry->getVertexArray()->getNumElements() != 4*numParticles)
		{
			_quadGeometry = new osg::Geometry;
			_quadGeometry->setUseVertexBufferObjects(true);
			needGeometryRebuild = true;
		}

		if (!_lineGeometry || _lineGeometry->getVertexArray()->getNumElements() != 2*numParticles)
		{
			_lineGeometry = new osg::Geometry;
			_lineGeometry->setUseVertexBufferObjects(true);
			needGeometryRebuild = true;
		}

		if (!_pointGeometry || _pointGeometry->getVertexArray()->getNumElements() != numParticles)
		{
			_pointGeometry = new osg::Geometry;
			_pointGeometry->setUseVertexBufferObjects(true);
			needGeometryRebuild = true;
		}

		if (needGeometryRebuild)
		{
			createGeometry(numParticles, _quadGeometry.get(), _lineGeometry.get(), _pointGeometry.get());
		}


		if (!_quadStateSet)
		{
			_quadStateSet = new osg::StateSet;

			osg::Program* program = new osg::Program;
			_quadStateSet->setAttribute(program);
			_quadStateSet->setRenderBinDetails(quadRenderBin,"DepthSortedBin");

#ifdef USE_LOCAL_SHADERS
			char vertexShaderSource[] =
				"uniform float inversePeriod;\n"
				"uniform vec4 particleColour;\n"
				"uniform float particleSize;\n"
				"\n"
				"uniform float osg_SimulationTime;\n"
				"uniform float osg_DeltaSimulationTime;\n"
				"\n"
				"varying vec4 colour;\n"
				"varying vec2 texCoord;\n"
				"\n"
				"void main(void)\n"
				"{\n"
				"    float offset = gl_Vertex.z;\n"
				"    float startTime = gl_MultiTexCoord1.x;\n"
				"    texCoord = gl_MultiTexCoord0.xy;\n"
				"\n"
				"    vec4 v_previous = gl_Vertex;\n"
				"    v_previous.z = fract( (osg_SimulationTime - startTime)*inversePeriod - offset);\n"
				"    \n"
				"    vec4 v_current =  v_previous;\n"
				"    v_current.z += (osg_DeltaSimulationTime*inversePeriod);\n"
				"    \n"
				"\n"
				"    colour = particleColour;\n"
				"    \n"
				"    vec4 v1 = gl_ModelViewMatrix * v_current;\n"
				"    vec4 v2 = gl_TextureMatrix[0] * v_previous;\n"
				"    \n"
				"    vec3 dv = v2.xyz - v1.xyz;\n"
				"    \n"
				"    vec2 dv_normalized = normalize(dv.xy);\n"
				"    dv.xy += dv_normalized * particleSize;\n"
				"    vec2 dp = vec2( -dv_normalized.y, dv_normalized.x ) * particleSize;\n"
				"    \n"
				"    float area = length(dv.xy);\n"
				"    colour.a = 0.05+(particleSize)/area;\n"
				"    \n"
				"\n"
				"    v1.xyz += dv*texCoord.y;\n"
				"    v1.xy += dp*texCoord.x;\n"
				"    \n"
				"    gl_Position = gl_ProjectionMatrix * v1;\n"
				"    gl_ClipVertex = v1;\n"
				"}\n";

			char fragmentShaderSource[] =
				"uniform sampler2D baseTexture;\n"
				"varying vec2 texCoord;\n"
				"varying vec4 colour;\n"
				"\n"
				"void main (void)\n"
				"{\n"
				"    gl_FragColor = colour * texture2D( baseTexture, texCoord);\n"
				"}\n";

			program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSource));
			program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource));
#else
			// get shaders from source
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("quad_rain.vert")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("rain.frag")));
#endif
		}


		if (!_lineStateSet)
		{
			_lineStateSet = new osg::StateSet;

			osg::Program* program = new osg::Program;
			_lineStateSet->setAttribute(program);
			_lineStateSet->setRenderBinDetails(lineRenderBin,"DepthSortedBin");

#ifdef USE_LOCAL_SHADERS
			char vertexShaderSource[] =
				"uniform float inversePeriod;\n"
				"uniform vec4 particleColour;\n"
				"uniform float particleSize;\n"
				"\n"
				"uniform float osg_SimulationTime;\n"
				"uniform float osg_DeltaSimulationTime;\n"
				"uniform mat4 previousModelViewMatrix;\n"
				"\n"
				"varying vec4 colour;\n"
				"varying vec2 texCoord;\n"
				"\n"
				"void main(void)\n"
				"{\n"
				"    float offset = gl_Vertex.z;\n"
				"    float startTime = gl_MultiTexCoord1.x;\n"
				"    texCoord = gl_MultiTexCoord0.xy;\n"
				"\n"
				"    vec4 v_previous = gl_Vertex;\n"
				"    v_previous.z = fract( (osg_SimulationTime - startTime)*inversePeriod - offset);\n"
				"    \n"
				"    vec4 v_current =  v_previous;\n"
				"    v_current.z += (osg_DeltaSimulationTime*inversePeriod);\n"
				"    \n"
				"    colour = particleColour;\n"
				"    \n"
				"    vec4 v1 = gl_ModelViewMatrix * v_current;\n"
				"    vec4 v2 = gl_TextureMatrix[0] * v_previous;\n"
				"    \n"
				"    vec3 dv = v2.xyz - v1.xyz;\n"
				"    \n"
				"    vec2 dv_normalized = normalize(dv.xy);\n"
				"    dv.xy += dv_normalized * particleSize;\n"
				"    \n"
				"    float area = length(dv.xy);\n"
				"    colour.a = (particleSize)/area;\n"
				"    \n"
				"    v1.xyz += dv*texCoord.y;\n"
				"    \n"
				"    gl_Position = gl_ProjectionMatrix * v1;\n"
				"    gl_ClipVertex = v1;\n"
				"}\n";

			char fragmentShaderSource[] =
				"uniform sampler2D baseTexture;\n"
				"varying vec2 texCoord;\n"
				"varying vec4 colour;\n"
				"\n"
				"void main (void)\n"
				"{\n"
				"    gl_FragColor = colour * texture2D( baseTexture, texCoord);\n"
				"}\n";

			program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSource));
			program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource));
#else
			// get shaders from source
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("line_rain.vert")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("rain.frag")));
#endif
		}


		if (!_pointStateSet)
		{
			_pointStateSet = new osg::StateSet;

			osg::Program* program = new osg::Program;
			_pointStateSet->setAttribute(program);

#ifdef USE_LOCAL_SHADERS
			char vertexShaderSource[] =
				"uniform float inversePeriod;\n"
				"uniform vec4 particleColour;\n"
				"uniform float particleSize;\n"
				"\n"
				"uniform float osg_SimulationTime;\n"
				"\n"
				"varying vec4 colour;\n"
				"\n"
				"void main(void)\n"
				"{\n"
				"    float offset = gl_Vertex.z;\n"
				"    float startTime = gl_MultiTexCoord1.x;\n"
				"\n"
				"    vec4 v_current = gl_Vertex;\n"
				"    v_current.z = fract( (osg_SimulationTime - startTime)*inversePeriod - offset);\n"
				"   \n"
				"    colour = particleColour;\n"
				"\n"
				"    gl_Position = gl_ModelViewProjectionMatrix * v_current;\n"
				"\n"
				"    float pointSize = abs(1280.0*particleSize / gl_Position.w);\n"
				"\n"
				"    //gl_PointSize = max(ceil(pointSize),2);\n"
				"    gl_PointSize = ceil(pointSize);\n"
				"    \n"
				"    colour.a = 0.05+(pointSize*pointSize)/(gl_PointSize*gl_PointSize);\n"
				"    gl_ClipVertex = gl_ModelViewMatrix * v_current;\n"
				"}\n";

			char fragmentShaderSource[] =
				"uniform sampler2D baseTexture;\n"
				"varying vec4 colour;\n"
				"\n"
				"void main (void)\n"
				"{\n"
				"    gl_FragColor = colour * texture2D( baseTexture, gl_TexCoord[0].xy);\n"
				"}\n";

			program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSource));
			program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource));
#else
			// get shaders from source
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("point_rain.vert")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("point_rain.frag")));
#endif

			/// Setup the point sprites
			osg::PointSprite *sprite = new osg::PointSprite();
			_pointStateSet->setTextureAttributeAndModes(0, sprite, osg::StateAttribute::ON);

#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
			_pointStateSet->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
#else
			OSG_NOTICE<<"Warning: ParticleEffect::setUpGeometries(..) not fully implemented."<<std::endl;
#endif

			_pointStateSet->setRenderBinDetails(pointRenderBin,"DepthSortedBin");
		}
	}


	void CWeatherEffectNode::traverse( osg::NodeVisitor& nv )
	{
		if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
		{
			if (_dirty) update();

			if (nv.getFrameStamp())
			{
				double currentTime = nv.getFrameStamp()->getSimulationTime();
				if (_previousFrameTime==FLT_MAX) _previousFrameTime = currentTime;

				double delta = currentTime - _previousFrameTime;
				_origin += _wind * delta;
				//_origin = _origin + _wind * delta + _particleSpeed * delta;
				_previousFrameTime = currentTime;                      
			}

			return;
		}

		if (nv.getVisitorType() == osg::NodeVisitor::NODE_VISITOR)
		{
			if (_dirty) update();

			osgUtil::GLObjectsVisitor* globjVisitor = dynamic_cast<osgUtil::GLObjectsVisitor*>(&nv);
			if (globjVisitor)
			{
				if (globjVisitor->getMode() & osgUtil::GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES)
				{
					compileGLObjects(globjVisitor->getRenderInfo());
				}
			}

			return;
		}


		if (nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
		{
			return;
		}

		osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
		if (!cv)
		{
			return;
		}

		ViewIdentifier viewIndentifier(cv, nv.getNodePath());

		{
			SWeatherEffectDrawableSet* pWeatherEffectDrawableSet = 0;

			{
				OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
				pWeatherEffectDrawableSet = &(_viewDrawableMap[viewIndentifier]);

				if (!pWeatherEffectDrawableSet->_quadWeatherDrawable)
				{
					pWeatherEffectDrawableSet->_quadWeatherDrawable = new CWeatherEffectDrawable;
					pWeatherEffectDrawableSet->_quadWeatherDrawable->setRequiresPreviousMatrix(true);
					pWeatherEffectDrawableSet->_quadWeatherDrawable->setGeometry(_quadGeometry.get());
					pWeatherEffectDrawableSet->_quadWeatherDrawable->setStateSet(_quadStateSet.get());
					pWeatherEffectDrawableSet->_quadWeatherDrawable->setDrawType(GL_QUADS);

					pWeatherEffectDrawableSet->_lineWeatherDrawable = new CWeatherEffectDrawable;
					pWeatherEffectDrawableSet->_lineWeatherDrawable->setRequiresPreviousMatrix(true);
					pWeatherEffectDrawableSet->_lineWeatherDrawable->setGeometry(_lineGeometry.get());
					pWeatherEffectDrawableSet->_lineWeatherDrawable->setStateSet(_lineStateSet.get());
					pWeatherEffectDrawableSet->_lineWeatherDrawable->setDrawType(GL_LINES);

					pWeatherEffectDrawableSet->_pointWeatherDrawable = new CWeatherEffectDrawable;
					//pWeatherEffectDrawableSet->_pointWeatherDrawable->setRequiresPreviousMatrix(false);
					pWeatherEffectDrawableSet->_pointWeatherDrawable->setRequiresPreviousMatrix(true);
					pWeatherEffectDrawableSet->_pointWeatherDrawable->setGeometry(_pointGeometry.get());
					pWeatherEffectDrawableSet->_pointWeatherDrawable->setStateSet(_pointStateSet.get());
					pWeatherEffectDrawableSet->_pointWeatherDrawable->setDrawType(GL_POINTS);
				}
			}

			cull(*pWeatherEffectDrawableSet, cv);
			/*if (_bIsWorld)
			{
			cull(*pWeatherEffectDrawableSet, cv);
			}
			else
			BuildAll_Lgb(*pWeatherEffectDrawableSet);*/

			cv->pushStateSet(_stateset.get());
			float depth = 0.0f;

			if (!pWeatherEffectDrawableSet->_quadWeatherDrawable->getCurrentCellMatrixMap().empty())
			{
				cv->pushStateSet(pWeatherEffectDrawableSet->_quadWeatherDrawable->getStateSet());
				cv->addDrawableAndDepth(pWeatherEffectDrawableSet->_quadWeatherDrawable.get(),cv->getModelViewMatrix(),depth);
				cv->popStateSet();
			}

			if (!pWeatherEffectDrawableSet->_lineWeatherDrawable->getCurrentCellMatrixMap().empty())
			{
				cv->pushStateSet(pWeatherEffectDrawableSet->_lineWeatherDrawable->getStateSet());
				cv->addDrawableAndDepth(pWeatherEffectDrawableSet->_lineWeatherDrawable.get(),cv->getModelViewMatrix(),depth);
				cv->popStateSet();
			}

			if (!pWeatherEffectDrawableSet->_pointWeatherDrawable->getCurrentCellMatrixMap().empty())
			{
				cv->pushStateSet(pWeatherEffectDrawableSet->_pointWeatherDrawable->getStateSet());
				cv->addDrawableAndDepth(pWeatherEffectDrawableSet->_pointWeatherDrawable.get(),cv->getModelViewMatrix(),depth);
				cv->popStateSet();
			}

			cv->popStateSet();

		}
	}

	void CWeatherEffectNode::BuildAll_Lgb(SWeatherEffectDrawableSet& pds)
	{
		float cellVolume = 4 * _vecWeatherPosition.z() * _vecWeatherPosition.z() * (_fWeatherHeight - baseHeight_Lgb);
		int numberOfParticles = (int)(_maximumParticleDensity * cellVolume);

		if (numberOfParticles==0) return;

		pds._quadWeatherDrawable->setNumberOfVertices(numberOfParticles*4);
		pds._lineWeatherDrawable->setNumberOfVertices(numberOfParticles*2);
		pds._pointWeatherDrawable->setNumberOfVertices(numberOfParticles);

		pds._quadWeatherDrawable->newFrame();
		pds._lineWeatherDrawable->newFrame();
		pds._pointWeatherDrawable->newFrame();

		osg::Vec3d eyeLocal;
		osg::Vec3d pos(_vecWeatherPosition.x(),_vecWeatherPosition.y(),_fWeatherHeight);
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),pos,eyeLocal);
		float rangeX = abs(_vec4WeatherRange.y() - _vec4WeatherRange.x());
		float rangeY = abs(_vec4WeatherRange.w() - _vec4WeatherRange.z());
		float interval = pow(_maximumParticleDensity, 1/3);

		int i_min = 0;
		int j_min = 0;
		int k_min = 0;

		int i_max = (int)(_vecWeatherPosition.z() * 2);
		int j_max = (int)(_vecWeatherPosition.z() * 2);
		int k_max = (int)(_fWeatherHeight - baseHeight_Lgb);

		float i_delta = rangeX / i_max;
		float j_delta = rangeY / j_max;
		float k_delta = (_fWeatherHeight - baseHeight_Lgb) / k_max;


		for(int i = i_min; i<=i_max; ++i)
		{
			for(int j = j_min; j<=j_max; ++j)
			{
				for(int k = k_min; k<=k_max; ++k)
				{
					osg::Vec3d posv(_vec4WeatherRange.x() + i_delta, _vec4WeatherRange.z() + j_delta, baseHeight_Lgb + k_delta);
					osg::Vec3d result;
					FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), posv, result);
					BuildSub_Lgb(result, pds);
				}
			}
		}
	}

	void CWeatherEffectNode::BuildSub_Lgb(osg::Vec3d& result, SWeatherEffectDrawableSet& pds)
	{

		CWeatherEffectDrawable::DepthMatrixStartTime& mstp = pds._lineWeatherDrawable->getCurrentCellMatrixMap()[CWeatherEffectDrawable::Cell(result.x(),result.y(),result.z())];

	}

	void CWeatherEffectNode::cull( SWeatherEffectDrawableSet& pds, osgUtil::CullVisitor* cv ) const
	{
#ifdef DO_TIMING
		osg::Timer_t startTick = osg::Timer::instance()->tick();
#endif

		float cellVolume = _cellSize.x() * _cellSize.y() * _cellSize.z();
		int numberOfParticles = (int)(_maximumParticleDensity * cellVolume);

		if (numberOfParticles==0) return;

		pds._quadWeatherDrawable->setNumberOfVertices(numberOfParticles*4);
		pds._lineWeatherDrawable->setNumberOfVertices(numberOfParticles*2);
		pds._pointWeatherDrawable->setNumberOfVertices(numberOfParticles);

		pds._quadWeatherDrawable->newFrame();
		pds._lineWeatherDrawable->newFrame();
		pds._pointWeatherDrawable->newFrame();

		osg::Matrix inverse_modelview;
		inverse_modelview.invert(*(cv->getModelViewMatrix()));

		osg::Vec3 eyeLocal = osg::Vec3(0.0f,0.0f,0.0f) * inverse_modelview;
		/*osg::Vec3d eyeLocal;\
		osg::Vec3d pos(_vecWeatherPosition.x(),_vecWeatherPosition.y(),_fWeatherHeight);
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),pos,eyeLocal);*/

		float eye_k = (eyeLocal-_origin)*_inverse_dw;
		osg::Vec3 eye_kPlane = eyeLocal-_dw*eye_k-_origin;

		float eye_i = eye_kPlane*_inverse_du;
		float eye_j = eye_kPlane*_inverse_dv;

		//以上计算代码与下面代码等同
		//osg::Vec3 eyeLocal = cv->getViewPoint();
		//osg::Vec3 eyeFrom = eyeLocal - _origin;
		//float eye_k = eyeFrom.z() * _inverse_dw.z();
		//osg::Vec3 eye_kPlane = osg::Vec3(eyeFrom.x(), eyeFrom.y(), 0.0);
		//float eye_i = eye_kPlane.x() * _inverse_du.x();
		//float eye_j = eye_kPlane.y() * _inverse_dv.y();


		osg::Polytope frustum;
		frustum.setToUnitFrustum(false,false);
		frustum.transformProvidingInverse(*(cv->getProjectionMatrix()));
		frustum.transformProvidingInverse(*(cv->getModelViewMatrix()));

		float i_delta = _nearTransition * _inverse_du.x();
		float j_delta = _nearTransition * _inverse_dv.y();
		//float k_delta = 1;//_nearTransition * _inverse_dw.z();
		float k_delta = _nearTransition * _inverse_dw.z();

		int i_min = (int)floor(eye_i - i_delta);
		int j_min = (int)floor(eye_j - j_delta);
		int k_min = (int)floor(eye_k - k_delta);

		int i_max = (int)ceil(eye_i + i_delta);
		int j_max = (int)ceil(eye_j + j_delta);
		int k_max = (int)ceil(eye_k + k_delta);

		//OSG_NOTICE<<"i_delta="<<i_delta<<" j_delta="<<j_delta<<" k_delta="<<k_delta<<std::endl;

		unsigned int numTested=0;
		unsigned int numInFrustum=0;

		float iCyle = 0.43;
		float jCyle = 0.64;

		for(int i = i_min; i<=i_max; ++i)
		{
			for(int j = j_min; j<=j_max; ++j)
			{
				for(int k = k_min; k<=k_max; ++k)
				{
					float startTime = (float)(i)*iCyle + (float)(j)*jCyle;
					startTime = (startTime-floor(startTime))*_period;

					if (build(eyeLocal, i,j,k, startTime, pds, frustum, cv)) ++numInFrustum;
					++numTested;
				}
			}
		}


#ifdef DO_TIMING
		osg::Timer_t endTick = osg::Timer::instance()->tick();

		OSG_NOTICE<<"time for cull "<<osg::Timer::instance()->delta_m(startTick,endTick)<<"ms  numTested="<<numTested<<" numInFrustum"<<numInFrustum<<std::endl;
		OSG_NOTICE<<"     quads "<<pds._quadWeatherDrawable->getCurrentCellMatrixMap().size()<<"   lines "<<pds._lineWeatherDrawable->getCurrentCellMatrixMap().size()<<"   points "<<pds._pointWeatherDrawable->getCurrentCellMatrixMap().size()<<std::endl;
#endif
	}

	bool CWeatherEffectNode::build( const osg::Vec3 eyeLocal, int i, int j, int k, float startTime, SWeatherEffectDrawableSet& pds, osg::Polytope& frustum, osgUtil::CullVisitor* cv ) const
	{
		//该处设置主要是为了将下雨的起始位置固定为设置的高度和局部雨雪的位置
		osg::Vec3 position = _origin + osg::Vec3(float(i)*_du.x(), float(j)*_dv.y(), float(k)*_dw.z());

		double lon=180, lat=90 ,hei=0;
		if (m_opRenderContext.valid())
		{
			osg::Vec3d vecLL;
			FeUtil::XYZ2RadianLLH(m_opRenderContext.get(), position, vecLL);

			lon = osg::RadiansToDegrees(vecLL.x());
			lat = osg::RadiansToDegrees(vecLL.y());
			hei = vecLL.z();
		}
		//如果不是全局天气，则另行处理
		if (_bIsWorld == false) //Lgb
		{        
			if (!(lon < _vec4WeatherRange.y() && lon > _vec4WeatherRange.x()
				&& lat > _vec4WeatherRange.z() && lat < _vec4WeatherRange.w()))
			{
				return false;
			}
		}

		//判断该高度是否允许产生天气效果 
		if (hei > _fWeatherHeight) //Lgb
		{
			return false;
		}

		osg::Vec3 scale(_du.x(), _dv.y(), _dw.z());

		osg::BoundingBox bb(position.x(), position.y(), position.z()-scale.z(),
			position.x()+scale.x(), position.y()+scale.y(), position.z());

		//if (!frustum.contains(bb)) return false; //Lgb

		osg::Vec3 center = position + scale*0.5f;
		float distance = (center-eyeLocal).length();

		osg::Matrix* mymodelview = 0;

#if 1
		{
			CWeatherEffectDrawable::DepthMatrixStartTime& mstp = pds._quadWeatherDrawable->getCurrentCellMatrixMap()[CWeatherEffectDrawable::Cell(i,k,j)];
			mstp.depth = 0;//distance;
			mstp.startTime = startTime;
			mymodelview = &mstp.modelview;
		}
#else
		{
			if (distance < _nearTransition)
			{
				CWeatherEffectDrawable::DepthMatrixStartTime& mstp = pds._quadWeatherDrawable->getCurrentCellMatrixMap()[CWeatherEffectDrawable::Cell(i,k,j)];
				mstp.depth = distance;
				mstp.startTime = startTime;
				mymodelview = &mstp.modelview;
			}
			else if (distance <= _farTransition)
			{
				if (_useFarLineSegments)
				{
					CWeatherEffectDrawable::DepthMatrixStartTime& mstp = pds._lineWeatherDrawable->getCurrentCellMatrixMap()[CWeatherEffectDrawable::Cell(i,k,j)];
					mstp.depth = distance;
					mstp.startTime = startTime;
					mymodelview = &mstp.modelview;
				}
				else
				{
					CWeatherEffectDrawable::DepthMatrixStartTime& mstp = pds._pointWeatherDrawable->getCurrentCellMatrixMap()[CWeatherEffectDrawable::Cell(i,k,j)];
					mstp.depth = distance;
					mstp.startTime = startTime;
					mymodelview = &mstp.modelview;
				}
			}
			else
			{
				return false;
			}
		}
#endif

		osg::Matrix mtxRotate;
		mtxRotate = osg::Matrix::rotate(osg::Vec3(0.0, 0.0, 1.0), -eyeLocal);
		osg::Quat quat = mtxRotate.getRotate();

		*mymodelview = *(cv->getModelViewMatrix());
		mymodelview->preMultTranslate(position);
		mymodelview->preMultScale(scale);  
		mymodelview->preMultRotate(quat);

		//cv->updateCalculatedNearFar(*(cv->getModelViewMatrix()),bb);

		return true;
	}

	void CWeatherEffectNode::SetWeatherType( EWeatherType type )
	{
		switch(type)
		{
		case LIGHT_RIAN:
			rain(0.3);
			break;
		case HEAVY_RAIN:
			rain(0.5);
			break;
		case LIGHT_SNOW:
			snow(0.3);
			break;
		case HEAVY_SNOW:
			snow(0.5);
			break;
		case NO_WEATHER:
			rain(0.0);
			snow(0.0);
			break;
		default:
			rain(0.0);
			snow(0.0);
			break;
		}
	}

	void CWeatherEffectNode::Show()
	{
		setNodeMask(~0x00000000);
		m_bShow = true;
	}

	void CWeatherEffectNode::Hide()
	{
		setNodeMask(0x00000000);
		m_bShow = false;
	}

	bool CWeatherEffectNode::IsHide() const
	{
		return !m_bShow;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//   Precipitation Drawable
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////

	CWeatherEffectDrawable::CWeatherEffectDrawable():
	_requiresPreviousMatrix(true),
		_drawType(GL_QUADS),
		_numberOfVertices(0)
	{
		setSupportsDisplayList(false);
	}

	CWeatherEffectDrawable::CWeatherEffectDrawable( 
		const CWeatherEffectDrawable& copy, const osg::CopyOp& copyop):
	osg::Drawable(copy,copyop),
		_requiresPreviousMatrix(copy._requiresPreviousMatrix),
		_geometry(copy._geometry),
		_drawType(copy._drawType),
		_numberOfVertices(copy._numberOfVertices)
	{
	}

	void FeKit::CWeatherEffectDrawable::drawImplementation( osg::RenderInfo& renderInfo ) const
	{
#if defined(OSG_GL_MATRICES_AVAILABLE)

		if (!_geometry) return;

		const osg::GLExtensions * extensions = osg::GLExtensions::Get(renderInfo.getContextID(), true);
		//const osg::Geometry::Extensions* extensions = osg::Geometry::getExtensions(renderInfo.getContextID(),true);

		// save OpenGL matrices
		glPushMatrix();

		if (_requiresPreviousMatrix)
		{
			renderInfo.getState()->setActiveTextureUnit(0);
			glMatrixMode( GL_TEXTURE );
			glPushMatrix();
		}

		typedef std::vector<const CellMatrixMap::value_type*> DepthMatrixStartTimeVector;
		DepthMatrixStartTimeVector orderedEntries;
		orderedEntries.reserve(_currentCellMatrixMap.size());

		for(CellMatrixMap::const_iterator citr = _currentCellMatrixMap.begin();
			citr != _currentCellMatrixMap.end();
			++citr)
		{
			orderedEntries.push_back(&(*citr));
		}

		std::sort(orderedEntries.begin(),orderedEntries.end(),LessFunctor());

		for(DepthMatrixStartTimeVector::reverse_iterator itr = orderedEntries.rbegin();
			itr != orderedEntries.rend();
			++itr)
		{
			extensions->glMultiTexCoord1f(GL_TEXTURE0+1, (*itr)->second.startTime);

			// load cells current modelview matrix
			if (_requiresPreviousMatrix)
			{
				glMatrixMode( GL_MODELVIEW );
				glLoadMatrix((*itr)->second.modelview.ptr());

				CellMatrixMap::const_iterator pitr = _previousCellMatrixMap.find((*itr)->first);
				if (pitr != _previousCellMatrixMap.end())
				{
					// load previous frame modelview matrix for motion blurr effect
					glMatrixMode( GL_TEXTURE );
					glLoadMatrix(pitr->second.modelview.ptr());
				}
				else
				{
					// use current modelview matrix as "previous" frame value, cancelling motion blurr effect
					glMatrixMode( GL_TEXTURE );
					glLoadMatrix((*itr)->second.modelview.ptr());
				}
			}
			else
			{
				glLoadMatrix((*itr)->second.modelview.ptr());
			}

			_geometry->draw(renderInfo);

			unsigned int numVertices = osg::minimum(_geometry->getVertexArray()->getNumElements(), _numberOfVertices);
			glDrawArrays(_drawType, 0, numVertices);
		}

		// restore OpenGL matrices
		if (_requiresPreviousMatrix)
		{
			glPopMatrix();
			glMatrixMode( GL_MODELVIEW );
		}

		glPopMatrix();
#else
		OSG_NOTICE<<"Warning: ParticleEffect::drawImplementation(..) not fully implemented."<<std::endl;
#endif
	}

}
