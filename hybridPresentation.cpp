#include <iostream>
#include <set>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <chrono>
#include <sstream>

using namespace std;

struct Vertex{
public:
    int steps;
    int cost;
    unsigned long long state;
    Vertex* parent;

    bool operator<(const Vertex& other) const {
        return cost < other.cost;
    }
};

struct PQSorter
{
    bool operator() (Vertex *lhs, Vertex * rhs)
    {
        return lhs->cost > rhs -> cost;
    }
};

//void setCost(Vertex* current){
//    int manhattan=0;
//    unsigned long long tile = current->state;
//    for(int i=0; i<16; i++){
//
//        unsigned long long tileTemp=(tile>>(i*4))&0xF;
//        int tempTile = tileTemp-1;
//        if(tileTemp!=0){
//            manhattan+=abs(i/4 - tempTile/4)+abs(i%4 - tempTile%4);
//        }
//    }
//    int cost = manhattan+current->steps;
//    current->cost=cost;
//}

int goalV[16][2]={{0,15},{1,0}, {2,4}, {3, 8},
                  {4, 12}, {5,1},{6,5}, {7, 9},
                  {8,13},{9,2}, {10,6},{11,10},
                  {12,14},{13, 3},{14, 7}, {15, 11}};
void setCost(Vertex* current){
    unsigned long long state = current->state;
    unsigned long long digitH=0;
    unsigned long long digitV=0;
    int tileBefore=0;
    unsigned long long digitHFurther=0;
    unsigned long long digitVFurther=0;
    int ver = 0;
    int hor = 0;
    for(int i=0; i<14; i++){
        digitH = (state>>(i*4))&0xFull;
        tileBefore=(state>>((i*4)%15)*4)&0xFull;
        //digitV=goalV[tileBefore];
        digitV= goalV[tileBefore][1];
        for(int j=i+1; j<15; j++){
            digitHFurther = (state>>(j*4))&0xFull;
            int tileAfter = (state>>((j*4)%15)*4)&0xFull;
            digitVFurther=goalV[tileAfter][1];
            if(digitH>digitHFurther&&digitH!=0&&digitHFurther!=0){
                hor++;
            }
            if(digitV>digitVFurther&&tileBefore!=0&&tileAfter!=0) {
                ver++;
            }
        }
    }

    int manhattan=0;
    unsigned long long tile = current->state;
    for(int i=0; i<16; i++){

        unsigned long long tileTemp=(tile>>(i*4))&0xF;
        int tempTile = tileTemp-1;
        if(tileTemp!=0){
            manhattan+=abs(i/4 - tempTile/4)+abs(i%4 - tempTile%4);
        }
    }

    int cost = current->steps + max(ver%3+ver/3+hor%3+hor/3, manhattan);
    current->cost=cost;
}

void createNeighbour(Vertex* vertex, int index, int newIndex, set<unsigned long long>* seen,
                     priority_queue<Vertex*, vector<Vertex*>, PQSorter>* queue){
    if(index<newIndex){
        int bufor = newIndex;
        newIndex = index;
        index = bufor;
    }
    unsigned long long newState = vertex->state;

    unsigned long long digit1 = (newState >> (4 * index)) & 0xFull;
    unsigned long long digit2 = (newState >> (4 * newIndex)) & 0xFull;

    unsigned long long filter = digit1<<(index-newIndex)*4;
    unsigned long long mask = digit2<<(index-newIndex)*4;

    filter=filter|digit2;
    mask = mask|digit1;

    filter=filter<<(newIndex)*4;
    mask=mask<<(newIndex)*4;
    newState = newState ^ filter;
    newState = newState | mask;
    if(seen->find(newState) != seen->end()){
        return;
    }
    else{
        Vertex* neighbour = new Vertex;
        neighbour->state = newState;
        neighbour->steps= vertex->steps+1;
        setCost(neighbour);
        neighbour->parent = vertex;
        queue->push(neighbour);
    }
}


