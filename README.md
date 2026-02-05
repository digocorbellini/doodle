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

# Process for Creating Entities
TBD

# Process for Adding a New System
TBD