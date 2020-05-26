#include <string>
#include <algorithm>
#include <ctype.h>

#include <osgDB/ConvertUTF>
#include <osg/TriangleFunctor>
#include <osg/MatrixTransform>
#include <osgEarthDrivers/engine_mp/MPGeometry>

#include <IntersectUtil.h>

namespace FeMath
{
	bool intersectWithEllipse(double aAxis,double bAxis,double cAxis,osg::Vec3d& p1,osg::Vec3d& p2,osg::Vec3d& out_pt1,osg::Vec3d& out_pt2)
	{
		double ta,tb,tc;
		double delt; 
		double ASquare,BSquare,CSquare; 
		ASquare = aAxis * aAxis;
		BSquare = bAxis * bAxis;
		CSquare = cAxis * cAxis;

		double x1 = p1.x();
		double y1 = p1.y();
		double z1 = p1.z();

		double x2 = p2.x();
		double y2 = p2.y();
		double z2 = p2.z();

		double A = (y2 -y1) / (x2 - x1);
		double B = (z2 -z1) / (x2 - x1);
		ta = BSquare * CSquare + A * A * ASquare * CSquare + B * B * ASquare * BSquare;
		tb = 2 * y1 * ASquare * CSquare * A - 2 * x1 * A * A * ASquare * CSquare + 2 * z1 * ASquare * BSquare * B - 2 * x1 * B * B * ASquare * BSquare;
		tc = ASquare * CSquare* y1 * y1 - 2 * y1 * ASquare * CSquare * A * x1 + A * A * x1 * x1 * ASquare * CSquare +
			ASquare * BSquare * z1 * z1 - 2 * z1 * ASquare * BSquare * B * x1 + B * B * x1 * x1 * ASquare * BSquare - ASquare * BSquare * CSquare;
		delt = tb*tb - 4*ta*tc;
		double t1,t2;
		if(delt>=0) 
		{
			t1 = (-tb + sqrt(delt))/(2 * ta);
			t2 = (-tb - sqrt(delt))/(2 * ta);
			double x =t1;
			double y = y1 + A * (x - x1);
			double z = z1 + B * (x - x1);
			out_pt1 = osg::Vec3d(x,y,z);
			x = t2;
			y = y1 + A * (x - x1);
			z = z1 + B * (x - x1);
			out_pt2 = osg::Vec3d(x,y,z);
			return true;
		}   
		else 
			return false;
	}

	int getLinePanelIntersec(osg::Vec3d linePoint, osg::Vec3d lineVector,osg::Vec3d planePoint,osg::Vec3d planeNor, double& tHit,osg::Vec3d& PHit)  
	{  
		osg::Vec3d A = linePoint;
		osg::Vec3d c = lineVector;
		osg::Vec3d B = planePoint;
		osg::Vec3d n = planeNor;

		if (n * c == 0)  
		{  
			return 0 ;  
		}  
		else   
		{  
			tHit = (n * (B - A)) / (n * c);        
			PHit = A + c * tHit;  
			if(n*c >0)  
			{  
				return 1; 
			}  
			else  
			{  
				return -1 ; 
			}  
		}  
	}  

	triangle3d::triangle3d()
	{
	}
	triangle3d::~triangle3d()
	{
	}
	triangle3d::triangle3d(osg::Vec3d pointA,osg::Vec3d pointB,osg::Vec3d pointC)
	{
		PointA = pointA;
		PointB = pointB;
		PointC = pointC;
	}

	void COctreeTriangleSelector::MyTriangleFunctor::operator () (const osg::Vec3& v1,const osg::Vec3& v2,const osg::Vec3& v3, bool treatVertexDataAsTemporary)
	{
		if(m_octreeNode)
		{
			osg::Vec3d worldV1 = v1;
			osg::Vec3d worldV2 = v2;
			osg::Vec3d worldV3 = v3;

			if(m_matrixList.size() > 0)
			{
				worldV1 = worldV1 * m_matrixList[0];
				worldV2 = worldV2 * m_matrixList[0];
				worldV3 = worldV3 * m_matrixList[0];
			}

			m_octreeNode->Triangles.push_back(triangle3d(worldV1,worldV2,worldV3));

			if(m_octreeNode->Triangles.size() == 1)
				m_octreeNode->Box.reset(worldV1);

			m_octreeNode->Box.addInternalPoint(worldV1);
			m_octreeNode->Box.addInternalPoint(worldV2);
			m_octreeNode->Box.addInternalPoint(worldV3);
		}
	}

	void COctreeTriangleSelector::ModelTriangleFunctor::operator()( const osg::Vec3& v1,const osg::Vec3& v2,const osg::Vec3& v3, bool treatVertexDataAsTemporary )
	{
		if(m_octreeNode)
		{
			osg::Vec3d worldV1 = v1;
			osg::Vec3d worldV2 = v2;
			osg::Vec3d worldV3 = v3;

		    worldV1 = worldV1 * m_worldMat;
			worldV2 = worldV2 * m_worldMat;
			worldV3 = worldV3 * m_worldMat;
			
			m_octreeNode->Triangles.push_back(triangle3d(worldV1,worldV2,worldV3));

			if(m_octreeNode->Triangles.size() == 1)
				m_octreeNode->Box.reset(worldV1);

			m_octreeNode->Box.addInternalPoint(worldV1);
			m_octreeNode->Box.addInternalPoint(worldV2);
			m_octreeNode->Box.addInternalPoint(worldV3);
		}
	}