void win(Vertex* vertex){
    unsigned long long states[vertex->steps+1];
    int steps = vertex->steps;
    for(int i = 0; i<=steps; i++){
        states[steps-i]= vertex->state;
        vertex=vertex->parent;
    }

    for(int i=0; i<=steps;i++){
        string show="\n";
        for(int j=0; j<16; j++){
            unsigned long long tile = (states[i] >> (j * 4)) & 0xFull;
            int tdigit=tile;
            if(tdigit == 0){
                show+="\033[31m00\033[0m ";
            }
            else if(tdigit < 10){
                string part = to_string(tdigit);
                show+="0"+part+" ";
            }
            else{
                show+= to_string(tdigit) + " ";
            }
            if(j%4==3){
                show+="\n";
            }
        }
        cout<<show<<endl;
    }

    cout<<"Rozwiązanie w "<<steps<<" ruchów";
}

bool astar(Vertex* start){
    priority_queue<Vertex*, vector<Vertex*>, PQSorter> queue;
    set<unsigned long long> seen;
    queue.push(start);
    start->parent=NULL;
    start->steps=0;
    setCost(start);
    cout<<start->cost<<endl;

    while(!queue.empty()){
        Vertex* vertex = queue.top();
        queue.pop();
        seen.insert(vertex->state);
        if(seen.size()%100000==0){
            printf("%d, cost: %d, steps: %d\n", seen.size(), vertex->cost, vertex->steps);
        }
        if(vertex->state == 0x0FEDCBA987654321){
            win(vertex);
            return true;
        }else{
            unsigned long long temp = vertex->state;
            int posZero=0;
            for(int i = 0; i<16; i++){
                if(((temp>>(4*i))&0xfull)==0){
                    posZero=i;
                    break;
                }
            }
            if(posZero%4>0){
                createNeighbour(vertex, posZero, posZero-1, &seen, &queue);
            }
            if(posZero%4<3){
                createNeighbour(vertex, posZero, posZero+1, &seen, &queue);
            }
            if(posZero/4>0){
                createNeighbour(vertex, posZero, posZero-4, &seen, &queue);
            }
            if(posZero/4<3){
                createNeighbour(vertex, posZero, posZero+4, &seen, &queue);
            }
        }
    }
    return false;
}

unsigned long long toSolvable(int puzzle[16]){
    //sprawdzam, czy stan gry ma sens
    for(int i =0 ;i<15;i++){
        for(int j=i+1;j<16; j++){
            if(puzzle[i]==puzzle[j]){
                return 0;
            }
        }
    }

    //biorę pozycję X zera licząc od jego pozycji końcowej.
    int row;
    for(int i=0; i<16;i++){
        if(puzzle[i]==0){
            row=((15-i)/4);
            break;
        }
    }
    int invCount=0;
    for(int i = 0; i<15;i++){
        if(puzzle[i]!=0){
            for(int j = i+1; j<16; j++){
                if(puzzle[j]!=0&&puzzle[i]>puzzle[j]){
                    invCount++;
                }
            }
        }
    }
    if(row%2==invCount%2){
        unsigned long long state =0;
        for(int i=15; i>=0; i--){
            state=state|puzzle[i];
            if(i!=0){
                state = state<<4;
            }
        }
        return state;
    }else{
        return 0;
    }
}

int main() {
    string board="";
    int tiles[16];
    for(int i=0; i<4; i++){
        for(int j=0;j<4; j++){
            string tile;
            cin>>tile;
            int number = stoi(tile);

            if(number>15||number<0){
                cout<<"Nie należy dawać liczb większych od 15.";
                return -1;
            }
            tiles[i*4+j]=number;
        }
    }
    unsigned long long state = toSolvable(tiles);
    if(state == 0){
        cout<<"Dana permutacja nie jest rozwiązywalna";
        return -1;
    }

    Vertex* start = new Vertex;
    start->state = state;

    astar(start);
    return 0;
}

