#include <iostream>
#include <fstream>
#include <cstdlib>

#include "Magic.hpp"

using namespace std;

// Magic Intro Function
Magic::Magic(Placement place, Routing route)
{
    //CreateLayout(route, place);
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
    /*
    const auto now = std::chrono::system_clock::now();
    const auto epoch = now.time_since_epoch();
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(epoch);
    return seconds.count();
    */

    return 1;
}

// Function to output the final Magic File
void Magic::Output(string szDirectory, string szFileName)
{
    OutputStandardCell(szDirectory);

    OutputFeedCell(szDirectory);

    //OutputLayout(szDirectory, szFileName);

    return;
}

// Create an object that has all the necessary info to create the Magic File
void Magic::CreateLayout(Routing route, Placement place)
{
    vector<vector<MCell>> MagCells;
    vector<vector<MNet>> MagNets;

    int totalTracks = 0;
    int y = 0;
    for (int i = 0; i < route.m_channelCount; i++)
    {
        int x = 0;
        totalTracks += route.m_channels[i].m_tracks.size();
        y = i * 6 + totalTracks * 2 + 1;
        vector<int> colsTransformation(route.m_colCount);
        for (int j = 0; j < route.m_colCount; j++)
        {
            string cellIDL = "";
            string cellID = route.m_TopRow[i].RowCells[j].Term.cellId;
            Cell cell = place.m_netlist.m_cells[cellID];

            if (j > 0)
            {
                cellIDL = route.m_TopRow[i].RowCells[j - 1].Term.cellId;
                if (cellID != cellIDL)
                {
                    if (cell.isFeedthrough())
                        x += 3;
                    else
                        x += 7;
                }
                else
                {
                    x += 3;
                }
            }

            if (cellID != cellIDL)
            {
                MCell mcell(x, y, cell.isFeedthrough(), cellID, cell.m_orientation);
                mcell.transform();

                MagCells[i].push_back(mcell);
            }

            colsTransformation[j] = x;
        }

        // go thru all nets
        for (unsigned int l = 0; l < route.m_Spans[i].size(); l++)
        {
            auto &j = route.m_Spans[i][l];
            //go thru all tracks for that net
            for (unsigned int k = 0; k < j.ranges.size(); k++)
            {
                MNet mnet;
                mnet.netID = j.net;

                MTrunk m_trunk;
                int left = j.ranges[k].first;
                int right = j.ranges[k].second;
                int newLeft = colsTransformation[left];
                int newRight = colsTransformation[right];
                m_trunk.x_locs = {newLeft, newRight};

                m_trunk.y = i * 6 + j.n_tracks[k] * 2;
                mnet.m_trunk = m_trunk;

                MBranch mbranchL;
                MBranch mbranchR;
                mbranchL.x = newLeft;
                mbranchR.x = newRight;

                int botNetID = route.m_BotRow[i].RowCells[left].NetID;
                int topNetID = route.m_TopRow[i].RowCells[left].NetID;
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
                MagNets[i].push_back(mnet);
            }
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

    // Do the important bit here

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
    MagRect T1(1, 0, 2, 1);
    MagRect T3(1, 5, 2, 6);
    vector<MagRect> Terms = {T1, T3};

    // Metal Rectangles
    outputStream << "<< metal1 >>" << endl;
    string type = "rect";

    for (auto &i : Terms)
        i.outputRect(outputStream, type);

    // Label Rectangles
    outputStream << "<< labels >>" << endl;
    type = "rlabel metal1";

    for (unsigned int i = 0; i < Terms.size(); i++)
        Terms[i].outputLabel(outputStream, type, i);

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
    MagRect T1(1, 5, 2, 6);
    MagRect T2(4, 5, 5, 6);
    MagRect T3(1, 0, 2, 1);
    MagRect T4(4, 0, 5, 1);
    vector<MagRect> Terms = {T1, T2, T3, T4};

    // Metal Rectangles
    outputStream << "<< metal1 >>" << endl;
    string type = "rect";

    for (auto &i : Terms)
        i.outputRect(outputStream, type);

    // Label Rectangles
    outputStream << "<< labels >>" << endl;
    type = "rlabel metal1";

    for (unsigned int i = 0; i < Terms.size(); i++)
        Terms[i].outputLabel(outputStream, type, i);

    // Exit and close
    Footer(outputStream);
    outputStream.close();

    return;
}

/*
Magic uses its own internal ASCII format for storing cells in disk files. Each cell name is stored in its
own file, named name.mag.
The first line in a .mag file is the string

    magic

to identify this as a Magic file.

The next line is optional and is used to identify the technology in which a cell was designed. If present,
it should be of the form

    tech techname

If absent, the technology defaults to a system-wide standard, currently nmos.
The next line is also optional and gives a timestamp for the cell. The line is of the format

    timestamp stamp

where stamp is a number of seconds since 00:00 GMT January 1, 1970 (i.e, the Unix time returned by
the library function time()). It should be the last time this cell or any of its children changed. The
timestamp is used to detect when a child is edited outside the context of its parent (the parent stores the
last timestamp it saw for each of its children; see below). When this occurs, the design-rule checker
must recheck the entire area of the child for subcell interaction errors. If this field is omitted in a cell,
Magic supplies a default value that forces the rechecks.

Next come lines describing the contents of the cell. There are three kinds of groups of lines, describing
mask rectangles, subcell uses, and labels. Each group must appear contiguously in the file, but the order
between groups is arbitrary.

Each group of mask rectangles is headed with a line of the format

    << layer >>

where layer is a layername known in the current technology (see the tech line above). Each line after
this header has the format

    rect xbot ybot xtop ytop

where (xbot, ybot) is the lower-left corner of the rectangle in Magic (lambda) coordinates, and
(xtop, ytop) is the upper-right corner. Degenerate rectangles are not allowed; xbot must be strictly less
than xtop, and ybot strictly less than ytop. The smallest legal value of xbot or ybot is -67108858, and
the largest legal value for xtop or ytop is 67108858. Values that approach these limits (within a factor
of 100 or 1000) may cause numerical overflows in Magic even though they are not strictly illegal. We
recommend using coordinates around zero as much as possible.

Rectangles should be non-overlapping, although this is not essential. They should also already have
been merged into maximal horizontal strips (the neighbor to the right or left of a rectangle should not be
of the same type), but this is also not essential.

The second kind of line group describes a single cell use. Each cell use group is of the following form:

    use filename use-id
    array xlo xhi xsep ylo yhi ysep
    timestamp stamp
    transform a b c d e f
    box xbot ybot xtop ytop

A group specifies a single instance of the cell named filename, with instance-identifier use-id. The
instance-identifier use-id must be unique among all cells used by this .mag file. If use-id is not
specified, a unique one is generated automatically.

The array line need only be present if the cell is an array. If so, the X indices run from xlo to xhi
inclusive, with elements being separated from each other in the X dimension by xsep lambda. The Y
indices run from ylo to yhi inclusive, with elements being separated from each other in the Y dimension
by ysep lambda. If xlo and xhi are equal, xsep is ignored; similarly if ylo and yhi are equal, ysep is
ignored.

The timestamp line is optional; if present, it gives the last time this cell was aware that the child
filename changed. If there is no timestamp line, a timestamp of 0 is assumed. When the subcell is
read in, this value is compared to the actual value at the beginning of the child cell. If there is a difference,
the ��timestamp mismatch�� message is printed, and Magic rechecks design-rules around the child.

The transform line gives the geometric transform from coordinates of the child filename into coordinates
of the cell being read. The six integers a, b, c, d, e, and f are part of the following transformation
matrix, which is used to postmultiply all coordinates in the child filename whenever their coordinates in
the parent are required:

    a d 0
    b e 0
    c f 1

Finally, box gives an estimate of the bounding box of cell filename (covering all the elements of the
array if an array line was present), in coordinates of the cell being read.
The third kind of line group in a .mag file is a list of labels. It begins with the line

    << labels >>

and is followed by zero or more lines of the following form:

    rlabel layer xbot ybot xtop ytop position text

Here layer is the name of one of the layers specified in the technology file for this cell. The label is
attached to material of this type. Layer may be space, in which case the label is not considered to be
attached to any layer.

Labels are rectangular. The lower-left corner of the label (the part attached to any geometry if layer is
non-space) is at (xbot, ybot), and the upper-right corner at (xtop, ytop). The width of the rectangle or its
height may be zero. In fact, most labels in Magic have a lower-left equal to their upper right.
The text of the label, text, may be any sequence of characters not including a newline. This text is
located at one of nine possible orientations relative to the center of the label�s rectangle. Position is an
integer between 0 and 8, each of which corresponds to a different orientation:

    0 center
    1 north
    2 northeast
    3 east
    4 southeast
    5 south
    6 southwest
    7 west
    8 northwest

A .mag file is terminated by the line

    << end >>

Everything following this line is ignored.

Any line beginning with a pound sigh (��#��) is considered to be a comment and ignored. Beware, however,
that these comments are discarded by Magic when it reads a cell, so if that cell is written again by
Magic, the comments will be lost.



NOTE FOR PROGRAMS THAT GENERATE MAGIC FILES
Magic�s incremental design rule checker expects that every cell is either completely checked, or contains
information to tell the checker which areas of the cell have yet to be examined for design-rule violations.
To make sure that the design-rule checker verifies all the material that has been generated for a
cell, programs that generate .mag files should place the following rectangle in each file:

    << checkpaint >>
    rect xbot ybot xtop ytop

This rectangle may appear anywhere a list of rectangles is allowed; immediately following the timestamp
line at the beginning of a .mag file is a good place. The coordinates xbot etc. should be large
enough to completely cover anything in the cell, and must surround all this material by at least one
lambda. Be careful, however, not to make this area too ridiculously large. For example, if you use the
maximum and minimum legal tile coordinates, it will take the design-rule checker an extremely long
time to recheck the area.


EXAMPLE

    magic
    tech scmos
    timestamp
    << pdiffusion >>
    rect -59 -53 -58 -52
    rect -57 -53 -56 -52
    << polysilicon >>
    rect -58 -54 -57 -53
    rect -58 -53 -57 -52
    << labels >>
    rlabel pdiffusion -55 -9 -55 -9 0 cellno=1
    rlabel pdiffusion -55 -9 -55 -9 0 cellno=1
    rlabel pdiffusion -35 -9 -35 -9 0 cellno=2
    rlabel pdiffusion -35 -9 -35 -9 0 cellno=2
    << metal1 >>
    rect -61 -55 -60 -54
    rect -60 -55 -59 -54
    << metal2 >>
    rect -32 -7 -31 -6
    rect -53 -6 -52 -5
    << end >>
    3.99seconds.

*/