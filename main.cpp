#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include<chrono>
#include<thread>
#include <fstream> // Include the necessary header for file I/O

const int textureSize = 32;
const int maxAnswerSize = 100;
const int textureSize = 8;
const int maxAnswerSize = 200;

std::vector<int> recursionInfo;
int depthOfRecursion = 0;

int timesBrokenEarlyBecauseOfCycle = 0;

int rotations[maxAnswerSize][maxAnswerSize];

struct allSprites {
    sf::Texture blobTexture;
    sf::Sprite blobSprite;

    sf::Texture straightTexture;
    sf::Sprite straightSprite;

    sf::Texture LshapeTexture;
    sf::Sprite LshapeSprite;

    sf::Texture TshapeTexture;
    sf::Sprite TshapeSprite;

    sf::Texture blobTextureActive;
    sf::Sprite blobSpriteActive;

    sf::Texture straightTextureActive;
    sf::Sprite straightSpriteActive;

    sf::Texture LshapeTextureActive;
    sf::Sprite LshapeSpriteActive;

    sf::Texture TshapeTextureActive;
    sf::Sprite TshapeSpriteActive;

    sf::Texture blobLockedTexture;
    sf::Sprite blobLockedSprite;

    sf::Texture blobActiveLockedTexture;
    sf::Sprite blobActiveLockedSprite;

    sf::Texture straightLockedTexture;
    sf::Sprite straightLockedSprite;

    sf::Texture straightActiveLockedTexture;
    sf::Sprite straightActiveLockedSprite;

    sf::Texture LshapeLockedTexture;
    sf::Sprite LshapeLockedSprite;

    sf::Texture LshapeActiveLockedTexture;
    sf::Sprite LshapeActiveLockedSprite;

    sf::Texture TshapeLockedTexture;
    sf::Sprite TshapeLockedSprite;

    sf::Texture TshapeActiveLockedTexture;
    sf::Sprite TshapeActiveLockedSprite;

    sf::Font font;
};

struct block {

    bool blob;
    bool straight;
    bool Lshape;
    bool Tshape;

    int rotation; //0-left, 1-up, 2-right, 3-down
    sf::Transform transform;

    bool locked;

    bool holeAtBottom;
    bool holeAtTop;
    bool holeAtRight;
    bool holeAtLeft;

    bool active;

};

void writeCurrentPuzzleToFile(block** game, int n, int m) {
    std::ofstream outFile("puzzle.txt"); // Open the file in write mode
    if (!outFile.is_open()) {
        std::cout << "Error opening the file for writing." << std::endl;
        return;
    }

    // Write the dimensions of the puzzle (n and m)
    outFile << n << " " << m << std::endl;

    // Write the puzzle configuration to the file
    for (int b = 0; b < m; b++) {
        for (int a = 0; a < n; a++) {
            if (game[a][b].blob) {
                outFile << "b";
            } else if (game[a][b].straight) {
                outFile << "s";
            } else if (game[a][b].Lshape) {
                outFile << "l";
            } else if (game[a][b].Tshape) {
                outFile << "t";
            }
        }
        outFile << std::endl;
    }

    outFile.close(); // Close the file after writing
}

void readPuzzleFromFile(block**& game, int& n, int& m) {
    std::ifstream inFile("puzzle.txt"); // Open the file in read mode
    if (!inFile.is_open()) {
        std::cout << "Error opening the file for reading." << std::endl;
        return;
    }

    int a, b;
    // Read the dimensions of the puzzle (n and m) from the file
    inFile >> a >> b;
    if (a!=n || b!=m) {
        std::cout << "READING FAILED, you currently have a " << n << "x" << m << " puzzle,\n";
        std::cout << " but you are trying to read a " << a << "x" << b << " puzzle\n";
        return;
    }

    // Resize the game array if necessary
    if (game != nullptr) {
        for (int i = 0; i < n; i++) {
            delete[] game[i];
        }
        delete[] game;
    }

    game = new block*[n];
    for (int i = 0; i < n; ++i) {
        game[i] = new block[m];
    }

    // Read the puzzle configuration from the file and update the game array
    for (int b = 0; b < m; b++) {
        std::string line;
        inFile >> line;
        for (int a = 0; a < n; a++) {
            char blockType = line[a];
            game[a][b].blob = (blockType == 'b');
            game[a][b].straight = (blockType == 's');
            game[a][b].Lshape = (blockType == 'l');
            game[a][b].Tshape = (blockType == 't');
            game[a][b].locked = false; // Reset the locked status when loading the puzzle
            game[a][b].rotation = 0; // Reset the rotation when loading the puzzle
        }
    }

    inFile.close(); // Close the file after reading
}

void copyFirstGameToSecondGame(block** game1, block** game2, int n, int m) {

    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            game2[a][b].blob = game1[a][b].blob;
            game2[a][b].straight = game1[a][b].straight;
            game2[a][b].Lshape = game1[a][b].Lshape;
            game2[a][b].Tshape = game1[a][b].Tshape;

            game2[a][b].rotation = game1[a][b].rotation;
            game2[a][b].transform = game1[a][b].transform;

            game2[a][b].locked = game1[a][b].locked;

            game2[a][b].holeAtBottom = game1[a][b].holeAtBottom;
            game2[a][b].holeAtTop = game1[a][b].holeAtTop;
            game2[a][b].holeAtRight = game1[a][b].holeAtRight;
            game2[a][b].holeAtLeft = game1[a][b].holeAtLeft;

            game2[a][b].active = game1[a][b].active;
        }
    }

}

void rotate(block** game, int a, int b) {
    game[a][b].rotation = (game[a][b].rotation + 1) % 4;
    game[a][b].transform = sf::Transform().rotate(game[a][b].rotation * 90);
}

void dfsGenerator(block** game, int n, int m) {

    //zero everything
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            game[a][b].blob = 0;
            game[a][b].straight = 0;
            game[a][b].Tshape = 0;
            game[a][b].Lshape = 0;

            game[a][b].locked = 0;

            game[a][b].holeAtBottom = 0;
            game[a][b].holeAtTop = 0;
            game[a][b].holeAtRight = 0;
            game[a][b].holeAtLeft = 0;
        }
    }

    std::vector<int> stack1;
    std::vector<int> stack2;

    int randomA = std::rand()%n;
    int randomB = std::rand()%m;

    game[randomA][randomB].locked = 1;
    stack1.push_back(randomA);
    stack2.push_back(randomB);

    while (stack1.size()>0) {
        
        int a = stack1.back();
        int b = stack2.back();
        stack1.pop_back();
        stack2.pop_back();

        // std::cout << "\nwe are looking at this cell: " << a << ", " << b << "\n";

        //does it have unvisited neighbours?
        int neighbours = 0;
        if (a - 1 >= 0) {
            if (game[a - 1][b].locked == false) {
                neighbours++;
            }
        }

        if (a + 1 < n) {
            if (game[a + 1][b].locked == false) {
                neighbours++;
            }
        }

        if (b - 1 >= 0) {
            if (game[a][b - 1].locked == false) {
                neighbours++;
            }
        }

        if (b + 1 < m) {
            if (game[a][b + 1].locked == false) {
                neighbours++;
            }
        }

        int currentCount = game[a][b].holeAtBottom + game[a][b].holeAtTop + game[a][b].holeAtLeft + game[a][b].holeAtRight;
        if (neighbours>0 && currentCount<3) {
            
            // std::cout << "it has unvisited neighbours!\n";

            stack1.push_back(a);
            stack2.push_back(b);

            //choose one neighbour
            bool doWeContinue = true;
            int neighbourCoordA;
            int neighbourCoordB;

            while (doWeContinue) {

                int direction = std::rand()%4;
                // std::cout << "stuck in dowecontinue, direction: " << direction << "\n";

                if (direction==0) {
                    //up
                    if (a+1<n) {
                        if (game[a+1][b].locked==false) {
                            // std::cout << "going up\n";
                            doWeContinue = false;
                            neighbourCoordA = a+1;
                            neighbourCoordB = b;

                            game[a+1][b].holeAtBottom = true;
                            game[a][b].holeAtTop = true;
                        }
                    }
                } else if (direction==1) {
                    //right
                    if (b+1<m) {
                        if (game[a][b+1].locked==false) {
                            // std::cout << "going right\n";
                            doWeContinue = false;
                            neighbourCoordA = a;
                            neighbourCoordB = b+1;

                            game[a][b+1].holeAtLeft = true;
                            game[a][b].holeAtRight = true;
                        }
                    }
                } else if (direction==2) {
                    //down
                    if (a-1>=0) {
                        if (game[a-1][b].locked==false) {
                            // std::cout << "going down\n";
                            doWeContinue = false;
                            neighbourCoordA = a-1;
                            neighbourCoordB = b;

                            game[a-1][b].holeAtTop = true;
                            game[a][b].holeAtBottom = true;
                        }
                    }
                } else {
                    //left
                    if (b-1>=0) {
                        if (game[a][b-1].locked==false) {
                            // std::cout << "going left\n";
                            doWeContinue = false;
                            neighbourCoordA = a;
                            neighbourCoordB = b-1;

                            game[a][b-1].holeAtRight = true;
                            game[a][b].holeAtLeft = true;
                        }
                    }
                }
            }

            //neighbour at neighbourcoords is chosen


            //mark the chosen cell as visited, push it to the stack
            game[neighbourCoordA][neighbourCoordB].locked = true;
            stack1.push_back(neighbourCoordA);
            stack2.push_back(neighbourCoordB);

        }

    }

    // std::cout << "EMPTY STACK REACHED!!!\n";

    //actually do stuff here
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {

            int count = game[a][b].holeAtBottom + game[a][b].holeAtTop + game[a][b].holeAtLeft + game[a][b].holeAtRight;
            if (count==4) {
                std::cout << "WE HAVE  ACOUNT OF FOUR!!!\n";
                //this should never happen 
            } else if (count==3) {
                game[a][b].blob = 0;
                game[a][b].straight = 0;
                game[a][b].Tshape = 1;
                game[a][b].Lshape = 0;
            } else if (count==1) {
                game[a][b].blob = 1;
                game[a][b].straight = 0;
                game[a][b].Tshape = 0;
                game[a][b].Lshape = 0;
            } else if (count==2) {

                if ((game[a][b].holeAtBottom && game[a][b].holeAtTop) || (game[a][b].holeAtLeft && game[a][b].holeAtRight)) {
                    game[a][b].blob = 0;
                    game[a][b].straight = 1;
                    game[a][b].Tshape = 0;
                    game[a][b].Lshape = 0;
                } else {
                    game[a][b].blob = 0;
                    game[a][b].straight = 0;
                    game[a][b].Tshape = 0;
                    game[a][b].Lshape = 1;
                }

            }

            int randomrotation = std::rand()%4;
            for (int a=0; a<randomrotation; a++) {
                rotate(game, a, b);
            }

        }
    }

    //unlock everything
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            game[a][b].locked = 0;
        }
    }

    return;

}

