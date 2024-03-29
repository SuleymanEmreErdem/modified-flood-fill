import API
import sys
import time
#from machine import Pin
#from micropython import const

INF = 0 #INF = const(0)
N = 16 #N = const(16)
dx, dy = [0, 1, 0, -1], [1, 0, -1, 0]

#use list as queue

def log(string):
    sys.stderr.write("{}\n".format(string))
    sys.stderr.flush()

#count directions for the current cell
def countWays():
    return (not API.wallFront()) + (not API.wallRight()) + (not API.wallLeft())

#move forward and update position values
def move(x, y, turnR):
    API.moveForward()
    new_x, new_y = x + dx[turnR], y + dy[turnR]
    return new_x, new_y

#left-handed turn
def left_handed(turnR):
    new_turnR = turnR
    if not API.wallLeft():
        API.turnLeft()
        new_turnR = (new_turnR + 3) % 4
    while API.wallFront():
        API.turnRight()
        new_turnR = (new_turnR + 1) % 4
    return new_turnR

#paint the visited cell
def markCell(color, x, y):
    API.setColor(x, y, color)

#check if position == final cells
def isFinish(x, y):
    if (x==7 and y==7) or (x==7 and y==8) or (x==8 and y==7) or (x==8 and y==8):
        return 1
    else:
        return 0

#check if position == start cell
def isStart(x, y):
    if x==0 and y==0:
        return 1
    else:
        return 0

#flood the grid to finish
def floodToFinish(walls):
    flood = [[INF for _ in range(N)] for _ in range(N)]

    flood[7][7], flood[7][8], flood[8][7], flood[8][8] = 1, 1, 1, 1
    q = [(7, 7), (7, 8), (8, 7), (8, 8)]

    while q:
        x, y = q.pop(0)
        for i in range(4):
            nx, ny = x + dx[i], y + dy[i]
            if 0 <= nx < N and 0 <= ny < N and flood[nx][ny] == INF and walls[x][y][i] == 0:
                flood[nx][ny] = flood[x][y] + 1
                q.append((nx, ny))

    return flood

#flood the grid to start
def floodToStart(walls):
    flood = [[INF for _ in range(N)] for _ in range(N)]

    flood[0][0] = 1
    q = [(0, 0)]

    while q:
        x, y = q.pop(0)
        for i in range(4):
            nx, ny = x + dx[i], y + dy[i]
            if 0 <= nx < N and 0 <= ny < N and flood[nx][ny] == INF and walls[x][y][i] == 0:
                flood[nx][ny] = flood[x][y] + 1
                q.append((nx, ny))

    return flood

#store explored wall data
def storeWalls(x, y, turnR, walls):
    new_walls = walls
    if API.wallFront():
        wx, wy = x + dx[turnR], y + dy[turnR]
        new_walls[x][y][turnR] = 1
        if 0 <= wx < N and 0 <= wy < N:
            new_walls[wx][wy][(turnR+2)%4] = 1

    if API.wallRight():
        wx, wy = x + dx[(turnR+1)%4], y + dy[(turnR+1)%4]
        new_walls[x][y][(turnR+1)%4] = 1
        if 0 <= wx < N and 0 <= wy < N:
            new_walls[wx][wy][(turnR+3)%4] = 1

    if API.wallLeft():
        wx, wy = x + dx[(turnR+3)%4], y + dy[(turnR+3)%4]
        new_walls[x][y][(turnR+3)%4] = 1
        if 0 <= wx < N and 0 <= wy < N:
            new_walls[wx][wy][(turnR+1)%4] = 1

    return new_walls

def main():
    x, y, turnR = 0, 0, 0
    walls = [[[0 for _ in range(4)] for _ in range(N)] for _ in range(N)]

    for i in range(3):
        while not isFinish(x, y):
            walls = storeWalls(x, y, turnR, walls)
            if countWays() < 2:
                turnR = left_handed(turnR)
            else:
                flood = floodToFinish(walls)
                if (0 <= x+dx[turnR] < N and 0 <= y+dy[turnR] < N) and flood[x+dx[turnR]][y+dy[turnR]] < flood[x][y] and (not API.wallFront()):
                    pass
                elif (0 <= x+dx[(turnR+1)%4] < N and 0 <= y+dy[(turnR+1)%4] < N) and flood[x+dx[(turnR+1)%4]][y+dy[(turnR+1)%4]] < flood[x][y] and (not API.wallRight()):
                    API.turnRight()
                    turnR = (turnR+1) % 4
                elif (0 <= x+dx[(turnR+3)%4] < N and 0 <= y+dy[(turnR+3)%4] < N) and flood[x+dx[(turnR+3)%4]][y+dy[(turnR+3)%4]] < flood[x][y] and (not API.wallLeft()):
                    API.turnLeft()
                    turnR = (turnR+3) % 4
                else:
                    turnR = left_handed(turnR)
            markCell('G', x, y)
            x, y = move(x, y, turnR)

        while not isStart(x, y):
            walls = storeWalls(x, y, turnR, walls)
            if countWays() < 2:
                turnR = left_handed(turnR)
            else:
                flood = floodToStart(walls)
                if (0 <= x+dx[turnR] < N and 0 <= y+dy[turnR] < N) and flood[x+dx[turnR]][y+dy[turnR]] < flood[x][y] and (not API.wallFront()):
                        pass
                elif (0 <= x+dx[(turnR+1)%4] < N and 0 <= y+dy[(turnR+1)%4] < N) and flood[x+dx[(turnR+1)%4]][y+dy[(turnR+1)%4]] < flood[x][y] and (not API.wallRight()):
                        API.turnRight()
                        turnR = (turnR+1) % 4
                elif (0 <= x+dx[(turnR+3)%4] < N and 0 <= y+dy[(turnR+3)%4] < N) and flood[x+dx[(turnR+3)%4]][y+dy[(turnR+3)%4]] < flood[x][y] and (not API.wallLeft()):
                        API.turnLeft()
                        turnR = (turnR+3) % 4
                else:
                    turnR = left_handed(turnR)
            markCell('G', x, y)
            x, y = move(x, y, turnR)

    time.sleep(2)

    flood = floodToFinish(walls)
    while not isFinish(x, y):
        if countWays() < 2:
            turnR = left_handed(turnR)
        else:
            if (0 <= x+dx[turnR] < N and 0 <= y+dy[turnR] < N) and flood[x+dx[turnR]][y+dy[turnR]] < flood[x][y] and (not API.wallFront()):
                pass
            elif (0 <= x+dx[(turnR+1)%4] < N and 0 <= y+dy[(turnR+1)%4] < N) and flood[x+dx[(turnR+1)%4]][y+dy[(turnR+1)%4]] < flood[x][y] and (not API.wallRight()):
                API.turnRight()
                turnR = (turnR+1) % 4
            elif (0 <= x+dx[(turnR+3)%4] < N and 0 <= y+dy[(turnR+3)%4] < N) and flood[x+dx[(turnR+3)%4]][y+dy[(turnR+3)%4]] < flood[x][y] and (not API.wallLeft()):
                API.turnLeft()
                turnR = (turnR+3) % 4
            else:
                turnR = left_handed(turnR)
        markCell('R', x, y)
        x, y = move(x, y, turnR)

if __name__ == '__main__':
    main()