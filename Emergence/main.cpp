
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
#include <utility>

using namespace std;
enum experienceType {WOLVES, BEES, BIRDS, FLUID};

experienceType experience = BEES;

int width = 3000;
int height = 3000;

int size; //Size of the individuals
int nbOfParticles;
float maxDelta; //maximum distance walked by a particle in one step
float limit; //confort zone of the particles with respect to each other
float seperationIntensity; //Maximum intensity of the repulsion force between 2 wolves
float hungerLevel; //Level of attraction of the points of interest
float distanceMin; //Perimeter in which the pointOfInterest is concidered reached
float velocityInfluence; //Influence of the velocity of the swarm
float swarmCohesion = .01; //Intensity of the cohesion of the group
class Particle;
float min(float first, float second);

using vector2D = pair<float, float>;
vector2D getBarycenter(std::vector<Particle> group);
float distanceBetween(vector2D& a, vector2D& b);
pair<int, int> randomPositionBetween(pair<int, int> inf, pair<int, int> sup);



struct Particle {
    constexpr static const float mass = 10.f; //kg
    constexpr static const float squaredMaxSpeed = 50.f;
    vector2D position;
    vector2D velocity;
    vector2D forcesOnParticle;
    vector2D forcesOnSwarm;
    
    sf::RectangleShape rectangle;
    
    Particle(pair<int, int> position, sf::Color color) {
        this->position = position;
        velocity = make_pair(0, 0);
        forcesOnParticle = make_pair(0, 0);
        forcesOnSwarm = make_pair(0, 0);
        rectangle.setSize(sf::Vector2f(size, size));
        rectangle.setFillColor(color);
        rectangle.setPosition(position.first, position.second);
    }
    
    void draw(sf::RenderWindow& window) {
        window.draw(rectangle);
    }
    
