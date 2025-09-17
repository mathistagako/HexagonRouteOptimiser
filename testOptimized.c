#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define MAX 30
#define MAX_QUEUE 50000

static const int odd_dx[6]  = {-1, 0, 0, 1, 1, 1};
static const int odd_dy[6]  = { 0, 1,-1, 1,-1, 0};
static const int even_dx[6] = {-1,-1,-1, 0, 0, 1};
static const int even_dy[6] = {-1, 1, 0, 1,-1, 0};

static uint32_t (*air_dest)[5] = NULL;
static uint8_t  (*air_cost)[5] = NULL;
static uint8_t  *air_num = NULL;

typedef struct Hexagon {

    int column;
    int row;
    int cost;

} Hexagon;

typedef struct{

    Hexagon* hex;
    int raggio;

} QueueStruct;

typedef struct{

    struct Hexagon* hex;
    int cost;

} PriorityQueueNode;

typedef struct{

    PriorityQueueNode queue[MAX_QUEUE];
    int size;

} PriorityQueue;

typedef struct{
    int cost;
    uint32_t start;
    uint32_t dest;
} travelCostResult;

Hexagon*** init(int n1,int n2);
Hexagon* createHexagon(int col,int row);
void changeCost(int col,int row,int v, int raggio, int n1, int n2, Hexagon*** islands);
void toggleAirRoute(int startCol, int startRow, int destCol, int destRow, int n1, int n2, Hexagon*** islands);
void travelCost(int startCol, int startRow, int destCol, int destRow, int n1, int n2 ,Hexagon*** islands,travelCostResult** results, size_t* resultsCount, size_t* resultsCap);
void queue_push(PriorityQueue* queue, Hexagon* hex, int cost);
PriorityQueueNode queue_pop(PriorityQueue* queue);
void free_map(int n1, int n2, Hexagon*** islands);
void emptyResults(travelCostResult** results, size_t* resultsCount, size_t* resultsCap);

int main(void)
{
    char input[MAX];
    int maxCol = 0;
    int maxRow = 0;
    int num1,num2,num3,num4; 
    char str[] = "init";
    Hexagon*** map = NULL;
    travelCostResult* results = NULL;
    size_t resultsCount = 0;
    size_t resultsCap = 0;

    while(scanf("%s", input)==1){

        if (strcmp(input,"exit")==0)
        {
            break;
        }
        
        if(strcmp(input,str) == 0){
            if(scanf("%d %d", &num1, &num2)==0){
                return 0;
            }
            if(map){
                free_map(maxCol, maxRow, map);
            }
            map = init(num1,num2);
            maxCol = num1;
            maxRow = num2;
            printf("OK\n");
        }else if (strcmp(input,"exit")!=0){
            if(scanf("%d %d %d %d", &num1, &num2, &num3, &num4)==0){
                return 0;
            }
            if(strcmp(input,"travel_cost")==0){
                travelCost(num1,num2,num3,num4,maxCol,maxRow,map,&results,&resultsCount,&resultsCap);
            }else if(strcmp(input,"change_cost")==0){
                changeCost(num1,num2,num3,num4,maxCol,maxRow,map);
                emptyResults(&results, &resultsCount, &resultsCap);
            }else if(strcmp(input,"toggle_air_route")==0){
                toggleAirRoute(num1,num2,num3,num4,maxCol,maxRow,map);
                emptyResults(&results, &resultsCount, &resultsCap);
            }
        }
    }
    return 0;
}

Hexagon*** init(int n1,int n2){

    Hexagon*** islands = malloc(n1 * sizeof(Hexagon**));
    for (int col = 0; col < n1; col++) {
        islands[col] = malloc(n2 * sizeof(Hexagon*));
        for (int row = 0; row < n2; row++) {
            islands[col][row] = createHexagon(col, row);
        }
    }

    size_t N = (size_t)n1 * (size_t)n2;
    if(air_dest){
        free(air_dest);
        air_dest = NULL;
    }
    if(air_cost){
        free(air_cost);
        air_cost = NULL;
    }
    if(air_num){
        free(air_num);
        air_num = NULL;
    }
    air_dest = malloc(N * sizeof *air_dest);
    air_cost = malloc(N * sizeof *air_cost);
    air_num  = calloc(N, 1);

    return islands;
}

Hexagon* createHexagon(int col,int row){

    Hexagon* hex = calloc(1, sizeof(Hexagon));
    hex->column = col;
    hex->row = row;
    hex->cost = 1;

    return hex;
}

