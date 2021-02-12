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
#include <limits>

using namespace std;

int LoadFile(string szFilename);
int SaveResults(string szFileNameSave);
int SaveDataAnalysis(string szFileNameSave);
int SetupNetlist();
int count2(vector<int>::iterator start, vector<int>::iterator end, int x);
int recalcDv(int x, int y, bool wholelist);
int BestSwap(int iteration);
int KLAlgorithm();
int CleanUp();
int timePassedMS(chrono::steady_clock::time_point start);

struct DataAnalysis{
    int NetSize;
    int CellSize;
    int CutsetF;
    string Filename;
    vector <pair <string, int>> TimePassed;
    vector <int> Cutset;
    vector <int> SubK;
    vector <vector<int>> PairGains;
};
vector <DataAnalysis> StoreData;

pair<unsigned int, unsigned int> UserInput();

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
int LoadFile(string szFilename) {
    if (szFilename.length() <= 0) return -1;

    ifstream myfile;
    string szline;
    stringstream stream;

    auto start = chrono::steady_clock::now();

    myfile.open(szFilename.c_str());

    //check if the file opened correctly
    if (myfile.is_open()) {

        //update the cellsize and netsize
        if (getline(myfile, szline)) {
            if (szline.empty()) return -1;
            stream.clear();
            stream << szline;
            stream >> iCellSize;
        }

        if (getline(myfile, szline)) {
            if (szline.empty()) return -1;
            stream.clear();
            stream << szline;
            stream >> iNetSize;
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

        StoreData.back().CellSize = iCellSize;
        StoreData.back().NetSize = iNetSize;

        myfile.close();
    }
    else {
        return -1;
    }

    cout << "\n\nLoading Data : " << timePassedMS(start) << " ms";
    StoreData.back().TimePassed.push_back({ "Loading Data", timePassedMS(start) });
    return 1;
}

// This is used to save the results
int SaveResults(string szFileNameSave) {
    if (szFileNameSave.length() <= 0) return -1;

    auto start = chrono::steady_clock::now();
    ofstream myfile;
    myfile.open(szFileNameSave.c_str());

    // calculate how many external wires are being cut
    int external = 0;
    for (int i = 0; i < iCellSize; i++) {
        for (unsigned int j = 0; j < vList[i].size(); j++)
            if (vGroup[vList[i][j]] ^ vGroup[i])
                external++;
    }

    //divide external wires by 2 since they are counted twice
    external /= 2;
    StoreData.back().CutsetF = external;

    if (myfile.is_open()) {
        myfile << external << "\n";

        //cout << "\nGroup A is : ";
        for (int i = 0; i < iCellSize; i++)
            if (vGroup[i] == 0)
                myfile << i + 1 << " ";

        myfile << "\n";

        //cout << "\nGroup B is : ";
        for (int i = 0; i < iCellSize; i++)
            if (vGroup[i] == 1)
                myfile << i + 1 << " ";

        myfile.close();
    }
    else {
        cout << "\nUnable to open" << szFileNameSave;
        return -1;
    }

    cout << "\nSaving Results : " << timePassedMS(start) << " ms";
    StoreData.back().TimePassed.push_back({ "Saving Results", timePassedMS(start) });

    cout << "\nCutset size is : " << external;

    return 1;
}

// This is used to save the results
int SaveDataAnalysis(string szFileNameSave) {
    if (szFileNameSave.length() <= 0) return -1;

    remove(szFileNameSave.c_str());

    ofstream myfile;
    myfile.open(szFileNameSave.c_str());

    /*
    struct DataAnalysis{
        string Filename;
        vector <pair <string, int>> TimePassed;
        vector <int> Cutset;
        vector <int> SubK;
        vector <vector<int>> PairGains;
    };
    */
    //StoreData

    if (myfile.is_open()) {
        for (unsigned int i = 0; i < StoreData.size(); i++) {
            myfile << "\nFileName," << StoreData[i].Filename;
            myfile << "\nCellSize," << StoreData[i].CellSize;
            myfile << "\nNetSize," << StoreData[i].NetSize;
            myfile << "\nFinal Cutset," << StoreData[i].CutsetF;

            for (unsigned int j = 0; j < StoreData[i].TimePassed.size(); j++) {
                myfile << "\n" << StoreData[i].TimePassed[j].first << "," << StoreData[i].TimePassed[j].second << ",ms";
            }
            myfile << "\niteration,Cutset Data,Subqueue Length Data";
            for (unsigned int j = 0; j < StoreData[i].Cutset.size(); j++) {
                myfile << "\n" << j << "," << StoreData[i].Cutset[j] << "," << StoreData[i].SubK[j];
            }
 
            myfile << "\nPairwise Gains";
            myfile << "\niterations,";
            for (unsigned int j = 0; j < StoreData[i].PairGains.size(); j++) {
                myfile << j << ",";
            }
 
            for (unsigned int j = 0; j < StoreData[i].CellSize / 2; j++) {
                myfile << "\n";
                for (unsigned int k = 0; k < StoreData[i].PairGains.size(); k++) {
                    myfile << "," << StoreData[i].PairGains[k][j];
                }
            }

            myfile << "\n\n";
        }
   
        myfile.close();
    }
    else {
        cout << "\nUnable to open" << szFileNameSave;
        return -1;
    }


    return 1;
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
    StoreData.back().TimePassed.push_back({ "Setting Up Netlist", timePassedMS(start) });
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
bool sortcol(const vector<int>& v1, const vector<int>& v2) {
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

    vector <int> vSubK;
    vector <int> vCutset;
    vector <vector<int>> vGains;
    vector <int> vTempG;

    auto start = chrono::steady_clock::now();

    do {
        vTempG.resize(0);
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
            int gt = BestSwap(i);
            g += gt;
            vTempG.push_back(gt);

            if (g > gmax) { 
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

        // recalculate the entire Dv list;
        vGroupTempSwap = vGroup;
        recalcDv(0, 0, true);

        int external = 0;
        for (int i = 0; i < iCellSize; i++) {
            for (unsigned int j = 0; j < vList[i].size(); j++)
                if (vGroup[vList[i][j]] ^ vGroup[i])
                    external++;
        }

        //divide external wires by 2 since they are counted twice
        external /= 2;
        vSubK.push_back(lastbesti);
        vCutset.push_back(external);
        vGains.push_back(vTempG);
        cout << "\nIteration Number : " << iterations;
        cout << "\tMax Gain : " << gmax;

        //stop if there is no good i, or there is no positive gain, or max iterations went too high as a failsafe
    } while (lastbesti >= 0 && gmax > 0 && iterations < maxiterations);

    StoreData.back().Cutset = vCutset;
    StoreData.back().SubK = vSubK;
    StoreData.back().PairGains = vGains;

    StoreData.back().TimePassed.push_back({ "Calculating KL", timePassedMS(start) });
    cout << "\nCalc KL : " << timePassedMS(start) << " ms";

    return 1;
}

// Premade file names, assumes there is a folder with the name dev_net in the same location as the .exe
const vector <string> paths = {
    "development_netlists/",
    "Benchmarks/" 
};
const vector <string> szFileNameUse0 = {
    "All Dev Benches",
    "bench_2.net",
    "bench_4.net",
    "bench_6.net",
    "bench_11.net",
    "bench_12.net",
    "bench_16.net",
};
const vector <string> szFileNameUse1 = {
    "All Benchmarks",
    "b_100_500",
    "b_500_20000",
    "b_1000_20000",
    "b_10000_100000",
    "b_50000_400000",
    "b_100000_500000",
    "b_100000_2000000",
    "b_200000_2000000",
    "b_250000_1000000",
    "b_500000_3000000",
};
const vector <vector<string>> szAllFileNamesUse = { 
    szFileNameUse0, 
    szFileNameUse1 
};
const vector <string> szFileNameSave0 = {
    "",
    "r_bench_2.net",
    "r_bench_4.net",
    "r_bench_6.net",
    "r_bench_11.net",
    "r_bench_12.net",
    "r_bench_16.net",
};
const vector <string> szFileNameSave1 = {
    "",
    "r_100_500",
    "r_500_20000",
    "r_1000_20000",
    "r_10000_100000",
    "r_50000_400000",
    "r_100000_500000",
    "r_100000_2000000",
    "r_200000_2000000",
    "r_250000_1000000",
    "r_500000_3000000",
};
const vector <vector<string>> szAllFileNamesSave = { 
    szFileNameSave0, 
    szFileNameSave1 
};

// Ask the user if they want to choose which netlist to use
pair<unsigned int, unsigned int> UserInput() {
    unsigned int BenchNum = 0;
    unsigned int BenchType = 0;

    cout << "\nType 0 to choose from premade benches: ";
    cout << "\nType 1 to choose from HW Benchmarks: ";
    cout << "\nType here: ";
    cin >> BenchType;

    if (BenchType > szAllFileNamesUse.size())
        BenchType = 1;

    for (unsigned int j = 0; j < szAllFileNamesUse[BenchType].size(); j++) {
        cout << "\nType " << j << " to use " << szAllFileNamesUse[BenchType][j];
    }

    cout << "\nType here: ";
    cin >> BenchNum;

    if (BenchNum > szAllFileNamesUse[BenchType].size())
        BenchNum = 1;

    return { BenchType, BenchNum };
}

// Clean up memory
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

    StoreData.clear();
    StoreData.shrink_to_fit();

    return 1;
}

// Helper function for time passed
int timePassedMS(chrono::steady_clock::time_point start) {
    auto end = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

// Entry point for code
int main(int argc, char* argv[]) {
    unsigned int iteration = 0;
    int iReturnVal = 1;
    bool flagAll = false;

    unsigned int BenchType = 0;
    unsigned int BenchNum = 0;

    vector<string> vargv(argv, argv + argc);
    string szfilenameUse = "";
    string szfilenameSave = "";
    

    do {
        StoreData.push_back(DataAnalysis());
        if (flagAll) {
            if (++BenchNum >= szAllFileNamesUse[BenchType].size()) break;
            szfilenameUse = paths[BenchType] + szAllFileNamesUse[BenchType][BenchNum];
            szfilenameSave = paths[BenchType] + szAllFileNamesSave[BenchType][BenchNum];
        }
        else if (argc == 1) {
            auto [BT, BN] = UserInput();
            BenchType = BT;
            BenchNum = BN;

            if (BenchNum == 0) {
                flagAll = true;
                BenchNum++;
            }

            szfilenameUse = paths[BenchType] + szAllFileNamesUse[BenchType][BenchNum];
            szfilenameSave = paths[BenchType] + szAllFileNamesSave[BenchType][BenchNum];
        }
        else if (argc == 3) {
            szfilenameUse = vargv[1];
            szfilenameSave = vargv[2];
            iReturnVal = -1;
        }
        else break;

        StoreData.back().Filename = szfilenameUse;

        cout << "\n\nUsing bench: " << szfilenameUse;

        //First step is to load the file
        if (LoadFile(szfilenameUse) < 0) {
            printf("\nError: File is not able to be opened");
            goto Error;
        }

        //Then setup the netlist
        if (SetupNetlist() < 0) goto Error;

        //Then implement the KL algorithm
        if (KLAlgorithm() < 0) goto Error;

        //Save the Results
        if (SaveResults(szfilenameSave) < 0) goto Error;

        //Save the Results
        if (SaveDataAnalysis(szfilenameSave + "DataAnalysis.csv") < 0) goto Error;

        //go ahead and clean up the memory
        CleanUp(); 

        if (argc == 1 && !flagAll) {
            cout << "\nWould you like to repeat? Type 0 for no: ";
            cin >> iReturnVal;
        }

    } while (iReturnVal > 0 && BenchNum < szAllFileNamesUse[BenchType].size());

    /*
    if (flagAll) {
        SaveDataAnalysis(paths[BenchType] + "DataAnal.csv");
    }
    */

    cout << "\nPress any key to exit..."; 
    cin.get();

    Error:
    CleanUp();
    return 0;
}


