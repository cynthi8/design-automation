#include "HW2.h"

// load the values from the file
int LoadFile(string szFilename) {
    ifstream myfile;
    string szline;
    stringstream stream;

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

    return 1;
}