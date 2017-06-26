
//
// Disclaimer:
// ----------
//
// This code will work only if you selected window, graphics and audio.
//
// Note that the "Run Script" build phase will copy the required frameworks
// or dylibs to your application bundle so you can execute it on any OS X
// computer.
//
// Your resource files (images, sounds, fonts, ...) are also copied to your
// application bundle. To get the path to these resources, use the helper
// function `resourcePath()` from ResourcePath.hpp
//

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

// Here is a small helper for you! Have a look.
#include "ResourcePath.hpp"
#include <vector>
#include <math.h>
#include <stdio.h>
#include <random>
#include <iostream>

using namespace std;
enum experienceType {WOLVES, BEES, BIRDS, FLUID};

experienceType experience = BEES;

int width = 800;
int height = 800;

int size; //Size of the individuals
int nbOfIndividuals;
float maxDelta; //maximum distance walked by a particle in one step
float limit; //confort zone of the particles with respect to each other
float groupLimit; //confort zone of the particles with respect to the group
float forceCoeff; //Maximum intensity of the repulsion force between 2 wolves
float hungerLevel; //Level of attraction of the points of interest
float distanceMin; //Perimeter in which the pointOfInterest is concidered reached
class Particle;
float min(float first, float second);
std::pair<float, float> getBarycenter(std::vector<Particle> group);
float distanceBetween(pair<float, float> a, pair<float, float> b);
pair<int, int> randomPositionBetween(pair<int, int> inf, pair<int, int> sup);


class Particle {
private:
    pair<float, float> position;
    sf::RectangleShape rectangle;
    
public:
    Particle(pair<int, int> position, sf::Color color) {
        this->position = position;
        rectangle.setSize(sf::Vector2f(size, size));
        rectangle.setFillColor(color);
        rectangle.setPosition(position.first, position.second);
    }
    
    void draw(sf::RenderWindow& window) {
        window.draw(rectangle);
    }
    void updatePos(pair<float, float> groupForce, pair<float, float> pointOfInterestAttraction) {
        float deltaX = groupForce.first + pointOfInterestAttraction.first;
        float deltaY = groupForce.second + pointOfInterestAttraction.second;
        
        
        position.first+=min(deltaX, maxDelta);
        position.second+=min(deltaY, maxDelta);
        rectangle.setPosition(round(position.first), round(position.second));
    }
    
    pair<float, float> getPosition() {
        return position;
    }
};


class Group {
private:
    std::vector<Particle> particles = std::vector<Particle>();
    std::pair<float, float> pointOfInterest1;
    std::pair<float, float> pointOfInterest2;
    sf::RectangleShape pointOfInterestRectangle1;
    sf::RectangleShape pointOfInterestRectangle2;
public:
    Group(sf::Color color) {
        for (int i=0 ; i<nbOfIndividuals ; i++) {
            Particle particle(randomPositionBetween(pair<int, int>(0,0), pair<int, int>(800, 600)), color);
            particles.push_back(particle);
        }
        
        newPointsOfInterest();
        pointOfInterestRectangle1.setSize(sf::Vector2f(3, 3));
        pointOfInterestRectangle1.setFillColor(sf::Color::Red);
        pointOfInterestRectangle2.setSize(sf::Vector2f(3, 3));
        pointOfInterestRectangle2.setFillColor(sf::Color::Red);
        
    }
    
    void updatePos() {
        float distanceToPoint;
        pair<float, float> pointOfInterestAttraction = pair<float, float>(0,0);
        pair<float, float> position;
        
        pair<float, float> groupForce;
        float distanceToParticle;
        
        for (auto p=particles.begin() ; p<particles.end() ; p++) {
            position = p->getPosition();
            
            if(distanceBetween(pointOfInterest1, p->getPosition())<distanceBetween(pointOfInterest2, p->getPosition())) {
                //Calculating the ATTRACTION of the point of interest on the wolf
                distanceToPoint = distanceBetween(pointOfInterest1, position);
                if(distanceToPoint > distanceMin) {
                    pointOfInterestAttraction.first = -hungerLevel*(position.first - pointOfInterest1.first) / distanceToPoint; //We divide by distanceToWolf to normalize the coordinate of the force vector
                    pointOfInterestAttraction.second = -hungerLevel*(position.second - pointOfInterest1.second) / distanceToPoint;
                }
            } else {
                //Calculating the ATTRACTION of the point of interest on the wolf
                distanceToPoint = distanceBetween(pointOfInterest2, position);
                if(distanceToPoint > distanceMin) {
                    pointOfInterestAttraction.first = -hungerLevel*(position.first - pointOfInterest2.first) / distanceToPoint; //We divide by distanceToWolf to normalize the coordinate of the force vector
                    pointOfInterestAttraction.second = -hungerLevel*(position.second - pointOfInterest2.second) / distanceToPoint;
                }
            }
            
            
            //Calculating the REPULSION of the horde on the wolf
            groupForce = pair<float, float>(0,0);
            for (auto otherParticle = particles.begin() ; otherParticle < particles.end() ; otherParticle++) {
                distanceToParticle = distanceBetween(otherParticle->getPosition(), position);
                if(distanceToParticle > limit) {
                    float forceIntensity = - forceCoeff * exp(-distanceToParticle/limit) * (1 - 2*distanceToParticle);
                    //                    float repulsionIntensity = repulsionCoeff * exp(-distanceToWolf/limit);
                    //                    cout << "distanceToWolf = " << distanceToWolf <<endl;
                    //                    cout << "forceIntensity = " << forceIntensity <<endl;
                    groupForce.first += forceIntensity*(position.first - otherParticle->getPosition().first) / distanceToParticle; //We divide by distanceToWolf to normalize the coordinate of the force vector
                    groupForce.second += forceIntensity*(position.second - otherParticle->getPosition().second) / distanceToParticle;
                }
            }
            p->updatePos(groupForce, pointOfInterestAttraction);
        }
    }
    
