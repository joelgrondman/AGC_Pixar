limit stencils:
The limit stencils are applied by updating the current coordinates of the mesh while storing the previous coordinates temporarily.
The buffers are then updates and the previous coordinates are reapplied. This way the limit stencil has to be reupdated as it isn't stored anywhere but the buffer
but should limit stencils be disabled the previous coordinates are always retained.
The main method that applies limit stencils is in function applyLimitStencils (meshtools.cpp) which uses the stencil described in ”Approximating
subdivision surfaces with Gregory patches for hardware tessellation.”

Tessalation shader:
When patch surfaces are enabled 16 vertices are expected for each polygon. To generate these vertices the current quads are combined with its 8 surrounding quads for a total of 16 points. This means multiple quads are reused. Non regular quads are not added to the polyindices buffer.
Patches of 16 points are enabled in opengl in order to render the patches. The tesselation control shader passes the vertices to the next stage after applying some inner/outer values. The generated vertices are then mapped to new coordinates using Bezier surfaces equation. 
This would lead to overlapping patches and in order to prevent this u,v coordinates are clamped between 1/3 and 2/3 in order only render the inner quad.

Normals:
The coordinates of the normals are interpolated as well with the Bezier curve equation.

Controlable inner levels:
the inner levels can be controlled by a slider, note that outer levels can be controlled as well but due to the outer vertices being clamped the effect is limited.

