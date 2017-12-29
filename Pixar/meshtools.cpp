#include "meshtools.h"

void subdivideCatmullClark(Mesh* inputMesh, Mesh* subdivMesh) {
  unsigned int numVerts, numHalfEdges, numFaces, sumFaceValences;
  unsigned int k, m, s, t;
  unsigned int vIndex, hIndex, fIndex;
  unsigned short n;
  HalfEdge* currentEdge;

  qDebug() << ":: Creating new Catmull-Clark mesh";

  numVerts = inputMesh->Vertices.size();
  numHalfEdges = inputMesh->HalfEdges.size();
  numFaces = inputMesh->Faces.size();

  // Reserve memory
  subdivMesh->Vertices.reserve(numFaces + numVerts + numHalfEdges/2);

  sumFaceValences = 0;
  for (k=0; k<numFaces; k++) {
    sumFaceValences += inputMesh->Faces[k].val;
  }

  subdivMesh->HalfEdges.reserve(2*numHalfEdges + 2*sumFaceValences);
  subdivMesh->Faces.reserve(sumFaceValences);

  // Create face points
  for (k=0; k<numFaces; k++) {
    n = inputMesh->Faces[k].val;
    // Coords (x,y,z), Out, Valence, Index
    subdivMesh->Vertices.append( Vertex(facePoint(inputMesh->Faces[k].side),
                                        nullptr,
                                        n,
                                        k) );
  }

  qDebug() << " * Created face points";

  vIndex = numFaces;

  // Create vertex points
  for (k=0; k<numVerts; k++) {
    n = inputMesh->Vertices[k].val;
    // Coords (x,y,z), Out, Valence, Index
    subdivMesh->Vertices.append( Vertex(vertexPoint(inputMesh->Vertices[k].out, subdivMesh),
                                        nullptr,
                                        n,
                                        vIndex) );
    vIndex++;
  }

  qDebug() << " * Created vertex points";

  // Create edge points
  for (k=0; k<numHalfEdges; k++) {
    currentEdge = &inputMesh->HalfEdges[k];

    if (k < currentEdge->twin->index) {
      m = (!currentEdge->polygon || !currentEdge->twin->polygon) ? 3 : 4;
      // Coords (x,y,z), Out, Valence, Index
      subdivMesh->Vertices.append( Vertex(edgePoint(currentEdge, subdivMesh),
                                          nullptr,
                                          m,
                                          vIndex) );
      vIndex++;
    }
  }

  qDebug() << " * Created edge points";

  // Split halfedges
  splitHalfEdges(inputMesh, subdivMesh, numHalfEdges, numVerts, numFaces);

  qDebug() << " * Split halfedges";

  hIndex = 2*numHalfEdges;
  fIndex = 0;

  // Create faces and remaining halfedges
  for (k=0; k<numFaces; k++) {
    currentEdge = inputMesh->Faces[k].side;
    n = inputMesh->Faces[k].val;

    for (m=0; m<n; m++) {

      s = currentEdge->prev->index;
      t = currentEdge->index;

      // Side, Val, Index
      subdivMesh->Faces.append(Face(nullptr,
                                    4,
                                    fIndex));

      subdivMesh->Faces[fIndex].side = &subdivMesh->HalfEdges[ 2*t ];

      // Target, Next, Prev, Twin, Poly, Index
      // sharpness is currently not decreased, which resulted in least anomalies
      // (no direct entry in paper that describes sharpness when adding edges due to face dubdivision)
      subdivMesh->HalfEdges.append(HalfEdge( &subdivMesh->Vertices[k],
                                             nullptr,
                                             &subdivMesh->HalfEdges[ 2*t ],
                                   nullptr,
                                   &subdivMesh->Faces[fIndex],
                                   hIndex
                                   ));

      subdivMesh->HalfEdges.append(HalfEdge( nullptr,
                                             &subdivMesh->HalfEdges[2*s+1],
                                   &subdivMesh->HalfEdges[hIndex],
                                   nullptr,
                                   &subdivMesh->Faces[fIndex],
                                   hIndex+1
                                   ));

      subdivMesh->HalfEdges[hIndex].next = &subdivMesh->HalfEdges[hIndex+1];
      subdivMesh->HalfEdges[hIndex+1].target = subdivMesh->HalfEdges[hIndex+1].next->twin->target;

      subdivMesh->HalfEdges[2*s+1].next = &subdivMesh->HalfEdges[2*t];
      subdivMesh->HalfEdges[2*s+1].prev = &subdivMesh->HalfEdges[hIndex+1];
      subdivMesh->HalfEdges[2*s+1].polygon = &subdivMesh->Faces[fIndex];

      subdivMesh->HalfEdges[2*t].next = &subdivMesh->HalfEdges[hIndex];
      subdivMesh->HalfEdges[2*t].prev = &subdivMesh->HalfEdges[2*s+1];
      subdivMesh->HalfEdges[2*t].polygon = &subdivMesh->Faces[fIndex];

      if (m > 0) {
        // Twins
        subdivMesh->HalfEdges[hIndex+1].twin = &subdivMesh->HalfEdges[hIndex-2];
        subdivMesh->HalfEdges[hIndex-2].twin = &subdivMesh->HalfEdges[hIndex+1];
      }

      // For edge points
      subdivMesh->HalfEdges[2*t].target->out = &subdivMesh->HalfEdges[hIndex];

      hIndex += 2;
      fIndex++;
      currentEdge = currentEdge->next;
    }

    subdivMesh->HalfEdges[hIndex-2*n+1].twin = &subdivMesh->HalfEdges[hIndex-2];
    subdivMesh->HalfEdges[hIndex-2].twin = &subdivMesh->HalfEdges[hIndex-2*n+1];

    // For face points
    subdivMesh->Vertices[k].out = &subdivMesh->HalfEdges[hIndex-1];

  }

  qDebug() << " * Created faces and remaining halfedges";

  // For vertex points
  for (k=0; k<numVerts; k++) {
    subdivMesh->Vertices[numFaces + k].out = &subdivMesh->HalfEdges[ 2*inputMesh->Vertices[k].out->index ];
  }

  qDebug() << " * Completed!";
  qDebug() << "   # Vertices:" << subdivMesh->Vertices.size();
  qDebug() << "   # HalfEdges:" << subdivMesh->HalfEdges.size();
  qDebug() << "   # Faces:" << subdivMesh->Faces.size();

}

