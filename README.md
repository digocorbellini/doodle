# DOODLE
Definitely One Odd Dumb Little Engine

# Libraries Used
* SFML (Multimedia Library) - https://www.sfml-dev.org/ 
* JSON parsing and serializing - https://github.com/nlohmann/json
* LuaJIT - https://luajit.org/

# Process for Adding a New Component
* Go to `components.h` and add your new component to the `COMPONENT_LIST` macro in the following format `COMPONENT(<YourComponentName>)`
    * If adding your component causes a static assert, the `MAX_COMPONENTS` value will have to be bumped 
* Define your component as a struct in the bottom of `components.h` in the section titled "Components"
    * The name of your component struct must exactly match your component definition in the `COMPONENT_LIST` macro
* You can now add your component to entities through the use of the `ComponentType` enum value, which matches your component name defined in `MAX_COMPONENTS`, in a `ComponentMask`
    * This mask can then be used to decide what entities should be modified by your desired system which utilizes this component 
* Go to `scene_component_parsers.cpp` and create a function with the following definition `void ComponentParser_<Component Name>Parser( const json& jsonComponentValues, <Component Type>* entityComponent, SceneLoader* sceneLoader )`
  * Note that the parameter names should match exactly if you want to use the helper parsing macros.
  * Within this new function, parse the content of `jsonComponentValues` into your new component struct found in the parameter `entityComponent`
    * The top of the file contains parsing helpers in the form of macros for simplifying the parsing logic
  * Ex:
    ```c++
    void ComponentParser_EntityTransform2DParser( const json& jsonComponentValues, EntityTransform2D* entityComponent, SceneLoader* sceneLoader )
    {
        PARSE_VECTOR2F_FIELD( position );
        PARSE_VECTOR2F_FIELD( scale );
    }
    ```

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
* Every frame function will have an `EntityIterator` as a parameter which can be used to iterate over all entities
  * Ex: 
    ```c++ 
    for ( EntityIterator itr = entityIterator; itr != entityIterator.end(); ++itr )
    {
        EntityID currEntity = *itr;
    }
    ```
* Within the entity iteration, you can use `ECS_GetEntityComponentsMask` to get a given entity's component mask to see if your system should operate on a given entity.
  * You can then use `ECS_GetEntityComponent` to get an entity's component
  * Ex:
    ```c++ 
    for ( EntityIterator itr = entityIterator; itr != entityIterator.end(); ++itr )
    {
        EntityID currEntity = *itr;

        // skip entities that do not have the required components 
        if ( !ECS_GetEntityComponentsMask( currEntity ).ContainsMask( ComponentsMask( { ComponentType::SpriteRenderer2D, ComponentType::EntityTransform2D } ) ) )
        {
            continue;
        }

        // get desired components 
        SpriteRenderer2D* currRend = ECS_GetEntityComponent<SpriteRenderer2D>( currEntity, ComponentType::SpriteRenderer2D );
        if ( !currRend )
        {
            continue;
        }

        // perform logic on entity components...
    }
    ```
* Create a static instance of your system. The constructor in the base class will register the system to be run during the game loop.

# Process for Adding a New Resource 
* In `resource_types.h`, add your new resource type to the `ALL_RESOURCE_TYPES` macro.
    * Define resources using the `RESOURCES( <enum name>, <type name> )` format.
    * The enum name and the type name should be the **same**, except if additional scoping is needed for the type (Ex: `sf::Texture`)
        * Example: `RESOURCE(Texture, sf::Texture)`
        * If they are not the same, some assumptions will be broken and your resource might not work.
* In `resource_manager.cpp` under the `Private Resource Loaders` section, overload the `LoadAndCacheResource` function for your new resource type.
    * This function should load in the new resource into the heap and then cache the new resource pointer using the `CacheResource` private helper function.
    * Use the `RESOURCE_DELETOR` macro for creating a resource deleting callback function if applicable. 

# Temporary Building Process
* Currently don't have a building pipeline, so using Visual Studio 2022 itself to build with MCVS C++ 20
* In "Configuration Properties", the "Debugging -> Working Directory" is set to `$(OutDir)` so that the debugging process can simulate running the built exe directly
* The "General -> Output Directory" is also set to `$(SolutionDir)\..\builds\$(Platform)\$(Configuration)\`
* The "General -> Intermediate Directory" is also set to `$(SolutionDir)\..\builds\intermediate\$(Platform)\$(Configuration)\`
* The long term plan is to have a tool which will perform the compilation and launching of the executable (along with some potential post-compilation tasks) 
    * This way these per-user settings don't have to be touched and you don't have to rely on using the vs code debugger.

# Exposing C++ Functions to Lua Scripts
* In `lua_function_registration.h`, find your desired library macro under the `Lua Function Macro Declarations` comment.
    * Ex: `LUA_ENGINE_SYSTEM_FUNCTIONS` is the library macro for the "Engine" library of API functions
    * A "library" in this case is just a way to group functions when they are called from Lua.
        * Ex: for the "Engine" library, its functions will be called like `Engine.functionName()`
* Create a new `LUA_FUNCTION` entry. This takes 2 parameters: the lua function signature, and your new API function signature in C++
* You can now define your new API function using the same function signature as in the `LUA_FUNCTION` macro in your selected library's .cpp file found in the `code\src\lua\scripts_api` directory.
* **All API functions should follow the following format:** `int <function name>( lua_State* luaState );`
* If you would like to create a new library to place your function into, create a new macro for your library under the `Lua Function Macro Declarations` comment
* Then go to the `LUA_FUNCTION_LIBRARIES` macro in the same file and create a new `LUA_LIBRARY` entry. This macro takes 2 parameters: the library name and the macro representing all of the library's API functions which should be declared above in this file.
    * Rcommended: also create a new .cpp file in the `code\src\lua\scripts_api` directory which is where all API function definitions for your library will live.