/*
 Shun Sato
 7/23/2018
 COMSC165
 Lab 3
 Console Sokoban game. About the game -> https://en.wikipedia.org/wiki/Sokoban
 
 Summary:
 This is a console version of Sokoban game. Sokoban is a tile based game. The goal is to move all boxes on a map to  goal tiles by manipulating a character.
 
 The main classes I made are followings.
 1. Tile: Represents one map tile.
 2. Board: Represents game board which is comprised by tiles.
 3. Boxes: Box objects on the board inherits movable class.
 4. Human: Human objects on the board inherits movable class. A player manipulates this object.
 
 The functions I made are followings:
 1.char getContinueCommand(): Get validated user input of 'y' or 'n'.
 2.int getMapID(): Get validated user input of map ID.
 
 The data structures I used are followings.
 1.std::vector (for tiles that consists a board. I used it because it needs to be able to be accessed by index.)
 2.std::list (for movable objects on a board. I used it because it doesn't require index access.)
 */

#include <iostream>
#include <vector>
#include <list>
#include <functional>
#include <limits>

class Tile;

const std::string BOX_CHARACTER = "B";
const std::string BOX_ON_GOAL_CHARACTER = "Ⓑ";
const std::string GOAL_CHARACTER = "◯";
const std::string AISLE_CHARACTER = "□";
const std::string WALL_CHARACTER = "■";
const std::string HUMAN_CHARACTER = "H";


/**
 * Map chip definition.
 */
namespace MapChip {
    enum Types {
        AISLE = 0,
        WALL = 1,
        GOAL = 2,
        HUMAN = 3,
        BOX = 4,
    };
};

/**
 * Map definition. You can add your original map here.
 */
