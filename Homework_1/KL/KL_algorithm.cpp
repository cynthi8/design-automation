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
#include <numeric>
#include <execution>
//#include <climits>
//#include <cstring> //c strings should be replaced with std::string

#define ARRAY_LEN(array) (sizeof((array))/sizeof((array)[0]))
#include <limits>

using namespace std;

int LoadFile(string szFilename, int iLength);
int SetupNetlist();
int count2(vector<int>::iterator start, vector<int>::iterator end, int x);
int recalcDv(int x, int y, bool wholelist);
int BestSwap(int iteration);
int KLAlgorithm();
int CleanUp();
int timePassedMS(chrono::steady_clock::time_point start);
string UserInput();

vector <int> A;                 //column 1 of textfile
vector <int> B;                 //colume 2 of textfile
vector <int> vD;                //difference of external to internal wires for each unit cell
vector <int> tempCheck;         //psuedo c matrix from class

vector <bool> vGroup;           //what group each unit cell is associated with, A=1, B=0
vector <bool> vGroupTempSwap;   //what the new temp grouping is
vector <bool> vLocked;          //lock the unit cell from switching if its switched before
vector <bool> vLockedUnsorted;  //Based on sortedvD, which unsorts it

vector <vector<int>> vList;     //a list of what cells a cell is connected to
vector <vector<int>> vQueue;    //queue of cells to switch

vector<int> values;
vector<vector<int>> sortedvD;

int iCellSize = 0;              //number of cells
int iNetSize = 0;               //number of total connections between each cell

// load the values from the file
int LoadFile(string szFilename, int iLength) {
    if (iLength <= 0) return -1;

    ifstream myfile;
    string szline;
    stringstream stream;

    auto start = chrono::steady_clock::now();

    //const unsigned int N = 128;
    //char Buffer[N];
    //myfile.rdbuf()->pubsetbuf(Buffer, N);
    myfile.open(szFilename.c_str());

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

        cout << "\nCellsize is : " << iCellSize;
        cout << "\nNetsize is : " << iNetSize;

        //resize the column vectors to fit all the numbers in a column
        if (iCellSize > 0 && iNetSize > 0) {
            A.resize(iNetSize);
            B.resize(iNetSize);
        }
        else return -1;

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

    cout << "\n\nLoading : " << timePassedMS(start) << " ms";

    return iLength;
}