void changeCost(int col,int row,int v, int raggio, int n1, int n2, Hexagon*** islands){
    if(islands==NULL || row<0 || row>=n2 || col<0 || col>=n1 || raggio==0 || v<-10 || v>10){
        printf("KO\n");
        return;
    }
    
    QueueStruct queue[MAX_QUEUE];
    int first = 0,last = 0;

    int visited[n1][n2];
    for (int i = 0; i < n1; i++){
        for (int j = 0; j < n2; j++){
            visited[i][j] = 0;
        }
    }

    queue[last] = (QueueStruct) {islands[col][row], 0};
    last++;
    visited[col][row] = 1;

    while(first<last){
        Hexagon* active = queue[first].hex;
        int dist = queue[first].raggio;
        first++;

        if(dist<raggio){

            int num = v * (raggio - dist);
            int den = raggio;
            int delta;
            if(num>=0){
                delta = (num / den);
            }else{
                delta = -(( -num + den - 1) / den);
            }

            active->cost += delta;
            if (active->cost < 0) active->cost = 0;
            if (active->cost > 100) active->cost = 100;

            uint32_t id = (uint32_t)(active->column) * (uint32_t)(n2) + (uint32_t)(active->row);
            for(int i=0; i<air_num[id]; i++){

                int cost = (int)air_cost[id][i] + delta;
                if(cost<0){
                    cost = 0;
                }else if(cost>100){
                    cost = 100;
                }
                air_cost[id][i] = (uint8_t)cost;
            }
        }

        if (dist + 1 < raggio) {
            for(int dir=0; dir<6; dir++){
                int next_col = active->column + ((active->row % 2 == 0) ? even_dx[dir] : odd_dx[dir]);
                int next_row = active->row + ((active->row % 2 == 0) ? even_dy[dir] : odd_dy[dir]);
                if(!visited[next_col][next_row] && (unsigned)(next_col) < (unsigned)(n1) && (unsigned)(next_row) < (unsigned)(n2)){
                    visited[next_col][next_row] = 1;
                    queue[last] = (QueueStruct){ islands[next_col][next_row], dist+1 };
                    last++;
                }
            }
        }

    }
    printf("OK\n");
}

void toggleAirRoute(int startCol, int startRow, int destCol, int destRow, int n1, int n2 ,Hexagon*** islands){
    if(islands == NULL){
        printf("KO\n");
        return;
    }
    if(startRow < 0 || destRow < 0 || startRow >= n2 || destRow >= n2 ||
       startCol < 0 || destCol < 0 || startCol >= n1 || destCol >= n1) {
        printf("KO\n");
        return;
    }

    Hexagon* startHex = islands[startCol][startRow];

    uint32_t start_id = (uint32_t)(startCol) * (uint32_t)(n2) + (uint32_t)(startRow);
    uint32_t dest_id = (uint32_t)(destCol) * (uint32_t)(n2) + (uint32_t)(destRow);

    int average = 0;
    int count = 0;

    for (int i = 0; i < air_num[start_id]; i++) {
        if(air_dest[start_id][i] == dest_id){
            air_num[start_id]--;
            air_dest[start_id][i] = air_dest[start_id][air_num[start_id]];
            air_cost[start_id][i] = air_cost[start_id][air_num[start_id]];
            printf("OK\n");
            return;
        }
        average += air_cost[start_id][i];
        count++;
    }

    if (air_num[start_id]==5) {
        printf("KO\n");
        return;
    }

    average += startHex->cost;
    count++;
    air_dest[start_id][air_num[start_id]] = dest_id;
    air_cost[start_id][air_num[start_id]] = (uint8_t)(average/count);
    air_num[start_id]++;
    

    printf("OK\n");
    return;
}

