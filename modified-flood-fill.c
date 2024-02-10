#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <limits.h>
#include "API.h"

int x=0, y=0, turnR=0;
int dx[4] = {0, 1, 0, -1}, dy[4] = {1, 0, -1, 0};
char buffer[50];
int flood[16][16] = {};
char walls[16][16][4];

// A structure to represent a queue
struct Queue {
	int front, rear, size;
	unsigned capacity;
	int* array;
};

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
	struct Queue* queue = (struct Queue*)malloc(
		sizeof(struct Queue));
	queue->capacity = capacity;
	queue->front = queue->size = 0;

	// This is important, see the enqueue
	queue->rear = capacity - 1;
	queue->array = (int*)malloc(
		queue->capacity * sizeof(int));
	return queue;
}

// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue* queue)
{
	return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{
	return (queue->size == 0);
}

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, int item)
{
	if (isFull(queue))
		return;
	queue->rear = (queue->rear + 1)
				% queue->capacity;
	queue->array[queue->rear] = item;
	queue->size = queue->size + 1;
}

// Function to remove an item from queue.
// It changes front and size
int dequeue(struct Queue* queue)
{
	if (isEmpty(queue))
		return INT_MIN;
	int item = queue->array[queue->front];
	queue->front = (queue->front + 1)
				% queue->capacity;
	queue->size = queue->size - 1;
	return item;
}

// Function to get front of queue
int front(struct Queue* queue)
{
	if (isEmpty(queue))
		return INT_MIN;
	return queue->array[queue->front];
}

// Function to get rear of queue
int rear(struct Queue* queue)
{
	if (isEmpty(queue))
		return INT_MIN;
	return queue->array[queue->rear];
}

//logi print
void logit(char* text) {
    fprintf(stderr, "%s\n", text);
    fflush(stderr);
}

void logWall(int a) {
    fprintf(stderr, "%d. exploration run...\n", a);
    fflush(stderr);
}

//count directions of cell
int countWays(){
    return !API_wallFront() + !API_wallRight() + !API_wallLeft();
}

//move and change position as x, y
void move(){
    API_moveForward();
    x += dx[turnR];
    y += dy[turnR];
}

//left-handed turn
void left_handed(){
    if (!API_wallLeft()) {
        API_turnLeft();
        turnR = (turnR + 3) % 4;
    }
    while (API_wallFront()){ //|| redMap[x+dx[turnR]][y+dy[turnR]]) {
        API_turnRight();
        turnR = (turnR + 1) % 4;
    }
}

//right-handed turn
void right_handed(){
    if(!API_wallRight()){
        API_turnRight;
        turnR = (turnR + 1) % 4;
    }
    while (API_wallFront()){ //|| redMap[x+dx[turnR]][y+dy[turnR]]){
        API_turnLeft();
        turnR = (turnR + 3) % 4;
    }
}

//mark green if explored
void markCell(char color){
    API_setColor(x, y, color);
   /*sprintf(buffer, "%d", countWays());
    API_setText(x, y, buffer);*/
}

//compare cells by distance to 7, 7
int getCost(int turn){
    return abs((x + dx[(turnR + turn) % 4]) - 7) + abs((y + dy[(turnR + turn) % 4]) - 7);
}


//return 1 if entered in finish grid
int isFinish(){
    if((x==7 && y==7) || (x==7 && y==8) || (x==8 && y==7) || (x==8 && y==8)) return 1;
    return 0;
}

int isStart(){
    if(x==0 && y==0) return 1;
    return 0;
}

//flood the grid considering walls
void floodToFinish(){
    int x, y, nx, ny;
    struct Queue* q = createQueue(200);
    for(int i = 0; i < 16; i++)
        for(int j = 0; j < 16; j++)
            flood[i][j] = -1;

    flood[7][7] = 0; flood[7][8] = 0; flood[8][7] = 0; flood[8][8] = 0;
    enqueue(q, 7); enqueue(q, 7); enqueue(q, 7); enqueue(q, 8); enqueue(q, 8); enqueue(q, 7); enqueue(q, 8); enqueue(q, 8);
    
    while(!isEmpty(q)){
        x = dequeue(q); y = dequeue(q);
		sprintf(buffer, "%d", flood[x][y]);
		API_setText(x, y, buffer);
        for(int i = 0; i < 4; i++){
			nx = x + dx[i]; ny = y + dy[i];
			if(0 <= nx && nx < 16 && 0 <= ny && ny < 16 && flood[nx][ny] == -1 && walls[x][y][i] != 1){
				flood[nx][ny] = flood[x][y] + 1;
				enqueue(q, nx); enqueue(q, ny);
			}
		}
    }
}

