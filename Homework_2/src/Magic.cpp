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
    /***********************
    * Create MCells
    ***********************/
    int y = 0;
    set<string> alreadyPlacedCellIds;
    for (int channelIndex = 0; channelIndex < route.m_channelCount - 1; channelIndex++)
    {
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

    /***********************
    * Create MNets
    ***********************/
    int nextChannelBottom = 0;
    for (int channelIndex = 0; channelIndex < route.m_channelCount; channelIndex++)
    {
        int channelBottom = nextChannelBottom;
        int channelTop = nextChannelBottom + 2 * route.m_channels[channelIndex].m_tracks.size() - 1;
        nextChannelBottom = channelTop + 6;
        for (auto span : route.m_Spans[channelIndex])
        {
            /* Assumption: All the ranges are in ascending order (left to right) and there is a max of 2 */

            int netId = span.net;
            MNet newMNet(netId);

            // Build Trunks
            for (unsigned int trunk = 0; trunk < span.ranges.size(); trunk++)
            {
                MTrunk newTrunk;
                newTrunk.y = channelBottom + 2 * span.n_tracks[trunk] + 1;
                newTrunk.x_locs = span.ranges[trunk];
                newMNet.m_trunks.push_back(newTrunk);

                //Build the Contacts, 2 contacts per trunk
                MContact contact;
                contact.y = newTrunk.y;
                contact.x = span.ranges[trunk].first;
                newMNet.m_contacts.push_back(contact);

                contact.x = span.ranges[trunk].second;
                newMNet.m_contacts.push_back(contact);
            }

            // Build Left Most Branch
            MBranch leftMostBranch;
            leftMostBranch.x = span.ranges[0].first;
            if (netId == route.m_BotRow[channelIndex].RowCells[leftMostBranch.x].NetID)
            {
                // Construct a branch down
                leftMostBranch.y_locs = {channelBottom, channelBottom + span.n_tracks[0]};
            }
            else if (netId == route.m_TopRow[channelIndex].RowCells[leftMostBranch.x].NetID)
            {
                // Construct a branch up
                leftMostBranch.y_locs = {channelBottom + span.n_tracks[0], channelTop};
            }
            else
            {
                throw("Left most branch should go to a row cell");
            }
            newMNet.m_branches.push_back(leftMostBranch);

            // Build Right Most Branch
            MBranch rightMostBranch;
            rightMostBranch.x = span.ranges.back().second;
            if (netId == route.m_BotRow[channelIndex].RowCells[rightMostBranch.x].NetID)
            {
                // Construct a branch down
                rightMostBranch.y_locs = {channelBottom, channelBottom + span.n_tracks.back()};
            }
            else if (netId == route.m_TopRow[channelIndex].RowCells[rightMostBranch.x].NetID)
            {
                // Construct a branch up
                rightMostBranch.y_locs = {channelBottom + span.n_tracks.back(), channelTop};
            }
            else
            {
                throw("Right most branch should go to a row cell");
            }
            newMNet.m_branches.push_back(rightMostBranch);

            if (span.ranges.size() == 2)
            {
                // Build the branch between the trunks
                MBranch middleBranch;
                middleBranch.x = span.ranges[0].second;
                middleBranch.y_locs = {span.n_tracks[0], span.n_tracks[1]};
                newMNet.m_branches.push_back(middleBranch);
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

    // Write Net M1
    outputStream << "<< metal1 >>" << endl;
    for (auto net : m_MNets)
    {
        outputStream << net.makeMetal1();
    }

    // Write Net M2
    outputStream << "<< metal2 >>" << endl;
    for (auto net : m_MNets)
    {
        outputStream << net.makeMetal2();
    }

    // Write Net M2c
    outputStream << "<< m2contact >>" << endl;
    for (auto net : m_MNets)
    {
        outputStream << net.makeMetal2Contact();
    }

    // Write cells
    for (auto cell : m_MCells)
    {
        outputStream << cell.makeCell();
    }

    // Write net labels
    outputStream << "<< labels >>" << endl;
    for (auto net : m_MNets)
    {
        outputStream << net.makeLabel();
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

// Create a Transformation string for the cell
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

string MNet::makeMetal1()
{
    string netBranches;

    for (auto &i : m_branches)
    {
        netBranches += "rect";
        MagRect T1(i.x, i.y_locs.first, i.x + 1, i.y_locs.second + 1);
        i.rect = T1;
        netBranches += T1.makeBoundingBox();
        netBranches += "\n";
    }

    return netBranches;
}

string MNet::makeLabel()
{
    //flabel space 0 11 6 17 0 FreeSans 8 0 0 0 Cell4
    string Labels;

    for (unsigned int i = 0; i < m_trunks.size(); i++)
    {
        if (abs(m_trunks[i].x_locs.second - m_trunks[i].x_locs.first) > 1)
        {
            Labels += "rlabel ";
            Labels += "metal1 ";
            Labels += m_trunks[i].rect.makeBoundingBox();
            Labels += " 0 ";
            Labels += to_string(netID);
            Labels += "\n";
        }
    }
    for (unsigned int i = 0; i < m_branches.size(); i++)
    {
        if (abs(m_branches[i].y_locs.second - m_branches[i].y_locs.first) > 1)
        {
            Labels += "rlabel ";
            Labels += "metal2 ";
            Labels += m_branches[i].rect.makeBoundingBox();
            Labels += " 0 ";
            Labels += to_string(netID);
            Labels += "\n";
        }
    }
    return Labels;
}

string MNet::makeMetal2()
{
    string netTrunks;

    for (auto &i : m_trunks)
    {
        netTrunks += "rect";
        MagRect T1(i.x_locs.first, i.y, i.x_locs.second + 1, i.y + 1);
        i.rect = T1;
        netTrunks += T1.makeBoundingBox();
        netTrunks += "\n";
    }

    return netTrunks;
}

string MNet::makeMetal2Contact()
{
    string netContacts;

    for (auto &i : m_contacts)
    {
        netContacts += "rect";
        MagRect T1(i.x, i.y, i.x + 1, i.y + 1);
        netContacts += T1.makeBoundingBox();
        netContacts += "\n";
    }

    return netContacts;
}

string MagRect::makeBoundingBox()
{
    string outputString;
    outputString += ' ' + to_string(xbot);
    outputString += ' ' + to_string(ybot);
    outputString += ' ' + to_string(xtop);
    outputString += ' ' + to_string(ytop);

    return outputString;
}

string MagRect::makeRect()
{
    return "rect" + makeBoundingBox() + '\n';
}

string MagRect::makeRlabel(string layer, int text)
{
    return "rlabel " + layer + makeBoundingBox() + " 0 " + to_string(text) + '\n';
}
string MagRect::makeRlabel(string layer, string text)
{
    return "rlabel " + layer + makeBoundingBox() + " 0 " + text + '\n';
}