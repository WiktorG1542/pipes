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

const int textureSize = 32;

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

void activateWater(block** game, int n, int m, int x, int y, bool &isThereALeak, bool &isThereACycle) {

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
                activateWater(game, n, m, x+1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
        } else if (game[x][y].rotation==1) {
            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
        } else if (game[x][y].rotation==2) {
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
        } else {
            //rotation = 3
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
        }

    } else if (game[x][y].Tshape) {

        if (game[x][y].rotation==0) {
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateRight) {
                activateWater(game, n, m, x+1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
        } else if (game[x][y].rotation==1) {
            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateRight) {
                activateWater(game, n, m, x+1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
        } else if (game[x][y].rotation==2) {
            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateRight) {
                activateWater(game, n, m, x+1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
        } else {
            //rotation = 3
            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            // if (canWeActivateRight) {
            //     activateWater(game, n, m, x, y+1);
            // }
        }

    } else if (game[x][y].straight) {

        if (game[x][y].rotation%2==0) {
            
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateRight) {
                activateWater(game, n, m, x+1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }

        } else {

            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }

        }

    } else {
        //lshape
        if (game[x][y].rotation==0) {
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateRight) {
                activateWater(game, n, m, x+1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
        } else if (game[x][y].rotation==1) {
            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateRight) {
                activateWater(game, n, m, x+1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
        } else if (game[x][y].rotation==2) {
            if (canWeActivateDown) {
                activateWater(game, n, m, x, y+1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
        } else {
            if (canWeActivateUp) {
                activateWater(game, n, m, x, y-1, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
            }
            if (canWeActivateLeft) {
                activateWater(game, n, m, x-1, y, isThereALeak, isThereACycle);
            } else {
                isThereALeak = true;
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

bool canWeRunBlobLogic(block** game, int n, int m, int x, int y) {

    bool canWeActivateUp = false;
    bool canWeActivateDown = false;
    bool canWeActivateLeft = false;
    bool canWeActivateRight = false;

    bool isLeftUnreachable = false;
    bool isRightUnreachable = false;
    bool isUpUnreachable = false;
    bool isDownUnreachable = false;

    //checking whether we can activate up
    if (y-1>=0) {
        if (game[x][y-1].blob) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].straight && (game[x][y-1].rotation%2==1) && game[x][y-1].locked) {
            canWeActivateUp = true;
        } else if (game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].Tshape && game[x][y-1].rotation!=0 && game[x][y-1].locked) {
            canWeActivateUp = true;
        } else if (game[x][y-1].locked) {
            isUpUnreachable = true;
        }
        if (game[x][y-1].Lshape && (game[x][y-1].rotation==1 || game[x][y-1].rotation==2) && game[x][y-1].locked) {
            canWeActivateUp = true;
        } else if (game[x][y-1].locked) {
            isUpUnreachable = true;
        }
    } else {
        isUpUnreachable = true;
    }

    //checking whether we can activate down
    if (y+1<m) {
        if (game[x][y+1].blob) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].straight && (game[x][y+1].rotation%2==1) && game[x][y+1].locked) {
            canWeActivateDown = true;
        } else if (game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].Tshape && game[x][y+1].rotation!=2 && game[x][y+1].locked) {
            canWeActivateDown = true;
        } else if (game[x][y+1].locked) {
            isDownUnreachable = true;
        }
        if (game[x][y+1].Lshape && (game[x][y+1].rotation==0 || game[x][y+1].rotation==3) && game[x][y+1].locked) {
            canWeActivateDown = true;
        } else if (game[x][y+1].locked) {
            isDownUnreachable = true;
        }
    } else {
        isDownUnreachable = true;
    }

    //checking whether we can activate left
    if (x-1>=0) {
        if (game[x-1][y].blob) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].straight && (game[x-1][y].rotation%2==0) && game[x-1][y].locked) {
            canWeActivateLeft = true;
        } else if (game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].Tshape && game[x-1][y].rotation!=3 && game[x-1][y].locked) {
            canWeActivateLeft = true;
        } else if (game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
        if (game[x-1][y].Lshape && (game[x-1][y].rotation==1 || game[x-1][y].rotation==0) && game[x-1][y].locked) {
            canWeActivateLeft = true;
        } else if (game[x-1][y].locked) {
            isLeftUnreachable = true;
        }
    } else {
        isLeftUnreachable = true;
    }

    //checking whether we can activate right
    if (x+1<n) {
        if (game[x+1][y].blob) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].straight && (game[x+1][y].rotation%2==0) && game[x+1][y].locked) {
            canWeActivateRight = true;
        } else if (game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].Tshape && game[x+1][y].rotation!=1 && game[x+1][y].locked) {
            canWeActivateRight = true;
        } else if (game[x+1][y].locked) {
            isRightUnreachable = true;
        }
        if (game[x+1][y].Lshape && (game[x+1][y].rotation==2 || game[x+1][y].rotation==3) && game[x+1][y].locked) {
            canWeActivateRight = true;
        } else if (game[x+1][y].locked) {
            isRightUnreachable = true;
        }
    } else {
        isRightUnreachable = true;
    }

    int count = canWeActivateDown + canWeActivateUp + canWeActivateLeft + canWeActivateRight;

    if (count==1) {

        //rotate it
        if (canWeActivateUp) {
            game[x][y].rotation = 3;
        } else if (canWeActivateDown) {
            game[x][y].rotation = 1;
        } else if (canWeActivateLeft) {
            game[x][y].rotation = 2;
        } else {
            game[x][y].rotation = 0;
        }

        game[x][y].locked = 1;
        return true;

    } else {

        //check the unreachables
        int unreachablesCount = isLeftUnreachable + isRightUnreachable + isDownUnreachable + isUpUnreachable;
        if (unreachablesCount==3) {
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
            return true;

        } else {
            return false;
        }

    }

}