void floodToStart(){
    int x, y, nx, ny;
    struct Queue* q = createQueue(100);
    for(int i = 0; i < 16; i++)
        for(int j = 0; j < 16; j++)
            flood[i][j] = -1;

    flood[0][0] = 0; enqueue(q, 0); enqueue(q, 0);
    
    while(!isEmpty(q)){
        x = dequeue(q); y = dequeue(q);
		sprintf(buffer, "%d", flood[x][y]);
		API_setText(x, y, buffer);
        for(int i = 0; i < 4; i++){
			nx = x + dx[i]; ny = y + dy[i];
			if(0 <= nx && nx < 16 && 0 <= ny && ny < 16 && flood[nx][ny] == -1 && walls[x][y][i] != 1){
				flood[nx][ny] = flood[x][y] + 1;
				enqueue(q, nx); enqueue(q, ny);
			}
		}
    }
}

void storeWalls(){
    char wx, wy;
    if(API_wallFront()){
        wx = x + dx[turnR]; wy = y + dy[turnR];
        walls[x][y][turnR] = 1;
        if(!(wx & 0xf0 || wy & 0xf0))
            walls[wx][wy][(turnR+2)%4] = 1;
    }
    if(API_wallRight()){
        wx = x + dx[(turnR+1)%4]; wy = y + dy[(turnR+1)%4];
        walls[x][y][(turnR+1)%4] = 1;
        if(!(wx & 0xf0 || wy & 0xf0))
            walls[wx][wy][(turnR+3)%4] = 1;
    }
    if(API_wallLeft()){
        wx = x + dx[(turnR+3)%4]; wy = y + dy[(turnR+3)%4];
        walls[x][y][(turnR+3)%4] = 1;
        if(!(wx & 0xf0 || wy & 0xf0))
            walls[wx][wy][(turnR+1)%4] = 1;
    }
}

int main(int argc, char* argv[]){

    for (int i=0; i<3; i++){
        logWall(i+1);
        while (!isFinish()) {
            storeWalls();
            floodToFinish();
            if(countWays() < 2){
                left_handed();
            }
            else{
                if(flood[x+dx[turnR]][y+dy[turnR]] < flood[x][y] && !API_wallFront());
                else if(flood[x+dx[(turnR+1)%4]][y+dy[(turnR+1)%4]] < flood[x][y] && !API_wallRight()){
                    API_turnRight();
                    turnR = (turnR+1) % 4;
                }
                else if(flood[x+dx[(turnR+3)%4]][y+dy[(turnR+3)%4]] < flood[x][y] && !API_wallLeft()){
                    API_turnLeft();
                    turnR = (turnR+3) % 4;
                }
                else left_handed();
            }
            markCell('G');
            move();
        }
        storeWalls();
        while(!isStart()){
            storeWalls();
            floodToStart();
            if(countWays() < 2){
                left_handed();
            }
            else{
                if(flood[x+dx[turnR]][y+dy[turnR]] < flood[x][y] && !API_wallFront());
                else if(flood[x+dx[(turnR+1)%4]][y+dy[(turnR+1)%4]] < flood[x][y] && !API_wallRight()){
                    API_turnRight();
                    turnR = (turnR+1) % 4;
                }
                else if(flood[x+dx[(turnR+3)%4]][y+dy[(turnR+3)%4]] < flood[x][y] && !API_wallLeft()){
                    API_turnLeft();
                    turnR = (turnR+3) % 4;
                }
                else left_handed();
            }
            markCell('G');
            move();
        }
        storeWalls();
    }

    Sleep(5000);
    logit("Run on optimised path...");
    floodToFinish();
    while(!isFinish()){
        if(countWays() < 2){
            left_handed();
        }
        else{
            if(flood[x+dx[turnR]][y+dy[turnR]] < flood[x][y] && !API_wallFront());
            else if(flood[x+dx[(turnR+1)%4]][y+dy[(turnR+1)%4]] < flood[x][y] && !API_wallRight()){
                API_turnRight();
                turnR = (turnR+1) % 4;
            }
            else if(flood[x+dx[(turnR+3)%4]][y+dy[(turnR+3)%4]] < flood[x][y] && !API_wallLeft()){
                API_turnLeft();
                turnR = (turnR+3) % 4;
            }
            else left_handed();
        }
        markCell('R');
        move();
    }

    return 0;
}