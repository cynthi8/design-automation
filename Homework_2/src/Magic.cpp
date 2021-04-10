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
    set<string> alreadyPlacedCellIds;
    for (int channelIndex = 0; channelIndex < route.m_channelCount - 1; channelIndex++)
    {
        // Create MCells
        int tracksInChannel = route.m_channels[channelIndex].m_tracks.size();
        y += tracksInChannel * 2;
        for (int col = 0; col < route.m_colCount; col++)
        {
            string cellId = route.m_TopRow[channelIndex].RowCells[col].Term.cellId;

            //Check if this is a real cellId, i.e. it corresponds to a cell in the netlist
            if (place.m_netlist.m_cells.find(cellId) == place.m_netlist.m_cells.end())
            {
                continue;
            }

            Cell cell = place.m_netlist.m_cells.at(cellId);

            if (alreadyPlacedCellIds.find(cellId) == alreadyPlacedCellIds.end())
            {
                // Since we scan Left to Right, the start of the cell will be 1 unit to the left of our column
                int x = col - 1;
                MCell mcell(x, y, cell.isFeedthrough(), cellId, cell.m_orientation);
                m_MCells.push_back(mcell);
                alreadyPlacedCellIds.insert(cellId);
            }
        }
        y += 6 + 1; // The next section will be a cell height above, plus 1 for spacing
    }

    y = 0;
    for (int channelIndex = 0; channelIndex < route.m_channelCount; channelIndex++)
    {
        int channelBottom = y;
        int channelTop = y + 2 * route.m_channels[channelIndex].m_tracks.size() - 1;
        map<int, MNet> channelNetMapping;
        for (auto track : route.m_channels[channelIndex].m_tracks)
        {
            // Each track has trunks on them
            // Add these trunks to the channelNetMapping
            for (unsigned int trunk = 0; trunk < track.m_nets.size(); trunk++)
            {
                int netId = track.m_nets[trunk];
                auto range = track.m_locs[trunk];
                MTrunk newTrunk = {range, y};
                channelNetMapping[netId].m_trunks.push_back(newTrunk);
                y += 2; // The next track is two units above this one
            }
        }
        y += 6 + 1; // The next section will be a cell height above, plus 1 for spacing

        // Build the branches between trunks in the MNets
        for (auto mapEntry : channelNetMapping)
        {
            int netId = mapEntry.first;
            MNet newMNet = mapEntry.second;

            // Add branches for each trunk
            for (auto trunk : newMNet.m_trunks)
            {
                MBranch leftBranch;
                MBranch rightBranch;
                leftBranch.x = trunk.x_locs.first;
                rightBranch.x = trunk.x_locs.second;

                // Build Left Branch
                int leftBotNetId = route.m_BotRow[channelIndex].RowCells[leftBranch.x].NetID;
                int leftTopNetId = route.m_TopRow[channelIndex].RowCells[leftBranch.x].NetID;
                if (leftBotNetId == netId)
                {
                    // Construct a branch down
                    leftBranch.y_locs = {channelBottom, trunk.y};
                }
                else if (leftTopNetId == netId)
                {
                    // Construct a branch up
                    leftBranch.y_locs = {trunk.y, channelTop};
                }
                else
                {
                    // Handle inter trunk connections
                }

                // Build Right Branch
                int rightBotNetId = route.m_BotRow[channelIndex].RowCells[rightBranch.x].NetID;
                int rightTopNetId = route.m_TopRow[channelIndex].RowCells[rightBranch.x].NetID;
                if (rightBotNetId == netId)
                {
                    // Construct a branch down
                    rightBranch.y_locs = {channelBottom, trunk.y};
                }
                else if (rightTopNetId == netId)
                {
                    // Construct a branch up
                    rightBranch.y_locs = {trunk.y, channelTop};
                }
                else
                {
                    // Doglegs :( Handle inter trunk connections
                }
                newMNet.m_branches.push_back(leftBranch);
                newMNet.m_branches.push_back(rightBranch);
            }
            m_MNets.push_back(newMNet);
        }
    }

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

void MCell::updateTransformString()
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

    m_transformString = string("transform") + " " + to_string(a) + " " + to_string(b) + " " + to_string(c) + " " + to_string(d) + " " + to_string(e) + " " + to_string(f) + '\n';
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
    string timestampString = "timestamp 1\n";
    cellGroup = useString + timestampString + m_transformString + boxString;

    return cellGroup;
}