#include "meshtools.h"

void subdivideLoop(Mesh* inputMesh, Mesh* outputSubdivMesh) {
  unsigned int numInputVerts, numInputHalfEdges, numInputFaces;
  unsigned int numOutputVerts, numOutputHalfEdges, numOutputFaces;
  unsigned int k, m, s, t;
  unsigned int vIndex, hIndex, fIndex;
  unsigned short n;
  HalfEdge* currentEdge;

  qDebug() << ":: Creating new Loop mesh";

  numInputVerts = inputMesh->Vertices.size();
  numInputHalfEdges = inputMesh->HalfEdges.size();
  numInputFaces = inputMesh->Faces.size();

  numOutputVerts = numInputVerts + numInputHalfEdges/2;
  numOutputHalfEdges = 2*numInputHalfEdges + 6*numInputFaces;
  numOutputFaces = 4*numInputFaces;

  // Reserve memory
  outputSubdivMesh->Vertices.reserve(numOutputVerts);
  outputSubdivMesh->HalfEdges.reserve(numOutputHalfEdges);
  outputSubdivMesh->Faces.reserve(numOutputFaces);

  // Create vertex points
  // from existing vertex points, immediatly updated to new position
  // currently not working for boundary vertices
  for (k=0; k<numInputVerts; k++) {
    n = inputMesh->Vertices[k].val;
    // Coords (x,y,z), Out, Valence, Index
    outputSubdivMesh->Vertices.append( Vertex( vertexPoint(inputMesh->Vertices[k].out),
                                         nullptr,
                                         n,
                                         k) );
  }

  vIndex = numInputVerts;
  qDebug() << " * Created vertex points";
  qDebug() << outputSubdivMesh->Vertices.size();
  // Create edge points
  // currently not working for boundary half edges
  for (k=0; k<numInputHalfEdges; k++) {
    currentEdge = &inputMesh->HalfEdges[k];
    //only 1 additional vertex for two twin half edge pairs
    if (k < currentEdge->twin->index) {
      // Coords (x,y,z), Out, Valence, Index
      outputSubdivMesh->Vertices.append( Vertex(edgePoint(currentEdge),
                                          nullptr,
                                          6,
                                          vIndex) );
      //qDebug() << currentEdge->twin->index << vIndex;
      vIndex++;
    }
  }

  qDebug() << " * Created edge points";
  qDebug() << outputSubdivMesh->Vertices.size();
  // Split halfedges
  splitHalfEdges(inputMesh, outputSubdivMesh, numInputHalfEdges, numInputVerts, 0);

  qDebug() << " * Split halfedges";




  hIndex = 2*numInputHalfEdges;
  fIndex = 0;

  // Create faces and remaining halfedges
  for (k=0; k<numInputFaces; k++) {
    currentEdge = inputMesh->Faces[k].side;

    // Three outer faces

    for (m=0; m<3; m++) {

      s = currentEdge->prev->index;
      t = currentEdge->index;

      // Side, Val, Index
      outputSubdivMesh->Faces.append(Face(nullptr,
                                    3,
                                    fIndex));

      outputSubdivMesh->Faces[fIndex].side = &outputSubdivMesh->HalfEdges[ 2*t ];

      // Target, Next, Prev, Twin, Poly, Index
      outputSubdivMesh->HalfEdges.append(HalfEdge( outputSubdivMesh->HalfEdges[2*s].target,
                                   &outputSubdivMesh->HalfEdges[2*s+1],
                                  &outputSubdivMesh->HalfEdges[ 2*t ],
                                  nullptr,
                                  &outputSubdivMesh->Faces[fIndex],
                                  hIndex ));

      outputSubdivMesh->HalfEdges.append(HalfEdge( outputSubdivMesh->HalfEdges[2*t].target,
                                   nullptr,
                                   nullptr,
                                   &outputSubdivMesh->HalfEdges[hIndex],
                                   nullptr,
                                   hIndex+1 ));

      outputSubdivMesh->HalfEdges[hIndex].twin = &outputSubdivMesh->HalfEdges[hIndex+1];

      outputSubdivMesh->HalfEdges[2*s+1].next = &outputSubdivMesh->HalfEdges[2*t];
      outputSubdivMesh->HalfEdges[2*s+1].prev = &outputSubdivMesh->HalfEdges[hIndex];
      outputSubdivMesh->HalfEdges[2*s+1].polygon = &outputSubdivMesh->Faces[fIndex];

      outputSubdivMesh->HalfEdges[2*t].next = &outputSubdivMesh->HalfEdges[hIndex];
      outputSubdivMesh->HalfEdges[2*t].prev = &outputSubdivMesh->HalfEdges[2*s+1];
      outputSubdivMesh->HalfEdges[2*t].polygon = &outputSubdivMesh->Faces[fIndex];

      // For edge points
      outputSubdivMesh->HalfEdges[2*t].target->out = &outputSubdivMesh->HalfEdges[hIndex];

      hIndex += 2;
      fIndex++;
      currentEdge = currentEdge->next;

    }

    // Inner face

    // Side, Val, Index
    outputSubdivMesh->Faces.append(Face(nullptr,
                                  3,
                                  fIndex));

    outputSubdivMesh->Faces[fIndex].side = &outputSubdivMesh->HalfEdges[ hIndex-1 ];

    for (m=0; m<3; m++) {

      if (m==2) {
        outputSubdivMesh->HalfEdges[hIndex - 1].next = &outputSubdivMesh->HalfEdges[hIndex - 5];
      }
      else {
        outputSubdivMesh->HalfEdges[hIndex - 5 + 2*m].next = &outputSubdivMesh->HalfEdges[hIndex - 5 + 2*(m+1)];
      }

      if (m==0) {
        outputSubdivMesh->HalfEdges[hIndex - 5].prev = &outputSubdivMesh->HalfEdges[hIndex - 1];
      }
      else {
        outputSubdivMesh->HalfEdges[hIndex - 5 + 2*m].prev = &outputSubdivMesh->HalfEdges[hIndex - 5 + 2*(m-1)];
      }

      outputSubdivMesh->HalfEdges[hIndex - 5 + 2*m].polygon = &outputSubdivMesh->Faces[fIndex];

    }

    fIndex++;

  }

  qDebug() << " * Created faces";





  // For vertex points
  for (k=0; k<numInputVerts; k++) {
    outputSubdivMesh->Vertices[k].out = &outputSubdivMesh->HalfEdges[ 2*inputMesh->Vertices[k].out->index ];
  }

  //now add the next and prev to the boundary edges
  //go over new halfedges,
  // if missing prev or next we get the twin and find a halfedge connected to the target of this twin
  unsigned int startBoundary;
  bool boundary = false;
  for (k=0; k<numOutputHalfEdges; ++k) {
      if (outputSubdivMesh->HalfEdges[k].next == nullptr) {
          if (!boundary) {
              startBoundary = k;
              boundary = true;
          }
          if (outputSubdivMesh->HalfEdges[k+1].next == nullptr) {
            outputSubdivMesh->HalfEdges[k].next = &outputSubdivMesh->HalfEdges[k+1];
            outputSubdivMesh->HalfEdges[k+1].prev = &outputSubdivMesh->HalfEdges[k];

          } else {
              outputSubdivMesh->HalfEdges[k].next = &outputSubdivMesh->HalfEdges[startBoundary];
              outputSubdivMesh->HalfEdges[startBoundary].prev = &outputSubdivMesh->HalfEdges[k];
          }

      }
  }
//well this was fun
/*
  for (int i = 0; i < outputSubdivMesh->HalfEdges.size(); ++i)
    qDebug() << outputSubdivMesh->HalfEdges[i].index << outputSubdivMesh->HalfEdges[i].next << outputSubdivMesh->HalfEdges[i].polygon
                << outputSubdivMesh->HalfEdges[i].prev << outputSubdivMesh->HalfEdges[i].sharpness << outputSubdivMesh->HalfEdges[i].target
                << outputSubdivMesh->HalfEdges[i].twin;
*/
}

