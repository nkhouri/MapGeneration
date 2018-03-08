/*This program creates a map using cellular automata methods

The program features creation of a map with choice of many variable changes including:
Width and height and resolution, percent chance to spawn, starvation and overpop thresholds, and itterations

Programmed by Nick Khouri 3/6/18
*/


#include <SFML\Graphics.hpp>
#include <cmath>
#include <iostream>
#include <chrono>
#include <thread>

const int WIDTH = 1000;
const int HEIGHT = 1000;
const int RES = 10;
const int GWIDTH = WIDTH / RES;
const int GHEIGHT = HEIGHT / RES;
const int CHANCE_TO_SPAWN = 21;				//modify as a percent
const int STARVATION_THRESHOLD = 2;			//if cell has less, it dies
const int OVERPOPULATION_THRESHOLD = 3;		//if more than this, it dies
const int ITTERATIONS = 50;
const bool OUTLINE_EDGES = true;

//constants used for room generation
const bool MAKE_ROOMS = true;
const int ROOM_COUNT = 5;
const int MIN_ROOM_SIZE = 2;				//referring to square side
const int MAX_ROOM_SIZE = 4;				//referring to square side


int calculateNeighbors(int grid[GWIDTH][GHEIGHT], int x, int y);
int countUDLR(int grid[GWIDTH][GHEIGHT], int x, int y);
void insertRoom(int grid[GWIDTH][GHEIGHT], int x, int y, int roomSide);
bool isEligable(int grid[GWIDTH][GHEIGHT], int x, int y, int length);

using namespace sf;
using namespace std;

struct eligableRooms {
	int x;
	int y;
};

int main() {
	int grid[GWIDTH][GHEIGHT];
	int grid2[GWIDTH][GHEIGHT];
	RenderWindow window(VideoMode(WIDTH, HEIGHT), "Map Generator");
	eligableRooms * a = new eligableRooms[GWIDTH*GHEIGHT];

	while (window.isOpen()) {
		srand(time(NULL));	//reset seed every run
		//create a random grid of 1 and 0 based on the chance to spawn
		for (int i = 0; i <= GWIDTH; i++) {
			for (int j = 0; j <= GHEIGHT; j++) {
				if (rand() % 100 < CHANCE_TO_SPAWN) {
					grid[i%GWIDTH][j%GHEIGHT] = 1;
				}
				else {
					grid[i%GWIDTH][j%GHEIGHT] = 0;
				}
			}
		}


		//update the logic for drawing and itterating the grid
		for (int r = 0; r < ITTERATIONS; r++) {
			for (int i = 0; i <= GWIDTH; i++) {
				for (int j = 0; j <= GHEIGHT; j++) {
					int neighbors = calculateNeighbors(grid, i, j);
					//check if alive and if the cell can be sustained
					if (grid[i%GWIDTH][j%GHEIGHT] == 1) {
						if (neighbors < STARVATION_THRESHOLD) {
							grid2[i%GWIDTH][j%GHEIGHT] = 0;
						}
						else {
							grid2[i%GWIDTH][j%GHEIGHT] = 1;
						}
					}
					//check if dead and if the cell can birth
					else {
						if (neighbors > OVERPOPULATION_THRESHOLD) {
							grid2[i%GWIDTH][j%GHEIGHT] = 1;
						}
						else {
							grid2[i%GWIDTH][j%GHEIGHT] = 0;
						}
					}
				}
			}

			//refresh the grid 
			for (int i = 0; i <= GWIDTH; i++) {
				for (int j = 0; j <= GHEIGHT; j++) {
					grid[i%GWIDTH][j%GHEIGHT] = grid2[i%GWIDTH][j%GHEIGHT];
				}
			}
		}


		if (MAKE_ROOMS == true) {
			int roomSize = (rand() % MAX_ROOM_SIZE) + MIN_ROOM_SIZE;

			//determine all possible room candidates
			int index = 0;

			for (int i = roomSize + 1; i <= GWIDTH - roomSize - 1; i++) {
				for (int j = roomSize + 1; j <= GHEIGHT - roomSize - 1; j++) {
					bool checker = isEligable(grid, i, j, MAX_ROOM_SIZE);
					if (checker == true) {
						a[index].x = i;
						a[index].y = j;
						index++;
					}
				}
			}
			cout << "Possibilities for room spawn: " << index << endl;

			//pick random points for the room creation and insert into the array 
			if (index != 0) {
				for (int i = 0; i < ROOM_COUNT; i++) {
					int roomPos = rand() % index - i;
					roomSize = (rand() % MAX_ROOM_SIZE) + MIN_ROOM_SIZE;
					insertRoom(grid, a[roomPos].x, a[roomPos].y, roomSize);
					a[roomPos] = a[index];
				}
			}
		}

		//make a wall border along the edges of the cave
		if (OUTLINE_EDGES == true) {
			for (int i = 0; i <= GWIDTH; i++) {
				for (int j = 0; j <= GHEIGHT; j++) {
					if (grid[i%GWIDTH][j%GHEIGHT] == 0) {
						int neighbors = countUDLR(grid, i, j);
						if (neighbors >= 1) {
							grid[i%GWIDTH][j%GHEIGHT] = 2;
						}
					}
				}
			}
		}

		for (int i = 0; i <= GWIDTH; i++) {
			for (int j = 0; j <= GHEIGHT; j++) {
				if (grid[i%GWIDTH][j%GHEIGHT] == 1) {
					RectangleShape block(Vector2f(RES, RES));
					block.setFillColor(Color(0,255,0));
					block.setPosition(i*RES, j*RES);
					window.draw(block);
				}
				else if(grid[i%GWIDTH][j%GHEIGHT] == 0){
					RectangleShape block(Vector2f(RES, RES));
					block.setFillColor(Color(0,0,0));
					block.setPosition(i*RES, j*RES);
					window.draw(block);
				}
				else {
					RectangleShape block(Vector2f(RES, RES));
					block.setFillColor(Color(210, 105, 30));
					block.setPosition(i*RES, j*RES);
					window.draw(block);
				}
			}
		}
		window.display();

		std::this_thread::sleep_for(std::chrono::seconds(5));
		window.clear();
	}
		window.close();
		delete[] a;
		return 0;
}