bool canWeRunStraightLogic(block** game, int n, int m, int x, int y) {

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
        return true;

    } else if (count==2) {

        //logic for two, only cases both up and down, or both left and right
        if (doWeHaveToActivateDown && doWeHaveToActivateUp) {
            game[x][y].rotation = 1;
        } else if (doWeHaveToActivateRight && doWeHaveToActivateLeft) {
            game[x][y].rotation = 0;
        }
        game[x][y].locked = true;
        return true;

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
        return true;

    } else if (unreachablesCount==2) {

        if (isUpUnreachable && isDownUnreachable) {
            game[x][y].rotation = 0;
        } else if (isLeftUnreachable && isRightUnreachable) {
            game[x][y].rotation = 1;
        }
        game[x][y].locked = true;
        return true;

    } else {
        return false;
    }

}

bool canWeRunTshapeLogic(block** game, int n, int m, int x, int y) {

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
        return true;

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
        return true;

    } else {
        return false;
    }

}

bool canWeRunLshapeLogic(block** game, int n, int m, int x, int y) {

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

    //each rotation has 4 cases
    if ((doWeHaveToActivateUp && doWeHaveToActivateRight) || (doWeHaveToActivateUp && isLeftUnreachable) || (doWeHaveToActivateRight && isDownUnreachable) || (isDownUnreachable && isLeftUnreachable)) {
        game[x][y].rotation = 0;
        game[x][y].locked = 1;
        return true;
    }

    if ((doWeHaveToActivateRight && doWeHaveToActivateDown) || (doWeHaveToActivateRight && isUpUnreachable) || (doWeHaveToActivateDown && isLeftUnreachable) || (isLeftUnreachable && isUpUnreachable)) {
        game[x][y].rotation = 1;
        game[x][y].locked = 1;
        return true;
    }

    if ((doWeHaveToActivateDown && doWeHaveToActivateLeft) || (doWeHaveToActivateDown && isRightUnreachable) || (doWeHaveToActivateLeft && isUpUnreachable) || (isUpUnreachable && isRightUnreachable)) {
        game[x][y].rotation = 2;
        game[x][y].locked = 1;
        return true;
    }

    if ((doWeHaveToActivateLeft && doWeHaveToActivateUp) || (doWeHaveToActivateLeft && isDownUnreachable) || (doWeHaveToActivateUp && isRightUnreachable) || (isRightUnreachable && isDownUnreachable)) {
        game[x][y].rotation = 3;
        game[x][y].locked = 1;
        return true;
    }

    return false;

}