    void draw(sf::RenderWindow& window) {
        for (auto p=particles.begin() ; p<particles.end() ; p++) {
            p->draw(window);
        }
        window.draw(pointOfInterestRectangle1);
        window.draw(pointOfInterestRectangle2);
        
    }
    
    std::pair<float, float> getBarycenter() {
        float avgX, avgY = 0;
        for (auto p=particles.begin() ; p<particles.end() ; p++) {
            avgX+=p->getPosition().first;
            avgY+=p->getPosition().second;
        }
        avgX = avgX/particles.size();
        avgY = avgY/particles.size();
        pair<float, float> avg = pair<float, float>(avgX, avgY);
        return avg;
    }
    
    void newPointsOfInterest() {
        pointOfInterest1 = randomPositionBetween(pair<int, int>(0,0), pair<int, int>(width, height));
        pointOfInterestRectangle1.setPosition(pointOfInterest1.first, pointOfInterest1.second);
        pointOfInterest2 = randomPositionBetween(pair<int, int>(0,0), pair<int, int>(width, height));
        pointOfInterestRectangle2.setPosition(pointOfInterest2.first, pointOfInterest2.second);
    }
    
    
};

int main(int, char const**)
{
    
    if(experience == experienceType::WOLVES) {
        size = 5;
        nbOfIndividuals = 20;
        maxDelta = 1; //maximum distance walked by a wolf in one step
        limit = 10; //confort zone of the wolves with respect to each other
        groupLimit = 5; //confort zone of the wolves with respect to the horde
        forceCoeff = 5; //Maximum intensity of the repulsion force between 2 wolves
        hungerLevel = 5; //Level of attraction of the points of interest
        distanceMin = 30; //Perimeter in which the pointOfInterest is concidered reached
    } else if(experience == experienceType::BEES) {
        size = 2;
        nbOfIndividuals = 1000;
        maxDelta = 10; //maximum distance walked by a wolf in one step
        limit = 3; //confort zone of the wolves with respect to each other
        groupLimit = 100; //confort zone of the wolves with respect to the horde
        forceCoeff = 2; //Maximum intensity of the repulsion force between 2 wolves
        hungerLevel = 5; //Level of attraction of the points of interest
        distanceMin = 20; //Perimeter in which the pointOfInterest is concidered reached
    } else if(experience == experienceType::BIRDS) {
        size = 4;
        nbOfIndividuals = 500;
        maxDelta = 20; //maximum distance walked by a wolf in one step
        limit = 10; //confort zone of the wolves with respect to each other
        groupLimit = 10; //confort zone of the wolves with respect to the horde
        forceCoeff = 5; //Maximum intensity of the repulsion force between 2 wolves
        hungerLevel = 1; //Level of attraction of the points of interest
        distanceMin = 100; //Perimeter in which the pointOfInterest is concidered reached
    } else if(experience == experienceType::FLUID) {
        size = 1;
        nbOfIndividuals = 2000;
        maxDelta = 20; //maximum distance walked by a wolf in one step
        limit = 2; //confort zone of the wolves with respect to each other
        groupLimit = 5; //confort zone of the wolves with respect to the horde
        forceCoeff = 5; //Maximum intensity of the repulsion force between 2 wolves
        hungerLevel = 5; //Level of attraction of the points of interest
        distanceMin = 10; //Perimeter in which the pointOfInterest is concidered reached
    }
    
    Group group1 = Group(sf::Color::Black);
//    Group group2 = Group(sf::Color::Green);
    
    sf::RenderWindow window(sf::VideoMode(width, height), "SFML window");
    window.setFramerateLimit(60);
    
    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            
            // Escape pressed: exit
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
            
            //Press space to generate new points of interest
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                group1.newPointsOfInterest();
//                group2.newPointsOfInterest();
            }
        }
        
        // Clear screen
        window.clear(sf::Color::White);
        
        
        group1.updatePos();
        group1.draw(window);
//        group2.updatePos();
//        group2.draw(window);
        window.display();
    }
    
    return EXIT_SUCCESS;
}

float distanceBetween(pair<float, float> a, pair<float, float> b) {
    return sqrt((a.first - b.first) * (a.first - b.first) + (a.second - b.second) * (a.second - b.second));
}

float min(float first, float second) {
    if (first<second) {
        return first;
    }
    else {
        return second;
    }
}

pair<int, int> randomPositionBetween(pair<int, int> inf, pair<int, int> sup) {
    pair<int, int> position;
    static std::random_device rd;
    static std::default_random_engine engine(rd());
    std::uniform_int_distribution<unsigned> distributionX(inf.first, sup.first);
    std::uniform_int_distribution<unsigned> distributionY(inf.second, sup.second);
    position.first = distributionX(engine);
    position.second = distributionY(engine);
    return position;
}
