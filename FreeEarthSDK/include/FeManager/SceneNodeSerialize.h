/**************************************************************************************************
* @file SceneNodeSerialize.h
* @note 定义了场景中对象的保存和读取方法
* @author 00008
* @date 2015.10.30
**************************************************************************************************/

#ifndef HG_SCENE_EX_NODE_SERILIZE_H
#define HG_SCENE_EX_NODE_SERILIZE_H

#include <FeExtNode/ExternNode.h>
#include <FeExtNode/ExPlaceNode.h>
#include <FeExtNode/ExBillBoardNode.h>
#include <FeExtNode/ExOverLayNode.h>
#include <FeExtNode/ExTextNode.h>
#include <FeExtNode/ExLineNode.h>
#include <FeExtNode/ExPolygonNode.h>
#include <FeExtNode/ExSectorNode.h>
#include <FeExtNode/ExEllipseNode.h>
#include <FeExtNode/ExRectNode.h>
#include <FeExtNode/ExLodModelNode.h>
#include <FeExtNode/ExTiltModelNode.h>
#include <FeExtNode/ExLabelNode.h>
#include <FeExtNode/ExParticleNode.h>
//#include <FeExtNode/ExAssaultNode.h>
//#include <FeExtNode/ExAttackNode.h>

#include <FePlots/ExternPlotsVisitor.h>
#include <FePlots/StraightArrow.h>
#include <FePlots/StraightMoreArrow.h>
#include <FePlots/DovetailDiagonalArrow.h>
#include <FePlots/DovetailDiagonalMoreArrow.h>
#include <FePlots/DoveTailStraightArrow.h>
#include <FePlots/DoveTailStraightMoreArrow.h>
#include <FePlots/BezierCurveArrow.h>
#include <FePlots/CloseCurve.h>
#include <FePlots/DiagonalArrow.h>
#include <FePlots/DiagonalMoreArrow.h>
#include <FePlots/DoubleArrow.h>
//#include <FePlots/RectFlag.h>
//#include <FePlots/CurveFlag.h>
//#include <FePlots/TriangleFlag.h>
#include <FePlots/RoundedRect.h>
#include <FePlots/GatheringPlace.h>
#include <FePlots/PolylineArrow.h>
#include <FePlots/SectorSearch.h>
#include <FePlots/CardinalCurveArrow.h>
#include <FePlots/BezierCurveArrow.h>
#include <FePlots/ParallelSearch.h>
#include <FePlots/StraightLineArrow.h>
//#include <FePlots/Freeline.h>
//#include <FePlots/FreePolygon.h>
#include <FeUtils/tinyXml/tinyxml.h>
#include <FeUtils/tinyXml/tinyutil.h>
#include <FeUtils/tinyXml/tinystr.h>

#include <FeUtils/FreeViewPoint.h>

#include <FeManager/Export.h>
#include <FeUtils/SceneSysCall.h>
#include <osgEarth/MapNode>
#include <osg/Vec3d>

namespace FeManager
{
	class CFreeMarkSys;

	//const unsigned int MARK_ROOT_ID = 000000001;
	const unsigned int MARK_ROOT_ID = 000000000;
	const std::string MARK_ROOT_TITLE = "MyMarks";

	/**
    * @class CMarkDeserialization
    * @brief 场景标反序列化,将保存在xml文件中的信息反序列化为场景节点
    * @note 提供了场景标记反序列化接口
    * @author 00008
    */
	class FEMANAGER_EXPORT CMarkDeserialization : public FeUtil::TiXmlVisitor
	{
	public:
		CMarkDeserialization();
		CMarkDeserialization(FeUtil::CRenderContext* pContext, CFreeMarkSys* pMarkSys);
		virtual ~CMarkDeserialization();

	public:
		/**  
		  * @brief 反序列换接口
		  * @note 将xml序列化文件转化为场景中的节点  
		  * @param pExternNode [inout] 默认的场景标记父节点，由系统开始创建
		  * @param strFilePath [in] xml序列化文件路径
		  * @return 返回值
		*/
		bool Execute(FeExtNode::CExComposeNode* pRootNode, const std::string& strFilePath);

