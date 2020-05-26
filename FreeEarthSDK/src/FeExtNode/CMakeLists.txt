set(OUT_NAME FeExtNode)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/FeExtNode)

SET(PROJECT_INCLUDE ${CMAKE_SOURCE_DIR}/include/FeExtNode)
SET(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src/FeExtNode)

###########################################################################
#                                                                         #
#                                 FEEXTNODE   	                          #
#                                                                         #
###########################################################################
set(FEEXTNODE_H
		${PROJECT_INCLUDE}/Export.h
		
		${PROJECT_INCLUDE}/ExternNode.h
		${PROJECT_INCLUDE}/ExternNodeVisitor.h
		${PROJECT_INCLUDE}/ExLodNode.h
		${PROJECT_INCLUDE}/ExComposeNode.h
		${PROJECT_INCLUDE}/ExPointNode.h
		${PROJECT_INCLUDE}/ExTextNode.h
		${PROJECT_INCLUDE}/ExPlaceNode.h
		${PROJECT_INCLUDE}/ExBillBoardNode.h
		${PROJECT_INCLUDE}/ExOverLayNode.h
		${PROJECT_INCLUDE}/ExFeatureNode.h
		${PROJECT_INCLUDE}/ExLineNode.h
		${PROJECT_INCLUDE}/ExPolygonNode.h
		${PROJECT_INCLUDE}/SceneMarkHandler.h
		${PROJECT_INCLUDE}/ExternNodeEditing.h
		${PROJECT_INCLUDE}/ExLocalizedNode.h
		${PROJECT_INCLUDE}/ExCircleNode.h
		${PROJECT_INCLUDE}/ExEllipseNode.h
		${PROJECT_INCLUDE}/ExSectorNode.h
		${PROJECT_INCLUDE}/ExRectNode.h
		${PROJECT_INCLUDE}/ExLabelNode.h
		${PROJECT_INCLUDE}/ExLabelCullEventHandler.h
		${PROJECT_INCLUDE}/ExTiltReaderWriter.h
		${PROJECT_INCLUDE}/ExTiltModelNode.h
		${PROJECT_INCLUDE}/ExLabelBindingNodeCallback.h
		${PROJECT_INCLUDE}/ExLodModelNode.h

		${PROJECT_INCLUDE}/FeRotateDragger.h
		${PROJECT_INCLUDE}/FeScaleDragger.h
		${PROJECT_INCLUDE}/FeTranslateDragger.h
		${PROJECT_INCLUDE}/FeDraggerAttribute.h
		${PROJECT_INCLUDE}/FeCompositeDragger.h
		${PROJECT_INCLUDE}/FeNodeEditTool.h
		${PROJECT_INCLUDE}/IFeNodeEditable.h
		
		${PROJECT_INCLUDE}/ExModelNode.h
		
		#${PROJECT_INCLUDE}/ExStraightArrowNode.h
		#${PROJECT_INCLUDE}/ExAssaultNode.h
		#${PROJECT_INCLUDE}/ExAttackNode.h

		${PROJECT_INCLUDE}/FeEventListenMgr.h
		
		${PROJECT_INCLUDE}/ExParticleNode.h
		${PROJECT_INCLUDE}/ExParticleInfoStruct.h

		${PROJECT_INCLUDE}/ExGridNode.h
		
)

set(FEEXTNODE_CPP 
		${PROJECT_SRC}/ExternNode.cpp
		${PROJECT_SRC}/ExternNodeVisitor.cpp
		${PROJECT_SRC}/ExLodNode.cpp
		${PROJECT_SRC}/ExComposeNode.cpp
		${PROJECT_SRC}/ExPointNode.cpp
		${PROJECT_SRC}/ExTextNode.cpp
		${PROJECT_SRC}/ExPlaceNode.cpp
		${PROJECT_SRC}/ExBillBoardNode.cpp
		${PROJECT_SRC}/ExOverLayNode.cpp
		${PROJECT_SRC}/ExFeatureNode.cpp
		${PROJECT_SRC}/ExLineNode.cpp
		${PROJECT_SRC}/ExPolygonNode.cpp
		${PROJECT_SRC}/SceneMarkHandler.cpp
		${PROJECT_SRC}/ExternNodeEditing.cpp
		${PROJECT_SRC}/ExLocalizedNode.cpp
		${PROJECT_SRC}/ExCircleNode.cpp
		${PROJECT_SRC}/ExEllipseNode.cpp
		${PROJECT_SRC}/ExSectorNode.cpp
		${PROJECT_SRC}/ExRectNode.cpp
		${PROJECT_SRC}/ExLabelNode.cpp
		${PROJECT_SRC}/ExLabelCullEventHandler.cpp
		${PROJECT_SRC}/ExTiltReaderWriter.cpp
		${PROJECT_SRC}/ExTiltModelNode.cpp
		${PROJECT_SRC}/ExLabelBindingNodeCallback.cpp
		${PROJECT_SRC}/ExLodModelNode.cpp
		
		${PROJECT_SRC}/FeRotateDragger.cpp
		${PROJECT_SRC}/FeScaleDragger.cpp
		${PROJECT_SRC}/FeTranslateDragger.cpp
		${PROJECT_SRC}/FeDraggerAttribute.cpp
		${PROJECT_SRC}/FeCompositeDragger.cpp
		${PROJECT_SRC}/FeNodeEditTool.cpp
		
		${PROJECT_SRC}/ExModelNode.cpp
		
		#${PROJECT_SRC}/ExStraightArrowNode.cpp
		#${PROJECT_SRC}/ExAssaultNode.cpp
		#${PROJECT_SRC}/ExAttackNode.cpp

		${PROJECT_SRC}/FeEventListenMgr.cpp

		${PROJECT_SRC}/ExParticleNode.cpp

		${PROJECT_SRC}/ExGridNode.cpp
		)

if(${BUILD_SHARED_LIBS} STREQUAL "On")
    add_library(${OUT_NAME} SHARED ${FEEXTNODE_H} ${FEEXTNODE_CPP})
    ADD_DEFINITIONS(-DFEEXNODE_LIBRARY)
else()
    add_library(${OUT_NAME} STATIC ${FEEXTNODE_H} ${FEEXTNODE_CPP})
    ADD_DEFINITIONS(-DFEEXNODE_LIBRARY_STATIC)
endif()


target_link_libraries(
    ${OUT_NAME} 
    ${OPENSCENEGRAPH_LIBRARIES} 
    debug ${osgEarth_DEBUG_LIBRARIES} optimized ${osgEarth_RELEASE_LIBRARIES} 
    debug ${osgEarthUtil_DEBUG_LIBRARIES} optimized ${osgEarthUtil_RELEASE_LIBRARIES}
	debug ${osgEarthSymbology_DEBUG_LIBRARIES} optimized ${osgEarthSymbology_RELEASE_LIBRARIES}
	debug ${osgEarthFeatures_DEBUG_LIBRARIES} optimized ${osgEarthFeatures_RELEASE_LIBRARIES}
	debug ${osgEarthAnnotation_DEBUG_LIBRARIES} optimized ${osgEarthAnnotation_RELEASE_LIBRARIES}
	FeUtils
	FeServiceProvider
	#FeAlg
 )

INSTALL(TARGETS ${OUT_NAME}  DESTINATION lib)
INSTALL(TARGETS ${OUT_NAME}  DESTINATION bin)
INSTALL(FILES ${FEEXTNODE_H} DESTINATION include/FeExtNode)