    void updatePos() {
        float squaredSpeed = pow(velocity.first, 2) + pow(velocity.second, 2);
        if(squaredSpeed > squaredMaxSpeed) {
            velocity.first /= (squaredSpeed / squaredMaxSpeed);
            velocity.second /= (squaredSpeed / squaredMaxSpeed);
        }
        static const float timeDelta = 1.f; //seconde
        velocity.first += timeDelta * forcesOnParticle.first / mass;
        velocity.second += timeDelta * forcesOnParticle.second / mass;
        
        velocity.first += timeDelta * forcesOnSwarm.first / mass;
        velocity.second += timeDelta * forcesOnSwarm.second / mass;
        
        position.first += velocity.first*timeDelta;
        position.second += velocity.second*timeDelta;
        
        rectangle.setPosition(round(position.first), round(position.second));
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
        for (int i=0 ; i<nbOfParticles ; i++) {
            Particle particle(randomPositionBetween(pair<int, int>(0,0), pair<int, int>(width, height)), color);
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
        vector2D pointOfInterestAttraction = vector2D(0,0);
        vector2D* pPosition;
        vector2D* otherPosition;
        float distanceToParticle;
        float forceIntensity;
        vector2D swarmCenter = getBarycenter();
        vector2D averageVelocity = getAverageVelocity();
        
        /** BIG LOOP **/
        for (int p=0 ; p<nbOfParticles ; p++) {
            
            //Initializing forces
            particles[p].forcesOnSwarm = make_pair(0, 0);
            particles[p].forcesOnParticle = make_pair(0, 0);
            
            //computeInterestPointsInfluenceHard(particles[p]);
            computeInterestPointsInfluenceSoft(particles[p]);
            
            
            pPosition = &(particles[p].position);
            /*** COHESION ***/
            particles[p].forcesOnParticle.first += swarmCohesion*(swarmCenter.first - pPosition->first);
            particles[p].forcesOnParticle.second += swarmCohesion*(swarmCenter.second - pPosition->second);
            
            
            /*** GLOBAL ALIGNMENT ***/
            particles[p].forcesOnParticle.first += velocityInfluence/50*averageVelocity.first;
            particles[p].forcesOnParticle.second += velocityInfluence/50*averageVelocity.second;
            
            
            
            /*** SEPERATION ***/
            //Calculating the REPULSION of the horde on the wolf
            /** SMALL LOOP **/
            for (int otherP = p+1 ; otherP < nbOfParticles ; otherP++) {
                
                otherPosition = &particles[otherP].position;
                distanceToParticle = distanceBetween(*otherPosition, *pPosition);
                
                
                //distanceToPoint = -min(-distanceToPoint,-1); // max
                
                
                forceIntensity = - 0.5*seperationIntensity * exp(-distanceToParticle/limit) * (0.5 - 3*distanceToParticle);
                
                particles[p].forcesOnParticle.first += forceIntensity*(pPosition->first - otherPosition->first) / distanceToParticle; //We divide by distanceToWolf to normalize the coordinate of the force vector
                particles[p].forcesOnParticle.second += forceIntensity*(pPosition->second - otherPosition->second) / distanceToParticle; //We divide by distanceToWolf to normalize the coordinate of the force vector

                
                particles[otherP].forcesOnParticle.first += -particles[p].forcesOnParticle.first; //We divide by distanceToWolf to normalize the coordinate of the force vector
                particles[otherP].forcesOnParticle.second += -particles[p].forcesOnParticle.second;
                
                
                /*** Custom alignment ***/
                //inverse proportional to the distance of the particle
                
                //*** ??? ***//
                
                particles[p].forcesOnParticle.first += 1.*velocityInfluence/pow(distanceToParticle/10.,2)*particles[otherP].velocity.first;
                particles[p].forcesOnParticle.second += 1.*velocityInfluence/pow(distanceToParticle/10.,2)*particles[otherP].velocity.second;
                
            }
        }
        
        for (int i = 0 ; i< nbOfParticles ; i++) {
            particles[i].updatePos();
        }
    }
    
    void draw(sf::RenderWindow& window) {
        for (auto p=particles.begin() ; p<particles.end() ; p++) {
            p->draw(window);
        }
        window.draw(pointOfInterestRectangle1);
        window.draw(pointOfInterestRectangle2);
        
    }
    
    vector2D getBarycenter() {
        float avgX, avgY = 0;
        for (auto p=particles.begin() ; p<particles.end() ; p++) {
            avgX+=p->position.first;
            avgY+=p->position.second;
        }
        avgX = avgX/particles.size();
        avgY = avgY/particles.size();
        vector2D avg = vector2D(avgX, avgY);
        return avg;
    }
    
    
    vector2D getAverageVelocity() {
        float avgX, avgY = 0;
        for (auto p=particles.begin() ; p<particles.end() ; p++) {
            avgX+=p->velocity.first;
            avgY+=p->velocity.second;
        }
        avgX = avgX/particles.size();
        avgY = avgY/particles.size();
        vector2D avg = vector2D(avgX, avgY);
        return avg;
    }
    
    void computeInterestPointsInfluenceHard(Particle& p) {
        if(distanceBetween(pointOfInterest1, p.position) < distanceBetween(pointOfInterest2, p.position)) {
            //Calculating the ATTRACTION of the point of interest on the wolf
            float distanceToPoint = distanceBetween(pointOfInterest1, p.position);
            if(distanceToPoint > distanceMin) {
                p.forcesOnParticle.first = hungerLevel*(pointOfInterest1.first - p.position.first) / distanceToPoint;
                p.forcesOnParticle.second = hungerLevel*(pointOfInterest1.second - p.position.second) / distanceToPoint;
            }
        } else {
            //Calculating the ATTRACTION of the point of interest on the wolf
            float distanceToPoint = distanceBetween(pointOfInterest2, p.position);
            if(distanceToPoint > distanceMin) {
                p.forcesOnParticle.first = hungerLevel*(pointOfInterest2.first - p.position.first) / distanceToPoint;
                p.forcesOnParticle.second = hungerLevel*(pointOfInterest2.second - p.position.second) / distanceToPoint;
            }
        }
        
    }
    
    void computeInterestPointsInfluenceSoft(Particle& p) {
        static int perimetre = 600;
        float distanceToPoint = distanceBetween(pointOfInterest1, p.position);
        if(distanceToPoint > perimetre) {
            p.forcesOnParticle.first += hungerLevel*(100 + distanceToPoint-perimetre)/10000.*(pointOfInterest1.first - p.position.first);
            p.forcesOnParticle.second += hungerLevel*(100 + distanceToPoint-perimetre)/10000.*(pointOfInterest1.second - p.position.second);
        }
        
    }
    
    
    void newPointsOfInterest() {
        pointOfInterest1 = randomPositionBetween(pair<int, int>(0,0), pair<int, int>(width, height));
        pointOfInterestRectangle1.setPosition(pointOfInterest1.first, pointOfInterest1.second);
        pointOfInterest2 = randomPositionBetween(pair<int, int>(0,0), pair<int, int>(width, height));
        pointOfInterestRectangle2.setPosition(pointOfInterest2.first, pointOfInterest2.second);
    }
    
    void centerPointsOfInterest(){
        pointOfInterest1 = make_pair(width/4, height/4);
        pointOfInterest2 = make_pair(width/4, height/4);
        pointOfInterestRectangle1.setPosition(pointOfInterest1.first, pointOfInterest1.second);
        pointOfInterestRectangle2.setPosition(pointOfInterest2.first, pointOfInterest2.second);
        
    }
    
    
};

int main(int, char const**)
{
    
    if(experience == experienceType::WOLVES) {
        size = 5;
        nbOfParticles = 20;
        maxDelta = 1; //maximum distance walked by a wolf in one step
        limit = 10; //confort zone of the wolves with respect to each other
        seperationIntensity = 5; //Maximum intensity of the repulsion force between 2 wolves
        hungerLevel = 1; //Level of attraction of the points of interest
        distanceMin = 30; //Perimeter in which the pointOfInterest is concidered reached
        velocityInfluence = 50;
    } else if(experience == experienceType::BEES) {
        size = 2;
        nbOfParticles = 1200;
        maxDelta = 10; //maximum distance walked by a wolf in one step
        limit = 3; //confort zone of the wolves with respect to each other
        seperationIntensity = 2; //Maximum intensity of the repulsion force between 2 wolves
        hungerLevel = 1; //Level of attraction of the points of interest
        distanceMin = 20; //Perimeter in which the pointOfInterest is concidered reached
        velocityInfluence = .1;
    } else if(experience == experienceType::BIRDS) {
        size = 4;
        nbOfParticles = 500;
        maxDelta = 1; //maximum distance walked by a wolf in one step
        limit = 10; //confort zone of the wolves with respect to each other
        seperationIntensity = 1; //Maximum intensity of the repulsion force between 2 wolves
        hungerLevel = 1; //Level of attraction of the points of interest
        distanceMin = 100; //Perimeter in which the pointOfInterest is concidered reached
        velocityInfluence = 1;
    } else if(experience == experienceType::FLUID) {
        size = 1;
        nbOfParticles = 3000;
        maxDelta = 20; //maximum distance walked by a wolf in one step
        limit = 2; //confort zone of the wolves with respect to each other
        seperationIntensity = 2; //Maximum intensity of the repulsion force between 2 wolves
        hungerLevel = 1; //Level of attraction of the points of interest
        distanceMin = 10; //Perimeter in which the pointOfInterest is concidered reached
        velocityInfluence = .01;
    }
    
    Group group1 = Group(sf::Color::White);
    Group group2 = Group(sf::Color::Green);
    Group group3 = Group(sf::Color::Red);
    
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
                group2.newPointsOfInterest();
                group3.newPointsOfInterest();
            }
            
            //Press space to generate new points of interest
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::C) {
                group1.centerPointsOfInterest();
                group2.centerPointsOfInterest();
                group3.centerPointsOfInterest();
            }
        }
        
        // Clear screen
        window.clear(sf::Color::Black);
        
        
        group1.updatePos();
        group1.draw(window);
        /*group2.updatePos();
        group2.draw(window);
        group3.updatePos();
        group3.draw(window);*/
        window.display();
    }
    
    return EXIT_SUCCESS;
}

float distanceBetween(vector2D& a, vector2D& b) {
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
