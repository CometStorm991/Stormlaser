# Stormlaser - A Vulkan Renderer
## Changelog
### 2026-10-11
I implemented model loading for glTF models via fastGLTF. Currently, the model loader supports diffuse, normal, and metallic-roughness textures, as well as samplers, PBR materials, and meshes. Nodes and transforms are yet to be included. This data gets processed into two separate descriptor sets, one for global data like textures and materials, and one for per-frame data like the MVP matrices. The vertices are also uploaded all at once via a staging buffer, much like materials and textures. The materials are put into one SSBO referenced by one descriptor, and the textures each have their own handles within a singular descriptor array. In the shader, only the diffuse and normal information is used currently.<br />
Next steps are working on gamma correction, hierarchical modeling for meshes, and PBR<br />
<img src="readmeAssets/ModelLoading.png" alt="Model Loading" width="400"/>

### 2026-09-24
I implemented texture batching and bindless textures. Textures are now uploaded with a single staging buffer, and textures are referenced via an index into a single descriptor array. This was the first addition I made to the renderer without the help of tutorials. Since learning to write high quality code is still a priority of mine, I didn't use AI to write the code, but I did ask it questions to understand Vulkan concepts better.
<img src="readmeAssets/TextureBatching0.png" alt="Texture Batching 0" width="400"/>
<img src="readmeAssets/TextureBatching1.png" alt="Texture Batching 0" width="400"/>

### 2026-09-06
I followed the [Vulkan tutorials](https://docs.vulkan.org/tutorial/latest/00_Introduction.html) up to loading OBJ models. I also added my own flycam.
<img src="readmeAssets/TutorialModelLoading.png" alt="Tutorial Model Loading" width="400"/>
