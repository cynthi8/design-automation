// HW1.cpp : KL Function
// VLSI Design: HW1
// Date: 2021-02-04
// Created by: Nathaniel Hernandez
// Group: Nathaniel Hernandez; Erin Cold

// Sort of want to use multithreading, who knows if that is a good idea

#include <algorithm> 
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <random>
#include <climits>
#include <cstring> //c strings should be replaced with std::string


#define ARRAY_LEN(array) (sizeof((array))/sizeof((array)[0]))

using namespace std;

int LoadFile(const char* szFilename, int iLength);
int SetupNetlist();
int count2(vector<int>::iterator start, vector<int>::iterator end, int x);
int recalcDv();
int BestSwap(int iteration);
int KLAlgorithm();
int UserInput();
int CleanUp();

vector <int> A;                 //column 1 of textfile
vector <int> B;                 //colume 2 of textfile
vector <int> vD;                //difference of external to internal wires for each unit cell

vector <bool> vGroup;           //what group each unit cell is associated with, A=1, B=0
vector <bool> vGroupTempSwap;   //what the new temp grouping is
vector <bool> vLocked;          //lock the unit cell from switching if its switched before

vector <vector<int>> vList;     //a list of what cells a cell is connected to
vector <vector<int>> vQueue;    //queue of cells to switch

int iCellSize = 0;              //number of cells
int iNetSize = 0;               //number of total connections between each cell

// load the values from the file
int LoadFile(const char* szFilename, int iLength) {
    if (iLength <= 0) return -1;

    ifstream myfile;
    string szline;
    stringstream stream;

    //const unsigned int N = 128;
    //char Buffer[N];
    //myfile.rdbuf()->pubsetbuf(Buffer, N);
    myfile.open(szFilename);

    //check if the file opened correctly
    if (myfile.is_open()) {

        //update the cellsize and netsize
        if (getline(myfile, szline)) {
            if (szline.empty()) return -1;
            stream.clear();
            stream << szline;
            stream >> iCellSize;
            //iCellSize = stoi(szline, nullptr, 0);
        }

        if (getline(myfile, szline)) {
            if (szline.empty()) return -1;
            stream.clear();
            stream << szline;
            stream >> iNetSize;
            //iNetSize = stoi(szline, nullptr, 0);
        }

        printf("cellsize is: %d\n", iCellSize);
        printf("netsize is: %d\n", iNetSize);

        //resize the column vectors to fit all the numbers in a column
        if (iCellSize > 0 && iNetSize > 0) {
            A.resize(iNetSize);
            B.resize(iNetSize);
        }
        else {
            return -1;
        }

        //add each number from the file to the corresponding column vector
        //lol for some reason this is taking so much time, but maybe a debug only problem?
        int j;
        for (j = 0; j < iNetSize && getline(myfile, szline, '\n'); j++) {
            int a = 0, b = 0;
            if (szline.empty()) break;

            stream.clear();
            stream << szline;
            stream >> a >> b;

            A[j] = a; B[j] = b;
        }

        if (j != iNetSize) { //heckin someone lied to me
            iNetSize = j;
            A.resize(iNetSize);
            A.shrink_to_fit();
            B.resize(iNetSize);
            B.shrink_to_fit();
        }

        myfile.close();
    }
    else {
        return -1;
    }

    return iLength;
}

// I suppose try and setup a sparse matrix with the netlist
int SetupNetlist() {
    vGroupTempSwap.resize(iCellSize);
    vGroup.resize(iCellSize);
    vList.resize(iCellSize);
    vD.resize(iCellSize);

    //Randomly assign a cell to either group A or B
    for (int i = 0; i < iCellSize; i+=2) {
        vGroup[i] = 1; // Group A = 1, Group B = 0
    }
    unsigned int seed = (unsigned int) std::chrono::system_clock::now().time_since_epoch().count();
    shuffle(vGroup.begin(), vGroup.end(), default_random_engine(seed)); //now shuffle the vector

    //Add the elements a unit cell is connected to, to the vList 2D vector
    //for example cell 1 is connected to 2, 3, 3, 4
    //vList[0] = {1, 2, 2, 3}

    for (int i = 0; i < iNetSize; i++) {
        vList[A[i]-1].push_back(B[i]-1);
        vList[B[i]-1].push_back(A[i]-1);
    }

    A.clear();
    B.clear();
    A.shrink_to_fit();
    B.shrink_to_fit();

    //Go ahead and sort the list, which may be useful later
    for (int i = 0; i < iCellSize; i++) {
        sort(vList[i].begin(), vList[i].end());
    }

    //Manually create the first D list
    //which is how many external connects - internal connections
    vGroupTempSwap = vGroup;
    recalcDv();
    return 1;
}