void activationHelper(block** game, int n, int m, int x, int y) {

    bool doWeHaveToActivateUp = false;
    bool doWeHaveToActivateDown = false;
    bool doWeHaveToActivateLeft = false;
    bool doWeHaveToActivateRight = false;

    bool isLeftUnreachable = false;
    bool isRightUnreachable = false;
    bool isUpUnreachable = false;
    bool isDownUnreachable = false;

    //checking whether we have to activate up;
    if (y-1>=0) {
        if (game[x][y-1].blob && game[x][y-1].rotation==1 && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].blob && !(game[x][y-1].rotation==1) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].straight && (game[x][y-1].rotation%2==1) && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].straight && !(game[x][y-1].rotation%2==1) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].Tshape && game[x][y-1].rotation!=0 && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].Tshape && !(game[x][y-1].rotation!=0) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].Lshape && (game[x][y-1].rotation==1 || game[x][y-1].rotation==2) && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].Lshape && !(game[x][y-1].rotation==1 || game[x][y-1].rotation==2) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
    } else {
        isUpUnreachable = true;
    }

    //checking whether we have to activate down;
    if (y+1<m) {
        if (game[x][y+1].blob && game[x][y+1].rotation==3 && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].blob && !(game[x][y+1].rotation==3) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].straight && (game[x][y+1].rotation%2==1) && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].straight && !(game[x][y+1].rotation%2==1) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].Tshape && game[x][y+1].rotation!=2 && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].Tshape && !(game[x][y+1].rotation!=2) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].Lshape && (game[x][y+1].rotation==0 || game[x][y+1].rotation==3) && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].Lshape && !(game[x][y+1].rotation==0 || game[x][y+1].rotation==3) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
    } else {
        isDownUnreachable = true;
    }

    //checking whether we have to activate left
    if (x-1>=0) {
        if (game[x-1][y].blob && game[x-1][y].rotation==0 && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].blob && !(game[x-1][y].rotation==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].straight && (game[x-1][y].rotation%2==0) && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].straight && !(game[x-1][y].rotation%2==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].Tshape && game[x-1][y].rotation!=3 && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].Tshape && !(game[x-1][y].rotation!=3) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].Lshape && (game[x-1][y].rotation==1 || game[x-1][y].rotation==0) && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].Lshape && !(game[x-1][y].rotation==1 || game[x-1][y].rotation==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
    } else {
        isLeftUnreachable = true;
    }

    //checking whether we have to activate right
    if (x+1<n) {
        if (game[x+1][y].blob && game[x+1][y].rotation==2 && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].blob && !(game[x+1][y].rotation==2) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].straight && (game[x+1][y].rotation%2==0) && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].straight && !(game[x+1][y].rotation%2==0) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].Tshape && game[x+1][y].rotation!=1 && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].Tshape && !(game[x+1][y].rotation!=1) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].Lshape && (game[x+1][y].rotation==2 || game[x+1][y].rotation==3) && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].Lshape && !(game[x+1][y].rotation==2 || game[x+1][y].rotation==3) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
    } else {
        isRightUnreachable = true;
    }

    //std::cout
    //unreachables count here
    int count = doWeHaveToActivateDown + doWeHaveToActivateUp + doWeHaveToActivateLeft + doWeHaveToActivateRight;
    int unreachablesCount = isLeftUnreachable + isRightUnreachable + isDownUnreachable + isUpUnreachable;


    std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
    std::cout << "u clicked a block at :" << x << ", " << y << "\n";

    std::cout << "Count: " << count << "\n\n";
    std::cout << "doWeHaveToActivateUp: " << doWeHaveToActivateUp << "\n";
    std::cout << "doWeHaveToActivateRight: " << doWeHaveToActivateRight << "\n";
    std::cout << "doWeHaveToActivateDown: " << doWeHaveToActivateDown << "\n";
    std::cout << "doWeHaveToActivateLeft: " << doWeHaveToActivateLeft << "\n\n\n\n";

    std::cout << "Unreachables Count: " << unreachablesCount << "\n\n";
    std::cout << "isUpUnreachable: " << isUpUnreachable << "\n";
    std::cout << "isRightUnreachable: " << isRightUnreachable << "\n";
    std::cout << "isDownUnreachable: " << isDownUnreachable << "\n";
    std::cout << "isLeftUnreachable: " << isLeftUnreachable << "\n";


}

void primsGenerator(block** game, int n, int m) {

    //zero everything
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            game[a][b].blob = 0;
            game[a][b].straight = 0;
            game[a][b].Tshape = 0;
            game[a][b].Lshape = 0;

            game[a][b].locked = 0;

            game[a][b].holeAtBottom = 0;
            game[a][b].holeAtTop = 0;
            game[a][b].holeAtRight = 0;
            game[a][b].holeAtLeft = 0;
        }
    }

    //visited;
    std::vector<int> stack1;
    std::vector<int> stack2;

    //visit the first random cell
    int randomA = std::rand()%n;
    int randomB = std::rand()%m;
    game[randomA][randomB].locked = 1;
    stack1.push_back(randomA);
    stack2.push_back(randomB);


    while (stack1.size()<n*m) {

        int coords = std::rand()%stack1.size();
        int a = stack1[coords];
        int b = stack2[coords];
        // std::cout << "WE CHOSE THIS BLOCK: " << a << ", " << b << "\n";

        //does it have unvisited neighbours?
        int neighbours = 0;
        if (a - 1 >= 0) {
            if (game[a - 1][b].locked == false) {
                neighbours++;
            }
        }

        if (a + 1 < n) {
            if (game[a + 1][b].locked == false) {
                neighbours++;
            }
        }

        if (b - 1 >= 0) {
            if (game[a][b - 1].locked == false) {
                neighbours++;
            }
        }

        if (b + 1 < m) {
            if (game[a][b + 1].locked == false) {
                neighbours++;
            }
        }

        int currentCount = game[a][b].holeAtBottom + game[a][b].holeAtTop + game[a][b].holeAtLeft + game[a][b].holeAtRight;

        if (neighbours>0 && currentCount<3) {
            
            // std::cout << "it has unvisited neighbours!\n";

            // stack1.push_back(a);
            // stack2.push_back(b);

            //choose one neighbour
            bool doWeContinue = true;
            int neighbourCoordA;
            int neighbourCoordB;

            while (doWeContinue) {

                int direction = std::rand()%4;
                // std::cout << "stuck in dowecontinue, direction: " << direction << "\n";

                if (direction==0) {
                    //up
                    if (a+1<n) {
                        if (game[a+1][b].locked==false) {
                            // std::cout << "going up\n";
                            doWeContinue = false;
                            neighbourCoordA = a+1;
                            neighbourCoordB = b;

                            game[a+1][b].holeAtBottom = true;
                            game[a][b].holeAtTop = true;
                        }
                    }
                } else if (direction==1) {
                    //right
                    if (b+1<m) {
                        if (game[a][b+1].locked==false) {
                            // std::cout << "going right\n";
                            doWeContinue = false;
                            neighbourCoordA = a;
                            neighbourCoordB = b+1;

                            game[a][b+1].holeAtLeft = true;
                            game[a][b].holeAtRight = true;
                        }
                    }
                } else if (direction==2) {
                    //down
                    if (a-1>=0) {
                        if (game[a-1][b].locked==false) {
                            // std::cout << "going down\n";
                            doWeContinue = false;
                            neighbourCoordA = a-1;
                            neighbourCoordB = b;

                            game[a-1][b].holeAtTop = true;
                            game[a][b].holeAtBottom = true;
                        }
                    }
                } else {
                    //left
                    if (b-1>=0) {
                        if (game[a][b-1].locked==false) {
                            // std::cout << "going left\n";
                            doWeContinue = false;
                            neighbourCoordA = a;
                            neighbourCoordB = b-1;

                            game[a][b-1].holeAtRight = true;
                            game[a][b].holeAtLeft = true;
                        }
                    }
                }
            }

            //neighbour at neighbourcoords is chosen


            //mark the chosen cell as visited, push it to the stack
            game[neighbourCoordA][neighbourCoordB].locked = true;
            stack1.push_back(neighbourCoordA);
            stack2.push_back(neighbourCoordB);

        }

    }

    // std::cout << "EMPTY STACK REACHED!!!\n";

    //actually do stuff here
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {

            int count = game[a][b].holeAtBottom + game[a][b].holeAtTop + game[a][b].holeAtLeft + game[a][b].holeAtRight;
            if (count==4) {
                std::cout << "WE HAVE  ACOUNT OF FOUR!!!\n";
                std::cout << "at a: " << a << ", b: " << b << "\n";
                //shouldnthappne 
            } else if (count==0) {
                //shouldnt happen either
                std::cout << "WE HAVE  ACOUNT OF ZERO!!! somehow....\n";
                std::cout << "at a: " << a << ", b: " << b << "\n";
            } else if (count==3) {
                game[a][b].blob = 0;
                game[a][b].straight = 0;
                game[a][b].Tshape = 1;
                game[a][b].Lshape = 0;
            } else if (count==1) {
                game[a][b].blob = 1;
                game[a][b].straight = 0;
                game[a][b].Tshape = 0;
                game[a][b].Lshape = 0;
            } else if (count==2) {

                if ((game[a][b].holeAtBottom && game[a][b].holeAtTop) || (game[a][b].holeAtLeft && game[a][b].holeAtRight)) {
                    game[a][b].blob = 0;
                    game[a][b].straight = 1;
                    game[a][b].Tshape = 0;
                    game[a][b].Lshape = 0;
                } else {
                    game[a][b].blob = 0;
                    game[a][b].straight = 0;
                    game[a][b].Tshape = 0;
                    game[a][b].Lshape = 1;
                }

            }

            int randomrotation = std::rand()%4;
            for (int a=0; a<randomrotation; a++) {
                rotate(game, a, b);
            }

        }
    }

    //unlock everything
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            game[a][b].locked = 0;
        }
    }

    return;

}

void activateWater(block** game, int n, int m, int x, int y) {

    if (game[x][y].active==true) {
        return;
    }

    game[x][y].active = 1;

    bool canWeActivateUp = false;
    bool canWeActivateDown = false;
    bool canWeActivateLeft = false;
    bool canWeActivateRight = false;

    //checking whether we can activate up
    if (y-1>=0) {
        if (game[x][y-1].blob && game[x][y-1].rotation==1) {
            // std::cout << "and we did it here\n";
            canWeActivateUp = true;
        }
        if (game[x][y-1].straight && (game[x][y-1].rotation%2==1)) {
            canWeActivateUp = true;
        }
        if (game[x][y-1].Tshape && game[x][y-1].rotation!=0) {
            canWeActivateUp = true;
        }
        if (game[x][y-1].Lshape && (game[x][y-1].rotation==1 || game[x][y-1].rotation==2)) {
            canWeActivateUp = true;
        }
    }

    //checking whether we can activate down
    if (y+1<m) {
        if (game[x][y+1].blob && game[x][y+1].rotation==3) {
            canWeActivateDown = true;
        }
        if (game[x][y+1].straight && (game[x][y+1].rotation%2==1)) {
            canWeActivateDown = true;
        }
        if (game[x][y+1].Tshape && game[x][y+1].rotation!=2) {
            canWeActivateDown = true;
        }
        if (game[x][y+1].Lshape && (game[x][y+1].rotation==0 || game[x][y+1].rotation==3)) {
            canWeActivateDown = true;
        }
    }

    //checking whether we can activate left
    if (x-1>=0) {
        if (game[x-1][y].blob && game[x-1][y].rotation==0) {
            canWeActivateLeft = true;
        }
        if (game[x-1][y].straight && (game[x-1][y].rotation%2==0)) {
            canWeActivateLeft = true;
        }
        if (game[x-1][y].Tshape && game[x-1][y].rotation!=3) {
            canWeActivateLeft = true;
        }
        if (game[x-1][y].Lshape && (game[x-1][y].rotation==1 || game[x-1][y].rotation==0)) {
            canWeActivateLeft = true;
        }
    }

    //checking whether we can activate right
    if (x+1<n) {
        if (game[x+1][y].blob && game[x+1][y].rotation==2) {
            canWeActivateRight = true;
        }
        if (game[x+1][y].straight && (game[x+1][y].rotation%2==0)) {
            canWeActivateRight = true;
        }
        if (game[x+1][y].Tshape && game[x+1][y].rotation!=1) {
            canWeActivateRight = true;
        }
        if (game[x+1][y].Lshape && (game[x+1][y].rotation==2 || game[x+1][y].rotation==3)) {
            canWeActivateRight = true;
        }
    }


    //recursion
    if (game[x][y].blob) {

        if (game[x][y].rotation==0) {
            if (canWeActivateRight) {
                activateWater(game, n, m, x+1, y);
            }
        } else if (game[x][y].rotation==1) {
            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1);
            }
        } else if (game[x][y].rotation==2) {
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y);
            }
        } else {
            //rotation = 3
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1);
            }
        }

    } else if (game[x][y].Tshape) {

        if (game[x][y].rotation==0) {
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y);
            }
            if (canWeActivateRight) {
                activateWater(game, n, m, x+1, y);
            }
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1);
            }
        } else if (game[x][y].rotation==1) {
            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1);
            }
            if (canWeActivateRight) {
                activateWater(game, n, m, x+1, y);
            }
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1);
            }
        } else if (game[x][y].rotation==2) {
            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1);
            }
            if (canWeActivateRight) {
                activateWater(game, n, m, x+1, y);
            }
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y);
            }
        } else {
            //rotation = 3
            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1);
            }
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1);
            }
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y);
            }
            // if (canWeActivateRight) {
            //     activateWater(game, n, m, x, y+1);
            // }
        }

    } else if (game[x][y].straight) {

        if (game[x][y].rotation%2==0) {
            
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y);
            }
            if (canWeActivateRight) {
                activateWater(game, n, m, x+1, y);
            }

        } else {

            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1);
            }
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1);
            }

        }

    } else {
        //lshape
        if (game[x][y].rotation==0) {
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1);
            }
            if (canWeActivateRight) {
                activateWater(game, n, m, x+1, y);
            }
        } else if (game[x][y].rotation==1) {
            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1);
            }
            if (canWeActivateRight) {
                activateWater(game, n, m, x+1, y);
            }
        } else if (game[x][y].rotation==2) {
            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1);
            }
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y);
            }
        } else {
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1);
            }
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y);
            }          
        }
    }

}

