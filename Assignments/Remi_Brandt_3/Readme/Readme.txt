How to move vertices to their limit position:
	Load a mesh and select the 'move vertices to their limit position' checkbox. 
         
How to render bezier patches which use tessellation:
	Load a mesh and select the 'Render bezier patches' checkbox. Optionally, tessellation levels can be changed 
	below said checkbox. (u,v) coordinates of a quad are indicated. The outer tessellation levels can be set in the outer square. The values for both horizontal and both vertical sides of the inner tessellation can be changed in the inner square. 
 
Known issues: 
	- When the 'move vertices to their limit position' checkbox is checked, and 'subdivision steps' is changed, the normals of the mesh are not updated. Check and then uncheck said checkbox to have to normals computed.