/**************************************************************************************************
* @file ContourMeasure.h
* @note 等高线
* @author g00037
* @data 2017-9-13
**************************************************************************************************/
#ifndef FE_CONTOUR_MEASURE_H
#define FE_CONTOUR_MEASURE_H

#include <FeMeasure/Export.h>
#include <FeMeasure/Measure.h>
#include <osgEarthDrivers/engine_mp/TileModel>
#include <osgEarthDrivers/engine_mp/TileNode>
namespace FeMeasure
{
	struct SPointData
	{
		osg::Vec3d vecLLH;
		int nRow;
		int nColumn;	
		int nLevel;
		SPointData()
		{
			vecLLH = osg::Vec3d(0.0,0.0,0.0);
			nRow = 0;
			nColumn = 0;
			nLevel = 0;
		}
	};
	struct SLineData
	{
		int nLevel;
		int nAX1;
		int nAY1;
		int nAX2;
		int nAY2;
		int nBX1;
		int nBY1;
		int nBX2;
		int nBY2;
		osg::Vec3d vecALLH;
		osg::Vec3d vecBLLH;
		SLineData()
		{
			nLevel = 0;
			nAX1 = 0;
			nAY1 = 0;
			nAX2 = 0;
			nAY2 = 0;
			nBX1 = 0;
			nBY1 = 0;
			nBX2 = 0;
			nBY2 = 0;
			vecALLH = osg::Vec3d(0.0,0.0,0.0);
			vecBLLH = osg::Vec3d(0.0,0.0,0.0);
		}
	};

	/**
	  * @class CContourResult
	  * @note 等高线结果类
	  * @author g00037
	*/
	class FEMEASURE_EXPORT CContourResult : public CMeasureResult
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CContourResult(FeUtil::CRenderContext* pContext);

		~CContourResult();

		void UpdateContourNode(bool bUpdateTile);

		void SetContourInterval(int nInterval);

		int GetContourInterval();

		void SetRow(int nRow);

		int GetRow();

		void SetColumn(int nColumn);

		int GetColumn();

		void Stop();

		int GetMaxContourValue();

		int GetMinContourValue();

	protected:
		virtual void run();
	private:
		/**
		*@note: 根据输入点计算测量结果
		*/
		virtual bool CalculateImplement(const osg::Vec3dArray& vecPointsLLH);

		/**
		*@note: 计算最终结果。部分测量为了提高效率，不会实时更新，而只有在测量结束时才显示结果
		*/
		virtual bool FinalCalculateImplement();

		/**
		*@note: 创建计算结果数据对象
		*/
		virtual CMeasureResultData* CreateResultData(){ return NULL; }

		bool ComputePointData();

		bool ComputevecTileNode();

		bool ComputePointDataLevel();

		bool CreatContourNode();

		std::map<int,std::list<SLineData*>> GetLineDataByPointData(std::vector<SPointData*> vecPointData);

		void PushLineData(int nCount,SPointData* pA1Data,SPointData* pA2Data,SPointData* pB1Data,SPointData* pB2Data,osg::Vec3d vecALLH,osg::Vec3d vecBLLH,std::map<int,std::list<SLineData*>>& mapLineData);

		bool Check(SLineData* pLineData,std::list<SLineData*>& listLineData,osg::ref_ptr<osgEarth::Symbology::LineString> rpLine,int nPos,int &nCount);

		void CreatText(osg::Vec3d vecLLH,int nLevel);

		int Min(int nA, int nB);

		int Max(int nA, int nB);

	private:	
		osgEarth::MapNode* m_pMapNode; 
		const osgEarth::SpatialReference* m_pSrs;
		osgEarth::ElevationLayerVector m_Elv;
		osgEarth::Map* m_pMap;
		osg::ref_ptr<osgEarth::Annotation::FeatureNode> m_rpBorderNode;
		osg::ref_ptr<osgEarth::Annotation::FeatureNode> m_rpContourNode;
		osg::ref_ptr<osg::Group> m_rpTextGroup;
		std::vector<SPointData*> m_vecPointData;
		std::vector<osgEarth::GeoHeightField> m_vecTileNode;	
		osg::Vec3d m_vecStart;
		osg::Vec3d m_vecEnd;
		osg::Vec3d m_vecLU;
		osg::Vec3d m_vecRU;
		osg::Vec3d m_vecRD;
		osg::Vec3d m_vecLD;
		int m_nInterval;
		int m_nTextInterval;
		double m_dMinX;
		double m_dMaxX;
		double m_dMinY;
		double m_dMaxY;
		int m_nColumns;
		int m_nRows;
		osgEarth::Style m_LabelStyle;
		osgEarth::Style m_Linestyle;
		bool            m_bUpdateTile;
		bool            m_bRunning;

		//等值线最大 最小值
		int				nMaxContourValue;
		int				nMinContourValue;
	};
}

namespace FeMeasure
{
	/**
	  * @class CContourMeasure
	  * @note 等高线分析类
	  * @author g00037
	*/
	class FEMEASURE_EXPORT CContourMeasure : public CMeasure
	{
		friend class CContourFactory;

	public:
		void UpdateContour(int nInterval,int nRow,int nColumn,bool bUpdateTile);

		int GetContourInterval();

		int GetContourRow();

		int GetContourColumn();

		void CancelContour();
	private:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CContourMeasure(FeUtil::CRenderContext* pContext);

		~CContourMeasure();

	private: 
		/**
		  *@note: 创建测量结果对象
		*/
		virtual CMeasureResult* CreateMeasureResult(){ return new CContourResult(GetRenderContext()); }

		/**
		*@note: 添加控制点具体实现
		*/
		virtual void AddCtrlPointImplement(const osg::Vec3d& vecPosLLH);
		
		/**
		*@note: 设置控制点集具体实现
		*/
		virtual void SetCtrlPointsImplement(const osg::Vec3dArray& vecPointsLLH);
	};
}
#endif //FE_FILL_CUT_MEASURE_H