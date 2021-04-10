#include <iostream>
#include <fstream>
#include <cstdlib>

#include "Magic.hpp"

using namespace std;

// Magic Intro Function
Magic::Magic(Placement place, Routing route)
{
    CreateLayout(route, place);
}

// Create a standard Header for Magic
void Magic::Header(std::ostream &outputStream)
{
    outputStream << "magic" << endl;
    outputStream << "tech scmos" << endl;
    outputStream << "timestamp ";
    outputStream << GetTime() << endl;
}

// End the Magic File
void Magic::Footer(std::ostream &outputStream)
{
    outputStream << "<< end >>" << endl;
}

// Get Current Time
long long Magic::GetTime()
{
    // We actually don't want a different time as then it rechecks the drc
    return 1;
}

// Function to output the final Magic File
void Magic::Output(string szDirectory, string szFileName)
{
    OutputStandardCell(szDirectory);

    OutputFeedCell(szDirectory);

    OutputLayout(szDirectory, szFileName);

    return;
}

// Create an object that has all the necessary info to create the Magic File
void Magic::CreateLayout(Routing route, Placement place)
{
    int y = 0;
    for (int channelIndex = 0; channelIndex < route.m_channelCount - 1; channelIndex++)
    {
        // Create MCells
        int x = 0;
        int tracksInChannel = route.m_channels[channelIndex].m_tracks.size();
        y = channelIndex * 6 + tracksInChannel * 2 + 1;
        for (int col = 0; col < route.m_colCount; col++)
        {
            string cellId = route.m_TopRow[channelIndex].RowCells[col].Term.cellId;
            set<string> alreadyPlaced;

            if (alreadyPlaced.find(cellId) == alreadyPlaced.end())
            {
                Cell cell = place.m_netlist.m_cells[cellId];

                MCell mcell(x, y, cell.isFeedthrough(), cellId, cell.m_orientation);

                m_MCells.push_back(mcell);

                if (cell.isFeedthrough())
                    x += 3;
                else
                    x += 7;

                alreadyPlaced.insert(cellId);
            }
        }
    }

    // go thru all nets
    /*
    for (unsigned int l = 0; l < route.m_Spans[channelIndex].size(); l++)
    {
        auto &j = route.m_Spans[channelIndex][l];
        //go thru all tracks for that net
        for (unsigned int k = 0; k < j.ranges.size(); k++)
        {
            MNet mnet;
            mnet.netID = j.net;

            MTrunk m_trunk;
            int left = j.ranges[k].first;
            int right = j.ranges[k].second;
            int newLeft = colsTransformation[i][left];
            int newRight = colsTransformation[i][right];
            m_trunk.x_locs = {newLeft, newRight};

            m_trunk.y = channelIndex * 6 + j.n_tracks[k] * 2;
            mnet.m_trunk = m_trunk;

            MBranch mbranchL;
            MBranch mbranchR;
            mbranchL.x = newLeft;
            mbranchR.x = newRight;

            int botNetID = route.m_BotRow[channelIndex].RowCells[left].NetID;
            int topNetID = route.m_TopRow[channelIndex].RowCells[left].NetID;
            if (k == 0 || k == (j.ranges.size() - 1))
            {
                if (mnet.netID == botNetID)
                {
                }
                else if (mnet.netID == topNetID)
                {
                }
                else if (mnet.netID == topNetID)
                {
                }
                else
                {
                }
            }

            mbranchL.y_locs = {};
            mbranchR.y_locs = {};

            mnet.m_branches = {mbranchL, mbranchR};
            MagNets[channelIndex].push_back(mnet);
        }
    }
    */
    return;
}

// Output the Final Result
void Magic::OutputLayout(string szDirectory, string szFileName)
{
    string filePath = szDirectory + '/' + szFileName;
    fstream outputStream;
    outputStream.open(filePath, ios::out);

    // Begin File
    Header(outputStream);

    // Write cells
    for (auto cell : m_MCells)
    {
        outputStream << cell.makeCell();
    }

    // Exit and close
    Footer(outputStream);
    outputStream.close();

    return;
}