void activityOfThisBlock(block** game, int n, int m, int x, int y) {

    bool canWeActivateUp = false;
    bool canWeActivateDown = false;
    bool canWeActivateLeft = false;
    bool canWeActivateRight = false;

    //checking whether we can activate up
    if (y-1>=0) {
        if (game[x][y-1].blob && game[x][y-1].rotation==1) {
            //std::cout << "and we did it here\n";
            canWeActivateUp = true;
        }
        if (game[x][y-1].straight && (game[x][y-1].rotation%2==1)) {
            canWeActivateUp = true;
        }
        if (game[x][y-1].Tshape && game[x][y-1].rotation!=0) {
            canWeActivateUp = true;
        }
        if (game[x][y-1].Lshape && (game[x][y-1].rotation==1 || game[x][y-1].rotation==2)) {
            canWeActivateUp = true;
        }
    }

    //checking whether we can activate down
    if (y+1<m) {
        if (game[x][y+1].blob && game[x][y+1].rotation==3) {
            canWeActivateDown = true;
        }
        if (game[x][y+1].straight && (game[x][y+1].rotation%2==1)) {
            canWeActivateDown = true;
        }
        if (game[x][y+1].Tshape && game[x][y+1].rotation!=2) {
            canWeActivateDown = true;
        }
        if (game[x][y+1].Lshape && (game[x][y+1].rotation==0 || game[x][y+1].rotation==3)) {
            canWeActivateDown = true;
        }
    }

    //checking whether we can activate left
    if (x-1>=0) {
        if (game[x-1][y].blob && game[x-1][y].rotation==0) {
            canWeActivateLeft = true;
        }
        if (game[x-1][y].straight && (game[x-1][y].rotation%2==0)) {
            canWeActivateLeft = true;
        }
        if (game[x-1][y].Tshape && game[x-1][y].rotation!=3) {
            canWeActivateLeft = true;
        }
        if (game[x-1][y].Lshape && (game[x-1][y].rotation==1 || game[x-1][y].rotation==0)) {
            canWeActivateLeft = true;
        }
    }

    //checking whether we can activate right
    if (x+1<n) {
        if (game[x+1][y].blob && game[x+1][y].rotation==2) {
            canWeActivateRight = true;
        }
        if (game[x+1][y].straight && (game[x+1][y].rotation%2==0)) {
            canWeActivateRight = true;
        }
        if (game[x+1][y].Tshape && game[x+1][y].rotation!=1) {
            canWeActivateRight = true;
        }
        if (game[x+1][y].Lshape && (game[x+1][y].rotation==2 || game[x+1][y].rotation==3)) {
            canWeActivateRight = true;
        }
    }

    std::cout << "canWeActivateUp: " << canWeActivateUp << "\n";
    std::cout << "canWeActivateRight: " << canWeActivateRight << "\n";
    std::cout << "canWeActivateDown: " << canWeActivateDown << "\n";
    std::cout << "canWeActivateLeft: " << canWeActivateLeft << "\n";

    return;
}

sf::Sprite makeSprite(const sf::Texture& texture, int squareSize) {
    sf::Sprite sprite(texture);
    sprite.setScale(squareSize/textureSize, squareSize/textureSize);
    return sprite;
}

int canWeRunBlobLogic(block** game, int n, int m, int x, int y) {

    bool doWeHaveToActivateUp = false;
    bool doWeHaveToActivateDown = false;
    bool doWeHaveToActivateLeft = false;
    bool doWeHaveToActivateRight = false;

    bool isLeftUnreachable = false;
    bool isRightUnreachable = false;
    bool isUpUnreachable = false;
    bool isDownUnreachable = false;

    //checking whether we have to activate up;
    if (y-1>=0) {
        if (game[x][y-1].blob) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].straight && (game[x][y-1].rotation%2==1) && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].straight && !(game[x][y-1].rotation%2==1) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].Tshape && game[x][y-1].rotation!=0 && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].Tshape && !(game[x][y-1].rotation!=0) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].Lshape && (game[x][y-1].rotation==1 || game[x][y-1].rotation==2) && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].Lshape && !(game[x][y-1].rotation==1 || game[x][y-1].rotation==2) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
    } else {
        isUpUnreachable = true;
    }

    //checking whether we have to activate down;
    if (y+1<m) {
        if (game[x][y+1].blob) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].straight && (game[x][y+1].rotation%2==1) && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].straight && !(game[x][y+1].rotation%2==1) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].Tshape && game[x][y+1].rotation!=2 && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].Tshape && (game[x][y+1].rotation==2) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].Lshape && (game[x][y+1].rotation==0 || game[x][y+1].rotation==3) && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].Lshape && !(game[x][y+1].rotation==0 || game[x][y+1].rotation==3) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
    } else {
        isDownUnreachable = true;
    }

    //checking whether we have to activate left
    if (x-1>=0) {
        if (game[x-1][y].blob) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].straight && (game[x-1][y].rotation%2==0) && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].straight && !(game[x-1][y].rotation%2==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].Tshape && game[x-1][y].rotation!=3 && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].Tshape && !(game[x-1][y].rotation!=3) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].Lshape && (game[x-1][y].rotation==1 || game[x-1][y].rotation==0) && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].Lshape && !(game[x-1][y].rotation==1 || game[x-1][y].rotation==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
    } else {
        isLeftUnreachable = true;
    }

    //checking whether we have to activate right
    if (x+1<n) {
        if (game[x+1][y].blob) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].straight && (game[x+1][y].rotation%2==0) && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].straight && !(game[x+1][y].rotation%2==0) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].Tshape && game[x+1][y].rotation!=1 && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].Tshape && !(game[x+1][y].rotation!=1) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].Lshape && (game[x+1][y].rotation==2 || game[x+1][y].rotation==3) && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].Lshape && !(game[x+1][y].rotation==2 || game[x+1][y].rotation==3) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
    } else {
        isRightUnreachable = true;
    }

    int count = doWeHaveToActivateDown + doWeHaveToActivateUp + doWeHaveToActivateLeft + doWeHaveToActivateRight;
    int unreachablesCount = isLeftUnreachable + isRightUnreachable + isDownUnreachable + isUpUnreachable;

    if (count>=2) {
        //activationHelper(game, n, m, x, y);
        //std::cout << "HERE\n";
        return 2;
    }

    if (isLeftUnreachable && isRightUnreachable && isDownUnreachable && isUpUnreachable) {
        //activationHelper(game, n, m, x, y);
        //std::cout << "THERE\n";
        return 2;
    }

    if (count==1) {

        //rotate it
        if (doWeHaveToActivateUp) {
            game[x][y].rotation = 3;
        } else if (doWeHaveToActivateDown) {
            game[x][y].rotation = 1;
        } else if (doWeHaveToActivateLeft) {
            game[x][y].rotation = 2;
        } else {
            game[x][y].rotation = 0;
        }

        game[x][y].locked = 1;
        return 1;

    } else if (unreachablesCount==3) {

        //check the unreachables
        if (!isUpUnreachable) {
            game[x][y].rotation = 3;
        } else if (!isDownUnreachable) {
            game[x][y].rotation = 1;
        } else if (!isLeftUnreachable) {
            game[x][y].rotation = 2;
        } else {
            game[x][y].rotation = 0;
        }

        game[x][y].locked = 1;
        return 1;

    } else if (count>=2 || unreachablesCount==4) {

        //check for mistake
        return 2;

    } else {
        return 0;
    }

    

}

int canWeRunStraightLogic(block** game, int n, int m, int x, int y) {

    bool doWeHaveToActivateUp = false;
    bool doWeHaveToActivateDown = false;
    bool doWeHaveToActivateLeft = false;
    bool doWeHaveToActivateRight = false;

    bool isLeftUnreachable = false;
    bool isRightUnreachable = false;
    bool isUpUnreachable = false;
    bool isDownUnreachable = false;

    //checking whether we have to activate up;
    if (y-1>=0) {
        if (game[x][y-1].blob && game[x][y-1].rotation==1 && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].blob && !(game[x][y-1].rotation==1) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].straight && (game[x][y-1].rotation%2==1) && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].straight && !(game[x][y-1].rotation%2==1) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].Tshape && game[x][y-1].rotation!=0 && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].Tshape && !(game[x][y-1].rotation!=0) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].Lshape && (game[x][y-1].rotation==1 || game[x][y-1].rotation==2) && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].Lshape && !(game[x][y-1].rotation==1 || game[x][y-1].rotation==2) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
    } else {
        isUpUnreachable = true;
    }

    //checking whether we have to activate down;
    if (y+1<m) {
        if (game[x][y+1].blob && game[x][y+1].rotation==3 && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].blob && !(game[x][y+1].rotation==3) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].straight && (game[x][y+1].rotation%2==1) && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].straight && !(game[x][y+1].rotation%2==1) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].Tshape && game[x][y+1].rotation!=2 && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].Tshape && !(game[x][y+1].rotation!=2) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].Lshape && (game[x][y+1].rotation==0 || game[x][y+1].rotation==3) && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].Lshape && !(game[x][y+1].rotation==0 || game[x][y+1].rotation==3) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
    } else {
        isDownUnreachable = true;
    }

    //checking whether we have to activate left
    if (x-1>=0) {
        if (game[x-1][y].blob && game[x-1][y].rotation==0 && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].blob && !(game[x-1][y].rotation==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].straight && (game[x-1][y].rotation%2==0) && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].straight && !(game[x-1][y].rotation%2==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].Tshape && game[x-1][y].rotation!=3 && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].Tshape && !(game[x-1][y].rotation!=3) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].Lshape && (game[x-1][y].rotation==1 || game[x-1][y].rotation==0) && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].Lshape && !(game[x-1][y].rotation==1 || game[x-1][y].rotation==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
    } else {
        isLeftUnreachable = true;
    }

    //checking whether we have to activate right
    if (x+1<n) {
        if (game[x+1][y].blob && game[x+1][y].rotation==2 && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].blob && !(game[x+1][y].rotation==2) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].straight && (game[x+1][y].rotation%2==0) && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].straight && !(game[x+1][y].rotation%2==0) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].Tshape && game[x+1][y].rotation!=1 && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].Tshape && !(game[x+1][y].rotation!=1) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].Lshape && (game[x+1][y].rotation==2 || game[x+1][y].rotation==3) && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].Lshape && !(game[x+1][y].rotation==2 || game[x+1][y].rotation==3) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
    } else {
        isRightUnreachable = true;
    }

    //block specific logic here;
    int count = doWeHaveToActivateDown + doWeHaveToActivateUp + doWeHaveToActivateLeft + doWeHaveToActivateRight;
    int unreachablesCount = isLeftUnreachable + isRightUnreachable + isDownUnreachable + isUpUnreachable;

    if (count>=3) {
        //dumb mistakes
        return 2;
    }

    if (unreachablesCount>=3) {
        return 2;
    }

    if (count==1) {

        //rotate it
        if (doWeHaveToActivateUp) {
            game[x][y].rotation = 1;
        } else if (doWeHaveToActivateDown) {
            game[x][y].rotation = 1;
        } else if (doWeHaveToActivateLeft) {
            game[x][y].rotation = 0;
        } else {
            game[x][y].rotation = 0;
        }

        game[x][y].locked = 1;
        return 1;

    } else if (count==2) {

        //logic for two, only cases both up and down, or both left and right
        if (doWeHaveToActivateDown && doWeHaveToActivateUp) {
            game[x][y].rotation = 1;
        } else if (doWeHaveToActivateRight && doWeHaveToActivateLeft) {
            game[x][y].rotation = 0;
        }
        game[x][y].locked = true;
        return 1;

    } else if (count==2) {
        return 2;
    } else if (unreachablesCount==1) {

        //check the unreachables
        if (isUpUnreachable) {
            game[x][y].rotation = 0;
        } else if (isDownUnreachable) {
            game[x][y].rotation = 0;
        } else if (isLeftUnreachable) {
            game[x][y].rotation = 1;
        } else {
            game[x][y].rotation = 1;
        }

        game[x][y].locked = 1;
        return 1;

    } else if (unreachablesCount==2) {

        if (isUpUnreachable && isDownUnreachable) {
            game[x][y].rotation = 0;
        } else if (isLeftUnreachable && isRightUnreachable) {
            game[x][y].rotation = 1;
        }
        game[x][y].locked = true;
        return 1;

    } else if (unreachablesCount==2) {
        return 2;
    } else if (count>=3 || unreachablesCount>=3) {
        //dumb mistakes
        return 2;
    } else {
        return 0;
    }

}