// ---

QVector3D vertexPoint(HalfEdge* firstEdge, Mesh* subdivMesh) {
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

  //count number of incident sharp edges
  HalfEdge * sharpEdge = currentEdge;
  int NsharpEdges = 0;
  float sharpAvg = 0.0;
  for (k=0; k<n; k++) {
    if (sharpEdge->sharpness > 0.0) {
        NsharpEdges++;
        sharpAvg += sharpEdge->sharpness;
    }
    sharpEdge = sharpEdge->prev->twin;
  }
  sharpAvg /= NsharpEdges;
  // one or no sharp edges
  if (NsharpEdges <= 1) {
      // Catmull-Clark (also supporting initial meshes containing n-gons)
      if (HalfEdge* boundaryEdge = vertOnBoundary(currentVertex)) {
        if (boundaryEdge->twin->target->val == 2) {
          // Interpolate corners
          vertexPt = boundaryEdge->twin->target->coords;
        } else {
          vertexPt  = 1.0 * boundaryEdge->target->coords;
          vertexPt += 6.0 * boundaryEdge->twin->target->coords;
          vertexPt += 1.0 * boundaryEdge->prev->twin->target->coords;
          vertexPt /= 8.0;
        }
      } else {
        for (k=0; k<n; k++) {
          sumStarPts += currentEdge->target->coords;
          sumFacePts += subdivMesh->Vertices[currentEdge->polygon->index].coords;
          currentEdge = currentEdge->prev->twin;
        }

        vertexPt = ((n-2)*currentVertex->coords + sumStarPts/n + sumFacePts/n)/n;
      }
      // three or more sharp edges
  } else if (NsharpEdges >= 3){
      //qDebug() << "three incident sharp edges!";
      if (sharpAvg < 1.0) {
        // using interpolation (same method as in paper)
        // first smooth coordinates
        if (HalfEdge* boundaryEdge = vertOnBoundary(currentVertex)) {
          if (boundaryEdge->twin->target->val == 2) {
            // Interpolate corners
            vertexPt = boundaryEdge->twin->target->coords;
          } else {
            vertexPt  = 1.0 * boundaryEdge->target->coords;
            vertexPt += 6.0 * boundaryEdge->twin->target->coords;
            vertexPt += 1.0 * boundaryEdge->prev->twin->target->coords;
            vertexPt /= 8.0;
          }
        } else {
          for (k=0; k<n; k++) {
            sumStarPts += currentEdge->target->coords;
            sumFacePts += subdivMesh->Vertices[currentEdge->polygon->index].coords;
            currentEdge = currentEdge->prev->twin;
          }
          vertexPt = ((n-2)*currentVertex->coords + sumStarPts/n + sumFacePts/n)/n;
        }
        vertexPt = (1.0 - sharpAvg) * vertexPt + sharpAvg * currentVertex->coords;
      // corner rule, vertex doesn't move
      } else {
        vertexPt = currentVertex->coords;
      }
    // two sharp edges
  } else {
      qDebug() << sharpAvg;
    // integer sharpness step
    // check for same crease here
    if (sharpAvg >= 1.0) {
      vertexPt = 6.0 * currentVertex->coords;
      sharpEdge = currentEdge;
      // find the two edges that are part of the crease and add them
      for (k=0; k<n; k++) {
        if (sharpEdge->sharpness > 0.0) {
          vertexPt += 1.0 * sharpEdge->target->coords;
        }
        sharpEdge = sharpEdge->prev->twin;
      }
      vertexPt /= 8.0;
    // semi-sharp interpolation
    } else {
      // sharp step
      vertexPt = 6.0 * currentVertex->coords;
      sharpEdge = currentEdge;
      // find the two sharp edges and add them
      for (k=0; k<n; k++) {
        if (sharpEdge->sharpness > 0.0) {
          vertexPt += 1.0 * sharpEdge->target->coords;
        }
        sharpEdge = sharpEdge->prev->twin;
      }
      vertexPt /= 8.0;

      // smooth coordinates
      QVector3D smoothPt;
      if (HalfEdge* boundaryEdge = vertOnBoundary(currentVertex)) {
        if (boundaryEdge->twin->target->val == 2) {
          // Interpolate corners
          smoothPt = boundaryEdge->twin->target->coords;
        } else {
          smoothPt  = 1.0 * boundaryEdge->target->coords;
          smoothPt += 6.0 * boundaryEdge->twin->target->coords;
          smoothPt += 1.0 * boundaryEdge->prev->twin->target->coords;
          smoothPt /= 8.0;
        }
      } else {
        for (k=0; k<n; k++) {
          sumStarPts += currentEdge->target->coords;
          sumFacePts += subdivMesh->Vertices[currentEdge->polygon->index].coords;
          currentEdge = currentEdge->prev->twin;
        }
        smoothPt = ((n-2)*currentVertex->coords + sumStarPts/n + sumFacePts/n)/n;
      }

      // not sure if by corner mask they mean the sharp corner rule TODO, leads to no anomalies
      vertexPt = sharpAvg * vertexPt + (1.0 - sharpAvg) * smoothPt;
    }
  }

  return vertexPt;

}