const std::vector<std::vector<std::vector<int>>> MAP = {
    {
        {0, 0, 0, 0, 0},
        {3, 4, 0, 1, 1},
        {0, 1, 0, 0, 0},
        {1, 0, 0, 0, 2},
        {1, 0, 0, 0, 0},
    },
    {
        {1, 1, 1, 1},
        {1, 0, 0, 0, 1, 1, 1, 1},
        {1, 0, 0, 0, 4, 3, 2, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
    },
    {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 4, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 0, 0, 4, 0, 0, 4, 0, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 2, 2, 1},
        {1, 0, 4, 0, 0, 4, 0, 0, 0, 0, 0, 0, 3, 0, 2, 2, 1},
        {1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 2, 2, 1},
        {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    },
};

/**
 * Delete pointers. Use this function instead of delete to avoid deleting same pointers twice.
 */
template<class T>
static inline void SafeDelete( T*& p ){
    delete (p);
    (p) = nullptr;
}

/**
 * 2 dimentional vector struct which represents coordinates. It contains useful functions for vector calculation.
 */
struct vec2n {
    vec2n(int x, int y)
    :x(x)
    ,y(y)
    {}
    vec2n operator+(const vec2n& v) const{
        return vec2n(x + v.x, y + v.y);
    }
    vec2n& operator+=(const vec2n& v) {
        x += v.x;
        y += v.y;
        return *this;
    }
    bool operator==(const vec2n& v) const{
        return (x == v.x && y == v.y);
    }
    int x;
    int y;
};

/**
 * Drawable interface. Every objects that need to draw itself on the display should inherit this class.
 */
class Drawable {
public:
    virtual ~Drawable() {}
    virtual void draw() const = 0;
};

/**
 * Tile class. A board is consists of tiles.
 */
class Tile: public Drawable {
public:
    // Tile status definition.
    enum Status: char {
        AISLE='I',
        WALL='W',
        GOAL='G',
    };
    Tile(MapChip::Types mapChip) {
        // Human and Boxes map chips are regarded as aisles since those objects are placed on passable area.
        switch (mapChip) {
            case MapChip::AISLE: status = AISLE; break;
            case MapChip::WALL: status = WALL; break;
            case MapChip::GOAL: status = GOAL; break;
            case MapChip::HUMAN: status = AISLE; break;
            case MapChip::BOX: status = AISLE; break;
        }
    }
    
    ~Tile() {}
    
    Status getStatus() const { return status; }
    // Wall is the only impassable tile.
    bool isPassable() const { return status != WALL; }
    void draw() const {
        switch (status) {
            case AISLE: std::cout << AISLE_CHARACTER; return;
            case WALL: std::cout << WALL_CHARACTER; return;
            case GOAL: std::cout << GOAL_CHARACTER; return;
        }
    }
private:
    Status status;
};

/**
 * Movable base class. Every movable objects(like Human and Boxes) on the map should inherit class.
 */
class Movable: public Drawable {
public:
    Movable(const vec2n& coordinates)
    :position(coordinates) {}
    virtual ~Movable() {}
    void moveBy(const vec2n& delta) {
        position += delta;
    }
    void setPosition(const vec2n& position) { this->position = position; }
    const vec2n& getPosition() const { return position; }
protected:
    vec2n position;
};

/**
 * Human class. A player operates this object.
 */
class Human : public Movable {
public:
    Human()
    :Movable(vec2n(0, 0))
    {}
    void draw() const { std::cout << HUMAN_CHARACTER; }
private:
};

/**
 * Box class. A player needs to move all the box to goals.
 */
class Box : public Movable {
public:
    Box(std::function<bool(const vec2n& coordinates)> isGoal, const vec2n& coordinates)
    :Movable(coordinates)
    ,isGoal(isGoal)
    {}
    void draw() const { std::cout << (isGoal(position) ? BOX_ON_GOAL_CHARACTER : BOX_CHARACTER); }
private:
    std::function<bool(const vec2n& coordinates)> isGoal;
};

/**
 * Board class which represents game map. Consists of tiles and movable objects.
 */
class Board {
public:
    Board() {}
    ~Board() {
        clearMap();
    }
    
    /**
     * Load map data and create tiles according to a map definition..
     * @param mapDefinition pass one of element in MAP.
     */
    void loadMap(const std::vector<std::vector<int>>& mapDefinition) {
        for (auto it=mapDefinition.begin(); it!=mapDefinition.end(); ++it) {
            std::vector<Tile*> row;
            for (auto itSecond=(*it).begin(); itSecond!=(*it).end(); ++itSecond) {
                auto coordinates = vec2n(static_cast<int>(it-mapDefinition.begin()), static_cast<int>(itSecond-(*it).begin()));
                if ((*itSecond) == MapChip::HUMAN) {
                    human.setPosition(coordinates);
                }
                if ((*itSecond) == MapChip::BOX) {
                    movables.push_back(new Box([=](const vec2n& coordinates) { return tiles[coordinates.x][coordinates.y]->getStatus() == Tile::GOAL; }, coordinates));
                }
                row.push_back(new Tile(static_cast<MapChip::Types>(*itSecond)));
            }
            tiles.push_back(row);
        }
    }
    
    /**
     * Clear all the tiles and movable objects.
     */
    void clearMap() {
        for (auto it=tiles.begin(); it!=tiles.end(); ++it) {
            for (auto itSecond=(*it).begin(); itSecond!=(*it).end(); ++itSecond) {
                SafeDelete(*itSecond);
            }
        }
        tiles.clear();
        for (auto it=movables.begin(); it!=movables.end(); ++it) {
            SafeDelete(*it);
        }
        movables.clear();
    }
    
    /**
     * Draw a game board.
     */
    void draw() const {
        for (auto it=tiles.begin(); it!=tiles.end(); ++it) {
            for (auto itSecond=(*it).begin(); itSecond!=(*it).end(); ++itSecond) {
                auto coordinates = vec2n(static_cast<int>(it-tiles.begin()), static_cast<int>(itSecond-(*it).begin()));
                if (human.getPosition() == coordinates) {
                    human.draw();
                    continue;
                }
                bool drawnObject = false;
                for (auto itMovable=movables.begin(); itMovable!=movables.end(); ++itMovable) {
                    if ((*itMovable)->getPosition() == coordinates) {
                        (*itMovable)->draw();
                        drawnObject = true;
                        break;
                    }
                }
                if (drawnObject) { continue; }
                (*itSecond)->draw();
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    
    /**
     * Move human objects by delta.
     * @param delta delta from current coordinates.
     */
    void moveHuman(const vec2n& delta) {
        moveObject(human, delta);
    }
    
    /**
     * @return true if all the boxes are on goal tiles.
     */
    bool isCleared() const {
        for (auto it=movables.begin(); it!=movables.end(); ++it) {
            auto position = (*it)->getPosition();
            if (tiles[position.x][position.y]->getStatus() != Tile::GOAL) {
                return false;
            }
        }
        return true;
    }
private:
    /**
     * Move movable objects recursively. A player can move 1 box with one operation.
     * @param movable object to move
     * @param delta the distance to move
     * @param depth current recursive depth to restrict moving multiple boxes at the same time.
     */
    bool moveObject(Movable& movable, const vec2n& delta, int depth=0) {
        if (depth > 1) {
            // cannot move 2 movable objects at one time
            return false;
        }
        
        auto newPosition = movable.getPosition() + delta;
        
        if (newPosition.x < 0 || tiles.size()-1 < newPosition.x
            || newPosition.y < 0 || tiles[newPosition.x].size()-1 < newPosition.y) {
            // outside of a board
            return false;
        }
        
        if (!tiles[newPosition.x][newPosition.y]->isPassable()) {
            // impassable tile
            return false;
        }
        
        // check collision with another object.
        for (auto it=movables.begin(); it!=movables.end(); ++it) {
            if ((*it)->getPosition() == newPosition) {
                // if there is an object on the same coordinate, try to move it.
                if (moveObject(*(*it), delta, depth+1)) {
                    movable.moveBy(delta);
                    return true;
                } else {
                    return false;
                }
            }
        }
        
        movable.moveBy(delta);
        return true;
    }
    
    /** A player manipulates this object. */
    Human human;
    /** movable objects on the board except for Human object. */
    std::list<Movable*> movables;
    std::vector<std::vector<Tile*>> tiles;
};

/**
 * Get validated user input of 'y' or 'n'.
 * @return character 'y' or 'n'
 */
char getContinueCommand() {
    char continueCommand = 'y';
    do {
        std::cout << "Do you want to try another map? y/n :";
        std::cin >> continueCommand;
    } while (continueCommand != 'y' && continueCommand != 'n');
    return continueCommand;
}

/**
 * Get validated user input of map ID.
 * @return validated map ID
 */
int getMapID() {
    int mapID = 0;
    do {
        std::cout << "Select map from 1~" << MAP.size() << ":";
        std::cin >> mapID;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } while(mapID < 1 || MAP.size() < mapID);
    return mapID;
}

/**
 * main function.
 */
int main(int argc, const char * argv[]) {
    if (MAP.empty()) { std::cout << "no map defined" << std::endl; return 0; }
    
    do {
        auto mapID = getMapID();
        auto board = Board();
        board.loadMap(MAP[mapID-1]);
        
        // Every time after a user inputs a command, it checks whether all the boxes are placed on the goals.
        while(!board.isCleared()) {
            board.draw();
            
            std::cout << "Move all the " << BOX_CHARACTER << " to " << GOAL_CHARACTER << std::endl;
            std::cout << "Enter 'w' or 'a' or 's' or 'd' to move. Enter 'u' to reset the board :";
            auto nextCommand = ' ';
            std::cin >> nextCommand;
            switch (nextCommand) {
                case 'w': board.moveHuman(vec2n(-1, 0)); break;
                case 'a': board.moveHuman(vec2n(0, -1)); break;
                case 's': board.moveHuman(vec2n(1, 0)); break;
                case 'd': board.moveHuman(vec2n(0, 1)); break;
                case 'u': board.clearMap(); board.loadMap(MAP[mapID-1]); break;
                default: std::cout << "Invalid command:" << nextCommand << std::endl << std::endl;
            }
        }
        
        // Cleared a game.
        board.draw();
        std::cout << "Congratulations!!!" << std::endl;
        
        // Ask a user to continue.
    } while(getContinueCommand() != 'n');
    
    return 0;
}