int canWeRunTshapeLogic(block** game, int n, int m, int x, int y) {

    bool doWeHaveToActivateUp = false;
    bool doWeHaveToActivateDown = false;
    bool doWeHaveToActivateLeft = false;
    bool doWeHaveToActivateRight = false;

    bool isLeftUnreachable = false;
    bool isRightUnreachable = false;
    bool isUpUnreachable = false;
    bool isDownUnreachable = false;

    //checking whether we have to activate up;
    if (y-1>=0) {
        if (game[x][y-1].blob && game[x][y-1].rotation==1 && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].blob && !(game[x][y-1].rotation==1) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].straight && (game[x][y-1].rotation%2==1) && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].straight && !(game[x][y-1].rotation%2==1) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].Tshape && game[x][y-1].rotation!=0 && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].Tshape && !(game[x][y-1].rotation!=0) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].Lshape && (game[x][y-1].rotation==1 || game[x][y-1].rotation==2) && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].Lshape && !(game[x][y-1].rotation==1 || game[x][y-1].rotation==2) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
    } else {
        isUpUnreachable = true;
    }

    //checking whether we have to activate down;
    if (y+1<m) {
        if (game[x][y+1].blob && game[x][y+1].rotation==3 && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].blob && !(game[x][y+1].rotation==3) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].straight && (game[x][y+1].rotation%2==1) && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].straight && !(game[x][y+1].rotation%2==1) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].Tshape && game[x][y+1].rotation!=2 && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].Tshape && !(game[x][y+1].rotation!=2) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].Lshape && (game[x][y+1].rotation==0 || game[x][y+1].rotation==3) && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].Lshape && !(game[x][y+1].rotation==0 || game[x][y+1].rotation==3) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
    } else {
        isDownUnreachable = true;
    }

    //checking whether we have to activate left
    if (x-1>=0) {
        if (game[x-1][y].blob && game[x-1][y].rotation==0 && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].blob && !(game[x-1][y].rotation==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].straight && (game[x-1][y].rotation%2==0) && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].straight && !(game[x-1][y].rotation%2==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].Tshape && game[x-1][y].rotation!=3 && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].Tshape && !(game[x-1][y].rotation!=3) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].Lshape && (game[x-1][y].rotation==1 || game[x-1][y].rotation==0) && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].Lshape && !(game[x-1][y].rotation==1 || game[x-1][y].rotation==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
    } else {
        isLeftUnreachable = true;
    }

    //checking whether we have to activate right
    if (x+1<n) {
        if (game[x+1][y].blob && game[x+1][y].rotation==2 && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].blob && !(game[x+1][y].rotation==2) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].straight && (game[x+1][y].rotation%2==0) && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].straight && !(game[x+1][y].rotation%2==0) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].Tshape && game[x+1][y].rotation!=1 && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].Tshape && !(game[x+1][y].rotation!=1) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].Lshape && (game[x+1][y].rotation==2 || game[x+1][y].rotation==3) && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].Lshape && !(game[x+1][y].rotation==2 || game[x+1][y].rotation==3) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
    } else {
        isRightUnreachable = true;
    }


    //block specific logic here;
    int count = doWeHaveToActivateDown + doWeHaveToActivateUp + doWeHaveToActivateLeft + doWeHaveToActivateRight;
    int unreachablesCount = isLeftUnreachable + isRightUnreachable + isDownUnreachable + isUpUnreachable;

    //mistakes first
    if (count==4) {
        return 2;
    }

    if (unreachablesCount>=2) {
        return 2;
    }

    if (count==3) {

        // std::cout << "rotated a Tshape thanks to count\n";
        //rotate it
        if (doWeHaveToActivateDown && doWeHaveToActivateLeft && doWeHaveToActivateRight) {
            game[x][y].rotation = 2;
        } else if (doWeHaveToActivateUp && doWeHaveToActivateLeft && doWeHaveToActivateRight) {
            game[x][y].rotation = 0;
        } else if (doWeHaveToActivateDown && doWeHaveToActivateUp && doWeHaveToActivateRight) {
            game[x][y].rotation = 1;
        } else if (doWeHaveToActivateDown && doWeHaveToActivateLeft && doWeHaveToActivateLeft) {
            game[x][y].rotation = 3;
        }

        game[x][y].locked = 1;
        return 1;

    } else if (unreachablesCount==1) {

        // std::cout << "rotated a Tshape thanks to unreachables\n";
        // std::cout << "isUpUnreachable: " << isUpUnreachable << "\n";
        // std::cout << "isRightUnreachable: " << isRightUnreachable << "\n";
        // std::cout << "isDownUnreachable: " << isDownUnreachable << "\n";
        // std::cout << "isLeftUnreachable: " << isLeftUnreachable << "\n";
        // //check the unreachables
        if (isUpUnreachable) {
            game[x][y].rotation = 2;
        } else if (isDownUnreachable) {
            game[x][y].rotation = 0;
        } else if (isLeftUnreachable) {
            game[x][y].rotation = 1;
        } else {
            game[x][y].rotation = 3;
        }

        game[x][y].locked = 1;
        return 1;

    } else if (count==4 || unreachablesCount>=2) {
        return 2;
    } else {
        return 0;
    }

}

int canWeRunLshapeLogic(block** game, int n, int m, int x, int y) {

    bool doWeHaveToActivateUp = false;
    bool doWeHaveToActivateDown = false;
    bool doWeHaveToActivateLeft = false;
    bool doWeHaveToActivateRight = false;

    bool isLeftUnreachable = false;
    bool isRightUnreachable = false;
    bool isUpUnreachable = false;
    bool isDownUnreachable = false;

    //checking whether we have to activate up;
    if (y-1>=0) {
        if (game[x][y-1].blob && game[x][y-1].rotation==1 && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].blob && !(game[x][y-1].rotation==1) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].straight && (game[x][y-1].rotation%2==1) && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].straight && !(game[x][y-1].rotation%2==1) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].Tshape && game[x][y-1].rotation!=0 && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].Tshape && !(game[x][y-1].rotation!=0) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].Lshape && (game[x][y-1].rotation==1 || game[x][y-1].rotation==2) && game[x][y-1].locked) {
            doWeHaveToActivateUp = true;
        } else if (game[x][y-1].Lshape && !(game[x][y-1].rotation==1 || game[x][y-1].rotation==2) && game[x][y-1].locked) {
            isUpUnreachable = true;
        }
    } else {
        isUpUnreachable = true;
    }

    //checking whether we have to activate down;
    if (y+1<m) {
        if (game[x][y+1].blob && game[x][y+1].rotation==3 && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].blob && !(game[x][y+1].rotation==3) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].straight && (game[x][y+1].rotation%2==1) && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].straight && !(game[x][y+1].rotation%2==1) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].Tshape && game[x][y+1].rotation!=2 && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].Tshape && !(game[x][y+1].rotation!=2) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].Lshape && (game[x][y+1].rotation==0 || game[x][y+1].rotation==3) && game[x][y+1].locked) {
            doWeHaveToActivateDown = true;
        } else if (game[x][y+1].Lshape && !(game[x][y+1].rotation==0 || game[x][y+1].rotation==3) && game[x][y+1].locked) {
            isDownUnreachable = true;
        }
    } else {
        isDownUnreachable = true;
    }

    //checking whether we have to activate left
    if (x-1>=0) {
        if (game[x-1][y].blob && game[x-1][y].rotation==0 && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].blob && !(game[x-1][y].rotation==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].straight && (game[x-1][y].rotation%2==0) && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].straight && !(game[x-1][y].rotation%2==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].Tshape && game[x-1][y].rotation!=3 && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].Tshape && !(game[x-1][y].rotation!=3) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].Lshape && (game[x-1][y].rotation==1 || game[x-1][y].rotation==0) && game[x-1][y].locked) {
            doWeHaveToActivateLeft = true;
        } else if (game[x-1][y].Lshape && !(game[x-1][y].rotation==1 || game[x-1][y].rotation==0) && game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
    } else {
        isLeftUnreachable = true;
    }

    //checking whether we have to activate right
    if (x+1<n) {
        if (game[x+1][y].blob && game[x+1][y].rotation==2 && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].blob && !(game[x+1][y].rotation==2) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].straight && (game[x+1][y].rotation%2==0) && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].straight && !(game[x+1][y].rotation%2==0) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].Tshape && game[x+1][y].rotation!=1 && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].Tshape && !(game[x+1][y].rotation!=1) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].Lshape && (game[x+1][y].rotation==2 || game[x+1][y].rotation==3) && game[x+1][y].locked) {
            doWeHaveToActivateRight = true;
        } else if (game[x+1][y].Lshape && !(game[x+1][y].rotation==2 || game[x+1][y].rotation==3) && game[x+1][y].locked) {
            isRightUnreachable = true;
        }
    } else {
        isRightUnreachable = true;
    }

    //Lshape specific logic here
    int count = doWeHaveToActivateDown + doWeHaveToActivateUp + doWeHaveToActivateLeft + doWeHaveToActivateRight;
    int unreachablesCount = isLeftUnreachable + isRightUnreachable + isDownUnreachable + isUpUnreachable;

    //mistakes
    if ((doWeHaveToActivateUp && doWeHaveToActivateDown) || (doWeHaveToActivateLeft && doWeHaveToActivateRight)) {
        return 2;
    }

    if ((isUpUnreachable && isDownUnreachable) || (isLeftUnreachable && isRightUnreachable)) {
        return 2;
    }

    if (unreachablesCount>=3) {
        return 2;
    }

    if (count>=3) {
        return 2;
    }

    //each rotation has 4 cases
    if ((doWeHaveToActivateUp && doWeHaveToActivateRight) || (doWeHaveToActivateUp && isLeftUnreachable) || (doWeHaveToActivateRight && isDownUnreachable) || (isDownUnreachable && isLeftUnreachable)) {
        game[x][y].rotation = 0;
        game[x][y].locked = 1;
        return 1;
    }

    if ((doWeHaveToActivateRight && doWeHaveToActivateDown) || (doWeHaveToActivateRight && isUpUnreachable) || (doWeHaveToActivateDown && isLeftUnreachable) || (isLeftUnreachable && isUpUnreachable)) {
        game[x][y].rotation = 1;
        game[x][y].locked = 1;
        return 1;
    }

    if ((doWeHaveToActivateDown && doWeHaveToActivateLeft) || (doWeHaveToActivateDown && isRightUnreachable) || (doWeHaveToActivateLeft && isUpUnreachable) || (isUpUnreachable && isRightUnreachable)) {
        game[x][y].rotation = 2;
        game[x][y].locked = 1;
        return 1;
    }

    if ((doWeHaveToActivateLeft && doWeHaveToActivateUp) || (doWeHaveToActivateLeft && isDownUnreachable) || (doWeHaveToActivateUp && isRightUnreachable) || (isRightUnreachable && isDownUnreachable)) {
        game[x][y].rotation = 3;
        game[x][y].locked = 1;
        return 1;
    }

    return 0;

}