QVector3D edgePoint(HalfEdge* firstEdge, Mesh* subdivMesh) {
  QVector3D EdgePt;
  HalfEdge* currentEdge;
  EdgePt = QVector3D();
  currentEdge = firstEdge;

  //smooth edge rule
  if (currentEdge->sharpness <= 0.0) {
      // Catmull-Clark (also supporting initial meshes containing n-gons)
      if (!currentEdge->polygon || !currentEdge->twin->polygon) {
        EdgePt  = 4.0 * currentEdge->target->coords;
        EdgePt += 4.0 * currentEdge->twin->target->coords;
        EdgePt /= 8.0;
      } else {
        EdgePt  = currentEdge->target->coords;
        EdgePt += currentEdge->twin->target->coords;
        EdgePt += subdivMesh->Vertices[currentEdge->polygon->index].coords;
        EdgePt += subdivMesh->Vertices[currentEdge->twin->polygon->index].coords;
        EdgePt /= 4.0;
      }
  // sharp edge rule
  } else if (currentEdge->sharpness >= 1.0) {
      EdgePt  = 1.0 * currentEdge->target->coords;
      EdgePt += 1.0 * currentEdge->twin->target->coords;
      EdgePt /= 2.0;
  // semi sharp edge rule
  } else {
      // smooth part
      if (!currentEdge->polygon || !currentEdge->twin->polygon) {
        EdgePt  = 4.0 * currentEdge->target->coords;
        EdgePt += 4.0 * currentEdge->twin->target->coords;
        EdgePt /= 8.0;
      } else {
        EdgePt  = currentEdge->target->coords;
        EdgePt += currentEdge->twin->target->coords;
        EdgePt += subdivMesh->Vertices[currentEdge->polygon->index].coords;
        EdgePt += subdivMesh->Vertices[currentEdge->twin->polygon->index].coords;
        EdgePt /= 4.0;
      }

      // sharp part
      QVector3D EdgePtsharp = QVector3D();
      EdgePtsharp  = 1.0 * currentEdge->target->coords;
      EdgePtsharp += 1.0 * currentEdge->twin->target->coords;
      EdgePtsharp /= 2.0;

      //interpolate

      EdgePt = (1.0 - currentEdge->sharpness) * EdgePt
              + currentEdge->sharpness * EdgePtsharp;
  }

  return EdgePt;

}

