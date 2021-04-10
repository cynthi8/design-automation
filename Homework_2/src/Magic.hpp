#ifndef MAGIC_H
#define MAGIC_H

#include "Graph.hpp"
#include "Routing.hpp"
#include <fstream>
#include <ctime>
#include <chrono>

using namespace std;

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