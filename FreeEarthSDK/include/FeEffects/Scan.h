#ifndef _FE_SCAN_H
#define _FE_SCAN_H

/*

卫星常规的扫瞄地球的类，由（卫星位置【世界坐标】）来共同决定卫星扫瞄波的形状
所有参数一次成形，不接受修改，临时方案

若对角度大小不满意，调整Scan.cpp中的10.0 
osg::Vec3d thirdPos = osg::Matrix(osg::Matrix::translate(m_pos) * osg::Matrix::rotate(osg::inDegrees(10.0), osg::Vec3d(1.0, 0.0, 0.0))).getTrans() - m_pos;

*/

#include <stack>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/NodeCallback>
#include <osg/Drawable>
#include <osg/PolygonMode>

#include <FeUtils/RenderContext.h>

#include <FeEffects/Export.h>

using namespace osg;

namespace FeEffect
{
	class DrawableDrawCallback : public osg::Drawable::DrawCallback
	{
	public:
		DrawableDrawCallback(osg::Vec3d pos);

		void SetPosition(osg::Vec3d vecPos);

	public:
		virtual void drawImplementation(osg::RenderInfo& renderInfo, const osg::Drawable* drawable) const;

	private:
		osg::Vec3d m_pos;
		mutable float m_radius;
	};

	class FEEFFECTS_EXPORT CScan : public osg::Group
	{
	public:
		//pos卫星位置-世界坐标，扫瞄角速度-度/秒
		CScan(osg::Vec3d pos);

		void SetPosition(osg::Vec3d vecPos);

	private:
		void Init();

	private:
		//卫星位置
		osg::Vec3d m_pos;

	private:
		osg::ref_ptr<DrawableDrawCallback> m_rpDrawCallback;
		//
		osg::Geode* m_gnode;
		osg::Geometry* m_geom;
		osg::Vec4d m_color;
	};

	class TestGeometry : public osg::Drawable
	{
	public:
		TestGeometry();
		~TestGeometry();
		virtual void drawImplementation(osg::RenderInfo& renderInfo) const;
	private:
		void createGeometry() const;
		void pushAllState() const;
		void popAllState(osg::RenderInfo& renderInfo) const;
		GLuint loadShader(bool vertexShader,const char* source) const;
		void loadShaders() const;
	private:
		mutable GLuint  m_meshVboID;
		mutable GLuint  m_meshIdxID;
		mutable GLvoid* m_vertexBuffer;
		mutable GLvoid* m_indexBuffer;
		mutable OpenThreads::Mutex               m_mutex;
		mutable std::stack<GLint> m_programStack;
		mutable GLenum  m_glewInit;
		mutable GLuint m_vertShader;
		mutable GLuint m_fragShader;
		mutable GLuint m_programCast;
		mutable GLint m_uLength;
		mutable bool m_hasProgramUniforms;
		float m_floatArr[2];
		mutable osg::observer_ptr<osg::GLExtensions> m_opGLExtension;
	};

	class TestShaderNode : public osg::Group
	{
		friend class TestGeometry;
	public:
		TestShaderNode();
	private:
		osg::ref_ptr<TestGeometry> m_rpGeometry;
	};

	class SearchScreen;

	class SearchScreenCallback : public osg::NodeCallback
	{
	public:
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
	};

	class SearchScreenPoint
	{
	public:
		SearchScreenPoint()
			:r(0)
			,a(0)
			,e(0)
		{
		}
		SearchScreenPoint(float ir,float ia,float ie)
			:r(ir)
			,a(ia)
			,e(ie)
		{
		}
	public:
		float r;
		float a;
		float e;
	};

	typedef std::vector<SearchScreenPoint> SearchScreenPointList;

	class MyGeometry : public osg::Drawable
	{	
	public:
		MyGeometry();
		~MyGeometry();
		virtual void drawImplementation(osg::RenderInfo& renderInfo) const;
		void setNewBuffer(GLvoid* vData,GLvoid* iData,GLvoid* cData);
		void setBoundBox(osg::BoundingBox box);
		void setNumIndices(unsigned int num);
		void setVertexNum(int num)
		{
			m_vertexNum = num;
		}
		int getVertexNum()
		{
			return m_vertexNum;
		}
		void setIndexNum(int num)
		{
			m_indexNum = num;
		}
		int getIndexNum()
		{
			return m_indexNum;
		}
	private:
		void createMesh(osg::RenderInfo& renderInfo) const;
		void updateMesh(osg::RenderInfo& renderInfo) const;
		void pushAllState() const;
		void popAllState(osg::RenderInfo& renderInfo) const;
	private:
		mutable OpenThreads::Mutex               m_mutex;
		mutable GLuint  m_meshVboID;
		mutable GLuint  m_meshIdxID;
		mutable GLuint  m_colorVboID;
		mutable std::stack<GLint> m_programStack;
		unsigned int m_curIndices;
		mutable bool m_needUpdateBuffer;
		mutable GLvoid* m_newVertexBuffer;
		mutable GLvoid* m_newColorBuffer;
		mutable GLvoid* m_newIndexBuffer;
		mutable osg::GLExtensions* m_pGLExtensions;
		int m_vertexNum;
		int m_indexNum;
	};