int calculateNeighbors(int grid[GWIDTH][GHEIGHT], int x, int y) {
	int count = 0;
	
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			if (i == 0 && j == 0) {
				//skip the original
			}
			else if (grid[(x + i)%GWIDTH][(y + j)%GHEIGHT] == 1) {
				count++;
			}
		}
	}
	return count;
}

//this function counts the amount of living neighbors in the UP DOWN LEFT and RIGHT positions of the cell
int countUDLR(int grid[GWIDTH][GHEIGHT], int x, int y) {
	int count = 0;

	//right
	if (grid[(x+1) % GWIDTH][(y) % GHEIGHT] == 1) {
		count++;
	}
	//left
	else if (grid[(x - 1) % GWIDTH][(y) % GHEIGHT] == 1) {
		count++;
	}
	//up
	else if (grid[(x) % GWIDTH][(y + 1) % GHEIGHT] == 1) {
		count++;
	}
	//down
	else if (grid[(x) % GWIDTH][(y - 1) % GHEIGHT] == 1) {
		count++;
	}

	return count;
}


bool isEligable(int grid[GWIDTH][GHEIGHT], int x, int y, int length) {
	for (int i = x - length; i < x + length + 1; i++) {
		for (int j = y - length; j < y + length + 1; j++) {
			if (grid[i%GWIDTH][j%GHEIGHT] == 1) {
				return false;
			}
		}
	}
	return true;
}

void insertRoom(int grid[GWIDTH][GHEIGHT], int x, int y, int roomSize) {
	for (int i = x - roomSize; i < x + roomSize + 1; i++) {
		for (int j = y - roomSize; j < y + roomSize + 1; j++) {
			grid[i%GWIDTH][j%GHEIGHT] = 1;
		}
	}
}