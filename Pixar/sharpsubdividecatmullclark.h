#ifndef SHARPSUBDIVIDECATMULLCLARK_H
#define SHARPSUBDIVIDECATMULLCLARK_H

#include "mesh.h"
#include <QVector3D>

void sharpSubdivideCatmullClark(Mesh* inputMesh, Mesh *subdivMesh);

QVector3D sharpVertexPoint(HalfEdge* firstEdge, Mesh *newMesh);
QVector3D sharpEdgePoint(HalfEdge* firstEdge, Mesh* newMesh);
QVector3D sharpFacePoint(HalfEdge* firstEdge);

HalfEdge* sharpVertOnBoundary(Vertex* currentVertex);

void sharpSplitHalfEdges(Mesh* inputMesh, Mesh* subdivMesh, unsigned int numHalfEdges, unsigned int numVertPts, unsigned int numFacePts);


#endif // SHARPSUBDIVIDECATMULLCLARK_H