bool runSolverOnce(block** game, int n, int m) {

    for (int a=0; a<n; a++) {
        for (int b=0; b<m; b++) {
            if (game[a][b].locked==false) {
                if (game[a][b].blob) {
                    if (canWeRunBlobLogic(game, n, m, a, b)) {
                        // std::cout << "just set a blob at " << a << ", " << b << " to rotation: " << game[a][b].rotation << "\n";
                        return true;
                    }
                } else if (game[a][b].straight) {
                    if (canWeRunStraightLogic(game, n, m, a, b)) {
                        // std::cout << "just set a straight at " << a << ", " << b << " to rotation: " << game[a][b].rotation << "\n";
                        return true;
                    }
                } else if (game[a][b].Tshape) {
                    if (canWeRunTshapeLogic(game, n, m, a, b)) {
                        // std::cout << "just set a tshape at " << a << ", " << b << " to rotation: " << game[a][b].rotation << "\n";
                        return true;
                    }
                } else if (game[a][b].Lshape) {
                    if (canWeRunLshapeLogic(game, n, m, a, b)) {
                        // std::cout << "just set a lshape at " << a << ", " << b << " to rotation: " << game[a][b].rotation << "\n";
                        return true;
                    }
                }
            }
        }
    }

    return false;

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


    // std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
    // std::cout << "u clicked a block at :" << x << ", " << y << "\n";

    // std::cout << "Count: " << count << "\n\n";
    // std::cout << "doWeHaveToActivateUp: " << doWeHaveToActivateUp << "\n";
    // std::cout << "doWeHaveToActivateRight: " << doWeHaveToActivateRight << "\n";
    // std::cout << "doWeHaveToActivateDown: " << doWeHaveToActivateDown << "\n";
    // std::cout << "doWeHaveToActivateLeft: " << doWeHaveToActivateLeft << "\n\n\n\n";

    // std::cout << "Unreachables Count: " << unreachablesCount << "\n\n";
    // std::cout << "isUpUnreachable: " << isUpUnreachable << "\n";
    // std::cout << "isRightUnreachable: " << isRightUnreachable << "\n";
    // std::cout << "isDownUnreachable: " << isDownUnreachable << "\n";
    // std::cout << "isLeftUnreachable: " << isLeftUnreachable << "\n";


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
    sf::Texture blobTexture;
    if (!blobTexture.loadFromFile("sprites/blob.png")) {
        std::cout << "FAILED TO LOAD BLOB SPRITE!!!\n";
        return -1;
    }
    sf::Sprite blobSprite = makeSprite(blobTexture, squareSize);

    sf::Texture straightTexture;
    if (!straightTexture.loadFromFile("sprites/straight.png")) {
        std::cout << "FAILED TO LOAD STRAIGHT SPRITE!!!\n";
        return -1;
    }
    sf::Sprite straightSprite(straightTexture);
    straightSprite.setScale(squareSize / straightTexture.getSize().x, squareSize / straightTexture.getSize().y);

    sf::Texture LshapeTexture;
    if (!LshapeTexture.loadFromFile("sprites/Lshape.png")) {
        std::cout << "FAILED TO LOAD LSHAPE SPRITE!!!\n";
        return -1;
    }
    sf::Sprite LshapeSprite(LshapeTexture);
    LshapeSprite.setScale(squareSize / LshapeTexture.getSize().x, squareSize / LshapeTexture.getSize().y);

    sf::Texture TshapeTexture;
    if (!TshapeTexture.loadFromFile("sprites/Tshape.png")) {
        std::cout << "FAILED TO LOAD TSHAPE SPRITE!!!\n";
        return -1;
    }
    sf::Sprite TshapeSprite(TshapeTexture);
    TshapeSprite.setScale(squareSize / TshapeTexture.getSize().x, squareSize / TshapeTexture.getSize().y);

    sf::Texture blobTextureActive;
    if (!blobTextureActive.loadFromFile("sprites/blob_active.png")) {
        std::cout << "FAILED TO LOAD ACTIVE BLOB SPRITE!!!\n";
        return -1;
    }
    sf::Sprite blobSpriteActive(blobTextureActive);
    blobSpriteActive.setScale(squareSize / blobTextureActive.getSize().x, squareSize / blobTextureActive.getSize().y);

    sf::Texture straightTextureActive;
    if (!straightTextureActive.loadFromFile("sprites/straight_active.png")) {
        std::cout << "FAILED TO LOAD ACTIVE STRAIGHT SPRITE!!!\n";
        return -1;
    }
    sf::Sprite straightSpriteActive(straightTextureActive);
    straightSpriteActive.setScale(squareSize / straightTextureActive.getSize().x, squareSize / straightTextureActive.getSize().y);

    sf::Texture LshapeTextureActive;
    if (!LshapeTextureActive.loadFromFile("sprites/Lshape_active.png")) {
        std::cout << "FAILED TO LOAD ACTIVE LSHAPE SPRITE!!!\n";
        return -1;
    }
    sf::Sprite LshapeSpriteActive(LshapeTextureActive);
    LshapeSpriteActive.setScale(squareSize / LshapeTextureActive.getSize().x, squareSize / LshapeTextureActive.getSize().y);

    sf::Texture TshapeTextureActive;
    if (!TshapeTextureActive.loadFromFile("sprites/Tshape_active.png")) {
        std::cout << "FAILED TO LOAD ACTIVE TSHAPE SPRITE!!!\n";
        return -1;
    }
    sf::Sprite TshapeSpriteActive(TshapeTextureActive);
    TshapeSpriteActive.setScale(squareSize / TshapeTextureActive.getSize().x, squareSize / TshapeTextureActive.getSize().y);

    sf::Texture blobLockedTexture;
    if (!blobLockedTexture.loadFromFile("sprites/blob_locked.png")) {
        std::cout << "FAILED TO LOAD BLOB LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite blobLockedSprite(blobLockedTexture);
    blobLockedSprite.setScale(squareSize / blobLockedTexture.getSize().x, squareSize / blobLockedTexture.getSize().y);

    sf::Texture blobActiveLockedTexture;
    if (!blobActiveLockedTexture.loadFromFile("sprites/blob_active_locked.png")) {
        std::cout << "FAILED TO LOAD ACTIVE BLOB LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite blobActiveLockedSprite(blobActiveLockedTexture);
    blobActiveLockedSprite.setScale(squareSize / blobActiveLockedTexture.getSize().x, squareSize / blobActiveLockedTexture.getSize().y);

    sf::Texture straightLockedTexture;
    if (!straightLockedTexture.loadFromFile("sprites/straight_locked.png")) {
        std::cout << "FAILED TO LOAD STRAIGHT LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite straightLockedSprite(straightLockedTexture);
    straightLockedSprite.setScale(squareSize / straightLockedTexture.getSize().x, squareSize / straightLockedTexture.getSize().y);

    sf::Texture straightActiveLockedTexture;
    if (!straightActiveLockedTexture.loadFromFile("sprites/straight_active_locked.png")) {
        std::cout << "FAILED TO LOAD ACTIVE STRAIGHT LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite straightActiveLockedSprite(straightActiveLockedTexture);
    straightActiveLockedSprite.setScale(squareSize / straightActiveLockedTexture.getSize().x, squareSize / straightActiveLockedTexture.getSize().y);

    sf::Texture LshapeLockedTexture;
    if (!LshapeLockedTexture.loadFromFile("sprites/Lshape_locked.png")) {
        std::cout << "FAILED TO LOAD LSHAPE LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite LshapeLockedSprite(LshapeLockedTexture);
    LshapeLockedSprite.setScale(squareSize / LshapeLockedTexture.getSize().x, squareSize / LshapeLockedTexture.getSize().y);

    sf::Texture LshapeActiveLockedTexture;
    if (!LshapeActiveLockedTexture.loadFromFile("sprites/Lshape_active_locked.png")) {
        std::cout << "FAILED TO LOAD ACTIVE LSHAPE LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite LshapeActiveLockedSprite(LshapeActiveLockedTexture);
    LshapeActiveLockedSprite.setScale(squareSize / LshapeActiveLockedTexture.getSize().x, squareSize / LshapeActiveLockedTexture.getSize().y);

    sf::Texture TshapeLockedTexture;
    if (!TshapeLockedTexture.loadFromFile("sprites/Tshape_locked.png")) {
        std::cout << "FAILED TO LOAD TSHAPE LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite TshapeLockedSprite(TshapeLockedTexture);
    TshapeLockedSprite.setScale(squareSize / TshapeLockedTexture.getSize().x, squareSize / TshapeLockedTexture.getSize().y);

    sf::Texture TshapeActiveLockedTexture;
    if (!TshapeActiveLockedTexture.loadFromFile("sprites/Tshape_active_locked.png")) {
        std::cout << "FAILED TO LOAD ACTIVE TSHAPE LOCKED SPRITE!!!\n";
        return -1;
    }
    sf::Sprite TshapeActiveLockedSprite(TshapeActiveLockedTexture);
    TshapeActiveLockedSprite.setScale(squareSize / TshapeActiveLockedTexture.getSize().x, squareSize / TshapeActiveLockedTexture.getSize().y);


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

    //for solver
    bool doWeSolve = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
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
                    //activationHelper(game, n, m, a, b);
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
                std::cout << "STARTING TO SOLVE!!!\n";
                doWeSolve = true;
            }
        }

        bool continueRendering = true;        

        while (continueRendering) {


            if (doWeSolve) {
                continueRendering = true;
            } else {
                continueRendering = false;
            }

            if (doWeSolve) {

                // int debugInt;
                // std::cin >> debugInt;

                if (!runSolverOnce(game, n, m)) {
                    std::cout << "SOLVING HAS ENDED!!!\n\n";
                    doWeSolve = false;
                }
                // std::cout << "\n";
            }
            
            if (doWeSolve) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            }

            window.clear();
            //reset activity
            for (int i=0; i<n; i++) {
                for (int j=0; j<m; j++) {
                    game[i][j].active = 0;
                }
            }
            //activate
            bool isThereALeak = false;
            bool isThereACycle = false;
            activateWater(game, n, m, waterOriginX, waterOriginY, isThereALeak, isThereACycle);

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
                                sprite = blobActiveLockedSprite;
                            } else {
                                sprite = blobSpriteActive;
                            }
                        } else {
                            if (game[i][j].locked) {
                                sprite = blobLockedSprite;
                            } else {
                                sprite = blobSprite;
                            }
                        }
                    } else if (game[i][j].straight) {
                        if (game[i][j].active) {
                            if (game[i][j].locked) {
                                sprite = straightActiveLockedSprite;
                            } else {
                                sprite = straightSpriteActive;
                            }
                        } else {
                            if (game[i][j].locked) {
                                sprite = straightLockedSprite;
                            } else {
                                sprite = straightSprite;
                            }
                        }
                    } else if (game[i][j].Lshape) {
                        if (game[i][j].active) {
                            if (game[i][j].locked) {
                                sprite = LshapeActiveLockedSprite;
                            } else {
                                sprite = LshapeSpriteActive;
                            }
                        } else {
                            if (game[i][j].locked) {
                                sprite = LshapeLockedSprite;
                            } else {
                                sprite = LshapeSprite;
                            }
                        }
                    } else {
                        if (game[i][j].active) {
                            if (game[i][j].locked) {
                                sprite = TshapeActiveLockedSprite;
                            } else {
                                sprite = TshapeSpriteActive;
                            }
                        } else {
                            if (game[i][j].locked) {
                                sprite = TshapeLockedSprite;
                            } else {
                                sprite = TshapeSprite;
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
    }

    return 0;
}
