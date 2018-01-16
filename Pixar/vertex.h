#ifndef VERTEX
#define VERTEX

#include <QVector3D>
#include <QDebug>

// Forward declaration
class HalfEdge;

class Vertex {
public:
  QVector3D coords;
  HalfEdge* out;
  unsigned short val;
  unsigned int index;
  unsigned short sharpness;
  bool sharp;

  // Inline constructors
  Vertex() {
    // qDebug() << "Default Vertex Constructor";
    coords = QVector3D();
    out = nullptr;
    val = 0;
    index = 0;
    sharpness = 0;
    sharp = false;
  }

  Vertex(QVector3D vcoords, HalfEdge* vout, unsigned short vval, unsigned int vindex, float vsharpness = 0, bool vsharp = false) {
    //qDebug() << "QVector3D Vertex Constructor";
    coords = vcoords;
    out = vout;
    val = vval;
    index = vindex;
    sharpness = vsharpness;
    sharp = vsharp;
  }
};

#endif // VERTEX