// This is a modified binary search algorithm for vectors
int count2(vector<int>::iterator start, vector<int>::iterator end, int x)
{
    vector<int>::iterator low;

    low = lower_bound(start, end, x);

    if (low == end || *low != x) return 0; // If element is not present, return 0 

    return upper_bound(low, end, x) - low;
}

//Manually create the first D list
//which is how many external connects - internal connections
int recalcDv() {
    // recalculate the entire Dv list; would love to make this faster but lets go with this for now
    for (int i = 0; i < iCellSize; i++) {
        int external = 0;
        for (unsigned int j = 0; j < vList[i].size(); j++)
            if (vGroupTempSwap[vList[i][j]] ^ vGroupTempSwap[i])
                external++;

        vD[i] = external * 2 - vList[i].size(); //ex - ((total - ex)=internal)
    }
    return 1;
}

// hard end stuff I suppose
int BestSwap(int iteration) {
    int iGainTempMax = INT_MIN;
    int iBestX = -1;
    int iBestY = -1;

    // I think this could possibly be multithreaded
    // go thru all cells
    for (int i = 0; i < iCellSize; i++) {

        if (vLocked[i]) //if the cell is locked, then skip
            continue;

        // go thru all cells except for the cells already checked from i
        for (int j = i+1; j < iCellSize; j++) {

            //check if the cell isn't locked and is in a different group than the first cell
            if (!vLocked[j] && vGroupTempSwap[j] ^ vGroupTempSwap[i]){
                int dupes = 0;
                int iGainTemp = 0;

                // go thru all the cells this current cell is connected to and see if j is included anywhere in there
                // even after modifying count, this still takes like 95% of the cpu time, maybe it could be precomputed somehow
                dupes = count2(vList[i].begin(), vList[i].end(), j);

                iGainTemp = vD[i] + vD[j] - 2 * dupes;
                if (iGainTemp > iGainTempMax) {
                    iGainTempMax = iGainTemp;
                    iBestX = i;
                    iBestY = j;
                }
            }
        }
    }

    //if the index locations are good, swap the elements, and store it in the queue
    if (iBestX >= 0 && iBestY >= 0) {
        vQueue[iteration] = { iBestX, iBestY };
        vLocked[iBestX] = true;
        vLocked[iBestY] = true;
        vGroupTempSwap[iBestX] = !vGroupTempSwap[iBestX];
        vGroupTempSwap[iBestY] = !vGroupTempSwap[iBestY];
        recalcDv();
    }

    return iGainTempMax;
}

