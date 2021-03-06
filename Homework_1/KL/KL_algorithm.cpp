// HW1.cpp : KL Function
// VLSI Design: HW1
// Date: 2021-02-12
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
#include <limits>
//#include <execution>

using namespace std;

int LoadFile(string szFilename);
int SaveResults(string szFileNameSave);
int SaveDataAnalysis(string szFileNameSave);
int SetupNetlist();
int count2(vector<int>::iterator start, vector<int>::iterator end, int x);
int recalcDv(int x, int y, bool wholelist);
long long BestSwap(int iteration);
int KLAlgorithm();
int CleanUp();
int timePassedMS(chrono::steady_clock::time_point start);
pair<unsigned int, unsigned int> UserInput();

// Don't know how large these variables will become. 
// Don't want them to become too large inside the functions
struct DataAnalysis{
    bool Store;
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

vector <pair<int, int>> NetCol; //all columns of textfile netlist

vector <int> vD;                //difference of external to internal wires for each unit cell
vector <int> tempCheck;         //psuedo c matrix from class

vector <bool> vGroup;           //what group each unit cell is associated with, A=1, B=0
vector <bool> vGroupTempSwap;   //what the new temp grouping is
vector <bool> vLockedUnsorted;  //Based on sortedvD, which unsorts it

vector <vector<int>> vList;     //a list of what cells a cell is connected to
vector <pair<int, int>> vQueue; //queue of cells to switch

vector <int> OriginalIndex;     //Just a vector from 0 to iCellSize, might be redundant now
vector <vector<int>> sortedvD;  //Sorted D values with corresponding original indices

int iCellSize = 0;              //number of cells
int iNetSize = 0;               //number of total connections between each cell
unsigned long long External = 0;//number of external wires being cut

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
            stream.str("");
            stream.clear();
            stream << szline;
            stream >> iCellSize;
        }

        if (getline(myfile, szline)) {
            if (szline.empty()) return -1;
            stream.str("");
            stream.clear();
            stream << szline;
            stream >> iNetSize;
        }

        cout << "\n\tCellsize is\t: " << iCellSize;
        cout << "\n\tNetsize is\t: " << iNetSize;

        //resize the column vectors to fit all the numbers in a column
        if (iCellSize > 0 && iNetSize > 0)
            NetCol.resize(iNetSize);
         else return -1;

        //add each number from the file to the corresponding column vector
        int j;
        for (j = 0; j < iNetSize && getline(myfile, szline, '\n'); j++) {
            if (szline.empty()) break;

            stream.str("");
            stream.clear();
            stream << szline;
            //following doesn't work on linux lol
            //sscanf_s(stream.str().c_str(), "%d %d", &NetCol[j].first, &NetCol[j].second);
            stream >> NetCol[j].first >> NetCol[j].second;
        }

        // somehow the netsize is wrong 
        // assumes the cellsize is always right
        if (j != iNetSize) { 
            iNetSize = j;
            NetCol.resize(iNetSize);
            NetCol.shrink_to_fit();
        }

        StoreData.back().CellSize = iCellSize;
        StoreData.back().NetSize = iNetSize;

        myfile.close();
    }
    else {
        return -1;
    }

    cout << "\n\tLoading Data\t: " << timePassedMS(start) << " ms";

    if (StoreData.back().Store)
        StoreData.back().TimePassed.push_back({ "Loading Data", timePassedMS(start) });

    return 1;
}

// This is used to save the results
// Similar format as above Loadfile function
int SaveResults(string szFileNameSave) {
    if (szFileNameSave.length() <= 0) return -1;

    auto start = chrono::steady_clock::now();
    ofstream myfile;
    myfile.open(szFileNameSave.c_str());

    StoreData.back().CutsetF = (int) External;

    if (myfile.is_open()) {
        //Cutsize
        myfile << External << "\n";

        //First Group
        for (int i = 0; i < iCellSize; i++)
            if (vGroup[i] == 0)
                myfile << i + 1 << " ";

        myfile << "\n";

        //Second Group
        for (int i = 0; i < iCellSize; i++)
            if (vGroup[i] == 1)
                myfile << i + 1 << " ";

        myfile.close();
    }
    else {
        cout << "\nUnable to open " << szFileNameSave;
        return -1;
    }

    cout << "\n\tSaving Results\t: " << timePassedMS(start) << " ms";

    if (StoreData.back().Store)
        StoreData.back().TimePassed.push_back({ "Saving Results", timePassedMS(start) });

    cout << "\n\tCutset size is\t: " << External;

    return 1;
}

