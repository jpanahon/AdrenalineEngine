# What is Adrenaline Engine?

It was created due to my frustrations working with Unity and Unreal due to them using up so much of my memory and storage space. Determined to one day create a full-fledged video game, I sought to create my own engine ~~with blackjack and hookers~~. 

**This engine is Free Open Source Software meaning you can use it however you want under the GNU GPLv3 license, which means all forks of this engine <u>must</u> be open source.**

NOTE: Currently this engine is in pre-beta meaning it is not suitable for end user use yet (Unless you know what you're doing).

# Current Features 🖥
<li> Support for rendering more than one 3D model </li>
<li> Limited glTF support (currently can load most glTF models with textures) </li>
<li> First person camera system </li>
<li> Limited graphical user interface (currently only thing in the user interface is camera properties) </li>
<li> Limited model manipulation (currently able to change the angle of rotation and axis, scale, and position of individual models) </li>

# Goals for the engine 🥅
<li> Implement Full User Interface.  </li>
<li> Make it very user accessable. </li>
<li> Support for 2D rendering. </li>
<li> Create a flexible framework that would easily allow other people to add in their own stuff to the engine. (Which could possible be merged into the original engine). </li>
<li> Lighting. </li>
<li> Audio. </li>
<li> Framework for game logic. </li>
<li> Possibly create a 3D modelling design suite. </li>
<li> Make it cross platform (Originally started writing this on a Macbook before I switched to a Windows laptop) </li>
<li> Ultimately create a game using the engine. </li>

# Libraries Used 📚
<li> Discord Rich Presence: </li>
<li> Rendering: https://vulkan.org/ </li>
<li> Window Creation: https://github.com/glfw/glfw </li> 
<li> Model Manipulation and Math: https://github.com/g-truc/glm </li>
<li> Model Loading: https://github.com/syoyo/tinygltf </li>
<li> Image Loading: https://github.com/nothings/stb </li>
<li> Memory Allocation: https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator </li>
<li> Graphical User Interface: https://github.com/ocornut/imgui </li>

# Special Thanks ❤
I'd like to thank the [Vulkan Discord](https://discord.gg/vulkan), [Graphics Programming Discord](https://discord.gg/6mgNGk7), [Vulkan Tutorial](https://vulkan-tutorial.com), [Vulkan Guide](https://vkguide.dev), [Brendan Galea's Vulkan Series](https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR), [Excal Engine](https://github.com/LiamHz/Excal), [Awesome Vulkan](https://github.com/vinjn/awesome-vulkan), and the greatest of all time Sascha Willems and their [Vulkan samples](https://github.com/SaschaWillems/Vulkan/tree/master/examples) for helping me figure out Vulkan.

# DISCLAIMER ⚠

This project is still in development, I consider this a hobby, and I work on it when I have time. <br>
I try to commit "stable" code, meaning the frequency of commits are low. <br>
Despite the lack of commits I am probably still working on implementing something.