// ---

QVector3D vertexPoint(HalfEdge* firstEdge) {
  unsigned short k, n;
  QVector3D sumStarPts, sumFacePts;
  QVector3D vertexPt;
  float stencilValue;
  HalfEdge* currentEdge;
  Vertex* currentVertex;

  currentVertex = firstEdge->twin->target;
  n = currentVertex->val;

  sumStarPts = QVector3D();
  sumFacePts = QVector3D();
  currentEdge = firstEdge;

  //detect whether vertex is part of a boundary
  bool boundary = false;
  HalfEdge *currentEdge2 = currentEdge;
  for (k=0; k<n; k++) {
    if (currentEdge2->polygon==nullptr || currentEdge2->twin->polygon==nullptr) {
        boundary = true;
        break;
    }

    currentEdge2 = currentEdge2->prev->twin;
  }

  if (!boundary) {
      for (k=0; k<n; k++) {
        sumStarPts += currentEdge->target->coords;

        currentEdge = currentEdge->prev->twin;
      }

      // Warren's rules
      if (n == 3) {
        stencilValue = 3.0/16.0;
      }
      else {
        stencilValue = 3.0/(8*n);
      }

      vertexPt = (1.0 - n*stencilValue) * currentVertex->coords + stencilValue * sumStarPts;
  } else { //boundary case

    sumStarPts += 6.0*currentVertex->coords;
    Vertex *one = nullptr, *two=nullptr;
    for (k=0; k<n; k++) {
        //something goes wrong here, too many boundary points detected or too little,
        //the if else structure solves the problem partially
      if (currentEdge->polygon==nullptr || currentEdge->twin->polygon==nullptr) {
          if (one == nullptr && one != currentVertex)
              one = currentEdge->target;
          else if (two == nullptr && one !=currentEdge->target && two != currentVertex)
              two = currentEdge->target;
          else
              break;
          sumStarPts += currentEdge->target->coords;

      }

      currentEdge = currentEdge->prev->twin;
    }
    vertexPt = sumStarPts/8.0;
              qDebug() << one << two;
        //Debug() << calls;
  }
  return vertexPt;

}

