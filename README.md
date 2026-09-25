# Stormlaser - A Vulkan Renderer
## Changelog
### 2026-09-24
I implemented texture batching and bindless textures. Textures are now uploaded with a single staging buffer, and textures are referenced via an index into a single descriptor array. This was the first addition I made to the renderer without the help of tutorials. Since learning to write high quality code is still a priority of mine, I didn't use AI to write the code, but I did ask it questions to understand Vulkan concepts better.
<img src="readmeAssets/TextureBatching0.png" alt="Texture Batching 0" width="400"/>
<img src="readmeAssets/TextureBatching1.png" alt="Texture Batching 0" width="400"/>

### 2026-09-06
I followed the [Vulkan tutorials](https://docs.vulkan.org/tutorial/latest/00_Introduction.html) up to loading OBJ models. I also added my own flycam.
<img src="readmeAssets/TutorialModelLoading.png" alt="Tutorial Model Loading" width="400"/>
