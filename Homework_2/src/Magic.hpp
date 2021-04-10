#ifndef MAGIC_H
#define MAGIC_H

#include "Graph.hpp"
#include "Routing.hpp"
#include <fstream>
#include <ctime>
#include <chrono>

using namespace std;

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

struct MContact
{
    int x;
    int y;
};

class MNet
{
public:
    MNet(int netID) : netID(netID){};
    int netID;
    vector<MTrunk> m_trunks;
    vector<MBranch> m_branches;
    vector<MContact> m_contacts;
    //string makePaint();
    string makeMetal1();
    string makeMetal2();
    string makeMetal2Contact();
    string makeLabel();
};

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

class MagRect
{
public:
    MagRect(int xbot, int ybot, int xtop, int ytop)
        : xbot(xbot), ybot(ybot), xtop(xtop), ytop(ytop) {}
    int xbot;
    int ybot;
    int xtop;
    int ytop;

    string makeBoundingBox();
    string makeRect();
    string makeRlabel(string layer, int text);
    string makeRlabel(string layer, string text);
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