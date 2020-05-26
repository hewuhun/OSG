#include "ShpAndKMLTypeParser.h"

#ifdef WIN32
#include <gdal/ogrsf_frmts.h>
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#else
#include <ogrsf_frmts.h>
#include <gdal.h>
#include <gdal_priv.h>
#endif

#include <osgDB/FileNameUtils>
#include "FeUtils/tinyXml/tinyxml.h"
#include <iostream>
#include <stack>
using namespace FeUtil;


ShpAndKMLTypeParser::ShpAndKMLTypeParser()
{
	GDALAllRegister();
}

ShpAndKMLTypeParser::~ShpAndKMLTypeParser()
{

}

TiXmlElement * ReadElement(TiXmlElement * InputElement, char * pName)
{
	TiXmlElement * ptemp = NULL;
	if (InputElement && 0 == strcmp(pName, InputElement->Value()))
	{
		ptemp = InputElement;
		return ptemp;
	}
	else
	{

	}

	TiXmlElement * tmpElement = InputElement;
	if (tmpElement->FirstChildElement())
	{
		ptemp = ReadElement(tmpElement->FirstChildElement(), pName);
	}
	if (!ptemp)
	{
		tmpElement = tmpElement->NextSiblingElement();
		if (tmpElement)
		{
			ptemp = ReadElement(tmpElement, pName);
		}
	}

	return ptemp;
}

TiXmlElement * ReadXmlByStack(TiXmlElement * pRootElement, char * pName, bool & bFind)
{
	std::stack<TiXmlElement *> ElementStack;

	ElementStack.push(pRootElement);
	TiXmlElement * pTempElement = NULL;
	while (ElementStack.size() > 0)
	{
		pTempElement = ElementStack.top();
		ElementStack.pop();

		if (0 == strcmp(pTempElement->Value(), pName))
		{
			bFind = true;
			break;
		}

		TiXmlElement * pTempSibLing = pTempElement->FirstChildElement();
		do 
		{
			if (pTempSibLing)
			{
				ElementStack.push(pTempSibLing);
				pTempSibLing = pTempSibLing->NextSiblingElement();
			}
		} while (pTempSibLing);
	}

	while (ElementStack.size() > 0)
	{
		ElementStack.pop();
	}

	if (bFind)
	{
		return pTempElement;
	}
	else
	{
		return NULL;
	}
}



std::vector<GeometryType> ShpAndKMLTypeParser::Parse(const std::string & fileName)
{
	std::vector<GeometryType> types;

	std::string ext = osgDB::getFileExtension(fileName);
	if (ext == "shp" || ext == "SHP")
	{
		GDALDataset * poDS;
		poDS = (GDALDataset *)GDALOpenEx(fileName.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
		if (poDS != NULL)
		{
			int layerCount = poDS->GetLayerCount();
			for (int i = 0; i < layerCount; ++i)
			{
				OGRLayer * layer = poDS->GetLayer(i);
				OGRwkbGeometryType ogrType = layer->GetGeomType();

				switch(ogrType)
				{
				case wkbPoint:
					types.push_back(POINT);
					break;
				case wkbLineString:
					types.push_back(POLYLINE);
					break;
				case wkbMultiPoint:
					types.push_back(POINT);
					break;
				case wkbPolygon:
					types.push_back(POLYGON);
					break;
				case wkbMultiPolygon:
					types.push_back(MULTI_POLYGON);
					break;
				case -2147483647:
					{
						int layerCount = poDS->GetLayerCount();
						for (int i = 0; i < layerCount; ++i)
						{
							OGRLayer * layer = poDS->GetLayer(i);
							if (layer != NULL)
							{
								OGRGeometry * geometry = layer->GetFeature(0)->GetGeometryRef();
								OGRwkbGeometryType t = geometry->getGeometryType();
								OGRPoint * pt = dynamic_cast<OGRPoint*>(geometry);
								if (pt != NULL)
								{
									types.push_back(POINT);
								}
							}
						}
					}
					break;
				default:
					types.push_back(UNKNOWN);
					break;
				}
			}
		}

		GDALClose(poDS);
	}
	else if (ext == "kml" || ext == "KML")
	{
		TiXmlDocument doc(fileName.c_str());
		bool loadOkay = doc.LoadFile();
		if (loadOkay)
		{
			TiXmlElement * ele = doc.RootElement();
			
			bool find = false;
			TiXmlElement * point = ReadXmlByStack(ele, "Point", find);
			TiXmlElement * lineRing = ReadXmlByStack(ele, "LinearRing", find);
			TiXmlElement * polygon = ReadXmlByStack(ele, "Polygon", find);

			if(point != NULL)
			{
				types.push_back(POINT);
			}
			if (lineRing != NULL)
			{
				types.push_back(POLYLINE);
			}
			if (polygon != NULL)
			{
				types.push_back(POLYGON);
			}
		}
	}

	return types;
}