// This is the KL implementation of what was shown in class
int KLAlgorithm(){
    int gmax = 0;
    int lastbesti = -1;
    int maxiterations = 0;
    vQueue.resize(iCellSize / 2, vector<int>(2));
    vLocked.resize(iCellSize);

    do {
        int g = 0;
        maxiterations++;
        lastbesti = -1;
        gmax = 0;

        fill(vLocked.begin(), vLocked.end(), false);
        vGroupTempSwap = vGroup;

        // This function is the one that chooses the best pair and calculates their gain 
        // and keeps track of the max of the sums of the gain
        for (int i = 0; i < iCellSize / 2; i++) {
            g += BestSwap(i);

            if (g > gmax) { //TODO think the issue is here //I think gmax has to start as 0
                gmax = g;
                lastbesti = i;
            }
        }

        // do the swapping
        for (int i = 0; i <= lastbesti && gmax > 0; i++) {
            int Anum = vQueue[i][0];
            int Bnum = vQueue[i][1];
            vGroup[Anum] = !vGroup[Anum];
            vGroup[Bnum] = !vGroup[Bnum];
        }

        // recalculate the entire Dv list; would love to make this faster but lets go with this for now
        recalcDv();

        //stop if there is no good i, or there is no positive gain, or max iterations went too high as a failsafe
    } while (lastbesti >= 0 && gmax > 0 && maxiterations < 20);

    // calculate how many external wires are being cut
    int external = 0;
    for (int i = 0; i < iCellSize; i++) {
        for (unsigned int j = 0; j < vList[i].size(); j++)
            if (vGroup[vList[i][j]] ^ vGroup[i])
                external++;
    }
    printf("\nCutset size is: %d", external/2); //divide external wires by 2 since they are counted twice

    printf("\nGroup A is: ");
    for (int i = 0; i < iCellSize; i++) {
        if (vGroup[i] == 0) {
            printf("%d ", i+1);
        }
    }
    
    printf("\nGroup B is: ");
    for (int i = 0; i < iCellSize; i++) {
        if (vGroup[i] == 1) {
            printf("%d ", i+1);
        }
    }

    return 1;
}

// premade file names, assumes there is a folder with the name dev_net in the same location as the .exe
// Should this be a vector of strings? Best to stay with C++ data types than revert to arrays of char*
const char* szfilename[] = {"development_netlists/bench_2.net",
                            "development_netlists/bench_4.net",
                            "development_netlists/bench_6.net",
                            "development_netlists/bench_11.net",
                            "development_netlists/bench_12.net",
                            "development_netlists/bench_16.net",
                            "development_netlists/bench_test.net", 
                            "development_netlists/bench_test2.net", 
                            "development_netlists/bench_test3large.net", };

// ask the user if they want to choose which netlist to use
int UserInput() {
    const int ArrayLength = ARRAY_LEN(szfilename);
    int i = 0;
    int iReturnVal = 0;

    cout << "\nType 1 to choose from premade benches: ";
    cin >> iReturnVal;

    if (iReturnVal == 1) {
        for (int j = 0; j < ArrayLength; j++) {
            printf("Type %d to use %s\n", j, szfilename[j]);
        }
        printf("Type here: ");
        cin >> i;
    }

    return i;
}

// clean up memory
int CleanUp() {
    vD.clear();
    vGroup.clear();
    vGroupTempSwap.clear();
    vLocked.clear();

    vList.clear();
    vQueue.clear();

    vD.shrink_to_fit();
    vGroup.shrink_to_fit();

    vGroupTempSwap.shrink_to_fit();
    vLocked.shrink_to_fit();
    vList.shrink_to_fit();
    vQueue.shrink_to_fit();

    return 1;
}

// entry point for code
int main() {
    const int ArrayLength = ARRAY_LEN(szfilename);
    int iReturnVal = 1;

    ios::sync_with_stdio(false); //This may speed up reading file functions

    while (iReturnVal > 0) {
        int i = UserInput();                                //Get the user input

        if (i < 0 || i > ArrayLength) i = 1;
        int iFileLength = strlen(szfilename[i]);

        cout << "Using bench: " << szfilename[i] << '\n';

        auto start = chrono::steady_clock::now();

        iReturnVal = LoadFile(szfilename[i], iFileLength);  //First step is to load the file
        if (iReturnVal < 0) return -1;

        iReturnVal = SetupNetlist();                        //Then setup the netlist
        if (iReturnVal < 0) return -1;

        iReturnVal = KLAlgorithm();                         //Then implement the KL algorithm
        if (iReturnVal < 0) return -1;

        printf("\nEnd of Program\n");
        fflush(stdout); //printf might not work right on linux :p

        auto end = chrono::steady_clock::now();
        cout << "Elapsed time in milliseconds : "
            << chrono::duration_cast<chrono::milliseconds>(end - start).count()
            << " ms" << '\n';

        CleanUp(); //terminal uses memory, dunno how to clear

        printf("\nWould you like to repeat? Type 0 for no: ");
        cin >> iReturnVal;
    }

    CleanUp();
    return 0;
}