int runSolverOnce(block** game, int n, int m) {

    bool didWeRunItOnce = false;
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            if (game[a][b].locked==false) {
                if (game[a][b].blob) {
                    int state = canWeRunBlobLogic(game, n, m, a, b);
                    if (state==1) {
                        didWeRunItOnce = true;
                        // return 1;
                    } else if (state==2) {
                        //std::cout << "FOUND BLOB ERROR AT " << a << " " << b << "\n";
                        return 2;
                    }
                } else if (game[a][b].straight) {
                    int state = canWeRunStraightLogic(game, n, m, a, b);
                    if (state==1) {
                        didWeRunItOnce = true;
                        // return 1;
                    } else if (state==2) {
                        //std::cout << "FOUND STRAIGHT ERROR AT " << a << " " << b << "\n";
                        return 2;
                    }
                } else if (game[a][b].Tshape) {
                    int state = canWeRunTshapeLogic(game, n, m, a, b);
                    if (state==1) {
                        didWeRunItOnce = true;
                        // return 1;
                    } else if (state==2) {
                        //std::cout << "FOUND TSHAPE ERROR AT " << a << " " << b << "\n";
                        return 2;
                    }
                } else if (game[a][b].Lshape) {
                    int state = canWeRunLshapeLogic(game, n, m, a, b);
                    if (state==1) {
                        didWeRunItOnce = true;
                        // return 1;
                    } else if (state==2) {
                        //std::cout << "FOUND LSHAPE ERROR AT " << a << " " << b << "\n";
                        return 2;
                    }
                }
            } else {
                //current block is locked
                if (game[a][b].blob) {
                    if (canWeRunBlobLogic(game, n, m, a, b)==2) {
                        //std::cout << "FOUND LOCKED BLOB ERROR AT " << a << " " << b << "\n";
                        return 2;
                    }
                } else if (game[a][b].straight) {
                    if (canWeRunStraightLogic(game, n, m, a, b)==2) {
                        //std::cout << "FOUND LOCKED STRAIGHT ERROR AT " << a << " " << b << "\n";
                        return 2;
                    }
                } else if (game[a][b].Tshape) {
                    if (canWeRunTshapeLogic(game, n, m, a, b)==2) {
                        //std::cout << "FOUND LOCKED TSHAPE ERROR AT " << a << " " << b << "\n";
                        return 2;
                    }
                } else if (game[a][b].Lshape) {
                    if (canWeRunLshapeLogic(game, n, m, a, b)==2) {
                        //std::cout << "FOUND LOCKED LSHAPE ERROR AT " << a << " " << b << "\n";
                        return 2;
                    }
                }
            }
        }
    }

    if (didWeRunItOnce) {
        return 1;
    }

    //this happens only when the solver either stalls or completes 
    return 0;

}

void draw(block** game, int n, int m, sf::RenderWindow& window, int waterOriginX, int waterOriginY, allSprites& sprites, int squareSize) {

    window.clear();

    //reset activity
    for (int i=0; i<n; i++) {
        for (int j=0; j<m; j++) {
            game[i][j].active = 0;
        }
    }

    //activate
    activateWater(game, n, m, waterOriginX, waterOriginY);

    //check the solution
    bool areTheyAllActive = true;
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            if (!game[a][b].active) {
                areTheyAllActive = false;
            }
        }
    }



    //draw
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
            square.setPosition(i * squareSize, j * squareSize);
            window.draw(square);
            sf::Sprite sprite;
            // Determine the sprite based on the block type
            if (game[i][j].blob) {
                if (game[i][j].active) {
                    if (game[i][j].locked) {
                        sprite = sprites.blobActiveLockedSprite;
                    } else {
                        sprite = sprites.blobSpriteActive;
                    }
                } else {
                    if (game[i][j].locked) {
                        sprite = sprites.blobLockedSprite;
                    } else {
                        sprite = sprites.blobSprite;
                    }
                }
            } else if (game[i][j].straight) {
                if (game[i][j].active) {
                    if (game[i][j].locked) {
                        sprite = sprites.straightActiveLockedSprite;
                    } else {
                        sprite = sprites.straightSpriteActive;
                    }
                } else {
                    if (game[i][j].locked) {
                        sprite = sprites.straightLockedSprite;
                    } else {
                        sprite = sprites.straightSprite;
                    }
                }
            } else if (game[i][j].Lshape) {
                if (game[i][j].active) {
                    if (game[i][j].locked) {
                        sprite = sprites.LshapeActiveLockedSprite;
                    } else {
                        sprite = sprites.LshapeSpriteActive;
                    }
                } else {
                    if (game[i][j].locked) {
                        sprite = sprites.LshapeLockedSprite;
                    } else {
                        sprite = sprites.LshapeSprite;
                    }
                }
            } else {
                if (game[i][j].active) {
                    if (game[i][j].locked) {
                        sprite = sprites.TshapeActiveLockedSprite;
                    } else {
                        sprite = sprites.TshapeSpriteActive;
                    }
                } else {
                    if (game[i][j].locked) {
                        sprite = sprites.TshapeLockedSprite;
                    } else {
                        sprite = sprites.TshapeSprite;
                    }
                }
            }
            // Set the position of the sprite based on the block's position
            sprite.setPosition(i * squareSize + squareSize / 2.f, j * squareSize + squareSize / 2.f);
            // Set the origin of the sprite to its center
            sprite.setOrigin(textureSize / 2.f, textureSize / 2.f);
            // Set the rotation of the sprite based on the block's rotation
            sprite.setRotation(game[i][j].rotation * 90);
            window.draw(sprite);
        }
    }

    // sf::Text depthText;
    // depthText.setFont(sprites.font);
    // depthText.setString("Depth of Recursion: " + std::to_string(depthOfRecursion));
    // depthText.setCharacterSize(24);
    // depthText.setFillColor(sf::Color::Red);
    // depthText.setPosition(10, 10); // Adjust position as needed

    // window.draw(depthText);

    window.display();

}

void drawWithoutWater(block** game, int n, int m, sf::RenderWindow& window, int waterOriginX, int waterOriginY, allSprites& sprites, int squareSize) {

    window.clear();

    //draw
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
            square.setPosition(i * squareSize, j * squareSize);
            window.draw(square);
            sf::Sprite sprite;
            // Determine the sprite based on the block type
            if (game[i][j].blob) {
                if (game[i][j].active) {
                    if (game[i][j].locked) {
                        sprite = sprites.blobActiveLockedSprite;
                    } else {
                        sprite = sprites.blobSpriteActive;
                    }
                } else {
                    if (game[i][j].locked) {
                        sprite = sprites.blobLockedSprite;
                    } else {
                        sprite = sprites.blobSprite;
                    }
                }
            } else if (game[i][j].straight) {
                if (game[i][j].active) {
                    if (game[i][j].locked) {
                        sprite = sprites.straightActiveLockedSprite;
                    } else {
                        sprite = sprites.straightSpriteActive;
                    }
                } else {
                    if (game[i][j].locked) {
                        sprite = sprites.straightLockedSprite;
                    } else {
                        sprite = sprites.straightSprite;
                    }
                }
            } else if (game[i][j].Lshape) {
                if (game[i][j].active) {
                    if (game[i][j].locked) {
                        sprite = sprites.LshapeActiveLockedSprite;
                    } else {
                        sprite = sprites.LshapeSpriteActive;
                    }
                } else {
                    if (game[i][j].locked) {
                        sprite = sprites.LshapeLockedSprite;
                    } else {
                        sprite = sprites.LshapeSprite;
                    }
                }
            } else {
                if (game[i][j].active) {
                    if (game[i][j].locked) {
                        sprite = sprites.TshapeActiveLockedSprite;
                    } else {
                        sprite = sprites.TshapeSpriteActive;
                    }
                } else {
                    if (game[i][j].locked) {
                        sprite = sprites.TshapeLockedSprite;
                    } else {
                        sprite = sprites.TshapeSprite;
                    }
                }
            }
            // Set the position of the sprite based on the block's position
            sprite.setPosition(i * squareSize + squareSize / 2.f, j * squareSize + squareSize / 2.f);
            // Set the origin of the sprite to its center
            sprite.setOrigin(textureSize / 2.f, textureSize / 2.f);
            // Set the rotation of the sprite based on the block's rotation
            sprite.setRotation(game[i][j].rotation * 90);
            window.draw(sprite);
        }
    }

    window.display();

}

bool doWeBreakEarly(block** game, int n, int m, sf::RenderWindow& window, int waterOriginX, int waterOriginY, allSprites& sprites, int squareSize, int delayMs) {

    //clear all activity
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            game[a][b].active = false;
        }
    }

    //activate water on all locked blocks (that are not a blob!!!), which have at least one unlocked neighbour
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            if (game[a][b].locked==true && game[a][b].blob==false) {

                bool doesItHaveAtLeastOneUnlockedNeighbour = false;
                if (a-1>=0) {
                    if (game[a-1][b].locked==false) {
                        doesItHaveAtLeastOneUnlockedNeighbour = true;
                    }
                }
                if (b-1>=0) {
                    if (game[a][b-1].locked==false) {
                        doesItHaveAtLeastOneUnlockedNeighbour = true;
                    }
                }
                if (a+1<n) {
                    if (game[a+1][b].locked==false) {
                        doesItHaveAtLeastOneUnlockedNeighbour = true;
                    }
                }
                if (b+1<m) {
                    if (game[a][b+1].locked==false) {
                        doesItHaveAtLeastOneUnlockedNeighbour = true;
                    }
                }

                if (doesItHaveAtLeastOneUnlockedNeighbour) {
                    activateWater(game, n, m, a, b);
                }
            }
        }
    }

    //if there is a locked block (that is not a blob!) without water, then we have to break early
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            if (game[a][b].locked==true && game[a][b].active==false && game[a][b].blob==false) {
                return true;
            }
        }
    }

    return false;
}

bool recursivelyActivateCluster(block** game, int n, int m) {

    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            if (game[a][b].locked==false && game[a][b].active==true) {
                if (a-1>=0) {
                    if (game[a-1][b].locked==false && game[a-1][b].active==false) {
                        game[a-1][b].active = true;
                        return true;
                    }
                }
                if (a+1<n) {
                    if (game[a+1][b].locked==false && game[a+1][b].active==false) {
                        game[a+1][b].active = true;
                        return true;
                    }
                }
                if (b-1>=0) {
                    if (game[a][b-1].locked==false && game[a][b-1].active==false) {
                        game[a][b-1].active = true;
                        return true;
                    }
                }
                if (b-1<m) {
                    if (game[a][b+1].locked==false && game[a][b+1].active==false) {
                        game[a][b+1].active = true;
                        return true;
                    }
                }
            }
        }
    }

    //this happens only when the solver either stalls or completes 
    return 0;

}

int returnsNumberOfActiveBlocks(block** game, int n, int m) {
    int numberOfActiveBlocks = 0;
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            if (game[a][b].active) {
                numberOfActiveBlocks++;
            }
        }
    }
    return numberOfActiveBlocks;
}

