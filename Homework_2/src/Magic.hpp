#ifndef MAGIC_H
#define MAGIC_H

#include "Graph.hpp"
#include "Routing.hpp"
#include <fstream>
#include <ctime>
#include <chrono>

using namespace std;

class MCells 
{
public:
    MCells(int x, int y, bool isFeed, string cellID, Flips m_orientation)
    : x(x), y(y), isFeed(isFeed), 
      cellID (cellID), m_orientation(m_orientation) {}

    int x;
    int y;
    bool isFeed;
    string cellID;
    Flips m_orientation;
    string transf;
    void transform() {
        //change the transf string according to x, y, and orientation 
    }
};

class MNets
{
public:
    int netID;
    MTrunk m_trunk;
    pair<MBranch, MBranch> m_branches;
};

class MTrunk
{
public:
    pair<int, int> x_locs;
    int y;
};

class MBranch
{
public:
    pair<int, int> y_locs;
    int x;
};

class magRect
{
public:
    magRect(int xbot, int ybot, int xtop, int ytop)
        : xbot(xbot), ybot(ybot), xtop(xtop), ytop(ytop) {}
    int xbot;
    int ybot;
    int xtop;
    int ytop;

    string szRect(string type) {
        type += " " + xbot;
        type += " " + ybot;
        type += " " + xtop;
        type += " " + xbot;

        return type;
    }
    void outputRect(std::ostream& outputStream, string type) {
        outputStream << szRect(type) << endl;
    }

    void outputLabel(std::ostream& outputStream, string type, int i) {
        outputStream << szRect(type) << " 0 " << i << endl;
    }
    void outputLabel(std::ostream& outputStream, string type, string i) {
        outputStream << szRect(type) << " 0 " << i << endl;
    }
};

class Magic
{
public:
    Magic(Routing route, Graph graph);
    void Print(string szDirectory, string szFileName);

    long long GetTime();
    void Header(std::ostream& outputStream);
    void Footer(std::ostream& outputStream);
    void OutputFeedCell(string szDirectory);
    void OutputStandardCell(string szDirectory);

    void CreateLayout(Routing route, Graph graph);
    void OutputLayout(string szDirectory, string szFileName);

private:
};

#endif