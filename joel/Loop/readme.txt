Subdivision for boundaries:
-VertexPoint and EdgePoint have been adjusted to detect boundaries and if a boundary is present use a different stencil
-stencil for vertex point on boundary is [1 8 1]/10 where 8 is for the current vertex and 1 for the neighbouring vertexes on the boundary.
-stencil for edge point is [1 1]/2, the average of the vertices of the edge
-halfedges do not have next and prev defined yet, next and prev are assigned in subdivideloop at the end of the function.

Isophotes:
-fragment shader calculates dot product of normal with incoming light vector to surface
The result is multiplied by a float (uniform, changeable by slider), modulo 2 applied and rounded down to nearest integer
The resulting integer applies a white or black color if 1 or 0 respectively
-Isophotes can be toggled and the float multiplier can be adjusted by a slider which increases/decreases the frequency of black/white color bands on the surface
(frequency depends on smoothness of surface and viewing angle as well)