void chooseBacktrackingSquare(block** game, int n, int m, sf::RenderWindow& window, int waterOriginX, int waterOriginY, allSprites& sprites, int squareSize, int delayMs, int& chosenX, int& chosenY) {

    //clear activity
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            game[a][b].active = false;
        }
    }

    std::vector<int> clusterX;
    std::vector<int> clusterY;
    std::vector<int> clusterSize;

    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            if (game[a][b].locked==false && game[a][b].active==false) {

                game[a][b].active = true;

                clusterX.push_back(a);
                clusterY.push_back(b);

                int before = returnsNumberOfActiveBlocks(game, n, m);

                bool state = true;
                while (state) {
                    state = recursivelyActivateCluster(game, n, m);
                }

                int after = returnsNumberOfActiveBlocks(game, n, m);
                clusterSize.push_back(after-before);
                
                // std::cout << "current cluster has a size of " << clusterSize.back() << ", and is located at: " << clusterX.back() << ", " << clusterY.back() << "\n";
                // drawWithoutWater(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize);

                // int debugInt;
                // std::cout << "enter debug int...\n";
                // std::cin >> debugInt;

            }
        }
    }

    //find the smallest cluster
    int smallestClusterSize = 10000;
    int smallestClusterPosition;
    for (int a=0; a<clusterSize.size(); a++) {
        // std::cout << "checking for smallest cluster... " << clusterSize[a] << " " << clusterX[a] << " " << clusterY[a] << "\n";
        if (smallestClusterSize>clusterSize[a]) {
            smallestClusterSize = clusterSize[a];
            smallestClusterPosition = a;
        }
    }

    // std::cout << "smallest cluster has a size of " << clusterSize[smallestClusterPosition] << ", and is located at: " << clusterX[smallestClusterPosition] << ", " << clusterY[smallestClusterPosition] << "\n";
    chosenX = clusterX[smallestClusterPosition];
    chosenY = clusterY[smallestClusterPosition];

    //clear activity
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            game[a][b].active = false;
        }
    }
    game[chosenX][chosenY].active = true;
    bool state = true;
    while (state) {
        state = recursivelyActivateCluster(game, n, m);
    }
    

    // for (int a=0; a<clusterX.size(); a++) {
    //     for (int a=0; a<n; a++) {
    //         for (int b=0; b<m; b++) {
    //             game[a][b].active = false;
    //         }
    //     }

    //     game[clusterX[a]][clusterY[a]].active = true;

    //     bool state = true;
    //     while (state) {
    //         state = recursivelyActivateCluster(game, n, m);
    //     }

    //     int debugInt;
    //     std::cin >> debugInt;

    //     drawWithoutWater(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize);
    // }

    //choose random square from smallest cluster
    // while (game[chosenX][chosenY].active==false) {
    //     chosenX = std::rand()%n;
    //     chosenY = std::rand()%m;
    // }

    // //choose square from smallest cluster
    for (int a=n-1; a>=0; a--) {
        for (int b=m-1; b>=0; b--) {
            if (game[a][b].active) {
                chosenX = a;
                chosenY = b;
            }
        }
    }

}

