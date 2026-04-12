# Libraries Used
* Multimedia Library - https://www.sfml-dev.org/ 
* JSON parsing and serializing - https://github.com/nlohmann/json

# Process for Adding a New Component
* Add a new component type in the `ComponentType` enum in `components.h`
    * If adding your component causes a static assert, the `MAX_COMPONENTS` value will have to be bumped 
* Define your component as a struct in the bottom of `components.h` in the section titled "Components"
* Add a string name for the component type you just made in `s_componentTypeStrings` in `components.cpp`
* Add an array of your component type in the `Components` struct in `ecs.h`. Make the size `MAX_ENTITIES`
* You can now add your component to entities through the use of your `ComponentType` in a `ComponentMask`
    * This mask can then be used to decide what entities should be modified by your desired system which
      utilizes this component 

# Process for Creating/Deleting Entities Dynamically
* `ECS_QueueEntityCreation` takes a `ComponentMask` and queues up the creation of a new entity at the start of the next frame. 
    * Returns the ID of the entity to be created. This can be used to access the entity's components and it can be used to delete the entity
* `ECS_QueueEntityRemoval` takes an `EntityID` and queues entity for removal at the end of the current frame. 

# Process for Adding a New System
* (Optional but recommended) Create a new file for your system
* Create a class for your system which inherits from the `System` class in `core/system.h`
* Override any of your desired frame methods which are needed for your system.
    * Here are the available helper methods that you can override and the order that they are run:
        1. **OnFrameStart** - Called at the beginning of the frame
        2. **OnFrame** - Main logic phase (e.g., movement, AI, game logic)
        3. **OnFrameEnd** - End-of-frame cleanup (e.g., post-processing, state finalization)
        4. **OnPhysicsFrame** - Physics simulation and collision resolution
        5. **OnDrawFrame** - Rendering (receives `RenderWindow` to draw to)

* Create a static instance of your system. The constructor in the base class will register the system to be run during the game loop.