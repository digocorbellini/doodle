# Libraries Used
* Multimedia Library - https://www.sfml-dev.org/ 
* JSON parsing and serializing - https://github.com/nlohmann/json

# Process for Adding a New Component
* Go to `components.h` and add your new component to the `COMPONENT_LIST` macro in the following format `COMPONENT(<YourComponentName>)`
    * If adding your component causes a static assert, the `MAX_COMPONENTS` value will have to be bumped 
* Define your component as a struct in the bottom of `components.h` in the section titled "Components"
    * The name of your component struct must exactly match your component definition in the `COMPONENT_LIST` macro
* You can now add your component to entities through the use of the `ComponentType` enum value, which matches your component name defined in `MAX_COMPONENTS`, in a `ComponentMask`
    * This mask can then be used to decide what entities should be modified by your desired system which utilizes this component 
* TODO: need to add JSON deserialization logic needed once the scene loader design is finalized.

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