void activateWaterWithParents(block** game, int n, int m, int x, int y, int parentX, int parentY, bool* isThereACycle) {

    if (game[x][y].active==true) {
        return;
    }

    game[x][y].active = 1;

    bool canWeActivateUp = false;
    bool canWeActivateDown = false;
    bool canWeActivateLeft = false;
    bool canWeActivateRight = false;

    //checking whether we can activate up
    if (y-1>=0) {
        if (game[x][y-1].blob && game[x][y-1].rotation==1 && game[x][y-1].locked) {
            // std::cout << "and we did it here\n";
            canWeActivateUp = true;
        }
        if (game[x][y-1].straight && (game[x][y-1].rotation%2==1) && game[x][y-1].locked) {
            canWeActivateUp = true;
        }
        if (game[x][y-1].Tshape && game[x][y-1].rotation!=0 && game[x][y-1].locked) {
            canWeActivateUp = true;
        }
        if (game[x][y-1].Lshape && (game[x][y-1].rotation==1 || game[x][y-1].rotation==2) && game[x][y-1].locked) {
            canWeActivateUp = true;
        }
    }

    //checking whether we can activate down
    if (y+1<m) {
        if (game[x][y+1].blob && game[x][y+1].rotation==3 && game[x][y+1].locked) {
            canWeActivateDown = true;
        }
        if (game[x][y+1].straight && (game[x][y+1].rotation%2==1) && game[x][y+1].locked) {
            canWeActivateDown = true;
        }
        if (game[x][y+1].Tshape && game[x][y+1].rotation!=2 && game[x][y+1].locked) {
            canWeActivateDown = true;
        }
        if (game[x][y+1].Lshape && (game[x][y+1].rotation==0 || game[x][y+1].rotation==3) && game[x][y+1].locked) {
            canWeActivateDown = true;
        }
    }

    //checking whether we can activate left
    if (x-1>=0) {
        if (game[x-1][y].blob && game[x-1][y].rotation==0 && game[x-1][y].locked) {
            canWeActivateLeft = true;
        }
        if (game[x-1][y].straight && (game[x-1][y].rotation%2==0) && game[x-1][y].locked) {
            canWeActivateLeft = true;
        }
        if (game[x-1][y].Tshape && game[x-1][y].rotation!=3 && game[x-1][y].locked) {
            canWeActivateLeft = true;
        }
        if (game[x-1][y].Lshape && (game[x-1][y].rotation==1 || game[x-1][y].rotation==0) && game[x-1][y].locked) {
            canWeActivateLeft = true;
        }
    }

    //checking whether we can activate right
    if (x+1<n) {
        if (game[x+1][y].blob && game[x+1][y].rotation==2 && game[x+1][y].locked) {
            canWeActivateRight = true;
        }
        if (game[x+1][y].straight && (game[x+1][y].rotation%2==0) && game[x+1][y].locked) {
            canWeActivateRight = true;
        }
        if (game[x+1][y].Tshape && game[x+1][y].rotation!=1 && game[x+1][y].locked) {
            canWeActivateRight = true;
        }
        if (game[x+1][y].Lshape && (game[x+1][y].rotation==2 || game[x+1][y].rotation==3) && game[x+1][y].locked) {
            canWeActivateRight = true;
        }
    }


    //recursion
    if (game[x][y].blob) {

        if (game[x][y].rotation==0) {
            if (canWeActivateRight) {
                if (x+1!=parentX && y!=parentY && game[x+1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 1\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x+1, y, x, y, isThereACycle);
            }
        } else if (game[x][y].rotation==1) {
            if (canWeActivateDown) {
                if (x!=parentX && y+1!=parentY && game[x][y+1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 2\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y+1, x, y, isThereACycle);
            }
        } else if (game[x][y].rotation==2) {
            if (canWeActivateLeft) {
                if (x-1!=parentX && y!=parentY && game[x-1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found 3\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x-1, y, x, y, isThereACycle);
            }
        } else {
            //rotation = 3
            if (canWeActivateUp) {
                if (x!=parentX && y-1!=parentY && game[x][y-1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found4\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y-1, x, y, isThereACycle);
            }
        }

    } else if (game[x][y].Tshape) {

        if (game[x][y].rotation==0) {
            if (canWeActivateLeft) {
                if (x-1!=parentX && y!=parentY && game[x-1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found 5\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x-1, y, x, y, isThereACycle);
            }
            if (canWeActivateRight) {
                if (x+1!=parentX && y!=parentY && game[x+1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 6\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x+1, y, x, y, isThereACycle);
            }
            if (canWeActivateUp) {
                if (x!=parentX && y-1!=parentY && game[x][y-1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found7\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y-1, x, y, isThereACycle);
            }
        } else if (game[x][y].rotation==1) {
            if (canWeActivateDown) {
                if (x!=parentX && y+1!=parentY && game[x][y+1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 8\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y+1, x, y, isThereACycle);
            }
            if (canWeActivateRight) {
                if (x+1!=parentX && y!=parentY && game[x+1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 9\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x+1, y, x, y, isThereACycle);
            }
            if (canWeActivateUp) {
                if (x!=parentX && y-1!=parentY && game[x][y-1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found10\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y-1, x, y, isThereACycle);
            }
        } else if (game[x][y].rotation==2) {
            if (canWeActivateDown) {
                if (x!=parentX && y+1!=parentY && game[x][y+1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 11\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y+1, x, y, isThereACycle);
            }
            if (canWeActivateRight) {
                if (x+1!=parentX && y!=parentY && game[x+1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 12\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x+1, y, x, y, isThereACycle);
            }
            if (canWeActivateLeft) {
                if (x-1!=parentX && y!=parentY && game[x-1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found 13\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x-1, y, x, y, isThereACycle);
            }
        } else {
            //rotation = 3
            if (canWeActivateDown) {
                if (x!=parentX && y+1!=parentY && game[x][y+1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 14\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y+1, x, y, isThereACycle);
            }
            if (canWeActivateUp) {
                if (x!=parentX && y-1!=parentY && game[x][y-1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found15\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y-1, x, y, isThereACycle);
            }
            if (canWeActivateLeft) {
                if (x-1!=parentX && y!=parentY && game[x-1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found 16\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x-1, y, x, y, isThereACycle);
            }
            // if (canWeActivateRight) {
            //     activateWater(game, n, m, x, y+1);
            // }
        }

    } else if (game[x][y].straight) {

        if (game[x][y].rotation%2==0) {
            
            if (canWeActivateLeft) {
                if (x-1!=parentX && y!=parentY && game[x-1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found 17\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x-1, y, x, y, isThereACycle);
            }
            if (canWeActivateRight) {
                if (x+1!=parentX && y!=parentY && game[x+1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 18\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x+1, y, x, y, isThereACycle);
            }

        } else {

            if (canWeActivateDown) {
                if (x!=parentX && y+1!=parentY && game[x][y+1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 19\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y+1, x, y, isThereACycle);
            }
            if (canWeActivateUp) {
                if (x!=parentX && y-1!=parentY && game[x][y-1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found20\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y-1, x, y, isThereACycle);
            }

        }

    } else {
        //lshape
        if (game[x][y].rotation==0) {
            if (canWeActivateUp) {
                if (x!=parentX && y-1!=parentY && game[x][y-1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found21\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y-1, x, y, isThereACycle);
            }
            if (canWeActivateRight) {
                if (x+1!=parentX && y!=parentY && game[x+1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 22\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x+1, y, x, y, isThereACycle);
            }
        } else if (game[x][y].rotation==1) {
            if (canWeActivateDown) {
                if (x!=parentX && y+1!=parentY && game[x][y+1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 23\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y+1, x, y, isThereACycle);
            }
            if (canWeActivateRight) {
                if (x+1!=parentX && y!=parentY && game[x+1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 24\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x+1, y, x, y, isThereACycle);
            }
        } else if (game[x][y].rotation==2) {
            if (canWeActivateDown) {
                if (x!=parentX && y+1!=parentY && game[x][y+1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found right 25\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y+1, x, y, isThereACycle);
            }
            if (canWeActivateLeft) {
                if (x-1!=parentX && y!=parentY && game[x-1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found 26\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x-1, y, x, y, isThereACycle);
            }
        } else {
            if (canWeActivateUp) {
                if (x!=parentX && y-1!=parentY && game[x][y-1].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found27\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x, y-1, x, y, isThereACycle);
            }
            if (canWeActivateLeft) {
                if (x-1!=parentX && y!=parentY && game[x-1][y].active) {
                    // std::cout << "(" << x << ", " << y << "), parents: " << parentX << ", " << parentY << "\n";
                    // std::cout << "cycle found 28\n";
                    *isThereACycle = true;
                }
                activateWaterWithParents(game, n, m, x-1, y, x, y, isThereACycle);
            }          
        }
    }

}

bool isThereACycleInTheLockedBlocks(block** game, int n, int m, sf::RenderWindow& window, int waterOriginX, int waterOriginY, allSprites& sprites, int squareSize, int delayMs) {

    //reset activity
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            game[a][b].active = false;
        }
    }

    bool isThereACycle = false;
    bool* ptr = &isThereACycle;

    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            if (game[a][b].locked==true && game[a][b].active==false) {
                // std::cout << "CALLING FOR " << a << ", " << b << "\n";
                int parentX = a-2;
                int parentY = b-2;
                if (parentX<0) {
                    parentX += 4;
                }
                if (parentY<0) {
                    parentY += 4;
                }

                activateWaterWithParents(game, n, m, a, b, parentX, parentY, ptr);

            }
        }
    }

    if (isThereACycle==true) {
        return true;
    } else {
        return false;
    }

}

bool backtrackingSolver(block** game, int n, int m, sf::RenderWindow& window, int waterOriginX, int waterOriginY, allSprites& sprites, int squareSize, int delayMs, bool doWeRender) {

    depthOfRecursion++;
    while (depthOfRecursion>recursionInfo.size()) {
        recursionInfo.push_back(0);
    }

    recursionInfo[depthOfRecursion-1]++;

    //first check if we have to break early
    if (doWeBreakEarly(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize, delayMs)) {
        //std::cout << "BREAKING EARLY BREAKING EARLY BREAKING EARLY \n";
        depthOfRecursion--;
        return false;
    }

    //check if there is a cycle, if there is then break early
    if (isThereACycleInTheLockedBlocks(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize, delayMs)) {
        // draw(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize);
        timesBrokenEarlyBecauseOfCycle++;
        depthOfRecursion--;
        return false;
    }

    bool doWeContinue = true;

    while (doWeContinue) {
        int state = runSolverOnce(game, n, m);

        if (state==2) {
            //mistake, we have to go back
            doWeContinue = false;
            depthOfRecursion--;
            return false;
        } else if (state==1) {
            //locked some blocks, continue
        } else if (state==0) {
            //solver has stalled, so either we have to randomly make a move or the puzzle is complete
            doWeContinue = false;

            //reset water
            for (int a=0; a<n; a++) {
                for (int b=0; b<m; b++) {
                    game[a][b].active = 0;
                }
            }
            activateWater(game, n, m, waterOriginX, waterOriginY);

            //is it all locked?
            bool isItAllLocked = true;
            for (int a=0; a<n; a++) {
                for (int b=0; b<m; b++) {
                    if (game[a][b].locked==false) {
                        isItAllLocked = false;
                    }
                }
            }

            if (isItAllLocked) {
                //is everything active?
                bool isEveryThingActive = true;
                for (int a=0; a<n; a++) {
                    for (int b=0; b<m; b++) {
                        if (game[a][b].active==false) {
                            isEveryThingActive = false;
                        }
                    }
                }

                if (isEveryThingActive) {
                    //then it is solved
                    if (doWeRender) {
                        draw(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize);
                    }
                    for (int a=0; a<n; a++) {
                        for (int b=0; b<m; b++) {
                            rotations[a][b] = game[a][b].rotation;
                        }
                    }
                    return true;
                } else {
                    //we have a fully locked game, with no water in some places. mistake happened earlier
                    depthOfRecursion--;
                    return false;
                }

            }

            //first check if we have to break early
            if (doWeBreakEarly(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize, delayMs)) {
                //std::cout << "BREAKING EARLY BREAKING EARLY BREAKING EARLY \n";
                depthOfRecursion--;
                return false;
            }

            //check if there is a cycle, if there is then break early
            if (isThereACycleInTheLockedBlocks(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize, delayMs)) {
                // draw(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize);
                timesBrokenEarlyBecauseOfCycle++;
                depthOfRecursion--;
                return false;
            }

            //reset water
            for (int a=0; a<n; a++) {
                for (int b=0; b<m; b++) {
                    game[a][b].active = 0;
                }
            }

            // Find the closest unlocked square to the edge
            int closestDistance = n + m; // Initialize with the maximum possible distance
            int chosenX = 0;
            int chosenY = 0;

            // chooseBacktrackingSquare(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize, delayMs, chosenX, chosenY);

            // for (int a = 0; a < n; a++) {
            //     for (int b = 0; b < m; b++) {
            //         if (game[a][b].locked == false) {
            //             // Calculate Manhattan distance from the edges
            //             int distance = std::min(std::min(a, n - 1 - a), std::min(b, m - 1 - b));
            //             if (distance < closestDistance) {
            //                 closestDistance = distance;
            //                 chosenX = a;
            //                 chosenY = b;
            //             }
            //         }
            //     }
            // }

            for (int a=0; a<n; a++) {
                for (int b=0; b<m; b++) {
                    if (game[a][b].locked==false) {
                        chosenX = a;
                        chosenY = b;
                    }
                }
            }

            // for (int a=n-1; a>=0; a--) {
            //     for (int b=m-1; b>=0; b--) {
            //         if (game[a][b].locked==false) {
            //             chosenX = a;
            //             chosenY = b;
            //         }
            //     }
            // }

            // while (game[chosenX][chosenY].locked==true) {
            //     chosenX = std::rand();
            //     chosenY = std::rand();
            // }

            //make copy of the game
            block** copyOfGame = new block*[n];
            for (int i = 0; i < n; ++i) {
                copyOfGame[i] = new block[m];
            }

            copyFirstGameToSecondGame(game, copyOfGame, n, m);
            copyOfGame[chosenX][chosenY].rotation = 0;
            copyOfGame[chosenX][chosenY].locked = true;
            if (backtrackingSolver(copyOfGame, n, m, window, waterOriginX, waterOriginY, sprites, squareSize, delayMs, doWeRender)) {
                return true;
            }

            copyFirstGameToSecondGame(game, copyOfGame, n, m);
            copyOfGame[chosenX][chosenY].rotation = 1;
            copyOfGame[chosenX][chosenY].locked = true;
            if (backtrackingSolver(copyOfGame, n, m, window, waterOriginX, waterOriginY, sprites, squareSize, delayMs, doWeRender)) {
                return true;
            }

            copyFirstGameToSecondGame(game, copyOfGame, n, m);
            copyOfGame[chosenX][chosenY].rotation = 2;
            copyOfGame[chosenX][chosenY].locked = true;
            if (backtrackingSolver(copyOfGame, n, m, window, waterOriginX, waterOriginY, sprites, squareSize, delayMs, doWeRender)) {
                return true;
            }

            copyFirstGameToSecondGame(game, copyOfGame, n, m);
            copyOfGame[chosenX][chosenY].rotation = 3;
            copyOfGame[chosenX][chosenY].locked = true;
            if (backtrackingSolver(copyOfGame, n, m, window, waterOriginX, waterOriginY, sprites, squareSize, delayMs, doWeRender)) {
                return true;
            }

            depthOfRecursion--;
            return false;
        }

        if (doWeRender) {
            draw(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize);
        }
        
        if (doWeRender) {
            //std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        }
    }

    return true;

}

int highestAverageOfTen(std::vector<int> times) {

    int highestAverage = 0;

    for (int a=0; a<times.size()-10; a++) {

        int currentAverage = 0;
        for (int b=0; b<10; b++) {
            currentAverage += times[a+b];
        }

        if (currentAverage>highestAverage) {
            highestAverage = currentAverage;
        }
    }

    return highestAverage/10;
}

int main(int argc, char* argv[]) {

    // Seed the random number generator
    std::srand(std::time(nullptr));

    int n = std::stoi(argv[1]);
    int m = std::stoi(argv[2]);
    int squareSize = std::stoi(argv[3]);
    int delayMs = std::stoi(argv[4]);


    if (squareSize>128) {
        std::cout << "too big square size\n";
        return -1;
    }

    sf::RenderWindow window(sf::VideoMode(n * squareSize, m * squareSize), "Pipes Game");

    //sprites
    allSprites sprites;

    if (!sprites.font.loadFromFile("sprites/Roboto-Black.ttf")) {
        // Handle font loading error
        std::cout << "FAILED TO LOAD FONT!!!!!!!!!!!!\n";
        return -1;
    }

    sf::Texture blobTexture;
    if (!blobTexture.loadFromFile("sprites/blob.png")) {
        std::cout << "FAILED TO LOAD BLOB SPRITE!!!\n";
        return -1;
    }
    sf::Sprite blobSprite = makeSprite(blobTexture, squareSize);

    sprites.blobSprite = blobSprite;
    sprites.blobTexture = blobTexture;    

    sf::Texture straightTexture;
    if (!straightTexture.loadFromFile("sprites/straight.png")) {
        std::cout << "FAILED TO LOAD STRAIGHT SPRITE!!!\n";
        return -1;
    }
    sf::Sprite straightSprite(straightTexture);
    straightSprite.setScale(squareSize / straightTexture.getSize().x, squareSize / straightTexture.getSize().y);
    sprites.straightSprite = straightSprite;
    sprites.straightTexture = straightTexture; 

    sf::Texture LshapeTexture;
    if (!LshapeTexture.loadFromFile("sprites/Lshape.png")) {
        std::cout << "FAILED TO LOAD LSHAPE SPRITE!!!\n";
        return -1;
    }
    sf::Sprite LshapeSprite(LshapeTexture);
    LshapeSprite.setScale(squareSize / LshapeTexture.getSize().x, squareSize / LshapeTexture.getSize().y);
    sprites.LshapeSprite = LshapeSprite;
    sprites.LshapeTexture = LshapeTexture;

    sf::Texture TshapeTexture;
    if (!TshapeTexture.loadFromFile("sprites/Tshape.png")) {
        std::cout << "FAILED TO LOAD TSHAPE SPRITE!!!\n";
        return -1;
    }
    sf::Sprite TshapeSprite(TshapeTexture);
    TshapeSprite.setScale(squareSize / TshapeTexture.getSize().x, squareSize / TshapeTexture.getSize().y);
    sprites.TshapeSprite = TshapeSprite;
    sprites.TshapeTexture = TshapeTexture;


    sf::Texture blobTextureActive;
    if (!blobTextureActive.loadFromFile("sprites/blob_active.png")) {
        std::cout << "FAILED TO LOAD ACTIVE BLOB SPRITE!!!\n";
        return -1;
    }
    sf::Sprite blobSpriteActive(blobTextureActive);
    blobSpriteActive.setScale(squareSize / blobTextureActive.getSize().x, squareSize / blobTextureActive.getSize().y);
    sprites.blobSpriteActive = blobSpriteActive;
    sprites.blobTextureActive = blobTextureActive;


    sf::Texture straightTextureActive;
    if (!straightTextureActive.loadFromFile("sprites/straight_active.png")) {
        std::cout << "FAILED TO LOAD ACTIVE STRAIGHT SPRITE!!!\n";
        return -1;
    }
    sf::Sprite straightSpriteActive(straightTextureActive);
    straightSpriteActive.setScale(squareSize / straightTextureActive.getSize().x, squareSize / straightTextureActive.getSize().y);
    sprites.straightSpriteActive = straightSpriteActive;
    sprites.straightTextureActive = straightTextureActive;


    sf::Texture LshapeTextureActive;
    if (!LshapeTextureActive.loadFromFile("sprites/Lshape_active.png")) {
        std::cout << "FAILED TO LOAD ACTIVE LSHAPE SPRITE!!!\n";
        return -1;
    }
    sf::Sprite LshapeSpriteActive(LshapeTextureActive);
    LshapeSpriteActive.setScale(squareSize / LshapeTextureActive.getSize().x, squareSize / LshapeTextureActive.getSize().y);
    sprites.LshapeSpriteActive = LshapeSpriteActive;
    sprites.LshapeTextureActive = LshapeTextureActive;



    sf::Texture TshapeTextureActive;
    if (!TshapeTextureActive.loadFromFile("sprites/Tshape_active.png")) {
        std::cout << "FAILED TO LOAD ACTIVE TSHAPE SPRITE!!!\n";
        return -1;
    }
    sf::Sprite TshapeSpriteActive(TshapeTextureActive);
    TshapeSpriteActive.setScale(squareSize / TshapeTextureActive.getSize().x, squareSize / TshapeTextureActive.getSize().y);
    sprites.TshapeSpriteActive = TshapeSpriteActive;
    sprites.TshapeTextureActive = TshapeTextureActive;


    sf::Texture blobLockedTexture;
    if (!blobLockedTexture.loadFromFile("sprites/blob_locked.png")) {
        std::cout << "FAILED TO LOAD BLOB LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite blobLockedSprite(blobLockedTexture);
    blobLockedSprite.setScale(squareSize / blobLockedTexture.getSize().x, squareSize / blobLockedTexture.getSize().y);
    sprites.blobLockedSprite = blobLockedSprite;
    sprites.blobLockedTexture = blobLockedTexture;

    sf::Texture blobActiveLockedTexture;
    if (!blobActiveLockedTexture.loadFromFile("sprites/blob_active_locked.png")) {
        std::cout << "FAILED TO LOAD ACTIVE BLOB LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite blobActiveLockedSprite(blobActiveLockedTexture);
    blobActiveLockedSprite.setScale(squareSize / blobActiveLockedTexture.getSize().x, squareSize / blobActiveLockedTexture.getSize().y);
    sprites.blobActiveLockedSprite = blobActiveLockedSprite;
    sprites.blobActiveLockedTexture = blobActiveLockedTexture;


    sf::Texture straightLockedTexture;
    if (!straightLockedTexture.loadFromFile("sprites/straight_locked.png")) {
        std::cout << "FAILED TO LOAD STRAIGHT LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite straightLockedSprite(straightLockedTexture);
    straightLockedSprite.setScale(squareSize / straightLockedTexture.getSize().x, squareSize / straightLockedTexture.getSize().y);
    sprites.straightLockedSprite = straightLockedSprite;
    sprites.straightLockedTexture = straightLockedTexture;


    sf::Texture straightActiveLockedTexture;
    if (!straightActiveLockedTexture.loadFromFile("sprites/straight_active_locked.png")) {
        std::cout << "FAILED TO LOAD ACTIVE STRAIGHT LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite straightActiveLockedSprite(straightActiveLockedTexture);
    straightActiveLockedSprite.setScale(squareSize / straightActiveLockedTexture.getSize().x, squareSize / straightActiveLockedTexture.getSize().y);
    sprites.straightActiveLockedSprite = straightActiveLockedSprite;
    sprites.straightActiveLockedTexture = straightActiveLockedTexture;


    sf::Texture LshapeLockedTexture;
    if (!LshapeLockedTexture.loadFromFile("sprites/Lshape_locked.png")) {
        std::cout << "FAILED TO LOAD LSHAPE LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite LshapeLockedSprite(LshapeLockedTexture);
    LshapeLockedSprite.setScale(squareSize / LshapeLockedTexture.getSize().x, squareSize / LshapeLockedTexture.getSize().y);
    sprites.LshapeLockedSprite = LshapeLockedSprite;
    sprites.LshapeLockedTexture = LshapeLockedTexture;


    sf::Texture LshapeActiveLockedTexture;
    if (!LshapeActiveLockedTexture.loadFromFile("sprites/Lshape_active_locked.png")) {
        std::cout << "FAILED TO LOAD ACTIVE LSHAPE LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite LshapeActiveLockedSprite(LshapeActiveLockedTexture);
    LshapeActiveLockedSprite.setScale(squareSize / LshapeActiveLockedTexture.getSize().x, squareSize / LshapeActiveLockedTexture.getSize().y);
    sprites.LshapeActiveLockedSprite = LshapeActiveLockedSprite;
    sprites.LshapeActiveLockedTexture = LshapeActiveLockedTexture;


    sf::Texture TshapeLockedTexture;
    if (!TshapeLockedTexture.loadFromFile("sprites/Tshape_locked.png")) {
        std::cout << "FAILED TO LOAD TSHAPE LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite TshapeLockedSprite(TshapeLockedTexture);
    TshapeLockedSprite.setScale(squareSize / TshapeLockedTexture.getSize().x, squareSize / TshapeLockedTexture.getSize().y);
    sprites.TshapeLockedSprite = TshapeLockedSprite;
    sprites.TshapeLockedTexture = TshapeLockedTexture;


    sf::Texture TshapeActiveLockedTexture;
    if (!TshapeActiveLockedTexture.loadFromFile("sprites/Tshape_active_locked.png")) {
        std::cout << "FAILED TO LOAD ACTIVE TSHAPE LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite TshapeActiveLockedSprite(TshapeActiveLockedTexture);
    TshapeActiveLockedSprite.setScale(squareSize / TshapeActiveLockedTexture.getSize().x, squareSize / TshapeActiveLockedTexture.getSize().y);
    sprites.TshapeActiveLockedSprite = TshapeActiveLockedSprite;
    sprites.TshapeActiveLockedTexture = TshapeActiveLockedTexture;


    //initialize the array of pieces;
    block** game = new block*[n];
    for (int i = 0; i < n; ++i) {
        game[i] = new block[m];
    }

    //lets say they are all blobs with rotation=0;
    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            game[a][b].blob = 1;
            game[a][b].straight = 0;
            game[a][b].Lshape = 0;
            game[a][b].Tshape = 0;
            game[a][b].locked = 0;

            game[a][b].rotation=0;
            game[a][b].transform = sf::Transform();

            game[a][b].holeAtBottom = 0;
            game[a][b].holeAtRight = 0;
            game[a][b].holeAtLeft = 0;
            game[a][b].holeAtTop = 0;
        }
    }

    //choose the origin point of water flow
    int waterOriginX = std::rand()%n;
    int waterOriginY = std::rand()%m;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if ((event.type == sf::Event::Closed) || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Q)) {
                // Window close event
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {

                int mouseX = event.mouseButton.x;
                int mouseY = event.mouseButton.y;

                int a = static_cast<int>(mouseX / squareSize);
                int b = static_cast<int>(mouseY / squareSize);

                if (a >= 0 && a < n && b >= 0 && b < m) {
                    // Rotate the clicked block (if its not locked!)
                    if (game[a][b].locked==false) {
                        rotate(game, a, b);
                    }
                    // activationHelper(game, n, m, a, b);
                    // std::cout << "the block you just clicked at: ("<<a<<", "<<b<<") has a rotation of: " << game[a][b].rotation << "\n";
                    // if (game[a][b].blob) {
                    //     canWeRunBlobLogic(game, n, m, a, b);
                    // }
                }
                
            } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
                    // on right click lock/unlock the block
                    int mouseX = event.mouseButton.x;
                    int mouseY = event.mouseButton.y;

                    int a = static_cast<int>(mouseX / squareSize);
                    int b = static_cast<int>(mouseY / squareSize);

                    if (a >= 0 && a < n && b >= 0 && b < m) {
                        game[a][b].locked = !game[a][b].locked;
                    }
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::D) {
                std::cout << "dfs-esque algorithm generated maze\n";
                dfsGenerator(game, n, m);
                //choose the origin point of water flow
                waterOriginX = std::rand()%n;
                waterOriginY = std::rand()%m;
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P) {
                std::cout << "modified prims algorithm generated maze\n";
                primsGenerator(game, n, m);
                //choose the origin point of water flow
                waterOriginX = std::rand()%n;
                waterOriginY = std::rand()%m;
                // std::cout << "activating water at " << waterOriginX << ", " << waterOriginY << "\n";
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S) {
                std::cout << "solving...\n";
                recursionInfo.clear();
                depthOfRecursion = 0;
                auto start = std::chrono::steady_clock::now();
                for (int a=0; a<maxAnswerSize; a++) {
                    for (int b=0; b<maxAnswerSize; b++) {
                        rotations[a][b] = 0;
                    }
                }
                timesBrokenEarlyBecauseOfCycle = 0;
                backtrackingSolver(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize, delayMs, 1);
                for (int a=0; a<n; a++) {
                    for (int b=0; b<m; b++) {
                        game[a][b].rotation = rotations[a][b];
                        game[a][b].locked = true;
                    }
                }
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                std::cout << "recursionInfo:\n";
                for (int a=0; a<recursionInfo.size(); a++) {
                    std::cout << "recursionInfo("<<a<<"): " << recursionInfo[a] << "\n";
                }
                std::cout << "times broken early because of locked cycle: " << timesBrokenEarlyBecauseOfCycle << "\n";
                std::cout << "solved in: " << duration << " milliseconds" << std::endl;
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F) {
                std::cout << "FAST solving...\n";
                recursionInfo.clear();
                depthOfRecursion = 0;
                timesBrokenEarlyBecauseOfCycle = 0;
                auto start = std::chrono::steady_clock::now();
                for (int a=0; a<maxAnswerSize; a++) {
                    for (int b=0; b<maxAnswerSize; b++) {
                        rotations[a][b] = 0;
                    }
                }
                backtrackingSolver(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize, delayMs, 0);
                for (int a=0; a<n; a++) {
                    for (int b=0; b<m; b++) {
                        game[a][b].rotation = rotations[a][b];
                        game[a][b].locked = true;
                    }
                }
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                std::cout << "recursionInfo:\n";
                for (int a=0; a<recursionInfo.size(); a++) {
                    std::cout << "recursionInfo("<<a<<"): " << recursionInfo[a] << "\n";
                }
                std::cout << "times broken early because of locked cycle: " << timesBrokenEarlyBecauseOfCycle << "\n";
                std::cout << "FAST solved in: " << duration << " milliseconds" << std::endl;
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::U) {
                std::cout << "unlocking everything and scrambling...\n";
                for (int a=0; a<n; a++) {
                    for (int b=0; b<m; b++) {
                        game[a][b].locked = false;
                        int numberOfRotations = std::rand() % 4;
                        for (int c=0; c<numberOfRotations; c++) {
                            rotate(game, a, b);
                        }
                    }
                }
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::X) {
                std::cout << "DUMB solve (all of the obvious moves, no backtracking)...\n";
                while (runSolverOnce(game, n, m)==1) {
                    //draw(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize);
                }
                int state = runSolverOnce(game, n, m);
                std::cout << "finished because state is: " << state << "\n";
                std::cout << "dumb solve done...\n";
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::V) {
                // std::cout << "choosing the square we will backtrack on...\n";
                // int chosenX, chosenY;
                // chooseBacktrackingSquare(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize, delayMs, chosenX, chosenY);
                // std::cout << "we will start backtracking on " << chosenX << " " << chosenY << "\n";
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::W) {
                std::cout << "Writing current puzzle to file...\n";
                writeCurrentPuzzleToFile(game, n, m);
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
                std::cout << "Reading puzzle from file...\n";
                readPuzzleFromFile(game, n, m);
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::T) {
                std::cout << "input the number of tests you want to conduct...\n";
                int numberOfTests;
                std::cin >> numberOfTests;

                int totalDuration = 0;
                int longestDuration = 0;

                std::vector<int> allTimes;

                for (int currentTest=0; currentTest<numberOfTests; currentTest++) {
                    //generate the new puzzle
                    primsGenerator(game, n, m);
                    waterOriginX = std::rand()%n;
                    waterOriginY = std::rand()%m;

                    //solve it
                    recursionInfo.clear();
                    depthOfRecursion = 0;
                    timesBrokenEarlyBecauseOfCycle = 0;
                    auto start = std::chrono::steady_clock::now();
                    for (int a=0; a<maxAnswerSize; a++) {
                        for (int b=0; b<maxAnswerSize; b++) {
                            rotations[a][b] = 0;
                        }
                    }
                    backtrackingSolver(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize, delayMs, 0);
                    for (int a=0; a<n; a++) {
                        for (int b=0; b<m; b++) {
                            game[a][b].rotation = rotations[a][b];
                            game[a][b].locked = true;
                        }
                    }
                    auto end = std::chrono::steady_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                    
                    //draw and handle the statistics
                    draw(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize);
                    totalDuration += duration;
                    if (duration>longestDuration) {
                        //we have a new game, that took the longest to solve
                        writeCurrentPuzzleToFile(game, n, m);
                        longestDuration = duration;
                    }

                    std::cout << currentTest << "\t: " << duration << "\n";
                    allTimes.push_back(duration);

                }


                std::sort(allTimes.begin(), allTimes.end());

                std::cout << "average duration: " << totalDuration/numberOfTests << "\n";
                std::cout << "longest duration: " << longestDuration << "\n";
                std::cout << "highest average of ten: " << highestAverageOfTen(allTimes) << "\n";

            }
        }

        
        draw(game, n, m, window, waterOriginX, waterOriginY, sprites, squareSize);

        
    }

    return 0;
}