void travelCost(int startCol, int startRow, int destCol, int destRow, int n1, int n2 ,Hexagon*** islands, travelCostResult** results, size_t* resultsCount, size_t* resultsCap){
    if(islands == NULL){
        printf("-1\n");
        return;
    }
    if(startRow < 0 || destRow < 0 || startRow >= n2 || destRow >= n2 ||
       startCol < 0 || destCol < 0 || startCol >= n1 || destCol >= n1) {
        printf("-1\n");
        return;
    }

    if(startCol==destCol && startRow==destRow){
        printf("0\n");
        return;
    }

    if(results == NULL){
        results = malloc(100 * sizeof(travelCostResult));
    }

    uint32_t s_id = (uint32_t)(startCol) * (uint32_t)(n2) + (uint32_t)(startRow);
    uint32_t d_id = (uint32_t)(destCol) * (uint32_t)(n2) + (uint32_t)(destRow);

    if(results && resultsCount && *resultsCount>0 && *results){
        travelCostResult* array = *results;
        for(int i = 0; i < *resultsCount; i++){
            if(array[i].start == s_id && array[i].dest == d_id){
                printf("%d\n", array[i].cost);
                return;
            }
        }
    }

    int distance[n1][n2];
    for (int i = 0; i < n1; i++){
        for (int j = 0; j < n2; j++){
            distance[i][j] = 1000000;
        }
    }

    PriorityQueue queue;
    queue.size = 0;

    distance[startCol][startRow] = 0;
    queue_push(&queue,islands[startCol][startRow],0);

    while (queue.size!=0)
    {
        PriorityQueueNode current = queue_pop(&queue);
        Hexagon* hex = current.hex;
        

        int cost = current.cost;
        int col = hex->column;
        int row = hex->row;

        if(destCol == col && destRow == row){

            if(results && resultsCount && resultsCap){
                if(*resultsCount == *resultsCap){
                    size_t newCap = (*resultsCap ? *resultsCap *2 : 128);
                    travelCostResult* temp = realloc(*results, newCap * sizeof(*temp));
                    if(!temp){
                        printf("%d\n",cost);
                        return;
                    }
                    *results = temp;
                    *resultsCap = newCap;
                }
                (*results)[*resultsCount] = (travelCostResult) {cost,s_id,d_id};
                (*resultsCount)++;
            }

            printf("%d\n",cost);
            return;
        }

        if(cost>distance[col][row]) continue;

        if(hex->cost>0){
            
            for(int dir=0; dir<6; dir++){
                int next_col = col + ((row % 2 == 0) ? even_dx[dir] : odd_dx[dir]);
                int next_row = row + ((row % 2 == 0) ? even_dy[dir] : odd_dy[dir]);
                if((unsigned)(next_col) < (unsigned)(n1) && (unsigned)(next_row) < (unsigned)(n2)){
                    int nextCost = cost + hex->cost;
                    if(distance[next_col][next_row]>nextCost){
                        distance[next_col][next_row] = nextCost;
                        queue_push(&queue, islands[next_col][next_row], nextCost);
                    }
                }
            }

            uint32_t id = (uint32_t)(col) * (uint32_t)(n2) + (uint32_t)(row);
            for(int i = 0; i<air_num[id];i++){
                if (air_cost[id][i] == 0) continue;
                uint32_t dest_id = air_dest[id][i];
                int next_col = (int)(dest_id / (uint32_t)n2);
                int next_row = (int)(dest_id % (uint32_t)n2);
                int airCost = cost + (int)air_cost[id][i];
                if(distance[next_col][next_row]>airCost){
                    distance[next_col][next_row] = airCost;
                    queue_push(&queue, islands[next_col][next_row], airCost);
                }
            }
            
        }
    }
    if(*resultsCount == *resultsCap){
                    size_t newCap = (*resultsCap ? *resultsCap *2 : 128);
                    travelCostResult* temp = realloc(*results, newCap * sizeof(*temp));
                    *results = temp;
                    *resultsCap = newCap;
                }
                (*results)[*resultsCount] = (travelCostResult) {-1,s_id,d_id};
                (*resultsCount)++;
    printf("-1\n");
    return;
}

void queue_push(PriorityQueue* queue, Hexagon* hex, int cost){
    if (queue->size >= MAX_QUEUE){
        return;
    }

    int i = queue->size;
    queue->queue[i].hex = hex;
    queue->queue[i].cost = cost;
    queue->size = i+1;

    while (i > 0) {
        int parent = (i - 1) / 2;
        if (queue->queue[parent].cost <= queue->queue[i].cost) break;
        PriorityQueueNode temp = queue->queue[parent];
        queue->queue[parent] = queue->queue[i];
        queue->queue[i] = temp;
        i = parent;
    }
}   

PriorityQueueNode queue_pop(PriorityQueue* queue){

    PriorityQueueNode min = queue->queue[0];
    queue->size = queue->size-1;
    queue->queue[0] = queue->queue[queue->size];

    int i = 0;
    while(1){
        int left = 2*i + 1;
        int right = 2*i + 2;
        int smallest = i;

        if(left<queue->size && queue->queue[left].cost<=queue->queue[smallest].cost){
            smallest = left;
        }
        if(right<queue->size && queue->queue[right].cost<=queue->queue[smallest].cost){
            smallest = right;
        }
        if(smallest == i) break;

        PriorityQueueNode temp = queue->queue[smallest];
        queue->queue[smallest] = queue->queue[i];
        queue->queue[i] = temp;
        i = smallest;
    }

    return min;
}

void free_map(int n1, int n2, Hexagon*** islands){
    if(!islands){
        return;
    }
    for(int col=0;col<n1;col++){
        for(int row=0;row<n2;row++){
            free(islands[col][row]);
        }
        free(islands[col]);
    }
    free(islands);
}

void emptyResults(travelCostResult** results, size_t* resultsCount, size_t* resultsCap){
    if(!results){
        return;
    }
    free(*results);
    *results = NULL;
    if(resultsCount){
        *resultsCount = 0;
    }
    if(resultsCap){
        *resultsCap = 0;
    }
}