	class FEEFFECTS_EXPORT SearchScreen : public osg::Group
	{
		friend class SearchScreenCallback;
	public:
		SearchScreen(FeUtil::CRenderContext* ct,const SearchScreenPointList& pl,const osg::Vec3& sPos);
		~SearchScreen();
	private:
		void initPatam();
		void createGeometry();
		void updateUniform(osg::NodeVisitor* nv);
		void updateSearcher();
	private:
		osg::ref_ptr<osg::Geometry> m_rpGeometry;
		osg::ref_ptr<osg::Geometry> m_rpOutLine;
		osg::ref_ptr<MyGeometry>    m_rpSearcher;
		osg::Vec4 m_backColor;
		osg::Vec4 m_searchColor;
		osg::Vec3 m_outLineColor;
		osg::ref_ptr<SearchScreenCallback> m_rpUpdateCallback;
		SearchScreenPointList m_pointList;
		typedef std::vector<osg::Vec3> WorldPosList;
		WorldPosList m_worldPosList;
		osg::Vec3 m_spherePos;
		osg::observer_ptr<FeUtil::CRenderContext> m_renderContext;
		float m_edgeLength;
		float m_searchRate;
		float m_searcherStartLength;
		float m_moveSpeed;

		typedef std::map<int,float> ScreenLengthMap;
		ScreenLengthMap m_screenLengthMap;
	};

	class TestNode;

	struct MeshVertex
	{
		float x, y, z, w;
	} ;

	struct MeshColor
	{
		float r, g, b, a;
	} ;

	class TestCallback : public osg::NodeCallback
	{
	public:
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
	};

	#define MAXNUMWAVE 10

	class PyramidNodeCallback1 : public osg::NodeCallback
	{
	public:
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
	};

	class FEEFFECTS_EXPORT PyramidNode : public osg::Group
	{
		friend class PyramidNodeCallback1;
	public:
		PyramidNode(float radius,float xAngle,float yAngle);
	private:
		void initPram();
		void initNode();
		void createNode();
		void updateUniform(osg::NodeVisitor* nv);
	private:
		float m_radius;
		float m_xAngle;
		float m_yAngle;
		osg::observer_ptr<osg::Geode> m_opGeode;
		typedef std::vector<osg::Vec3> PointList;
		PointList m_pointList;

		osg::observer_ptr<osg::Geometry> m_opLeftFace;
		osg::observer_ptr<osg::Geometry> m_opRightFace;
		osg::observer_ptr<osg::Geometry> m_opDownFace;
		osg::observer_ptr<osg::Geometry> m_opUpFace;
		osg::observer_ptr<osg::Geometry> m_opFrontFace;
		osg::observer_ptr<osg::Geometry> m_opOutLine;

		osg::Vec3 m_lastLookDir;
		int m_numVert;
		osg::ref_ptr<PyramidNodeCallback1> m_rpCullCallback;
		float m_edgeLength;
		float m_waveWidth;
		osg::Vec4 m_baseColor;
		osg::Vec4 m_waveColor;
		int m_numWave;
		bool m_isSmooth;
		float m_startLength[MAXNUMWAVE];
		float m_waveStartOffset;
		float m_widthBetweenWave;
		float m_moveSpeed;
	};

	class FEEFFECTS_EXPORT TestNode : public osg::Group
	{
		friend class TestCallback;
		friend class MyGeometry;

	public:
		TestNode(float length,float angle,osg::Vec3 up = osg::Vec3(0,0,1));
		void setNumWave(int num);
		void setWaveColor(osg::Vec3 color);
		void setBaseColor(osg::Vec4 color);

		void setWaveWidth(float width);
		float getWaveWidth()
		{
			return m_waveWidth;
		}

		void setWidthBetweenWave(float width);
		float getWidthBetweenWave()
		{
			return m_widthBetweenWave;
		}

		void setMoveSpeed(float s)
		{
			m_moveSpeed = s;
		}
		float getMoveSpeed()
		{
			return m_moveSpeed;
		}

		float getEdgeLength()
		{
			return m_edgeLength;
		}

		void setIsSmooth(bool flag)
		{
			m_isSmooth = flag;
		}
		void setWaveStartOffset(float offset);