	template<class T>
	void COctreeTriangleSelector::DoubleTriangleFunctor<T>::setVertexArray(unsigned int count,const osg::Vec3d* pVertex)
	{
		this->_vertexArraySize = count;
		this->m_vertex3dArrayPtr = pVertex;
	}
	//template<class T>
	//void COctreeTriangleSelector::DoubleTriangleFunctor<T>::setVertexArray(unsigned int count,const osg::Vec3* vertices)
	//{
	//	_vertexArraySize = count;
	//	m_vertexArrayPtr = vertices;
	//}
	template<class T>
	void COctreeTriangleSelector::DoubleTriangleFunctor<T>::drawArrays(GLenum mode,GLint first,GLsizei count)
	{
		if (this->_vertexArrayPtr == 0 || count == 0) return;

		switch(mode)
		{
		case(GL_TRIANGLES):
			{
				const osg::Vec3* vlast = &this->_vertexArrayPtr[first+count];
				for(const osg::Vec3* vptr = &this->_vertexArrayPtr[first];vptr<vlast;vptr+=3)
					this->operator()(*(vptr),*(vptr+1),*(vptr+2),this->_treatVertexDataAsTemporary);
				break;
			}
		case(GL_TRIANGLE_STRIP):
			{
				const osg::Vec3* vptr = &this->_vertexArrayPtr[first];
				for(GLsizei i=2;i<count;++i,++vptr)
				{
					if ((i%2)) this->operator()(*(vptr),*(vptr+2),*(vptr+1),this->_treatVertexDataAsTemporary);
					else       this->operator()(*(vptr),*(vptr+1),*(vptr+2),this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_QUADS):
			{
				const osg::Vec3* vptr = &this->_vertexArrayPtr[first];
				for(GLsizei i=3;i<count;i+=4,vptr+=4)
				{
					this->operator()(*(vptr),*(vptr+1),*(vptr+2),this->_treatVertexDataAsTemporary);
					this->operator()(*(vptr),*(vptr+2),*(vptr+3),this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_QUAD_STRIP):
			{
				const osg::Vec3* vptr = &this->_vertexArrayPtr[first];
				for(GLsizei i=3;i<count;i+=2,vptr+=2)
				{
					this->operator()(*(vptr),*(vptr+1),*(vptr+2),this->_treatVertexDataAsTemporary);
					this->operator()(*(vptr+1),*(vptr+3),*(vptr+2),this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
		case(GL_TRIANGLE_FAN):
			{
				const osg::Vec3* vfirst = &this->_vertexArrayPtr[first];
				const osg::Vec3* vptr = vfirst+1;
				for(GLsizei i=2;i<count;++i,++vptr)
				{
					this->operator()(*(vfirst),*(vptr),*(vptr+1),this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_POINTS):
		case(GL_LINES):
		case(GL_LINE_STRIP):
		case(GL_LINE_LOOP):
		default:
			// can't be converted into to triangles.
			break;
		}
	}
	template<class T>
	void COctreeTriangleSelector::DoubleTriangleFunctor<T>::drawElements(GLenum mode,GLsizei count,const GLubyte* indices)
	{
		if (indices==0 || count==0) return;

		typedef const GLubyte* IndexPointer;

		switch(mode)
		{
		case(GL_TRIANGLES):
			{
				IndexPointer ilast = &indices[count];
				for(IndexPointer  iptr=indices;iptr<ilast;iptr+=3)
					this->operator()(this->_vertexArrayPtr[*iptr],this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
				break;
			}
		case(GL_TRIANGLE_STRIP):
			{
				IndexPointer iptr = indices;
				for(GLsizei i=2;i<count;++i,++iptr)
				{
					if ((i%2)) this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+2)],this->_vertexArrayPtr[*(iptr+1)],this->_treatVertexDataAsTemporary);
					else       this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_QUADS):
			{
				IndexPointer iptr = indices;
				for(GLsizei i=3;i<count;i+=4,iptr+=4)
				{
					this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
					this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+2)],this->_vertexArrayPtr[*(iptr+3)],this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_QUAD_STRIP):
			{
				IndexPointer iptr = indices;
				for(GLsizei i=3;i<count;i+=2,iptr+=2)
				{
					this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
					this->operator()(this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+3)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
		case(GL_TRIANGLE_FAN):
			{
				IndexPointer iptr = indices;
				const osg::Vec3d& vfirst = this->_vertexArrayPtr[*iptr];
				++iptr;
				for(GLsizei i=2;i<count;++i,++iptr)
				{
					this->operator()(vfirst,this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+1)],this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_POINTS):
		case(GL_LINES):
		case(GL_LINE_STRIP):
		case(GL_LINE_LOOP):
		default:
			// can't be converted into to triangles.
			break;
		}
	}
	template<class T>
	void COctreeTriangleSelector::DoubleTriangleFunctor<T>::drawElements(GLenum mode,GLsizei count,const GLushort* indices)
	{
		if (indices==0 || count==0) return;

		typedef const GLushort* IndexPointer;

		switch(mode)
		{
		case(GL_TRIANGLES):
			{
				IndexPointer ilast = &indices[count];
				for(IndexPointer  iptr=indices;iptr<ilast;iptr+=3)
				{
					this->operator()(this->_vertexArrayPtr[*iptr],this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_TRIANGLE_STRIP):
			{
				IndexPointer iptr = indices;
				for(GLsizei i=2;i<count;++i,++iptr)
				{
					if ((i%2)) this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+2)],this->_vertexArrayPtr[*(iptr+1)],this->_treatVertexDataAsTemporary);
					else       this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_QUADS):
			{
				IndexPointer iptr = indices;
				for(GLsizei i=3;i<count;i+=4,iptr+=4)
				{
					this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
					this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+2)],this->_vertexArrayPtr[*(iptr+3)],this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_QUAD_STRIP):
			{
				IndexPointer iptr = indices;
				for(GLsizei i=3;i<count;i+=2,iptr+=2)
				{
					this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
					this->operator()(this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+3)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
		case(GL_TRIANGLE_FAN):
			{
				IndexPointer iptr = indices;
				const osg::Vec3d& vfirst = this->_vertexArrayPtr[*iptr];
				++iptr;
				for(GLsizei i=2;i<count;++i,++iptr)
				{
					this->operator()(vfirst,this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+1)],this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_POINTS):
		case(GL_LINES):
		case(GL_LINE_STRIP):
		case(GL_LINE_LOOP):
		default:
			// can't be converted into to triangles.
			break;
		}
	}
	template<class T>
	void COctreeTriangleSelector::DoubleTriangleFunctor<T>::drawElements(GLenum mode,GLsizei count,const GLuint* indices)
	{
		if (indices==0 || count==0) return;

		typedef const GLuint* IndexPointer;

		switch(mode)
		{
		case(GL_TRIANGLES):
			{
				IndexPointer ilast = &indices[count];
				for(IndexPointer  iptr=indices;iptr<ilast;iptr+=3)
					this->operator()(this->_vertexArrayPtr[*iptr],this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
				break;
			}
		case(GL_TRIANGLE_STRIP):
			{
				IndexPointer iptr = indices;
				for(GLsizei i=2;i<count;++i,++iptr)
				{
					if ((i%2)) this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+2)],this->_vertexArrayPtr[*(iptr+1)],this->_treatVertexDataAsTemporary);
					else       this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_QUADS):
			{
				IndexPointer iptr = indices;
				for(GLsizei i=3;i<count;i+=4,iptr+=4)
				{
					this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
					this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+2)],this->_vertexArrayPtr[*(iptr+3)],this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_QUAD_STRIP):
			{
				IndexPointer iptr = indices;
				for(GLsizei i=3;i<count;i+=2,iptr+=2)
				{
					this->operator()(this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
					this->operator()(this->_vertexArrayPtr[*(iptr+1)],this->_vertexArrayPtr[*(iptr+3)],this->_vertexArrayPtr[*(iptr+2)],this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
		case(GL_TRIANGLE_FAN):
			{
				IndexPointer iptr = indices;
				const osg::Vec3d& vfirst = this->_vertexArrayPtr[*iptr];
				++iptr;
				for(GLsizei i=2;i<count;++i,++iptr)
				{
					this->operator()(vfirst,this->_vertexArrayPtr[*(iptr)],this->_vertexArrayPtr[*(iptr+1)],this->_treatVertexDataAsTemporary);
				}
				break;
			}
		case(GL_POINTS):
		case(GL_LINES):
		case(GL_LINE_STRIP):
		case(GL_LINE_LOOP):
		default:
			// can't be converted into to triangles.
			break;
		}
	}


	COctreeTriangleSelector::COctreeTriangleSelector()
		:TotalTriangleCount(0)
		,Root(NULL)
	{
		Root = new SOctreeNode();
	}
	COctreeTriangleSelector::~COctreeTriangleSelector()
	{
		if(Root != NULL)
		{
			delete Root;
			Root = NULL;
		}
	}
	int COctreeTriangleSelector::GetTriangleCount()
	{
		return TotalTriangleCount;
	}
	void COctreeTriangleSelector::ConstructTriangleOctree(osg::Drawable* pDrawable)
	{
		if(!pDrawable)
			return;

		TotalTriangleCount = 0;

		if(Root != NULL)
			delete Root;
		Root = new SOctreeNode();


		DoubleTriangleFunctor<MyTriangleFunctor> ti;

		Root->Child[0] = new SOctreeNode();
		ti.set(Root->Child[0]);

		osg::MatrixList ml = pDrawable->getWorldMatrices();
		ti.set(ml);

		osgEarth::Drivers::MPTerrainEngine::MPGeometry* mg = dynamic_cast<osgEarth::Drivers::MPTerrainEngine::MPGeometry*>(pDrawable);
		if(mg)
		{
			/*bool oldUse = mg->_useCustomPrimitiveSetIndex;
			unsigned int oldIndex = mg->_customPrimitiveSetIndex;

			mg->_useCustomPrimitiveSetIndex = true;
			mg->_customPrimitiveSetIndex = 0;*/

			mg->accept(ti);

			/*mg->_useCustomPrimitiveSetIndex = oldUse;
			mg->_customPrimitiveSetIndex = oldIndex;*/

			TotalTriangleCount += Root->Child[0]->Triangles.size();

			constructOctree(Root->Child[0],100);

			Root->Box.reset(Root->Child[0]->Box.MinEdge);
			Root->Box.addInternalPoint(Root->Child[0]->Box.MinEdge);
			Root->Box.addInternalPoint(Root->Child[0]->Box.MaxEdge);
		}	
		else
		{
			osg::Geometry* gm = dynamic_cast<osg::Geometry*>(pDrawable);
			if(gm)
			{
				mg->accept(ti);						

				TotalTriangleCount += Root->Child[0]->Triangles.size();

				constructOctree(Root->Child[0],100);

				Root->Box.reset(Root->Child[0]->Box.MinEdge);
				Root->Box.addInternalPoint(Root->Child[0]->Box.MinEdge);
				Root->Box.addInternalPoint(Root->Child[0]->Box.MaxEdge);
			}
		}							
	}
	//void COctreeTriangleSelector::ConstructTriangleOctree(const osg::Group& triangleNode)
	//{
	//	TotalTriangleCount = 0;
	//	int numChild = triangleNode.getNumChildren();

	//	if(numChild > 8)
	//		numChild = 8;

	//	if(Root != NULL)
	//		delete Root;
	//	Root = new SOctreeNode();

	//	if(numChild > 0)
	//	{
	//		DoubleTriangleFunctor<MyTriangleFunctor> ti;
	//		for (int i = 0; i < numChild; i++)
	//		{
	//			osg::Node* tn = const_cast<osg::Node*>(triangleNode.getChild(i));
	//			if(tn)
	//			{
	//				osg::MatrixTransform* mt = dynamic_cast<osg::MatrixTransform*>(tn);
	//				if(mt)
	//				{
	//					if(mt->getNumChildren() > 0)
	//					{
	//						osg::Geode* gn = dynamic_cast<osg::Geode*>(mt->getChild(0));
	//						if(gn)
	//						{
	//							if(gn->getNumDrawables() > 0)
	//							{
	//								Root->Child[i] = new SOctreeNode();
	//								ti.set(Root->Child[i]);

	//								osg::MatrixList ml = gn->getWorldMatrices();
	//								ti.set(ml);

	//								osgEarth::Drivers::MPTerrainEngine::MPGeometry* mg = dynamic_cast<osgEarth::Drivers::MPTerrainEngine::MPGeometry*>(gn->getDrawable(0));
	//								if(mg)
	//								{
	//									/*bool oldUse = mg->_useCustomPrimitiveSetIndex;
	//									unsigned int oldIndex = mg->_customPrimitiveSetIndex;

	//									mg->_useCustomPrimitiveSetIndex = true;
	//									mg->_customPrimitiveSetIndex = 0;*/

	//									mg->accept(ti);

	//									/*mg->_useCustomPrimitiveSetIndex = oldUse;
	//									mg->_customPrimitiveSetIndex = oldIndex;*/

	//									TotalTriangleCount += Root->Child[i]->Triangles.size();

	//									constructOctree(Root->Child[i],100);

	//									if(i == 0)
	//										Root->Box.reset(Root->Child[i]->Box.MinEdge);

	//									Root->Box.addInternalPoint(Root->Child[i]->Box.MinEdge);
	//									Root->Box.addInternalPoint(Root->Child[i]->Box.MaxEdge);
	//								}	
	//							}
	//						}
	//					}
	//				}		
	//			}	
	//		}
	//	}
	//}

	void COctreeTriangleSelector::constructOctree(SOctreeNode* node,int minimalPolysPerNode)
	{
		const osg::Vec3d middle = node->Box.getCenter();
		osg::Vec3d edges[8];
		node->Box.getEdges(edges);

		aabbox3d box;
		list<triangle3d> keepTriangles;

		if (!node->Box.isEmpty() && (int)node->Triangles.size() > minimalPolysPerNode)
		{
			for (int ch = 0; ch < 8; ++ch)
			{
				box.reset(middle);
				box.addInternalPoint(edges[ch]);
				node->Child[ch] = new SOctreeNode();

				for (list<triangle3d>::iterator ite = node->Triangles.begin();ite != node->Triangles.end();ite++)
				{
					if ((*ite).isTotalInsideBox(box))
					{
						node->Child[ch]->Triangles.push_back((*ite));

						if(node->Child[ch]->Triangles.size() == 1)
							node->Child[ch]->Box.reset((*ite).PointA);

						node->Child[ch]->Box.addInternalPoint((*ite).PointA);
						node->Child[ch]->Box.addInternalPoint((*ite).PointB);
						node->Child[ch]->Box.addInternalPoint((*ite).PointC);
					}
					else
					{
						keepTriangles.push_back((*ite));
					}
				}

				/*node->Triangles.clear();
				for (list<triangle3d>::iterator kIt = keepTriangles.begin();kIt != keepTriangles.end();kIt++)
				{
				node->Triangles.push_back((*kIt));
				}*/
				node->Triangles.swap(keepTriangles);
				keepTriangles.clear();

				if (node->Child[ch]->Triangles.empty())
				{
					delete node->Child[ch];
					node->Child[ch] = 0;
				}
				else
					ConstructOctree(node->Child[ch],minimalPolysPerNode);
			}
		}	
	}
	void COctreeTriangleSelector::ConstructTriangleOctree(std::list<triangle3d> triangles,int minimalPolysPerNode)
	{
		if(Root != NULL)
		{
			TotalTriangleCount = (int)triangles.size();
			Root->Triangles = triangles;
			ConstructOctree(Root, minimalPolysPerNode);
		}
	}

	bool COctreeTriangleSelector::ConstructTriangleOctree( osg::Node* n,osg::Matrix sm,int minimalPolysPerNode )
	{
		if(n && minimalPolysPerNode > 0)
		{
			if(Root != NULL)
				delete Root;
			Root = new SOctreeNode();

			DoubleTriangleFunctor<ModelTriangleFunctor> ti;
		
			ti.setRootNode(Root);

			ti.setWorldMat(sm);

			MyGeometryVisitor mv(&ti);
			n->accept(mv);

			TotalTriangleCount = Root->Triangles.size();
			if(TotalTriangleCount > 0)
			{
			    ConstructOctree(Root, minimalPolysPerNode);
				return true;
			}
		}

		return false;
	}

	void COctreeTriangleSelector::ConstructTriangleOctree( osg::Group& triangleNode,int minimalPolysPerNode /*= 100*/ )
	{
		if(minimalPolysPerNode > 0)
		{
			if(Root != NULL)
				delete Root;
			Root = new SOctreeNode();

			DoubleTriangleFunctor<ModelTriangleFunctor> ti;

			ti.setRootNode(Root);

			MyGeometryVisitor mv(&ti);
			triangleNode.accept(mv);

			TotalTriangleCount = Root->Triangles.size();
			if(TotalTriangleCount > 0)
			{
				ConstructOctree(Root, minimalPolysPerNode);
			}
		}
	}

	void COctreeTriangleSelector::ConstructOctree(SOctreeNode* node,int minimalPolysPerNode)
	{
		list<triangle3d>::iterator ite = node->Triangles.begin();
		//node->Box.reset((*ite).PointA);

		/*for (ite = node->Triangles.begin();ite != node->Triangles.end();ite++)
		{
		node->Box.addInternalPoint((*ite).PointA);
		node->Box.addInternalPoint((*ite).PointB);
		node->Box.addInternalPoint((*ite).PointC);
		}*/

		const osg::Vec3d middle = node->Box.getCenter();
		osg::Vec3d edges[8];
		node->Box.getEdges(edges);

		aabbox3d box;
		list<triangle3d> keepTriangles;

		if (!node->Box.isEmpty() && (int)node->Triangles.size() > minimalPolysPerNode)
			for (int ch = 0; ch < 8; ++ch)
			{
				box.reset(middle);
				box.addInternalPoint(edges[ch]);
				node->Child[ch] = new SOctreeNode();

				for (ite = node->Triangles.begin();ite != node->Triangles.end();ite++)
				{
					if ((*ite).isTotalInsideBox(box))
					{
						node->Child[ch]->Triangles.push_back((*ite));

						if(node->Child[ch]->Triangles.size() == 1)
								node->Child[ch]->Box.reset((*ite).PointA);

						node->Child[ch]->Box.addInternalPoint((*ite).PointA);
						node->Child[ch]->Box.addInternalPoint((*ite).PointB);
						node->Child[ch]->Box.addInternalPoint((*ite).PointC);
					}
					else
					{
						keepTriangles.push_back((*ite));
					}
				}

				/*node->Triangles.clear();
				for (list<triangle3d>::iterator kIt = keepTriangles.begin();kIt != keepTriangles.end();kIt++)
				{
				node->Triangles.push_back((*kIt));
				}*/
				node->Triangles.swap(keepTriangles);
				keepTriangles.clear();

				if (node->Child[ch]->Triangles.empty())
				{
					delete node->Child[ch];
					node->Child[ch] = 0;
				}
				else
					ConstructOctree(node->Child[ch],minimalPolysPerNode);
			}
	}
	void COctreeTriangleSelector::GetTrianglesFromOctree(
		SOctreeNode* node, int& trianglesWritten,
		int maximumSize, const aabbox3d& box,
		osg::Matrixd mat, list<triangle3d>& triangles)
	{
		if (!box.intersectsWithBox(node->Box))
			return;

		int cnt = (int)node->Triangles.size();
		if (cnt + trianglesWritten > maximumSize)
			cnt -= cnt + trianglesWritten - maximumSize;

		int i=0;

		for (list<triangle3d>::iterator it=node->Triangles.begin();it!=node->Triangles.end();it++)
		{
			if(i < cnt)
			{
				osg::Vec3d p1;
				p1 = (*it).PointA * mat;
				osg::Vec3d p2;
				p2 = (*it).PointB * mat;
				osg::Vec3d p3;
				p3 = (*it).PointC * mat;
				triangles.push_back(triangle3d(p1,p2,p3));
				++trianglesWritten;
				i++;
			}
		}

		for (i = 0; i < 8; ++i)
			if (node->Child[i])
				GetTrianglesFromOctree(node->Child[i], trianglesWritten,maximumSize, box, mat, triangles);
	}
	//
	/*{
	depth++;
	if(node)
	{
	int triangleSize = node->Triangles.size();
	fs.write((char*)(&triangleSize),sizeof(triangleSize));
	for (std::list<FeMath::triangle3d>::iterator it = node->Triangles.begin();it != node->Triangles.end();it++)
	{
	fs.write((char*)(&(*it)),sizeof((*it)));
	}
	fs.write((char*)(&node->Box),sizeof(node->Box));

	for (int i = 0;i < 8;i++)
	{
	if(depth == 1)
	{
	m_pModelToolDialog->m_curProgress += 5;
	m_pModelToolDialog->ui.progressBar_Handle->setValue(m_pModelToolDialog->m_curProgress);
	}

	bool childValid = false;
	if(node->Child[i] != NULL)
	{
	childValid = true;
	fs.write((char*)(&childValid),sizeof(childValid));
	writeNodeToFileImp(node->Child[i],fs,depth);
	}
	else
	fs.write((char*)(&childValid),sizeof(childValid));				
	}
	}
	}*/
	//

	bool COctreeTriangleSelector::ConstructTriangleOctreeFromFile( std::fstream& fs )
	{
		if(Root != NULL)
			delete Root;
		Root = new SOctreeNode();

		ConstructTriangleOctreeFromFileImp(fs,Root);

		return true;
	}

	void COctreeTriangleSelector::ConstructTriangleOctreeFromFileImp( std::fstream& fs, SOctreeNode* node )
	{
		if(node)
		{
			std::list<triangle3d>::size_type triangelSize = 0;
			fs.read((char*)(&triangelSize),sizeof(triangelSize));
			for (std::list<triangle3d>::size_type i = 0;i < triangelSize;i++)
			{
				triangle3d t;
				fs.read((char*)(&t),sizeof(t));
				node->Triangles.push_back(t);
			}

			TotalTriangleCount += triangelSize;

			fs.read((char*)(&node->Box),sizeof(node->Box));

			for (int i = 0;i < 8;i++)
			{
				bool childValid;
				fs.read((char*)(&childValid),sizeof(childValid));
				if(childValid)
				{
					node->Child[i] = new SOctreeNode();
					ConstructTriangleOctreeFromFileImp(fs,node->Child[i]);
				}
			}
		}
	}

	osg::Vec3d CSceneCollisionManager::getCollisionResultPosition(
		COctreeTriangleSelector* selector,
		const osg::Vec3d& position, 
		const osg::Vec3d& radius,
		const osg::Vec3d& direction,
		triangle3d& triout,
		osg::Vec3d& hitPosition,
		bool& outFalling,
		double slidingSpeed,
		const osg::Vec3d& gravity,
		bool& foundCollision)
	{
		return collideEllipsoidWithWorld(selector, position,
			radius, direction, slidingSpeed, gravity, triout, hitPosition, outFalling,foundCollision);
	}
	osg::Vec3d CSceneCollisionManager::collideEllipsoidWithWorld(
		COctreeTriangleSelector* selector,
		const osg::Vec3d& position,
		const osg::Vec3d& radius,  
		const osg::Vec3d& velocity,
		double slidingSpeed,
		const osg::Vec3d& gravity,
		triangle3d& triout,
		osg::Vec3d& hitPosition,
		bool& outFalling,
		bool& foundCollision)
	{
		if (!selector || radius.x() == 0.0f || radius.y() == 0.0f || radius.z() == 0.0f)
			return position;

		SCollisionData colData;
		colData.R3Position = position;
		colData.R3Velocity = velocity;
		colData.eRadius = radius;
		colData.nearestDistance = FLT_MAX;
		colData.selector = selector;
		colData.slidingSpeed = slidingSpeed;
		colData.triangleHits = 0;
		colData.triangleIndex = -1;

		osg::Vec3d eSpacePosition = osg::Vec3d(colData.R3Position.x() / colData.eRadius.x(),
			colData.R3Position.y() / colData.eRadius.y(),
			colData.R3Position.z() / colData.eRadius.z());
		osg::Vec3d eSpaceVelocity = osg::Vec3d(colData.R3Velocity.x() / colData.eRadius.x(),
			colData.R3Velocity.y() / colData.eRadius.y(),
			colData.R3Velocity.z() / colData.eRadius.z());

		osg::Vec3d finalPos = collideWithWorld(
			0, colData, eSpacePosition, eSpaceVelocity,foundCollision);

		outFalling = false;

		if (gravity != osg::Vec3d(0,0,0))
		{
			colData.R3Position = osg::Vec3d(finalPos.x() * colData.eRadius.x(),
				finalPos.y() * colData.eRadius.y(),
				finalPos.z() * colData.eRadius.z());
			colData.R3Velocity = gravity;
			colData.triangleHits = 0;

			eSpaceVelocity = osg::Vec3d(gravity.x() / colData.eRadius.x(),
				gravity.y() / colData.eRadius.y(),
				gravity.z() / colData.eRadius.z()); 

			finalPos = collideWithWorld(0, colData,
				finalPos, eSpaceVelocity,foundCollision);

			outFalling = (colData.triangleHits == 0);
		}

		finalPos = osg::Vec3d(finalPos.x() * colData.eRadius.x(),
			finalPos.y() * colData.eRadius.y(),
			finalPos.z() * colData.eRadius.z());
		hitPosition = osg::Vec3d(colData.intersectionPoint.x() * colData.eRadius.x(),
			colData.intersectionPoint.y() * colData.eRadius.y(),
			colData.intersectionPoint.z() * colData.eRadius.z()); 

		return finalPos;
	}
	osg::Vec3d CSceneCollisionManager::collideWithWorld(
		double recursionDepth,
		SCollisionData &colData, 
		osg::Vec3d pos, 
		osg::Vec3d vel,
		bool& foundCollision)
	{
		double veryCloseDistance = colData.slidingSpeed;

		if (recursionDepth > 5)
			return pos;

		colData.velocity = vel;
		colData.normalizedVelocity = vel;
		colData.normalizedVelocity.normalize();
		colData.basePoint = pos;
		colData.foundCollision = false;
		colData.nearestDistance = FLT_MAX;

		aabbox3d box(colData.R3Position);
		box.addInternalPoint(colData.R3Position + colData.R3Velocity);
		box.MinEdge -= colData.eRadius;
		box.MaxEdge += colData.eRadius;

		int totalTriangleCnt = colData.selector->GetTriangleCount();
		list<triangle3d> triangleList;

		osg::Matrixd scaleMatrix;
		scaleMatrix.makeScale(1.0f / colData.eRadius.x(),1.0f / colData.eRadius.y(),1.0f / colData.eRadius.z());

		int triangleCnt = 0;
		colData.selector->GetTrianglesFromOctree (colData.selector->Root,triangleCnt,totalTriangleCnt,box,scaleMatrix,triangleList);

		for (list<triangle3d>::iterator it = triangleList.begin();it != triangleList.end();it++)
			testTriangleIntersection(&colData, (*it));

		foundCollision = colData.foundCollision;

		if (!colData.foundCollision)
		{
			return pos + vel;
		}	
		else
		{
			vel.normalize();
			return pos + vel * colData.nearestDistance;
		}
	}
	bool CSceneCollisionManager::testTriangleIntersection(SCollisionData* colData,
		triangle3d& triangle)
	{
		const plane3d testTrianglePlane = triangle.getPlane();

		if ( testTrianglePlane.isFrontFacing(colData->normalizedVelocity) )
		{
			//triangle = triangle3d(triangle.PointA,triangle.PointC,triangle.PointB);
		}
		else
		{
			return false;
		}
		const plane3d trianglePlane = triangle.getPlane();

		double t1, t0;
		bool embeddedInPlane = false;

		double signedDistToTrianglePlane = trianglePlane.getDistanceTo(
			colData->basePoint);
		if(signedDistToTrianglePlane < 0)
			return false;

		double normalDotVelocity = trianglePlane.Normal * colData->velocity;

		if ( iszero ( normalDotVelocity ) )
		{
			if (fabs(signedDistToTrianglePlane) >= 1.0f)
				return false; // no collision possible
			else
			{
				embeddedInPlane = true;
				t0 = 0.0;
				t1 = 1.0;
			}
		}
		else
		{
			// N.D is not 0. Calculate intersection interval
			t0 = (-1.f - signedDistToTrianglePlane) / normalDotVelocity;
			t1 = (1.f - signedDistToTrianglePlane) / normalDotVelocity;

			// Swap so t0 < t1
			if (t0 > t1) { double tmp = t1; t1 = t0; t0 = tmp;	}

			// check if at least one value is within the range
			if (t0 > 1.0f || t1 < 0.0f)
				return false; // both t values are outside 1 and 0, no collision possible

			// clamp to 0 and 1
			t0 = clamp ( t0, 0.f, 1.f );
			t1 = clamp ( t1, 0.f, 1.f );
		}

		// at this point we have t0 and t1, if there is any intersection, it
		// is between this interval
		osg::Vec3d collisionPoint;
		bool foundCollision = false;
		double t = 1.0f;

		// first check the easy case: Collision within the triangle;
		// if this happens, it must be at t0 and this is when the sphere
		// rests on the front side of the triangle plane. This can only happen
		// if the sphere is not embedded in the triangle plane.
		if (!embeddedInPlane)
		{
			osg::Vec3d planeIntersectionPoint =
				(colData->basePoint - trianglePlane.Normal)
				+ (colData->velocity * t0);

			if (triangle.isPointInside(planeIntersectionPoint))
			{
				foundCollision = true;
				t = t0;
				collisionPoint = planeIntersectionPoint;
			}
		}

		// if we havent found a collision already we will have to sweep
		// the sphere against points and edges of the triangle. Note: A
		// collision inside the triangle will always happen before a
		// vertex or edge collision.
		if (!foundCollision)
		{
			osg::Vec3d velocity = colData->velocity;
			osg::Vec3d base = colData->basePoint;

			double velocitySqaredLength = velocity.length2();
			double a,b,c;
			double newT;

			// for each edge or vertex a quadratic equation has to be solved:
			// a*t^2 + b*t + c = 0. We calculate a,b, and c for each test.

			// check against points
			a = velocitySqaredLength;

			// p1
			b = 2.0f * (velocity * (base - triangle.PointA));
			c = (triangle.PointA - base).length2() - 1.f;
			if (getLowestRoot(a,b,c,t, &newT))
			{
				t = newT;
				foundCollision = true;
				collisionPoint = triangle.PointA;
			}

			// p2
			if (!foundCollision)
			{
				b = 2.0f * (velocity * (base - triangle.PointB));
				c = (triangle.PointB - base).length2() - 1.f;
				if (getLowestRoot(a,b,c,t, &newT))
				{
					t = newT;
					foundCollision = true;
					collisionPoint = triangle.PointB;
				}
			}

			// p3
			if (!foundCollision)
			{
				b = 2.0f * (velocity * (base - triangle.PointC));
				c = (triangle.PointC-base).length2() - 1.f;
				if (getLowestRoot(a,b,c,t, &newT))
				{
					t = newT;
					foundCollision = true;
					collisionPoint = triangle.PointC;
				}
			}

			// check against edges:

			// p1 --- p2
			osg::Vec3d edge = triangle.PointB - triangle.PointA;
			osg::Vec3d baseToVertex = triangle.PointA - base;
			double edgeSqaredLength = edge.length2();
			double edgeDotVelocity = edge * velocity;
			double edgeDotBaseToVertex = edge * baseToVertex;

			// calculate parameters for equation
			a = edgeSqaredLength * -velocitySqaredLength +
				edgeDotVelocity*edgeDotVelocity;
			b = edgeSqaredLength * (2.f * (velocity * baseToVertex)) -
				2.0f * edgeDotVelocity * edgeDotBaseToVertex;
			c = edgeSqaredLength * (1.f - baseToVertex.length2()) +
				edgeDotBaseToVertex * edgeDotBaseToVertex;

			// does the swept sphere collide against infinite edge?
			if (getLowestRoot(a,b,c,t,&newT))
			{
				double f = (edgeDotVelocity * newT - edgeDotBaseToVertex) / edgeSqaredLength;
				if (f >= 0.0f && f <= 1.0f)
				{
					// intersection took place within segment
					t = newT;
					foundCollision = true;
					collisionPoint = triangle.PointA + (edge*f);
				}
			}

			// p2 --- p3
			edge = triangle.PointC-triangle.PointB;
			baseToVertex = triangle.PointB - base;
			edgeSqaredLength = edge.length2();
			edgeDotVelocity = edge * velocity;
			edgeDotBaseToVertex = edge * baseToVertex;

			// calculate parameters for equation
			a = edgeSqaredLength * -velocitySqaredLength +
				edgeDotVelocity * edgeDotVelocity;
			b = edgeSqaredLength * (2 * (velocity * baseToVertex)) -
				2.0f * edgeDotVelocity * edgeDotBaseToVertex;
			c = edgeSqaredLength * (1 - baseToVertex.length2()) +
				edgeDotBaseToVertex * edgeDotBaseToVertex;

			// does the swept sphere collide against infinite edge?
			if (getLowestRoot(a,b,c,t,&newT))
			{
				double f = (edgeDotVelocity * newT - edgeDotBaseToVertex) /
					edgeSqaredLength;
				if (f >= 0.0f && f <= 1.0f)
				{
					// intersection took place within segment
					t = newT;
					foundCollision = true;
					collisionPoint = triangle.PointB + (edge*f);
				}
			}

			// p3 --- p1
			edge = triangle.PointA - triangle.PointC;
			baseToVertex = triangle.PointC - base;
			edgeSqaredLength = edge.length2();
			edgeDotVelocity = edge * velocity;
			edgeDotBaseToVertex = edge * baseToVertex;

			// calculate parameters for equation
			a = edgeSqaredLength * -velocitySqaredLength +
				edgeDotVelocity * edgeDotVelocity;
			b = edgeSqaredLength * (2 * (velocity * baseToVertex)) -
				2.0f * edgeDotVelocity * edgeDotBaseToVertex;
			c = edgeSqaredLength * (1 - baseToVertex.length2()) +
				edgeDotBaseToVertex * edgeDotBaseToVertex;

			// does the swept sphere collide against infinite edge?
			if (getLowestRoot(a,b,c,t,&newT))
			{
				double f = (edgeDotVelocity * newT - edgeDotBaseToVertex) /
					edgeSqaredLength;
				if (f >= 0.0f && f <= 1.0f)
				{
					// intersection took place within segment
					t = newT;
					foundCollision = true;
					collisionPoint = triangle.PointC + (edge*f);
				}
			}
		}// end no collision found

		// set result:
		if (foundCollision)
		{
			// distance to collision is t
			double distToCollision = t * colData->velocity.length();
			// does this triangle qualify for closest hit?
			if (!colData->foundCollision ||
				distToCollision	< colData->nearestDistance)
			{
				colData->nearestDistance = distToCollision;
				colData->intersectionPoint = collisionPoint;
				colData->foundCollision = true;
				colData->intersectionTriangle = triangle;
				++colData->triangleHits;
				return true;
			}
		}// end found collision

		return false;
	}

	osg::Vec3d CSceneCollisionManager::getCollisionResultPositionWithModel( COctreeTriangleSelector* selector, const osg::Vec3d& position, const osg::Vec3d& radius, const osg::Vec3d& direction, triangle3d& triout, osg::Vec3d& hitPosition, bool& outFalling, double slidingSpeed, const osg::Vec3d& gravity, bool& foundCollision )
	{
		return collideEllipsoidWithModel(selector, position,
			radius, direction, slidingSpeed, gravity, triout, hitPosition, outFalling,foundCollision);
	}

	osg::Vec3d CSceneCollisionManager::collideEllipsoidWithModel( COctreeTriangleSelector* selector, const osg::Vec3d& position, const osg::Vec3d& radius, const osg::Vec3d& velocity, double slidingSpeed, const osg::Vec3d& gravity, triangle3d& triout, osg::Vec3d& hitPosition, bool& outFalling, bool& foundCollision )
	{
		if (!selector || radius.x() == 0.0f || radius.y() == 0.0f || radius.z() == 0.0f)
			return position;

		SCollisionData colData;
		colData.R3Position = position;
		colData.R3Velocity = velocity;
		colData.eRadius = radius;
		colData.nearestDistance = FLT_MAX;
		colData.selector = selector;
		colData.slidingSpeed = slidingSpeed;
		colData.triangleHits = 0;
		colData.triangleIndex = -1;

		osg::Vec3d eSpacePosition = osg::Vec3d(colData.R3Position.x() / colData.eRadius.x(),
			colData.R3Position.y() / colData.eRadius.y(),
			colData.R3Position.z() / colData.eRadius.z());
		osg::Vec3d eSpaceVelocity = osg::Vec3d(colData.R3Velocity.x() / colData.eRadius.x(),
			colData.R3Velocity.y() / colData.eRadius.y(),
			colData.R3Velocity.z() / colData.eRadius.z());

		osg::Vec3d finalPos = collideWithModel(
			0, colData, eSpacePosition, eSpaceVelocity,foundCollision);

		outFalling = false;

		if (gravity != osg::Vec3d(0,0,0))
		{
			colData.R3Position = osg::Vec3d(finalPos.x() * colData.eRadius.x(),
				finalPos.y() * colData.eRadius.y(),
				finalPos.z() * colData.eRadius.z());
			colData.R3Velocity = gravity;
			colData.triangleHits = 0;

			eSpaceVelocity = osg::Vec3d(gravity.x() / colData.eRadius.x(),
				gravity.y() / colData.eRadius.y(),
				gravity.z() / colData.eRadius.z()); 

			finalPos = collideWithModel(0, colData,
				finalPos, eSpaceVelocity,foundCollision);

			outFalling = (colData.triangleHits == 0);
		}

		finalPos = osg::Vec3d(finalPos.x() * colData.eRadius.x(),
			finalPos.y() * colData.eRadius.y(),
			finalPos.z() * colData.eRadius.z());
		hitPosition = osg::Vec3d(colData.intersectionPoint.x() * colData.eRadius.x(),
			colData.intersectionPoint.y() * colData.eRadius.y(),
			colData.intersectionPoint.z() * colData.eRadius.z()); 

		return finalPos;
	}

	osg::Vec3d CSceneCollisionManager::collideWithModel( double recursionDepth, SCollisionData &colData, osg::Vec3d pos, osg::Vec3d vel, bool& foundCollision )
	{
		double veryCloseDistance = colData.slidingSpeed;

		if (recursionDepth > 5)
			return pos;

		colData.velocity = vel;
		colData.normalizedVelocity = vel;
		colData.normalizedVelocity.normalize();
		colData.basePoint = pos;
		colData.foundCollision = false;
		colData.nearestDistance = FLT_MAX;

		aabbox3d box(colData.R3Position);
		box.addInternalPoint(colData.R3Position + colData.R3Velocity);
		box.MinEdge -= colData.eRadius;
		box.MaxEdge += colData.eRadius;

		int totalTriangleCnt = colData.selector->GetTriangleCount();
		list<triangle3d> triangleList;

		osg::Matrixd scaleMatrix;
		scaleMatrix.makeScale(1.0f / colData.eRadius.x(),1.0f / colData.eRadius.y(),1.0f / colData.eRadius.z());

		int triangleCnt = 0;
		colData.selector->GetTrianglesFromOctree (colData.selector->Root,triangleCnt,totalTriangleCnt,box,scaleMatrix,triangleList);

		for (list<triangle3d>::iterator it = triangleList.begin();it != triangleList.end();it++)
			testTriangleIntersectionWithModel(&colData, (*it));

		foundCollision = colData.foundCollision;

		if (!colData.foundCollision)
		{
			return pos + vel;
		}	

		// original destination point
		const osg::Vec3d destinationPoint = pos + vel;
		osg::Vec3d newBasePoint = pos;

		// only update if we are not already very close
		// and if so only move very close to intersection, not to the
		// exact point
		if (colData.nearestDistance >= veryCloseDistance)
		{
			osg::Vec3d v = vel;
			v.normalize();
			v *= colData.nearestDistance - veryCloseDistance;
			newBasePoint = colData.basePoint + v;

			v.normalize();
			colData.intersectionPoint -= (v * veryCloseDistance);
		}

		// calculate sliding plane

		const osg::Vec3d slidePlaneOrigin = colData.intersectionPoint;
		osg::Vec3d temp = newBasePoint - colData.intersectionPoint;
		temp.normalize();
		const osg::Vec3d slidePlaneNormal = temp;

		plane3d slidingPlane(slidePlaneOrigin, slidePlaneNormal);

		osg::Vec3d newDestinationPoint =
			destinationPoint -
			(slidePlaneNormal * slidingPlane.getDistanceTo(destinationPoint));

		// generate slide vector

		const osg::Vec3d newVelocityVector = newDestinationPoint -
			colData.intersectionPoint;

		if (newVelocityVector.length() < veryCloseDistance)
			return newBasePoint;

		return collideWithModel(recursionDepth+1, colData,
			newBasePoint, newVelocityVector,foundCollision);
	}

	bool CSceneCollisionManager::testTriangleIntersectionWithModel( SCollisionData* colData, triangle3d& triangle )
	{
		const plane3d testTrianglePlane = triangle.getPlane();

		if (!testTrianglePlane.isFrontFacing(colData->normalizedVelocity) )
		{
			triangle = triangle3d(triangle.PointA,triangle.PointC,triangle.PointB);
		}

		const plane3d trianglePlane = triangle.getPlane();

		double t1, t0;
		bool embeddedInPlane = false;

		double signedDistToTrianglePlane = trianglePlane.getDistanceTo(
			colData->basePoint);
		if(signedDistToTrianglePlane < 0)
			return false;

		double normalDotVelocity = trianglePlane.Normal * colData->velocity;

		if ( iszero ( normalDotVelocity ) )
		{
			if (fabs(signedDistToTrianglePlane) >= 1.0f)
				return false; // no collision possible
			else
			{
				embeddedInPlane = true;
				t0 = 0.0;
				t1 = 1.0;
			}
		}
		else
		{
			// N.D is not 0. Calculate intersection interval
			t0 = (-1.f - signedDistToTrianglePlane) / normalDotVelocity;
			t1 = (1.f - signedDistToTrianglePlane) / normalDotVelocity;

			// Swap so t0 < t1
			if (t0 > t1) { double tmp = t1; t1 = t0; t0 = tmp;	}

			// check if at least one value is within the range
			if (t0 > 1.0f || t1 < 0.0f)
				return false; // both t values are outside 1 and 0, no collision possible

			// clamp to 0 and 1
			t0 = clamp ( t0, 0.f, 1.f );
			t1 = clamp ( t1, 0.f, 1.f );
		}

		// at this point we have t0 and t1, if there is any intersection, it
		// is between this interval
		osg::Vec3d collisionPoint;
		bool foundCollision = false;
		double t = 1.0f;

		// first check the easy case: Collision within the triangle;
		// if this happens, it must be at t0 and this is when the sphere
		// rests on the front side of the triangle plane. This can only happen
		// if the sphere is not embedded in the triangle plane.
		if (!embeddedInPlane)
		{
			osg::Vec3d planeIntersectionPoint =
				(colData->basePoint - trianglePlane.Normal)
				+ (colData->velocity * t0);

			if (triangle.isPointInside(planeIntersectionPoint))
			{
				foundCollision = true;
				t = t0;
				collisionPoint = planeIntersectionPoint;
			}
		}

		// if we havent found a collision already we will have to sweep
		// the sphere against points and edges of the triangle. Note: A
		// collision inside the triangle will always happen before a
		// vertex or edge collision.
		if (!foundCollision)
		{
			osg::Vec3d velocity = colData->velocity;
			osg::Vec3d base = colData->basePoint;

			double velocitySqaredLength = velocity.length2();
			double a,b,c;
			double newT;

			// for each edge or vertex a quadratic equation has to be solved:
			// a*t^2 + b*t + c = 0. We calculate a,b, and c for each test.

			// check against points
			a = velocitySqaredLength;

			// p1
			b = 2.0f * (velocity * (base - triangle.PointA));
			c = (triangle.PointA - base).length2() - 1.f;
			if (getLowestRoot(a,b,c,t, &newT))
			{
				t = newT;
				foundCollision = true;
				collisionPoint = triangle.PointA;
			}

			// p2
			if (!foundCollision)
			{
				b = 2.0f * (velocity * (base - triangle.PointB));
				c = (triangle.PointB - base).length2() - 1.f;
				if (getLowestRoot(a,b,c,t, &newT))
				{
					t = newT;
					foundCollision = true;
					collisionPoint = triangle.PointB;
				}
			}

			// p3
			if (!foundCollision)
			{
				b = 2.0f * (velocity * (base - triangle.PointC));
				c = (triangle.PointC-base).length2() - 1.f;
				if (getLowestRoot(a,b,c,t, &newT))
				{
					t = newT;
					foundCollision = true;
					collisionPoint = triangle.PointC;
				}
			}

			// check against edges:

			// p1 --- p2
			osg::Vec3d edge = triangle.PointB - triangle.PointA;
			osg::Vec3d baseToVertex = triangle.PointA - base;
			double edgeSqaredLength = edge.length2();
			double edgeDotVelocity = edge * velocity;
			double edgeDotBaseToVertex = edge * baseToVertex;

			// calculate parameters for equation
			a = edgeSqaredLength * -velocitySqaredLength +
				edgeDotVelocity*edgeDotVelocity;
			b = edgeSqaredLength * (2.f * (velocity * baseToVertex)) -
				2.0f * edgeDotVelocity * edgeDotBaseToVertex;
			c = edgeSqaredLength * (1.f - baseToVertex.length2()) +
				edgeDotBaseToVertex * edgeDotBaseToVertex;

			// does the swept sphere collide against infinite edge?
			if (getLowestRoot(a,b,c,t,&newT))
			{
				double f = (edgeDotVelocity * newT - edgeDotBaseToVertex) / edgeSqaredLength;
				if (f >= 0.0f && f <= 1.0f)
				{
					// intersection took place within segment
					t = newT;
					foundCollision = true;
					collisionPoint = triangle.PointA + (edge*f);
				}
			}

			// p2 --- p3
			edge = triangle.PointC-triangle.PointB;
			baseToVertex = triangle.PointB - base;
			edgeSqaredLength = edge.length2();
			edgeDotVelocity = edge * velocity;
			edgeDotBaseToVertex = edge * baseToVertex;

			// calculate parameters for equation
			a = edgeSqaredLength * -velocitySqaredLength +
				edgeDotVelocity * edgeDotVelocity;
			b = edgeSqaredLength * (2 * (velocity * baseToVertex)) -
				2.0f * edgeDotVelocity * edgeDotBaseToVertex;
			c = edgeSqaredLength * (1 - baseToVertex.length2()) +
				edgeDotBaseToVertex * edgeDotBaseToVertex;

			// does the swept sphere collide against infinite edge?
			if (getLowestRoot(a,b,c,t,&newT))
			{
				double f = (edgeDotVelocity * newT - edgeDotBaseToVertex) /
					edgeSqaredLength;
				if (f >= 0.0f && f <= 1.0f)
				{
					// intersection took place within segment
					t = newT;
					foundCollision = true;
					collisionPoint = triangle.PointB + (edge*f);
				}
			}

			// p3 --- p1
			edge = triangle.PointA - triangle.PointC;
			baseToVertex = triangle.PointC - base;
			edgeSqaredLength = edge.length2();
			edgeDotVelocity = edge * velocity;
			edgeDotBaseToVertex = edge * baseToVertex;

			// calculate parameters for equation
			a = edgeSqaredLength * -velocitySqaredLength +
				edgeDotVelocity * edgeDotVelocity;
			b = edgeSqaredLength * (2 * (velocity * baseToVertex)) -
				2.0f * edgeDotVelocity * edgeDotBaseToVertex;
			c = edgeSqaredLength * (1 - baseToVertex.length2()) +
				edgeDotBaseToVertex * edgeDotBaseToVertex;

			// does the swept sphere collide against infinite edge?
			if (getLowestRoot(a,b,c,t,&newT))
			{
				double f = (edgeDotVelocity * newT - edgeDotBaseToVertex) /
					edgeSqaredLength;
				if (f >= 0.0f && f <= 1.0f)
				{
					// intersection took place within segment
					t = newT;
					foundCollision = true;
					collisionPoint = triangle.PointC + (edge*f);
				}
			}
		}// end no collision found

		// set result:
		if (foundCollision)
		{
			// distance to collision is t
			double distToCollision = t * colData->velocity.length();
			// does this triangle qualify for closest hit?
			if (!colData->foundCollision ||
				distToCollision	< colData->nearestDistance)
			{
				colData->nearestDistance = distToCollision;
				colData->intersectionPoint = collisionPoint;
				colData->foundCollision = true;
				colData->intersectionTriangle = triangle;
				++colData->triangleHits;
				return true;
			}
		}// end found collision

		return false;
	}

	COctreeTriangleSelector::MyGeometryVisitor::MyGeometryVisitor( DoubleTriangleFunctor<ModelTriangleFunctor>* tf )
		:m_triangleFunctor(tf)
	{
		setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
	}

	void COctreeTriangleSelector::MyGeometryVisitor::apply( osg::Geometry& geometry )
	{
		if(m_triangleFunctor)
		{
			osg::MatrixList ml = geometry.getWorldMatrices();
			osg::Matrix m;
			m.makeIdentity();

			if(ml.size() > 0)
			   m = ml[0];

			m_triangleFunctor->setWorldMat(m);
			geometry.accept(*m_triangleFunctor);
		}
	}

}