QVector3D edgePoint(HalfEdge* firstEdge) {
  QVector3D EdgePt;
  EdgePt = QVector3D();
  //TODO
  // if edge has no face or twin has no face we can't use the regular stencil
  if(firstEdge->twin->polygon==NULL || firstEdge->polygon==NULL) {
      EdgePt  = 1.0 * firstEdge->target->coords;
      EdgePt += 1.0 * firstEdge->twin->target->coords;
      //EdgePt += 2.0 * firstEdge->next->target->coords;
      //EdgePt += 2.0 * firstEdge->twin->next->target->coords;
      EdgePt /= 2.0;

  } else {

      EdgePt  = 6.0 * firstEdge->target->coords;
      EdgePt += 2.0 * firstEdge->next->target->coords;
      EdgePt += 6.0 * firstEdge->twin->target->coords;
      EdgePt += 2.0 * firstEdge->twin->next->target->coords;
      EdgePt /= 16.0;

  }

  return EdgePt;

}

void splitHalfEdges(Mesh* inputMesh, Mesh* subdivMesh, unsigned int numHalfEdges, unsigned int numVertPts, unsigned int numFacePts) {
  unsigned int k, m;
  unsigned int vIndex;
  HalfEdge* currentEdge;

  vIndex = numFacePts + numVertPts;

  //source of problems, after 3 iterations target next and prev remain unassigned
  for (k=0; k<numHalfEdges; k++) {
    currentEdge = &inputMesh->HalfEdges[k];
    m = currentEdge->twin->index;
    //qDebug() << m;

    // create halfedges, parse over all previous halfedges and two are created in succesion
    // with increasing index
    // Target, Next, Prev, Twin, Poly, Index
    subdivMesh->HalfEdges.append(HalfEdge(nullptr,
                                          nullptr,
                                          nullptr,
                                          nullptr,
                                          nullptr,
                                          2*k));

    subdivMesh->HalfEdges.append(HalfEdge(nullptr,
                                          nullptr,
                                          nullptr,
                                          nullptr,
                                          nullptr,
                                          2*k+1));

    // index of the twin (of previous mesh) compared to index of new mesh
    // used to prevent assigning twins twice

    if (k < m) {
      subdivMesh->HalfEdges[2*k].target = &subdivMesh->Vertices[ vIndex ];
      subdivMesh->HalfEdges[2*k+1].target = &subdivMesh->Vertices[ numFacePts + currentEdge->target->index ];
      vIndex++;
    }
    else {
      subdivMesh->HalfEdges[2*k].target = subdivMesh->HalfEdges[2*m].target;
      subdivMesh->HalfEdges[2*k+1].target = &subdivMesh->Vertices[ numFacePts + currentEdge->target->index ];

      // Assign Twins
      subdivMesh->HalfEdges[2*k].twin = &subdivMesh->HalfEdges[2*m+1];
      subdivMesh->HalfEdges[2*k+1].twin = &subdivMesh->HalfEdges[2*m];
      subdivMesh->HalfEdges[2*m].twin = &subdivMesh->HalfEdges[2*k+1];
      subdivMesh->HalfEdges[2*m+1].twin = &subdivMesh->HalfEdges[2*k];
    }
  }

  //boundary halfedges occur in chain halfedge->next is another boundary edge, with this next and ultimately
  //prev can be assigned for these halfedges
  //periods of

  // Note that Next, Prev and Poly are not yet assigned at this point.
  // these are assigned when subdividing the faces, however, next and prev
  // won't be assigned for boundary edges since they are normally added in the adding faces
  // procedure
}