// I suppose try and setup a sparse matrix with the netlist
int SetupNetlist() {
    vGroupTempSwap.resize(iCellSize);
    vGroup.resize(iCellSize);
    vList.resize(iCellSize);
    vD.resize(iCellSize);

    auto start = chrono::steady_clock::now();

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
    recalcDv(0, 0, true);

    cout << "\nSetting Up : " << timePassedMS(start) << " ms";

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


//which is how many external connects - internal connections
int recalcDv(int x, int y, bool wholelist) {

    if (wholelist) { //Manually create the first D list
        for (int i = 0; i < iCellSize; i++) {
            int external = 0;
            for (unsigned int j = 0; j < vList[i].size(); j++) {
                if (vGroupTempSwap[vList[i][j]] ^ vGroupTempSwap[i])
                    external++;
            }
            vD[i] = external * 2 - vList[i].size(); //ex - ((total - ex)=internal)
        }
    } 

    else {
        for (unsigned int i = 0; i < vList[x].size(); i++) {
            int num = vList[x][i];
            int xa = (vGroupTempSwap[num] == vGroupTempSwap[x]) ? -1 : 1;
            vD[num] = vD[num] + 2 * xa;
        }

        for (unsigned int i = 0; i < vList[y].size(); i++) {
            int num = vList[y][i];
            int yb = (vGroupTempSwap[num] == vGroupTempSwap[y]) ? -1 : 1;
            vD[num] = vD[num] + 2 * yb;
        }
        //these might actually be unnecessary since they become locked anyways
        vD[x] = -vD[x] + count2(vList[x].begin(), vList[x].end(), y); 
        vD[y] = -vD[y] + count2(vList[y].begin(), vList[y].end(), x);
    }

    return 1;
}

// function to sort a 2D vector with column 0
bool sortcol(const vector<int>& v1,
    const vector<int>& v2) {
    return v1[0] > v2[0];
}



// hard end stuff I suppose
int BestSwap(int iteration) {
    int iGainTempMax = numeric_limits<int>::min();
    int iBestX = -1, iBestY = -1;
    int itestX = -1, itestY = -1;
    int i = 0;
    //auto firstzero = find(std::execution::par_unseq, test.begin(), test.end(), 0);
    //int index = firstzero - test.begin();
    static int indexi = 0;

    for (int ia = indexi; ia < iCellSize; ia++) {
        if (vLocked[sortedvD[ia][1]]) //if the cell is locked, then skip
            continue; //This if statement might be able to be removed now

        i = sortedvD[ia][1];

        fill(tempCheck.begin(), tempCheck.end(), 0);
        for (unsigned int j = 0; j < vList[i].size(); j++) {
            tempCheck[vList[i][j]]++;
        }

        // go thru all cells except for the cells already checked from i
        for (int jb = ia + 1; jb < iCellSize; jb++) {
            int j = sortedvD[jb][1];

            // check if the cell isn't locked and is in a different group than the first cell
            if (!vLocked[j] && vGroupTempSwap[j] ^ vGroupTempSwap[i]) {
                // if the gain here is less than the max, then there's no point in checking any more
                if (vD[i] + vD[j] < iGainTempMax)
                    goto Foundbestpair;

                int iGainTemp = vD[i] + vD[j] - 2 * tempCheck[j];

                if (iGainTemp > iGainTempMax) {
                    iGainTempMax = iGainTemp;
                    iBestX = i; iBestY = j;
                    itestX = ia; itestY = jb;
                }
            }
        }
    }

    Foundbestpair:
    //if the index locations are good, swap the elements, and store it in the queue
    if (iBestX >= 0 && iBestY >= 0) {
        vQueue[iteration] = { iBestX, iBestY };
        vLocked[iBestX] = true;
        vLocked[iBestY] = true;
        vLockedUnsorted[itestX] = true;
        vLockedUnsorted[itestY] = true;

        vGroupTempSwap[iBestX] = !vGroupTempSwap[iBestX];
        vGroupTempSwap[iBestY] = !vGroupTempSwap[iBestY];
        recalcDv(iBestX, iBestY, false);

        while (indexi < iCellSize && vLockedUnsorted[indexi]) {
            indexi++;
        }
        if (indexi == iCellSize) indexi = 0;
    }

    return iGainTempMax;
}

// This is the KL implementation of what was shown in class
int KLAlgorithm(){
    const int maxiterations = 1000;
    int iterations = 0;

    long long gmax = 0;
    int lastbesti = -1;
    
    vQueue.resize(iCellSize / 2, vector<int>(2));
    vLocked.resize(iCellSize);
    tempCheck.resize(iCellSize);
    vGroupTempSwap = vGroup;

    values.resize(iCellSize);
    iota(values.begin(), values.end(), 0);
    sortedvD.resize(iCellSize, vector<int>(2));
    
    vLockedUnsorted.resize(iCellSize);

    auto start = chrono::steady_clock::now();

    do {
        long long g = 0;
        iterations++;
        lastbesti = -1;
        gmax = 0;

        fill(vLocked.begin(), vLocked.end(), false);
        fill(vLockedUnsorted.begin(), vLockedUnsorted.end(), false);

        for (int i = 0; i < iCellSize; i++) {
            sortedvD[i] = { vD[i] , values[i] };
        }

        //how the heck do I sort a 2d vector based on a row instead of a column
        sort(std::execution::par_unseq, sortedvD.begin(), sortedvD.end(), sortcol);

        // This function is the one that chooses the best pair and calculates their gain 
        // and keeps track of the max of the sums of the gain
        // dividing by 4 instead of 2 gives a speed boost but maybe only for small cellsizes < 100k
        for (int i = 0; i < iCellSize / 2; i++) {
            g += BestSwap(i);
            if (g > gmax) { 
                gmax = g;
                lastbesti = i;
            }
        }
        
        cout << "\nIteration Number : " << iterations;
        cout << "\tBest Gain : " << gmax;

        // do the swapping
        for (int i = 0; i <= lastbesti && gmax > 0; i++) {
            int Anum = vQueue[i][0];
            int Bnum = vQueue[i][1];
            vGroup[Anum] = !vGroup[Anum];
            vGroup[Bnum] = !vGroup[Bnum];
        }

        // recalculate the entire Dv list;
        vGroupTempSwap = vGroup;
        recalcDv(0, 0, true);

        //stop if there is no good i, or there is no positive gain, or max iterations went too high as a failsafe
    } while (lastbesti >= 0 && gmax > 0 && iterations < maxiterations);

    cout << "\nCalc KL : " << timePassedMS(start) << " ms";

    // calculate how many external wires are being cut
    int external = 0;
    for (int i = 0; i < iCellSize; i++) {
        for (unsigned int j = 0; j < vList[i].size(); j++)
            if (vGroup[vList[i][j]] ^ vGroup[i])
                external++;
    }

    cout << "\n\nCutset size is : " << external / 2; //divide external wires by 2 since they are counted twice
    
    cout << "\nDo you want to see all of A and B\nType 1 Here for yes : "; 

    int iReturnVal; cin >> iReturnVal;
    if (iReturnVal == 1) {
        cout << "\nGroup A is : ";
        for (int i = 0; i < iCellSize; i++)
            if (vGroup[i] == 0)
                cout << i + 1 << " ";

        cout << "\nGroup B is : ";
        for (int i = 0; i < iCellSize; i++)
            if (vGroup[i] == 1)
                cout << i + 1 << " ";
    }

    return 1;
}

// premade file names, assumes there is a folder with the name dev_net in the same location as the .exe
// Should this be a vector of strings? Best to stay with C++ data types than revert to arrays of char*
// It should be a vector of strings, but I was just used to using arrays of chars
const vector<string> szfilename = {
"development_netlists/bench_2.net",
"development_netlists/bench_4.net",
"development_netlists/bench_6.net",
"development_netlists/bench_11.net",
"development_netlists/bench_12.net",
"development_netlists/bench_16.net",
};

const vector<string> szfilename2 = {
"Benchmarks/b_100_500",
"Benchmarks/b_500_20000",
"Benchmarks/b_1000_20000",
"Benchmarks/b_10000_100000",
"Benchmarks/b_50000_400000",
"Benchmarks/b_100000_500000",
"Benchmarks/b_100000_2000000",
"Benchmarks/b_200000_2000000",
"Benchmarks/b_250000_1000000",
"Benchmarks/b_500000_3000000",
};


// ask the user if they want to choose which netlist to use
// updated to use strings
string UserInput() {
    unsigned int i = 0;
    int iReturnVal = 0;
    vector<string> szusestring;

    cout << "\nType 1 to choose from premade benches: ";
    cout << "\nType 2 to choose from HW Benchmarks: ";

    cout << "\nType here: ";
    cin >> iReturnVal;

    switch (iReturnVal) {
        case 1:
            szusestring = szfilename;
            break;
        case 2:
            szusestring = szfilename2;
            break;
        default:
            return szfilename2[0];
    }

    for (unsigned int j = 0; j < szusestring.size(); j++) {
        cout << "\nType " << j << " to use " << szusestring[j];
    }

    cout << "\nType here: ";
    cin >> i;

    if (i < 0 || i > szusestring.size()) i = 0;

    return szusestring[i];
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

    A.clear();
    B.clear();
    A.shrink_to_fit();
    B.shrink_to_fit();

    values.clear();
    sortedvD.clear();
    values.shrink_to_fit();
    sortedvD.shrink_to_fit();

    tempCheck.clear();
    vLockedUnsorted.clear();
    tempCheck.shrink_to_fit();
    vLockedUnsorted.shrink_to_fit();

    return 1;
}

int timePassedMS(chrono::steady_clock::time_point start) {
    auto end = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

// entry point for code
int main(int argc, char* argv[]) {
    int iReturnVal = 1;
    int iteration = 0;

    vector<string> vargv(argv, argv + argc);
    string szfilenameUse;
    //ios::sync_with_stdio(false); //This may speed up reading file functions

    do {
        if(argc == 1){
            szfilenameUse = UserInput();
        }
        else if (iteration++ < argc) {
           //Automatically go thru all agruments passed into main
           szfilenameUse = vargv[iteration];
        }
        else {
            break;
        }

        int iFileLength = szfilenameUse.length();

        cout << "\n\nUsing bench: " << szfilenameUse;

        //First step is to load the file
        if (LoadFile(szfilenameUse, iFileLength) < 0) {         
            printf("\nError: File is not able to be opened");
            goto Error;
        }

        //Then setup the netlist
        if (SetupNetlist() < 0) goto Error;

        //Then implement the KL algorithm
        if (KLAlgorithm() < 0) goto Error;

        //go ahead and clean up the memory
        CleanUp(); 
        
        if (argc == 1) {
            cout << "\nWould you like to repeat? Type 0 for no: ";
            cin >> iReturnVal;
        }

        //cout << "\f"; // want to clear terminal maybe
    } while (iReturnVal > 0 && iteration <= argc);

    Error:
    CleanUp();
    return 0;
}