// This is used to save the analysis
int SaveDataAnalysis(string szFileNameSave) {
    if (szFileNameSave.length() <= 0) return -1;

    remove(szFileNameSave.c_str());

    ofstream myfile;
    myfile.open(szFileNameSave.c_str());

    if (myfile.is_open()) {
        // In a loop since I originally intended to save all the data and store it in one csv file
        // in retrospect probably a bad idea
        for (unsigned int i = 0; i < StoreData.size(); i++) {
            // Basic Info
            myfile << "\nFileName," << StoreData[i].Filename;
            myfile << "\nCellSize," << StoreData[i].CellSize;
            myfile << "\nNetSize," << StoreData[i].NetSize;
            myfile << "\nFinal Cutset," << StoreData[i].CutsetF;

            // Save the timing info per major func
            for (unsigned int j = 0; j < StoreData[i].TimePassed.size(); j++) {
                myfile << "\n" << StoreData[i].TimePassed[j].first << "," << StoreData[i].TimePassed[j].second << ",ms";
            }

            // Save the cutset and subqueue data
            myfile << "\niteration,Cutset Data,Subqueue Length Data";
            for (unsigned int j = 0; j < StoreData[i].Cutset.size(); j++) {
                myfile << "\n" << j << "," << StoreData[i].Cutset[j] << "," << StoreData[i].SubK[j];
            }
 
            // Save all of the gains from each iteration
            myfile << "\nPairwise Gains";
            myfile << "\niterations,";
            for (unsigned int j = 0; j < StoreData[i].PairGains.size(); j++) {
                myfile << j << ",";
            }
            
            // Saving in the csv format, also had to switch rows and columns because of size limits
            for (int j = 0; j < StoreData[i].CellSize / 2; j++) {
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

    // Tried different random number generators. The seed is the current time
    long unsigned int seed = (long unsigned int) std::chrono::system_clock::now().time_since_epoch().count();
    //std::default_random_engine randeng(seed);
    //shuffle(vGroup.begin(), vGroup.end(), randeng); //now shuffle the vector

    //std::random_device rd;
    //std::mt19937 gen(rd());
    mt19937 gen(seed);
    shuffle(vGroup.begin(), vGroup.end(), gen); //now shuffle the vector

    //Add the elements a unit cell is connected to, to the vList 2D vector
    //for example cell 1 is connected to 2, 3, 3, 4
    //vList[0] = {1, 2, 2, 3}

    for (int i = 0; i < iNetSize; i++) {
        vList[NetCol[i].first - 1].push_back(NetCol[i].second - 1);
        vList[NetCol[i].second - 1].push_back(NetCol[i].first - 1);
    }

    // Netcol has finished its job, go ahead and clear its memory
    NetCol.clear();
    NetCol.shrink_to_fit();

    // Go ahead and sort each vector for each cell, which may be useful later
    for (int i = 0; i < iCellSize; i++) {
        sort(vList[i].begin(), vList[i].end());
    }

    //Manually create the first D list
    //which is how many external connects - internal connections
    vGroupTempSwap = vGroup;
    recalcDv(0, 0, true);

    cout << "\n\tSetting Up Net\t: " << timePassedMS(start) << " ms";
    cout << "\n\tSeed\t\t: " << seed;

    if (StoreData.back().Store)
        StoreData.back().TimePassed.push_back({ "Setting Up Netlist", timePassedMS(start) });

    return 1;
}

// This is a modified binary search algorithm for vectors
// Was used a lot more in the first iteration of this program
int count2(vector<int>::iterator start, vector<int>::iterator end, int x)
{
    vector<int>::iterator low;

    low = lower_bound(start, end, x);

    if (low == end || *low != x) return 0; // If element is not present, return 0 

    return upper_bound(low, end, x) - low;
}

//which is how many external connects - internal connections
int recalcDv(int x, int y, bool wholelist) {

    //Manually create the first D list
    if (wholelist) { 
        External = 0;

        for (int i = 0; i < iCellSize; i++) {
            unsigned long long extcells = 0;
            for (auto j : vList[i])
                if (vGroupTempSwap[j] ^ vGroupTempSwap[i])
                    extcells++;

            //go ahead and save the total external wires
            External += extcells;

            vD[i] = ((int) extcells) * 2 - vList[i].size(); //ex - ((total - ex)=internal)
        }

        External /= 2; //divide by 2 since they are counted twice
    } 
    // Update the D list for only the modified values
    else {
        for (auto num : vList[x]) {
            int xa = (vGroupTempSwap[num] == vGroupTempSwap[x]) ? -1 : 1;
            vD[num] = vD[num] + 2 * xa;
        }

        for (auto num : vList[y]) {
            int ya = (vGroupTempSwap[num] == vGroupTempSwap[y]) ? -1 : 1;
            vD[num] = vD[num] + 2 * ya;
        }

        /*
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
        */
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

// This function is to find the best swap for the current D list
long long BestSwap(int iteration) {

    long long iGainTempMax = numeric_limits<long long>::min();

    pair <int, int> iBest = {-1, -1};
    pair <int, int> iBestUn = {-1, -1};

    static int indexi = 0;
    unsigned int i;

    //vector <int>::iterator it = OriginalIndex.begin();

    // iterate thru sorted d list
    for (int ia = indexi; ia < iCellSize; ia++) {
        if (vLockedUnsorted[ia]) { //if the cell is locked, then skip
            continue; 
        }

        i = sortedvD[ia][1]; //why does the profiler say this is taking so long

        fill(tempCheck.begin(), tempCheck.end(), 0);
        for (auto j : vList[i])
            tempCheck[j]++;

        // go thru all cells except for the cells already checked from i
        for (int jb = ia + 1; jb < iCellSize; jb++) {
            unsigned int j = sortedvD[jb][1];

            // check if the cell isn't locked and is in a different group than the first cell
            if (!vLockedUnsorted[jb] && vGroupTempSwap[j] ^ vGroupTempSwap[i]) {

                // if the gain here is less than the max, then there's no point in checking any more
                if ((long long) vD[i] + vD[j] < iGainTempMax)
                    goto Foundbestpair;

                long long iGainTemp = (long long) vD[i] + vD[j] - 2 * (long long) tempCheck[j];

                if (iGainTemp > iGainTempMax) {
                    iGainTempMax = iGainTemp;
                    iBest = {i, j};
                    iBestUn = { ia, jb };
                }
            }
        }
    }

    Foundbestpair:
    //if the index locations are good, swap the elements, and store it in the queue
    if (iBest.first >= 0 && iBest.second >= 0) {
        vQueue[iteration] = iBest;

        vLockedUnsorted[iBestUn.first] = true;
        vLockedUnsorted[iBestUn.second] = true;

        vGroupTempSwap[iBest.first] = !vGroupTempSwap[iBest.first];
        vGroupTempSwap[iBest.second] = !vGroupTempSwap[iBest.second];

        recalcDv(iBest.first, iBest.second, false);

        while (indexi < iCellSize && vLockedUnsorted[indexi])
            indexi++;

        if (indexi == iCellSize) indexi = 0;
    }

    return iGainTempMax;
}

// This is the KL implementation of what was shown in class
int KLAlgorithm(){
    const int maxiterations = 1000;
    int iterations = 0;

    long long gmax = 0;
    long long g = 0;
    int ki = -1;
    
    vQueue.resize(iCellSize / 2);
    //vLocked.resize(iCellSize);
    tempCheck.resize(iCellSize);
    vGroupTempSwap = vGroup;

    OriginalIndex.resize(iCellSize);
    iota(OriginalIndex.begin(), OriginalIndex.end(), 0);
    sortedvD.resize(iCellSize, vector<int>(2));
    
    vLockedUnsorted.resize(iCellSize);

    vector <int> vSubK;
    vector <int> vCutset;
    vector <vector<int>> vGains;
    vector <int> vTempG;
   
    auto start = chrono::steady_clock::now();



    //do loop since we don't know how many iterations will be required
    do { 
        if (StoreData.back().Store)
            vTempG.resize(0);

        iterations++;
        ki = -1;
        gmax = 0;
        g = 0;

        fill(vLockedUnsorted.begin(), vLockedUnsorted.end(), false);

        //link these two so when you sort the D list, it keeps the original index
        for (int i = 0; i < iCellSize; i++) { 
            sortedvD[i] = { vD[i] , OriginalIndex[i] };
        }

        //how do I sort a 2d vector based on a row instead of a column
        //sort(std::execution::par_unseq, sortedvD.begin(), sortedvD.end(), sortcol);
        sort(sortedvD.begin(), sortedvD.end(), sortcol);


        // This loop is the one that chooses the best pair and calculates their gain 
        // and keeps track of the max of the sums of the gain
        for (int i = 0; i < iCellSize / 2; i++) {
            long long gt = BestSwap(i); 

            if (gt < numeric_limits<long long>::min() + 1)
                break;

            g += gt;

            if (StoreData.back().Store)
                vTempG.push_back((int)gt);

            if (g > gmax) { 
                gmax = g;
                ki = i;
            }
        }

        // Do the swapping up to K from the subqueue
        for (int i = 0; i <= ki && gmax > 0; i++) {
            auto [Anum, Bnum] = vQueue[i];
            vGroup[Anum] = !vGroup[Anum];
            vGroup[Bnum] = !vGroup[Bnum];
        }

        // recalculate the entire Dv list instead of trying to calc; its easier
        vGroupTempSwap = vGroup;
        recalcDv(0, 0, true);

        if (StoreData.back().Store) {
            vSubK.push_back(ki);
            vCutset.push_back((int)External);
            vGains.push_back(vTempG);
        }

        if (gmax > 0) {
            cout << "\n\tIteration Number : " << iterations;
            cout << "\t\tMax Gain : " << gmax;
            cout << "     \tNum Swaps : " << ki * 2 + 2;
            cout << "     \tCur Cutset : " << External;
        }

        //stop if there is no good K length, or there is no positive gain, or max iterations went too high as a failsafe
    } while (ki >= 0 && gmax > 0 && iterations < maxiterations);

    if (StoreData.back().Store) {
        StoreData.back().Cutset = vCutset;
        StoreData.back().SubK = vSubK;
        StoreData.back().PairGains = vGains;
        StoreData.back().TimePassed.push_back({ "Calculating KL", timePassedMS(start) });
    }

    cout << "\n\tCalculating KL\t: " << timePassedMS(start) << " ms";

    return 1;
}

// Premade file names, assumes there is are folders with the path names in the same location as the .exe
// Otherwise you can pipe in arguments from the command line manually
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

// Ask the user if they want to choose which netlist to use if no additional arguments were made
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

// Clean up memory between runs
int CleanUp() {
    vD.clear();
    vGroup.clear();
    vGroupTempSwap.clear();
    vList.clear();
    vQueue.clear();

    vD.shrink_to_fit();
    vGroup.shrink_to_fit();
    vGroupTempSwap.shrink_to_fit();
    vList.shrink_to_fit();
    vQueue.shrink_to_fit();

    NetCol.clear();
    NetCol.shrink_to_fit();

    OriginalIndex.clear();
    sortedvD.clear();
    OriginalIndex.shrink_to_fit();
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
    int iReturnVal = 1;
    bool flagAll = false;

    //unsigned int iteration = 0;
    unsigned int BenchType = 0;
    unsigned int BenchNum = 0;

    vector<string> vargv(argv, argv + argc);
    string szfilenameUse = "";
    string szfilenameSave = "";

    do {
        StoreData.resize(1);
        // Set to false so you don't overwrite data that we've already saved them, also this bloats the memory
        // This has to be manually changed in the source code and then compiled
        StoreData.back().Store = false; 

        // If there were no additional arguments, then ask the user from the premade lists for convience
        if (argc == 1) {
            if (flagAll) {
                if (++BenchNum >= szAllFileNamesUse[BenchType].size()) 
                    break;
            }
            else {
                auto [x, y] = UserInput();
                BenchType = x; BenchNum = y;

                if (BenchNum == 0) {
                    flagAll = true; BenchNum++;
                }
            }
            szfilenameUse = paths[BenchType] + szAllFileNamesUse[BenchType][BenchNum];
            szfilenameSave = paths[BenchType] + szAllFileNamesSave[BenchType][BenchNum];
        }
        else if (argc == 3) {
            // If there were additional arguments, use them
            szfilenameUse = vargv[1];  //what file to load
            szfilenameSave = vargv[2]; //what file to save
            iReturnVal = -1; //don't loop
        }
        else break;

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

        //Save the Performance Analysis
        if (StoreData.back().Store) {
            StoreData.back().Filename = szfilenameUse;
            if(SaveDataAnalysis(szfilenameSave + "DataAnalysis.csv") < 0) goto Error;
        }

        //go ahead and clean up the memory
        CleanUp(); 

        if (argc == 1 && !flagAll) {
            cout << "\nWould you like to repeat? Type 0 for no: ";
            cin >> iReturnVal;
        }

    } while (iReturnVal > 0 && BenchNum < szAllFileNamesUse[BenchType].size());

    Error:
    CleanUp();
    return 0;
}


