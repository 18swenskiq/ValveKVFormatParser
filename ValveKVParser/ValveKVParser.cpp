#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <chrono>

int main(int argc, char* argv[])
{
    enum KVToken
    {
        T_ObjectHeader,
        T_ObjectStart,
        T_KeyName,
        T_KeyValue,
        T_ObjectEnd,
        T_CommentInline,
        T_CommentOwnline,
        T_EmptyLine
    };

    // Exit if no file name provided
    if (argc < 2)
    {
        std::cout << "Error: no file name provided" << std::endl;
        return 1;
    }

    // Attempt to open the file name in the argument
    std::fstream myfile;
    myfile.open(argv[1], std::ios::in);
    if (!myfile.is_open())
    {
        std::cout << "Error: file could not be opened" << std::endl;
        return 1;
    }
    
    auto start = std::chrono::high_resolution_clock::now();

    // Get all lines from file and set them up into a vector
    char buf[256];
    std::vector<std::string> filestrings;
    while (myfile.getline(buf, 256))
    {
        filestrings.push_back(buf);
    }
    myfile.close();

    // Stack is for holding tokens as we go through the strings
    std::queue<std::pair<enum KVToken, std::string>*> tokenstack;

    // Iterate through the vector, then parse every string and put its information on the stack
    for (int i = 0; i < filestrings.size(); i++)
    {
        std::string currentstring = "";
        std::pair<enum KVToken, std::string>* temppair;

        std::string thisline = filestrings.at(i);
        if (thisline == "")
        {
            temppair = new std::pair<enum KVToken, std::string>;
            temppair->first = T_EmptyLine;
            temppair->second = "";
            tokenstack.push(temppair);
            continue;
        }
        if (i == 24)
        {
           //std::cout << "TESTTESTESTEST" << std::endl;
        }

        bool hitfirstinfo = false;
        bool inside_quotes = false;
        bool addedkey = false;
        bool commentmode = false;

        if (thisline.at(0) == '/' && thisline.at(1) == '/')
        {
            commentmode = true; 
        }

        for (int j = 0; j < thisline.size(); j++)
        {
            if (commentmode) goto skipswitch;
            switch (thisline.at(j))
            {
                // If there's a quote then we'll just swap the quote state
                case '\"':
                    if (!inside_quotes) inside_quotes = true;
                    else inside_quotes = false;
                    if (j + 1 == thisline.size())
                    {
                        if (currentstring == "") break;
                        temppair = new std::pair<enum KVToken, std::string>;
                        if (addedkey)
                        {
                            temppair->first = T_KeyValue;
                            temppair->second = currentstring;
                            tokenstack.push(temppair);
                            currentstring = "";
                            break;
                        }
                        else
                        {
                            temppair->first = T_ObjectHeader;
                            temppair->second = currentstring;
                            tokenstack.push(temppair);
                            currentstring = "";
                            break;
                        }
                    }
                    if (thisline.at(j + 1) == ' ' || thisline.at(j + 1) == '\t')
                    {
                        temppair = new std::pair<enum KVToken, std::string>;
                        temppair->first = T_KeyName;
                        temppair->second = currentstring;
                        tokenstack.push(temppair);
                        currentstring = "";
                        addedkey = true;
                        break;
                    }
                    break;

                // If we have whitespace, either ignore it, or make sure our objects are split properly
                case ' ':
                case '\t':
                    if (inside_quotes)
                    {
                        currentstring.push_back(thisline.at(j));
                        break;
                    }
                    if (j + 1 == thisline.size())
                    {
                        if (currentstring == "") break;
                        temppair = new std::pair<enum KVToken, std::string>;
                        if (addedkey)
                        {
                            temppair->first = T_KeyValue;
                            temppair->second = currentstring;
                            tokenstack.push(temppair);
                        }
                        else
                        {
                            temppair->first = T_ObjectHeader;
                            temppair->second = currentstring;
                            tokenstack.push(temppair);
                        }
                    }
                    if (!hitfirstinfo) break;
                    if (inside_quotes) break;
                    if (currentstring == "") break;
                    temppair = new std::pair<enum KVToken, std::string>;
                    if (!addedkey) temppair->first = T_KeyName;
                    else temppair->first = T_KeyValue;
                    temppair->second = currentstring;
                    tokenstack.push(temppair);
                    addedkey = true;
                    currentstring = "";
                    break;

                // Take note of { and }
                case '{':
                    hitfirstinfo = true;
                    temppair = new std::pair<enum KVToken, std::string>;
                    temppair->first = T_ObjectStart;
                    temppair->second = "{";
                    tokenstack.push(temppair);
                    break;
                case '}':
                    hitfirstinfo = true;
                    temppair = new std::pair<enum KVToken, std::string>;
                    temppair->first = T_ObjectEnd;
                    temppair->second = "}";
                    tokenstack.push(temppair);
                    break;

                case '/':
                    hitfirstinfo = true;
                    if (thisline.at(j + 1) == '/')
                    {
                        commentmode = true;
                        j++;
                        currentstring.append("//");
                    }
                    else
                    {
                        currentstring.push_back(thisline.at(j));
                        if (j + 1 == thisline.size())
                        {
                            if (currentstring == "") break;
                            temppair = new std::pair<enum KVToken, std::string>;
                            if (addedkey)
                            {
                                temppair->first = T_CommentInline;
                                temppair->second = currentstring;
                                tokenstack.push(temppair);
                            }
                            else
                            {
                                temppair->first = T_CommentOwnline;
                                temppair->second = currentstring;
                                tokenstack.push(temppair);
                            }
                        }
                        break;
                    }
                    break;

                // If it's not a special character, just add it to the current string
                default:
                    skipswitch:
                    hitfirstinfo = true;
                    currentstring.push_back(thisline.at(j));
                    if (j + 1 == thisline.size())
                    {
                        temppair = new std::pair<enum KVToken, std::string>;
                        if (addedkey)
                        {
                            if (commentmode) temppair->first = T_CommentInline;
                            else temppair->first = T_KeyValue;
                            temppair->second = currentstring;
                            tokenstack.push(temppair);                     
                        }
                        else
                        {
                            if (commentmode) temppair->first = T_CommentOwnline;
                            else temppair->first = T_ObjectHeader;
                            temppair->second = currentstring;
                            tokenstack.push(temppair);
                        }
                    }
                    break;
            }
        }
    }
    int stacksize = tokenstack.size();
    int indentationlevel = 0;
    for (int x = 0; x < stacksize; x++)
    {
        std::pair<enum KVToken, std::string>* tpair = tokenstack.front();

        switch (tpair->first)
        {
            case T_CommentInline:
                std::cout << " " << tpair->second << std::endl;
                break;
            case T_CommentOwnline:
                for (int i = 0; i < indentationlevel; i++) std::cout << "\t";
                std::cout << tpair->second << std::endl;
                break;
            case T_ObjectStart:
                for (int i = 0; i < indentationlevel; i++) std::cout << "\t";
                std::cout << "{";
                indentationlevel++;
                break;
            case T_ObjectEnd:
                indentationlevel--;
                for (int i = 0; i < indentationlevel; i++) std::cout << "\t";
                std::cout << "}";
                break;
            case T_ObjectHeader:
            case T_KeyName:
                for (int i = 0; i < indentationlevel; i++) std::cout << "\t";
                std::cout << tpair->second;
                break;
            case T_KeyValue:
                std::cout << "  " << tpair->second;
                break;
        }

        tokenstack.pop();
        if (x + 1 == stacksize) break;
        if (tokenstack.front()->first == T_CommentInline) continue;
        if (tokenstack.front()->first == T_KeyValue) continue;
        std::cout << std::endl;
    }

    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "\nEXECUTION TIME: " << duration.count() << std::endl;
}

// Original Algorithm timing (Release x64):
// Test 1:  19136
// Test 2:  21752
// Test 3:  21616
// Test 4:  20043
// Test 5:  19273
// Test 6:  20912
// Test 7:  20210
// Test 8:  21405
// Test 9:  21110
// Test 10: 20899
// Average: 20635.5