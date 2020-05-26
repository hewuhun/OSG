#include <osg/Texture2D>
#include <FeUtils/PathRegistry.h>
#include <osgViewer/Renderer>
#include <osg/CullFace>

#include <FeMeasure/ViewAnalyseNode.h>

namespace FeMeasure
{
	static const char *ViewAnalyseVertSource = 
	{
		"varying vec4 v_fPos;\n"
		"varying vec4 v_vPos;\n"
		"varying vec3 v_normal;\n"
		"uniform mat4 u_mvpMat;\n"
		"void main(void)\n"
		"{\n"
			"gl_FrontColor = gl_Color;\n"
			"gl_TexCoord[0] = gl_MultiTexCoord0;\n"
			"gl_Position = u_mvpMat * gl_Vertex;\n"
			"v_fPos = gl_Position;\n"
			"v_vPos = gl_Vertex;\n"
			"v_normal = gl_Normal;\n"
		"}\n"
	};

	static const char *ViewAnalyseFragSource = 
	{
		"uniform bool u_hasTex;\n"
		"uniform sampler2D u_tex0;\n"
		"uniform sampler2D u_depthTex;\n"
		"uniform bool u_isDepth;\n"
		"varying vec4 v_fPos;\n"
		"varying vec4 v_vPos;\n"
		"varying vec3 v_normal;\n"
		"uniform mat4 u_texMat;\n"
		"uniform mat4 u_texMVMat;\n"
		"void main(void)\n" 
		"{\n"
			"if(u_isDepth)\n"
			"{\n"
			"    float z = v_fPos.z / v_fPos.w;\n"
			"    gl_FragColor = z * 0.5 + 0.5;\n"
			"}\n"
			"else\n"
			"{\n"
				" vec4 modelColor = gl_Color;\n"
				" if(u_hasTex)\n"
				" {\n"
				"    modelColor = texture2D(u_tex0,gl_TexCoord[0].st);\n"
				" }\n"
				" vec4 depthPos = u_texMat * v_vPos;\n"
				" float vx = depthPos.x / depthPos.w;\n"
				" float vy = depthPos.y / depthPos.w;\n"
				" float vz = depthPos.z / depthPos.w;\n"
				" vec4 viewPos = u_texMVMat * v_vPos;\n"
				" float curDepth = vz;\n"
				" float texDepth = texture2D(u_depthTex,vec2(vx,vy)).x;\n"
				" vec4 depthColor = vec4(0.0,1.0,0.0,1.0);\n"
				" v_normal = (mat3)(u_texMVMat) * v_normal;\n"
				" vec3 viewDir = -normalize(viewPos).xyz;\n"
				" float dotNL = dot(viewDir,v_normal);\n"
				" if(dotNL > 0)\n"
				" {\n"
					" float cosTheta = clamp(dotNL,0,1);\n"
					" float bias = 0.005 * tan(acos(cosTheta)); \n"
					" bias = clamp(bias, 0,0.01);\n"
					" if(curDepth > texDepth + 0)\n"
					" {\n"
					"    depthColor = vec4(1.0,0.0,0.0,1.0);\n"
					" }\n"
				" }\n"	
				" else\n"
				" {\n"
					 "depthColor = vec4(1.0,0.0,0.0,1.0);\n"
				" }\n"

				" gl_FragColor = modelColor * 0.5 + depthColor * 0.5;\n"

				" if(vx <= 0.0 || vx >= 1.0 || vy <= 0.0 || vy >= 1.0 || vz <= 0.0 || vz >= 1.0)\n"
				" {\n"
				"   gl_FragColor = modelColor;\n"
				" }\n"
				//" gl_FragColor = cosTheta;\n"
			"}\n"
		"}\n"
	};