	private:
		float m_centerLength;
		float m_edgeLength;
		float m_waveWidth;
		float m_widthBetweenWave;
		float m_startLength[MAXNUMWAVE];
		float m_angle;
		float m_moveSpeed;
		bool  m_isSmooth;
		unsigned int m_circleVertNum;
		osg::ref_ptr<TestCallback> m_updateCallback;
		osg::observer_ptr<MyGeometry> m_opFrontDrawable;
		osg::observer_ptr<MyGeometry> m_opBackDrawable;
		osg::observer_ptr<osg::Geode> m_opGeode;
		osg::BoundingBox m_boundingBox;
		osg::Vec4 m_baseColor;
		osg::Vec4 m_waveColor;
		int m_numWave;
		typedef std::vector<osg::Vec3> PointList;
		PointList m_pointList;
		osg::Vec3 m_up;
		float m_waveStartOffset;
		osg::Vec3 m_lastLookDir;
	private:
		void initPram();
		void initNode();
		void updateGeometry(osg::Vec3d lookDir);
		void updateUniform(osg::NodeVisitor* nv);
		void createGeode(osg::Vec3 center,osg::Vec3 up);
	};

	class PipeLine;

	class PipeLineCallback : public osg::NodeCallback
	{
	public:
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
	};

	struct OutLineBoundBoxCallback : osg::Drawable::ComputeBoundingBoxCallback
	{
	public:
		OutLineBoundBoxCallback(PipeLine* pl);
		virtual osg::BoundingBox computeBound(const osg::Drawable& dr) const;
	private:
		osg::observer_ptr<PipeLine> m_opPipeLine;
	};

	struct PreDrawCallback : public osg::Camera::DrawCallback 
	{ 
		PreDrawCallback(PipeLine* pl)
			:m_opPipeLine(pl)
		{
		}
		virtual void operator()( osg::RenderInfo& renderInfo ) const;
	private:
		osg::observer_ptr<PipeLine> m_opPipeLine;
	}; 

	#define MAXNUMMAT 20

	class FEEFFECTS_EXPORT PipeLine : public osg::Group
	{
		friend class PipeLineCallback;
		friend struct OutLineBoundBoxCallback;
		friend struct PreDrawCallback;
	public:
		typedef std::vector<osg::Vec4> CenterLineList;
	private:
		typedef std::vector<osg::Vec3> LocalLineList;
		typedef std::map<int,osg::Vec4> CenterLineLengthMap;
		typedef std::vector<osg::Matrix> OutLineTransMatList;
		typedef std::vector<float> OutLineStartLengthList;
	public:
		PipeLine(const CenterLineList& ll,FeUtil::CRenderContext* ct);
		~PipeLine();

		osg::Vec4 getPipeColor()
		{
			return m_pipeColor;
		}
		void setPipeColor(const osg::Vec4& c)
		{
			m_pipeColor = c;
		}

		osg::Vec4 getOutLineColor()
		{
			return m_outLineColor;
		}
		void setOutLineColor(const osg::Vec4& c)
		{
			m_outLineColor = c;
		}

		osg::Vec3 getDiffColor()
		{
			return m_diffColor;
		}
		void setDiffColor(const osg::Vec3& c)
		{
			m_diffColor = c;
		}

		float getMoveSpeed()
		{
			return m_moveSpeed;
		}
		void setMoveSpeed(float s)
		{
			m_moveSpeed = s;
		}

		osg::PolygonMode::Mode getPolygonMode()
		{
			return m_polygonMode;
		}
		void setPolygonMode(const osg::PolygonMode::Mode& pm)
		{
			m_polygonMode = pm;
			m_needUpdatePolygonMode = true;
		}

		bool getEnableLighting()
		{
			return m_enableLighting;
		}
		void setEnableLighting(bool flag)
		{
			m_enableLighting = flag;
		}

		void setOutLineVisible(bool flag);

	private:
		void createPipeNode();
		void updateOutLine(osg::NodeVisitor* nv);
		void updateUniform();

	private:
		osg::ref_ptr<osg::Geometry> m_rpPipeLineGeometry;
		osg::ref_ptr<osg::Geometry> m_rpBackPipeLineGeometry;
		osg::ref_ptr<osg::Geometry> m_rpOutLineGeometry;
		OutLineTransMatList m_outLineTransMatList;
		CenterLineList m_centerLineList;
		int m_numPointPerRing;
		osg::observer_ptr<FeUtil::CRenderContext> m_opRenderContext;
		osg::Vec4 m_pipeColor;
		osg::Vec4 m_outLineColor;
		osg::Vec3 m_lightDir;
		LocalLineList m_localLineList;
		osg::ref_ptr<PipeLineCallback> m_rpPipeLineCallback;
		CenterLineLengthMap m_centerLineLengthMap;
		float m_moveSpeed;
		float m_centerLineLength;
		int m_numMoveLine;
		OutLineStartLengthList m_outLineStartLengthList;
		osg::ref_ptr<PreDrawCallback> m_rpPreDrawCallback;
		osg::ref_ptr<osg::Camera> m_rpPreCamera;
		osg::Vec3 m_diffColor;
		osg::PolygonMode::Mode m_polygonMode;
		bool m_needUpdatePolygonMode;
		bool m_enableLighting;
	};

}

#endif // _FE_SCAN_H

