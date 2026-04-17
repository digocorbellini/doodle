#include <SFML/Graphics.hpp>
#include "core/ecs.h"

// TODO: testing
#include "core/scene_loader.h"

int main ()
{
    // TODO: have to call all "Init"s for all core systems
    // then have to start game loop
    //Resource *test = new TextureResource( "hi there!" );
    
    //sf::RenderWindow window ( sf::VideoMode ( 200, 200 ), "SFML works!" );
    //sf::CircleShape shape ( 100.f );
    //shape.setFillColor ( sf::Color::Green );

    //// TODO: testing
    //SceneLoader::GetInstance().LoadSceneFile( "" );


    //while( window.isOpen () )
    //{
    //    sf::Event event;
    //    while( window.pollEvent ( event ) )
    //    {
    //        if( event.type == sf::Event::Closed )
    //            window.close ();
    //    }

    //    window.clear ();
    //    window.draw ( shape );
    //    window.display ();
    //}

    // TODO: remove this testing logic
    ResourceManager_LoadSceneAssets( nullptr );

    ECS_StartGameLoop();

    return 0;
}