	static const char *DepthVertSource = 
	{
		"uniform mat4 u_depthMVPMat;\n"
		"uniform mat4 osg_ModelViewProjectionMatrix;\n"
		"varying vec2 v_zw;\n" 
		"void main(void)\n"
		"{\n"
		"gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		"v_zw.x = gl_Position.z;\n"
		"v_zw.y = gl_Position.w;\n"
		"}\n"
	};

	static const char *DepthFragSource = 
	{	
		"varying vec2 v_zw;\n" 
		"void main(void)\n"    
		"{\n"
		  " float depth = v_zw.x / v_zw.y;\n"     
		  " depth = depth * 0.5 + 0.5;\n"  
		  " gl_FragColor = depth;\n"
		"}\n"
	};

	EndDepthCallback::EndDepthCallback( ViewAnalyseNode* vn )
		:m_opViewAnalyseNode(vn)
	{
	}

	void EndDepthCallback::operator()( osg::RenderInfo& renderInfo ) const
	{
		if(m_opViewAnalyseNode.valid())
			m_opViewAnalyseNode->m_isDepth = false;
	}

	BeginDepthCallback::BeginDepthCallback( ViewAnalyseNode* vn )
		:m_opViewAnalyseNode(vn)
	{
	}

	void BeginDepthCallback::operator()( osg::RenderInfo& renderInfo ) const
	{
		if(m_opViewAnalyseNode.valid())
			m_opViewAnalyseNode->m_isDepth = true;
	}

	ViewAnalyseNodeVisitor::ViewAnalyseNodeVisitor( ViewAnalyseNode* vn )
		:m_opViewAnalyseNode(vn)
	{
		setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
	}

	void ViewAnalyseNodeVisitor::apply( osg::Drawable& drawable )
	{
		drawable.setUseDisplayList(false);
		drawable.setUseVertexBufferObjects(true);

		ViewAnalyseDrawCallback* cb = new ViewAnalyseDrawCallback(m_opViewAnalyseNode.get());
		drawable.setDrawCallback(cb);
	}

	ViewAnalyseNode::ViewAnalyseNode(FeUtil::CRenderContext* rc)
		:m_rpNodeVisitor(NULL)
		,m_opRenderContext(rc)
		,m_isDepth(false)
		,m_nearClip(10.0)
		,m_farClip(2000.0)
		,m_fov(30.0)
		,m_aspect(1.0)
		,m_rpUpdateCallback(NULL)
		,m_rpCamGeometry(NULL)
		,m_frustumLineColor(1.0,1.0,0.0,1.0)
		,m_bInit(false)
	{
		m_rpNodeVisitor = new ViewAnalyseNodeVisitor(this);

		m_rpUpdateCallback = new ViewAnalyseUpdateCallback();
		addUpdateCallback(m_rpUpdateCallback);
	}

	ViewAnalyseNode::~ViewAnalyseNode()
	{
	}

	void ViewAnalyseNode::init()
	{
		if(m_bInit)
		{
			return;
		}
		m_bInit = true;

		initDepthCamera();
		this->accept(*m_rpNodeVisitor);

		//addHud();
	}

