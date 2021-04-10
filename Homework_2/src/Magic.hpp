#ifndef MAGIC_H
#define MAGIC_H

#include "Graph.hpp"
#include "Routing.hpp"
#include <fstream>
#include <ctime>
#include <chrono>

using namespace std;

class MCell
{
public:
    MCell(int x, int y, bool isFeed, string cellId, Flips m_orientation)
        : x(x), y(y), isFeed(isFeed), m_cellId(cellId), m_orientation(m_orientation)
    {
        updateTransformString();
    }
    void updateTransformString();
    string makeCell();

    int x;
    int y;
    bool isFeed;
    string m_cellId;
    Flips m_orientation;
    string m_transformString;
};

struct MBranch
{
public:
    pair<int, int> y_locs;
    int x;
};

struct MTrunk
{
    pair<int, int> x_locs;
    int y;
};

class MNet
{
public:
    int netID;
    vector<MTrunk> m_trunks;
    vector<MBranch> m_branches;
};

class MagRect
{
public:
    MagRect(int xbot, int ybot, int xtop, int ytop)
        : xbot(xbot), ybot(ybot), xtop(xtop), ytop(ytop) {}
    int xbot;
    int ybot;
    int xtop;
    int ytop;

    string makeBoundingBox()
    {
        string outputString;
        outputString += ' ' + to_string(xbot);
        outputString += ' ' + to_string(ybot);
        outputString += ' ' + to_string(xtop);
        outputString += ' ' + to_string(ytop);

        return outputString;
    }

    string makeRect()
    {
        return "rect" + makeBoundingBox() + '\n';
    }

    string makeRlabel(string layer, int text)
    {
        return "rlabel " + layer + makeBoundingBox() + " 0 " + to_string(text) + '\n';
    }
    string makeRlabel(string layer, string text)
    {
        return "rlabel " + layer + makeBoundingBox() + " 0 " + text + '\n';
    }
};

class Magic
{
public:
    Magic(Placement place, Routing route);
    void Output(string szDirectory, string szFileName);

    long long GetTime();
    void Header(std::ostream &outputStream);
    void Footer(std::ostream &outputStream);
    void OutputFeedCell(string szDirectory);
    void OutputStandardCell(string szDirectory);

    void CreateLayout(Routing route, Placement place);
    void OutputLayout(string szDirectory, string szFileName);

private:
    vector<MCell> m_MCells;
    vector<MNet> m_MNets;
};

#endif