// Create a 3x6 2 terminal Feed Cell for reference
void Magic::OutputFeedCell(string szDirectory)
{
    string filePath = szDirectory + "/FeedCell.mag";
    fstream outputStream;
    outputStream.open(filePath, ios::out);

    Header(outputStream);

    // Terminals = rect xbot ybot xtop ytop
    MagRect cell(0, 0, 3, 6);
    MagRect T1(1, 0, 2, 1);
    MagRect T3(1, 5, 2, 6);
    vector<MagRect> Terms = {T1, T3};

    // Metal Rectangles
    outputStream << "<< metal1 >>" << endl;
    for (auto &term : Terms)
        outputStream << term.makeRect();

    // Label Rectangles
    outputStream << "<< labels >>" << endl;
    outputStream << cell.makeRlabel("metal1", ".");
    outputStream << Terms[0].makeRlabel("metal1", 1);
    outputStream << Terms[1].makeRlabel("metal1", 3);

    // Exit and close
    Footer(outputStream);
    outputStream.close();

    return;
}

// Create a standard 6x6 4 terminal Cell for reference
void Magic::OutputStandardCell(string szDirectory)
{
    string filePath = szDirectory + "/Cell.mag";
    fstream outputStream;
    outputStream.open(filePath, ios::out);

    Header(outputStream);

    // Terminals = rect xbot ybot xtop ytop
    MagRect cell(0, 0, 6, 6);
    MagRect T1(1, 5, 2, 6);
    MagRect T2(4, 5, 5, 6);
    MagRect T3(1, 0, 2, 1);
    MagRect T4(4, 0, 5, 1);
    vector<MagRect> Terms = {T1, T2, T3, T4};

    // Metal Rectangles
    outputStream << "<< metal1 >>" << endl;
    for (auto &term : Terms)
    {
        outputStream << term.makeRect();
    }

    // Label Rectangles
    outputStream << "<< labels >>" << endl;
    outputStream << cell.makeRlabel("metal1", ".");

    int termId = 1;
    for (auto &term : Terms)
    {
        outputStream << term.makeRlabel("metal1", termId);
        termId++;
    }

    // Exit and close
    Footer(outputStream);
    outputStream.close();

    return;
}

void MCell::transform()
{
    // Update the m_transformString string according to x, y, and orientation
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    if (m_orientation == FlipNone)
    {
        a = 1;
        e = 1;
    }
    else if (m_orientation == FlipLR)
    {
        a = -1;
        c = 6;
        e = 1;
    }
    else if (m_orientation == FlipTB)
    {
        a = 1;
        e = -1;
        f = 6;
    }
    else if (m_orientation == FlipBoth)
    {
        a = -1;
        c = 6;
        e = -1;
        f = 6;
    }
    c += x;
    f += y;

    m_transformString = "transform" + ' ' + to_string(a) + ' ' + to_string(b) + ' ' + to_string(c) + ' ' + to_string(d) + ' ' + to_string(e) + ' ' + to_string(f);
}

string MCell::makeCell()
{
    /*
    Return the Use Cell group string

    For example, it might return:
    use Cell 1
    timestamp 1
    transform 1 0 0 0 1 0
    box 0 0 6 6
    */

    string cellGroup;

    string useString;
    string boxString;

    if (isFeed)
    {
        useString = "use FeedCell " + m_cellId + '\n';
        boxString = "box 0 0 3 6\n";
    }
    else
    {
        useString = "use Cell " + m_cellId + '\n';
        boxString = "box 0 0 6 6\n";
    }
    string timestampString = "timestamp 1";
    cellGroup = useString + timestampString + m_transformString + boxString;

    return cellGroup;
}