	public:
		///进入xml节点
		virtual bool VisitEnter(const FeUtil::TiXmlElement& ele , const FeUtil::TiXmlAttribute* attr);

		///离开xml节点
		virtual bool VisitExit(const FeUtil::TiXmlElement& ele );

	protected:
		// 标记
		void EnterFolderMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void ExitFolderMark(const FeUtil::TiXmlElement& ele );
		void EnterLineMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterPolygonMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterBillboardMark(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr);
		void EnterOverLayMark(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr);
		void EnterPointMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterLodModelMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterTiltModelMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterSectorMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterArcMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterCircleMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterEllipseMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterTextMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterRectMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		//void EnterStraightArrowMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		//void EnterAssaultMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		//void EnterAttackMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterParticleMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterLabelMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterLabelItem( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );

		//军标
		void EnterStraightArrow( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterStraightMoreArrow( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterDovetailDiagonalArrow( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterDovetailDiagonalMoreArrow( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterDoveTailStraightArrow( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterDoveTailStraightMoreArrow( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterDiagonalArrow( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterDiagonalMoreArrow( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterDoubleArrow( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		//void EnterRectFlag( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		//void EnterCurveFlag( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		//void EnterTriangleFlag( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterRoundedRect( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterGatheringPlace( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterCloseCurve( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr );
		void EnterBezierCurveArrow(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr);
		void EnterPolyLineArrow(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr);
		void EnterParallelSearch(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr);
		void EnterSectorSearch(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr);
		void EnterCardinalCurveArrow(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr);
		void EnterStraightLineArrow(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr);
		//void EnterFreeLine(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr);
		//void EnterFreePolygon(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr);

		osg::Vec3d ParseOffset(const FeUtil::TiXmlElement& element);
		osg::Vec3d ParsePosition(const FeUtil::TiXmlElement& element);
		osg::Vec3d ParseAngle(const FeUtil::TiXmlElement& element);
		osg::Vec3d ParsePosture( const FeUtil::TiXmlElement& element );
		osg::Vec3d ParseRScale( const FeUtil::TiXmlElement& element );
		osg::Vec4d ParseColor( const FeUtil::TiXmlElement& ele);
		osg::Vec4d ParseFillColor( const FeUtil::TiXmlElement& ele);
		std::vector<osg::Vec3d> ParseVertex( const FeUtil::TiXmlElement& ele);
		FeUtil::CFreeViewPoint ParseViewPoint(const FeUtil::TiXmlElement& element);
		void ParseLodModel(const FeUtil::TiXmlElement& ele, const std::string& eleName, 
			std::string& path, double& min, double& max);

	protected:
		typedef std::vector< osg::ref_ptr<FeExtNode::CExComposeNode> >  MarkGroupList;
		MarkGroupList			m_markList; //作为一个保存父节点的列表，只是一个临时的容器

		osg::observer_ptr<FeManager::CFreeMarkSys>  m_opMarkSys;
		/// 因为反序列化采用深度遍历， 所以使用栈结构保存 LabelNode 当前父节点
		std::stack< std::pair< int ,osg::ref_ptr<FeExtNode::CExLabelItemNode> > > m_stackLabel;
		std::pair< int ,osg::ref_ptr<FeExtNode::CExLabelNode> > m_rpLabelNodeParent;  // 应该将 CExLabelNode 重构为 从 CExLabelItemNode 继承！！！

		osg::observer_ptr<FeUtil::CRenderContext>	m_opRenderContext;
	};



    /**
    * @class CMarkSerialization
    * @brief 场景标记序列化，将场景中的节点序列化为xml描述文件
    * @note 提供了场景标记序列化接口
    * @author 00008
    */
	class FEMANAGER_EXPORT CMarkSerialization : public FeExtNode::CExternNodeVisitor, public FePlots::CExternPlotsVisitor
	{
	public:
		CMarkSerialization();
		virtual ~CMarkSerialization();

	public:
		/**  
		  * @brief 序列化
		  * @note 将场景中的节点序列化成为xml文件保存
		  * @param pExternNode [in] 需要序列化的场景Root节点
		  * @param strFilePath [in] 保存序列化xml的路径
		  * @return 返回值
		*/
		bool Execute(FeExtNode::CExComposeNode* pExternNode, const std::string& strFilePath);

		// 标记
		virtual bool VisitEnter(FeExtNode::CExComposeNode& externNode);
		virtual bool VisitExit(FeExtNode::CExComposeNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExPlaceNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExLodModelNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExTiltModelNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExLineNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExPolygonNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExBillBoardNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExOverLayNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExSectorNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExArcNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExCircleNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExEllipseNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExTextNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExRectNode& externNode);
		//virtual bool VisitEnter(FeExtNode::CExStraightArrowNode& externNode);
		//virtual bool VisitEnter(FeExtNode::CExAssaultNode& externNode);
		//virtual bool VisitEnter(FeExtNode::CExAttackNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExLabelNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExParticleNode& externNode);

		// 军标
		virtual bool VisitEnter(FePlots::CStraightArrow& externNode);
		virtual bool VisitEnter(FePlots::CStraightMoreArrow& externNode);
		virtual bool VisitEnter(FePlots::CDovetailDiagonalArrow& externNode);
		virtual bool VisitEnter(FePlots::CDovetailDiagonalMoreArrow& externNode);
		virtual bool VisitEnter(FePlots::CDoveTailStraightArrow& externNode);
		virtual bool VisitEnter(FePlots::CDoveTailStraightMoreArrow& externNode);
		virtual bool VisitEnter(FePlots::CDiagonalArrow& externNode);
		virtual bool VisitEnter(FePlots::CDiagonalMoreArrow& externNode);
		virtual bool VisitEnter(FePlots::CDoubleArrow& externNode);
		//virtual bool VisitEnter(FePlots::CRectFlag& externNode);
		//virtual bool VisitEnter(FePlots::CCurveFlag& externNode);
		//virtual bool VisitEnter(FePlots::CTriangleFlag& externNode);
		virtual bool VisitEnter(FePlots::CRoundedRect& externNode);
		virtual bool VisitEnter(FePlots::CGatheringPlace& externNode);
		virtual bool VisitEnter(FePlots::CCloseCurve& externNode);
		virtual bool VisitEnter(FePlots::CBezierCurveArrow& externNode);
		virtual bool VisitEnter(FePlots::CPolyLineArrow& externNode);
		virtual bool VisitEnter(FePlots::CParallelSearch& externNode);
		virtual bool VisitEnter(FePlots::CSectorSearch& externNode);
		virtual bool VisitEnter(FePlots::CCardinalCurveArrow& externNode);
		virtual bool VisitEnter(FePlots::CStraightLineArrow& externNode);
		//virtual bool VisitEnter(FePlots::CFreeLine& externNode);
		//virtual bool VisitEnter(FePlots::CFreePolygon& externNode);

	protected:
		void LinkLabelItem(FeUtil::TiXmlElement* pElementParent, FeExtNode::CExLabelItemNode* pNode, int nParentID);

		FeUtil::TiXmlElement* BuildViewPointElement(const FeUtil::CFreeViewPoint& viewPoint);
		FeUtil::TiXmlElement* BuildAngleElement(const osg::Vec3d& value);
		FeUtil::TiXmlElement* BuildMarkOffsetElement(const osg::Vec3d& value);
		FeUtil::TiXmlElement* BuildMarkPositionElement(const osg::Vec3d& value);
		FeUtil::TiXmlElement* BuildMarkPostureElement(const osg::Vec3d& value);
		FeUtil::TiXmlElement* BuildMarkScaleElement(const osg::Vec3d& value);
		FeUtil::TiXmlElement* BuildLineColor(const osg::Vec4d& veccolor);
		FeUtil::TiXmlElement* BuildFillColor(const osg::Vec4d& veccolor);

		FeUtil::TiXmlElement* BuildLodNodeElement(FeExtNode::CExLodModelNode::LodNodeInfo& nodeInfo, const std::string& strName);

		void BuildVertex(FeUtil::TiXmlElement* pParentElement,std::vector<osg::Vec3d> vecVertexs);
		

	protected:
		typedef std::vector< FeUtil::TiXmlElement* >  MarkGroupList;
		MarkGroupList			m_markList; //作为一个保存父节点的列表，只是一个临时的容器
	};

}

#endif