QVector3D facePoint(HalfEdge* firstEdge) {
  unsigned short k, n;
  QVector<float> stencil;
  QVector3D facePt;
  HalfEdge* currentEdge;

  n = firstEdge->polygon->val;

  // Bilinear, Catmull-Clark, Dual
  stencil.clear();
  stencil.fill(1.0/n, n);

  facePt = QVector3D();
  currentEdge = firstEdge;

  for (k=0; k<n; k++) {
    // General approach
    facePt += stencil[k] * currentEdge->target->coords;
    currentEdge = currentEdge->next;
  }

  return facePt;

}

HalfEdge* vertOnBoundary(Vertex* currentVertex) {

  unsigned short n = currentVertex->val;
  int k;
  HalfEdge* currentEdge = currentVertex->out;

  for (k=0; k<n; k++) {
    if (!currentEdge->polygon) {
      return currentEdge;
    }
    currentEdge = currentEdge->prev->twin;
  }

  return nullptr;
}

// For Bilinear, Catmull-Clark and Loop
void splitHalfEdges(Mesh* inputMesh, Mesh* subdivMesh, unsigned int numHalfEdges, unsigned int numVertPts, unsigned int numFacePts) {
  unsigned int k, m;
  unsigned int vIndex;
  HalfEdge* currentEdge;

  vIndex = numFacePts + numVertPts;

  for (k=0; k<numHalfEdges; k++) {
    currentEdge = &inputMesh->HalfEdges[k];
    m = currentEdge->twin->index;

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

    if (k < m) {
      subdivMesh->HalfEdges[2*k].target = &subdivMesh->Vertices[ vIndex ];
      subdivMesh->HalfEdges[2*k+1].target = &subdivMesh->Vertices[ numFacePts + currentEdge->target->index ];
      vIndex++;
    } else {
      subdivMesh->HalfEdges[2*k].target = subdivMesh->HalfEdges[2*m].target;
      subdivMesh->HalfEdges[2*k+1].target = &subdivMesh->Vertices[ numFacePts + currentEdge->target->index ];

      // Assign Twins
      subdivMesh->HalfEdges[2*k].twin = &subdivMesh->HalfEdges[2*m+1];
      subdivMesh->HalfEdges[2*k+1].twin = &subdivMesh->HalfEdges[2*m];
      subdivMesh->HalfEdges[2*m].twin = &subdivMesh->HalfEdges[2*k+1];
      subdivMesh->HalfEdges[2*m+1].twin = &subdivMesh->HalfEdges[2*k];

      // Boundary edges are added last when importing a mesh, so their index will always be higher than their twins.
      if (!currentEdge->polygon) {
        subdivMesh->HalfEdges[2*k].next = &subdivMesh->HalfEdges[2*k+1];
        subdivMesh->HalfEdges[2*k+1].prev = &subdivMesh->HalfEdges[2*k];

        if (currentEdge > currentEdge->next) {
          m = currentEdge->next->index;
          subdivMesh->HalfEdges[2*k+1].next = &subdivMesh->HalfEdges[2*m];
          subdivMesh->HalfEdges[2*m].prev = &subdivMesh->HalfEdges[2*k+1];
        }

        if (currentEdge > currentEdge->prev) {
          m = currentEdge->prev->index;
          subdivMesh->HalfEdges[2*k].prev = &subdivMesh->HalfEdges[2*m+1];
          subdivMesh->HalfEdges[2*m+1].next = &subdivMesh->HalfEdges[2*k];
        }
      }
    }

    // twins are assigned the same crease and sharpness value
    if (k < m) {
        // the current crease
        int crease = currentEdge->crease;
        if (crease >= 0) {
            if (currentEdge->next->crease != crease && currentEdge->prev->crease != crease) {
                currentEdge = currentEdge->twin;
                qDebug() << "swapped";
            }

            // default value of sharpness is 0 if no neighbouring edge is found
            float sharpnessA = 0, sharpnessB = 0;
            HalfEdge *nEdge = currentEdge->target->out;
            int val = currentEdge->target->val;

            // find next edge in sequence
            for (int eout = 0; eout < val; ++eout) {
                if (nEdge->crease == crease && nEdge->twin->index != currentEdge->index)
                    sharpnessB = nEdge->sharpness;
                nEdge = nEdge->prev->twin;
            }

            // find previous edge in sequence
            nEdge = currentEdge->twin->target->out;
            val = currentEdge->twin->target->val;
            for (int eout = 0; eout < val; ++eout) {
                if (nEdge->twin->crease == crease && nEdge->index != currentEdge->index)
                    sharpnessA = nEdge->sharpness;
                nEdge = nEdge->prev->twin;
            }

            //Chaikins subdivision
            subdivMesh->HalfEdges[2*k].sharpness =
                    std::fmax((3.0* currentEdge->sharpness + sharpnessA)/4.0 - 1.0,0.0);
            subdivMesh->HalfEdges[2*k + 1].sharpness =
                    std::fmax((3.0* currentEdge->sharpness + sharpnessB)/4.0 - 1.0,0.0);

            // create refined crease
            subdivMesh->HalfEdges[2*k].crease = crease;
            subdivMesh->HalfEdges[2*k + 1].crease = crease;

        } else {
            // normal subdivision, assume same sharpness everywhere
            subdivMesh->HalfEdges[2*k].sharpness =
                    std::fmax(currentEdge->sharpness - 1.0,0.0);
            subdivMesh->HalfEdges[2*k + 1].sharpness =
                    std::fmax(currentEdge->sharpness - 1.0,0.0);

        }
    } else {
        subdivMesh->HalfEdges[2*k].crease = subdivMesh->HalfEdges[2*k].twin->crease;
        subdivMesh->HalfEdges[2*k + 1].crease = subdivMesh->HalfEdges[2*k + 1].twin->crease;
        subdivMesh->HalfEdges[2*k].sharpness = subdivMesh->HalfEdges[2*k].twin->sharpness;
        subdivMesh->HalfEdges[2*k + 1].sharpness = subdivMesh->HalfEdges[2*k + 1].twin->sharpness;

    }
  // Note that Next, Prev and Poly are not yet assigned at this point.
  }

}