	void ViewAnalyseNode::initDepthCamera()
	{		
		int textureSize = 1024 * 2; 

		m_rpDepthMap = new osg::Texture2D; 
		m_rpDepthMap->setTextureSize(textureSize, textureSize); 
		m_rpDepthMap->setInternalFormat(GL_LUMINANCE32F_ARB); 
		m_rpDepthMap->setSourceFormat(GL_LUMINANCE); 
		m_rpDepthMap->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
		m_rpDepthMap->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
		m_rpDepthMap->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_EDGE);
		m_rpDepthMap->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_EDGE);

		m_rpDepthCamera = new osg::Camera(); 
		m_rpDepthCamera->setName("depth_cam");
		m_rpDepthCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT); 
		m_rpDepthCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); 
		float defaultColor = m_farClip + 1000;
		m_rpDepthCamera->setClearColor(osg::Vec4(defaultColor, defaultColor, defaultColor, 1.0f)); 
		m_rpDepthCamera->setViewport(0, 0, textureSize, textureSize); 
		m_rpDepthCamera->setRenderOrder(osg::Camera::PRE_RENDER); 
		m_rpDepthCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
		m_rpDepthCamera->attach(osg::Camera::COLOR_BUFFER, m_rpDepthMap); 
		m_rpDepthCamera->setAllowEventFocus(false);
		m_rpDepthCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
		m_rpDepthCamera->setInheritanceMask(osg::CullSettings::NO_VARIABLES);

		osg::Program* prog = new osg::Program();
		prog->addShader(new osg::Shader(osg::Shader::VERTEX,DepthVertSource));
		prog->addShader(new osg::Shader(osg::Shader::FRAGMENT,DepthFragSource));

		osg::StateSet* ss = m_rpDepthCamera->getOrCreateStateSet();
		ss->setAttributeAndModes(prog,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		ss->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT),osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

		BeginDepthCallback* bcb = new BeginDepthCallback(this);
		m_rpDepthCamera->setInitialDrawCallback(bcb);

		EndDepthCallback* ecb = new EndDepthCallback(this);
		m_rpDepthCamera->setFinalDrawCallback(ecb);

		m_rpDepthCameraCullCallback = new DepthCameraCullCallback(this);
		m_rpDepthCamera->setCullCallback(m_rpDepthCameraCullCallback);

		int numChild = getNumChildren();
		if(numChild > 0)
		{
			for (int i = 0;i < numChild;i++)
			{
				m_rpDepthCamera->addChild(getChild(i));
			}
		}
		this->insertChild(0,m_rpDepthCamera);
	}

	void ViewAnalyseNode::updateDepthCamera()
	{
		if(m_rpDepthCamera.valid())
		{
			osg::Vec3d lookDir = m_lookPos - m_eyePos;
			lookDir.normalize();
			osg::Vec3d upDir = m_eyePos;
			upDir.normalize();

			/*float dotUp = lookDir * upDir;
			if(osg::equivalent(std::abs(dotUp),1.0f))
			{
			upDir = osg::Vec3d(0,1,0);
			}*/

			osg::Matrix viewMat = osg::Matrix::lookAt(m_eyePos,m_lookPos,upDir);
			osg::Matrix projMat = osg::Matrix::perspective(m_fov,m_aspect,m_nearClip,m_farClip);

			m_rpDepthCamera->setViewMatrix(viewMat);
			m_rpDepthCamera->setProjectionMatrix(projMat);
		}

		if(m_rpDepthCamera.valid())
		{
			if(m_rpCamGeometry.valid())
			{
				this->removeChild(m_rpCamGeometry);
				m_rpCamGeometry = NULL;
			}

			osg::Geometry* gm = createCameraGeometry();

			osg::Geode* gn = new osg::Geode();
			gn->addDrawable(gm);

			osg::MatrixTransform* mt = new osg::MatrixTransform();
			mt->addChild(gn);

			m_rpCamGeometry = new osg::Group();
			m_rpCamGeometry->addChild(mt);
			this->addChild(m_rpCamGeometry);
		}
		
		if(m_rpCamGeometry.valid())
		{
			if(m_rpCamGeometry->getNumChildren() > 0)
			{
				osg::MatrixTransform* mt = dynamic_cast<osg::MatrixTransform*>(m_rpCamGeometry->getChild(0));
				if(mt)
				{
					osg::Matrix sm;
					osg::Matrix viewMat = m_rpDepthCamera->getViewMatrix();
					sm(0,0) = viewMat(0,0);
					sm(1,0) = viewMat(0,1);
					sm(2,0) = viewMat(0,2);
					sm(0,1) = viewMat(1,0);
					sm(1,1) = viewMat(1,1);
					sm(2,1) = viewMat(1,2);
					sm(0,2) = viewMat(2,0);
					sm(1,2) = viewMat(2,1);
					sm(2,2) = viewMat(2,2);

					osg::Matrix tm;
					tm.makeTranslate(m_eyePos);

					mt->setMatrix(sm * tm);	
				}
			}
		}
	}

	void ViewAnalyseNode::addHud()
	{
		float w = 800;
		float h = 600;

		m_rpHud = new osg::Camera();
		m_rpHud->setProjectionMatrix(osg::Matrix::ortho2D(0,w,0,h));
		m_rpHud->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		m_rpHud->setViewMatrix(osg::Matrix::identity());
		m_rpHud->setClearMask(GL_DEPTH_BUFFER_BIT);
		m_rpHud->setRenderOrder(osg::Camera::POST_RENDER);
		m_rpHud->setAllowEventFocus(false);

		osg::Geometry* gm = new osg::Geometry();

		float sc = 3.0;
		osg::Vec3Array* va = new osg::Vec3Array();
		va->push_back(osg::Vec3(0,0,0));
		va->push_back(osg::Vec3(w / sc,0,0));
		va->push_back(osg::Vec3(w / sc,h / sc,0));
		va->push_back(osg::Vec3(0,h / sc,0));

		osg::Vec2Array* ta = new osg::Vec2Array();
		ta->push_back(osg::Vec2(0,0));
		ta->push_back(osg::Vec2(1,0));
		ta->push_back(osg::Vec2(1,1));
		ta->push_back(osg::Vec2(0,1));

		osg::DrawElementsUByte* ia = new osg::DrawElementsUByte(osg::PrimitiveSet::TRIANGLES);
		ia->push_back(0);
		ia->push_back(1);
		ia->push_back(2);
		ia->push_back(0);
		ia->push_back(2);
		ia->push_back(3);

		gm->setVertexArray(va);
		gm->addPrimitiveSet(ia);
		gm->setTexCoordArray(0,ta);

		/*osg::Image* img = osgDB::readImageFile(FeFileReg->GetFullPath("texture/logo/logo.png"));
		if(img)
		{
		osg::Texture2D* tx = new osg::Texture2D();
		tx->setImage(img);
		gm->getOrCreateStateSet()->setTextureAttributeAndModes(0,tx);
		}*/
		gm->getOrCreateStateSet()->setTextureAttributeAndModes(0,m_rpDepthMap);

		gm->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

		osg::Geode* gn = new osg::Geode();
		gn->addDrawable(gm);

		m_rpHud->addChild(gn);

		if(m_opRenderContext.valid())
			m_opRenderContext->GetRoot()->addChild(m_rpHud);
	}

	osg::Geometry* ViewAnalyseNode::createCameraGeometry()
	{
		osg::Geometry* gm = new osg::Geometry();

		osg::Vec3Array* va = new osg::Vec3Array();
		va->push_back(osg::Vec3(0,0,0));

		float upToCenter = m_farClip * std::tan(osg::DegreesToRadians(m_fov / 2.0));
		float leftToCenter = upToCenter * m_aspect;

		osg::Vec3d zDir(0,0,-1);
		osg::Vec3d yDir(0,1,0);
		osg::Vec3d xDir(1,0,0);

		osg::Vec3 leftUp = zDir * m_farClip - xDir * leftToCenter + yDir * upToCenter;
		osg::Vec3 rightUp = leftUp + xDir * leftToCenter * 2;
		osg::Vec3 rightDown = rightUp - yDir * upToCenter * 2;
		osg::Vec3 leftDown = rightDown - xDir * leftToCenter * 2;

		va->push_back(leftUp);
		va->push_back(rightUp);
		va->push_back(rightDown);
		va->push_back(leftDown);

		osg::Vec4Array* ca = new osg::Vec4Array();
		ca->push_back(m_frustumLineColor);

		osg::DrawElementsUShort* ia = new osg::DrawElementsUShort(GL_LINES);
		ia->push_back(0);
		ia->push_back(1);
		ia->push_back(0);
		ia->push_back(2);
		ia->push_back(0);
		ia->push_back(3);
		ia->push_back(0);
		ia->push_back(4);
		ia->push_back(1);
		ia->push_back(2);
		ia->push_back(2);
		ia->push_back(3);
		ia->push_back(3);
		ia->push_back(4);
		ia->push_back(4);
		ia->push_back(1);

		gm->setVertexArray(va);
		gm->setColorArray(ca,osg::Array::BIND_OVERALL);
		gm->addPrimitiveSet(ia);

		gm->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

		return gm;
	}

	void ViewAnalyseNode::setEyePos( const osg::Vec3d& pos )
	{
		m_eyePos = pos;
		updateDepthCamera();
	}

	void ViewAnalyseNode::setLookPos( const osg::Vec3d& pos )
	{
		m_lookPos = pos;
		updateDepthCamera();
	}

	void ViewAnalyseNode::setNearClip( const float& fVal )
	{
		m_nearClip = fVal;
		updateDepthCamera();
	}

	void ViewAnalyseNode::setFarClip( const float& fVal )
	{
		m_farClip = fVal;
		updateDepthCamera();
	}

	void ViewAnalyseNode::setFov( const float& fVal )
	{
		m_fov = fVal;
		updateDepthCamera();
	}

	void ViewAnalyseNode::setFovAspect( const float& fVal )
	{
		m_aspect = fVal;
		updateDepthCamera();
	}

	void ViewAnalyseNode::setFrustumLineColor( const osg::Vec4& vecColor )
	{
		m_frustumLineColor = vecColor;
		updateDepthCamera();
	}

	void ViewAnalyseUpdateCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		ViewAnalyseNode* vn = dynamic_cast<ViewAnalyseNode*>(node);
		if(vn && vn->m_bInit)
		{
			//测试旋转代码
			osg::Vec3d upDir = vn->m_eyePos;
			upDir.normalize();
			static float ra = 0.0;
			static osg::Vec3d lookPos = vn->m_lookPos;
			ra += 0.1;
            ra = std::fmod(ra,360);
			osg::Matrix rm = osg::Matrix::rotate(osg::DegreesToRadians(ra),upDir);
			osg::Matrix tm0;
			tm0.makeTranslate(-vn->m_eyePos);
			osg::Matrix tm1;
			tm1.makeTranslate(vn->m_eyePos);
			osg::Matrix m = tm0 * rm * tm1;
			vn->m_lookPos = lookPos * m;
			//测试旋转代码

			vn->updateDepthCamera();
		}
	}

	DepthCameraCullCallback::DepthCameraCullCallback( ViewAnalyseNode* vn )
		:m_opViewAnalyseNode(vn)
	{
	}

	void DepthCameraCullCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		if(m_opViewAnalyseNode.valid())
		{
			if(nv->getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
			{
				m_opViewAnalyseNode->m_isDepth = true;
				traverse(node,nv);
				m_opViewAnalyseNode->m_isDepth = false;
			}
			else
			{
				traverse(node,nv);
			}
		}
	}

	ViewAnalyseDrawCallback::ViewAnalyseDrawCallback( ViewAnalyseNode* vn )
		:m_opViewAnalyseNode(vn)
		,m_opGLExtensions(NULL)
		,m_vertShader(0)
		,m_fragShader(0)
		,m_programCast(0)
		,m_shaderInit(false)
		,m_hasTex(-1)
		,m_isTexUniformSet(false)
		,m_mvpMat(-1)
		,m_isDepth(-1)
		,m_texMat(-1)
		,m_tex0(-1)
		,m_depthTex(-1)
		,m_texMVMat(-1)
	{
	}

	unsigned int ViewAnalyseDrawCallback::loadShader( bool vertexShader ) const
	{
		if(m_opGLExtensions.valid())
		{
			char *data = 0;

			unsigned int shader = 0;

			shader = m_opGLExtensions->glCreateShader(vertexShader ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

			const char* sources[1];
			if(vertexShader)
				sources[0] = ViewAnalyseVertSource;
			else
				sources[0] = ViewAnalyseFragSource;

			m_opGLExtensions->glShaderSource(shader, 1, sources, NULL);

			m_opGLExtensions->glCompileShader(shader);

			GLint ok;
			m_opGLExtensions->glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
			if (!ok)
			{
				return 0;
			}
			else
				return shader;
		}

		return 0;
	}

	bool ViewAnalyseDrawCallback::loadShaders(unsigned int id) const
	{
		m_opGLExtensions = osg::GLExtensions::Get(id,false);

		if(m_opGLExtensions.valid())
		{
			m_vertShader = loadShader(true);
			m_fragShader = loadShader(false);

			if (m_vertShader && m_fragShader)
			{
				m_programCast = m_opGLExtensions->glCreateProgram();

				m_opGLExtensions->glAttachShader(m_programCast, m_vertShader);
				m_opGLExtensions->glAttachShader(m_programCast, m_fragShader);

				m_opGLExtensions->glLinkProgram(m_programCast);

				GLint linkStatus;
				m_opGLExtensions->glGetProgramiv(m_programCast, GL_LINK_STATUS, &linkStatus);
				if (linkStatus == GL_FALSE)
				{
					return false;
				}
				else
				{
					initUniforms();
					return true;
				}

			}
		}

		return false;
	}

	void ViewAnalyseDrawCallback::pushAllState() const
	{
		GLint currentProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
		m_programStack.push(currentProgram);
	}

	void ViewAnalyseDrawCallback::popAllState() const
	{
		if (!m_programStack.empty())
		{
			GLint savedProgram = m_programStack.top();
			m_opGLExtensions->glUseProgram(savedProgram);
			m_programStack.pop();
		}
	}

	void ViewAnalyseDrawCallback::initUniforms() const
	{
		if(m_programCast && m_opGLExtensions.valid())
		{
			m_hasTex = m_opGLExtensions->glGetUniformLocation(m_programCast, "u_hasTex");
			m_mvpMat = m_opGLExtensions->glGetUniformLocation(m_programCast, "u_mvpMat");
			m_isDepth = m_opGLExtensions->glGetUniformLocation(m_programCast, "u_isDepth");
			m_texMat = m_opGLExtensions->glGetUniformLocation(m_programCast, "u_texMat");
			m_tex0 = m_opGLExtensions->glGetUniformLocation(m_programCast, "u_tex0");
			m_depthTex = m_opGLExtensions->glGetUniformLocation(m_programCast, "u_depthTex");
			m_texMVMat = m_opGLExtensions->glGetUniformLocation(m_programCast, "u_texMVMat");
		}
	}

	void ViewAnalyseDrawCallback::updateUniforms(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const
	{
		if(m_opGLExtensions.valid() && m_programCast)
		{
			osg::Drawable* dr = const_cast<osg::Drawable*>(drawable);
			if(dr)
			{
				osg::Geometry* gm = dynamic_cast<osg::Geometry*>(dr);
				if(gm)
				{
					if(!m_isTexUniformSet)
					{
						if(m_hasTex != -1)
						{
							int flag = 1;
							if(gm->getTexCoordArrayList().empty())
								flag = 0;
							m_opGLExtensions->glUniform1i(m_hasTex,flag);
						}

						if(m_tex0 != -1)
						{
							m_opGLExtensions->glUniform1i(m_tex0,0);
						}

						int textUnit = 1;
						gm->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit,m_opViewAnalyseNode->m_rpDepthMap,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
						if(m_depthTex != -1)
						{
							m_opGLExtensions->glUniform1i(m_depthTex,textUnit);
						}

						m_isTexUniformSet = true;
					}	

					if(m_mvpMat != -1)
					{
						osg::Matrix mvpMat;
						osg::State* st = renderInfo.getState();
						if(st)
						{
							mvpMat = st->getModelViewMatrix() * st->getProjectionMatrix();
						}
					
						float m[16];
						int i = 0;
						for (int row = 0; row < 4; row++) 
						{
							for (int col = 0; col < 4; col++)
							{
								m[i++] = (float)(mvpMat(row,col));
							}
						}

						m_opGLExtensions->glUniformMatrix4fv(m_mvpMat,1,GL_FALSE,m);
					}

					if(m_isDepth != -1)
					{
						int flag = 0;
						if(m_opViewAnalyseNode.valid())
						{
							if(m_opViewAnalyseNode->m_isDepth)
								flag = 1;
						}

						m_opGLExtensions->glUniform1i(m_isDepth,flag);
					}

					if(m_texMVMat != -1)
					{
						osg::Matrix mvMat;
						osg::Matrix worldMat;
						osg::State* st = renderInfo.getState();
						if(st)
						{
							worldMat = st->getWorldMatrix();
						}
						if(m_opViewAnalyseNode->m_rpDepthCamera.valid())
						{
							mvMat = worldMat * m_opViewAnalyseNode->m_rpDepthCamera->getViewMatrix();
						}

						float m[16];
						int i = 0;
						for (int row = 0; row < 4; row++) 
						{
							for (int col = 0; col < 4; col++)
							{
								m[i++] = (float)(mvMat(row,col));
							}
						}

						m_opGLExtensions->glUniformMatrix4fv(m_texMVMat,1,GL_FALSE,m);
					}

					if(m_texMat != -1)
					{
						const osg::Matrix bias(0.5, 0.0, 0.0, 0.0, 
							0.0, 0.5, 0.0, 0.0, 
							0.0, 0.0, 0.5, 0.0, 
							0.5, 0.5, 0.5, 1.0); 
						osg::Matrix texMat;
						osg::Matrix worldMat;
						osg::State* st = renderInfo.getState();
						if(st)
						{
							worldMat = st->getWorldMatrix();
						}
						if(m_opViewAnalyseNode->m_rpDepthCamera.valid())
						{
							texMat = worldMat * m_opViewAnalyseNode->m_rpDepthCamera->getViewMatrix() * m_opViewAnalyseNode->m_rpDepthCamera->getProjectionMatrix() * bias;
						}

						float m[16];
						int i = 0;
						for (int row = 0; row < 4; row++) 
						{
							for (int col = 0; col < 4; col++)
							{
								m[i++] = (float)(texMat(row,col));
							}
						}

						m_opGLExtensions->glUniformMatrix4fv(m_texMat,1,GL_FALSE,m);
					}
				}
			}
		}
	}

	void ViewAnalyseDrawCallback::drawImplementation( osg::RenderInfo& renderInfo,const osg::Drawable* drawable ) const
	{
		if(drawable)
		{	
			if(!m_shaderInit)
			{
				m_shaderInit = loadShaders(renderInfo.getContextID());
			}

			pushAllState();

			if(m_programCast && m_opGLExtensions.valid())
				m_opGLExtensions->glUseProgram(m_programCast);

			updateUniforms(renderInfo,drawable);

			drawable->drawImplementation(renderInfo);

			popAllState();
		}


	}

	ViewAnalyseDrawCallback::~ViewAnalyseDrawCallback()
	{
		if (m_vertShader && m_programCast && m_opGLExtensions.valid() )
		{
			m_opGLExtensions->glDetachShader(m_programCast, m_vertShader);
			m_opGLExtensions->glDeleteShader(m_vertShader);
			m_vertShader = 0;
		}

		if (m_fragShader && m_programCast && m_opGLExtensions.valid()) 
		{
			m_opGLExtensions->glDetachShader(m_programCast, m_fragShader);
			m_opGLExtensions->glDeleteShader(m_fragShader);
			m_fragShader = 0;
		}

		if (m_programCast)
		{
			m_opGLExtensions->glDeleteProgram(m_programCast);
			m_programCast = 0;
		}
	}

}

