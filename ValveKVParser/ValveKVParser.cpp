#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <chrono>
#include "KeyValues.h"

int main(int argc, char* argv[])
{

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

    KeyValues *kvstorage = new KeyValues();

    // Iterate through the vector, then parse every string and put its information on the stack
    for (int i = 0; i < filestrings.size(); i++)
    {
        std::string currentstring = "";
        std::pair<enum KeyValues::KVToken, std::string>* temppair;

        std::string thisline = filestrings.at(i);
        if (thisline == "")
        {
            temppair = new std::pair<enum KeyValues::KVToken, std::string>;
            temppair->first = KeyValues::KVToken::T_EmptyLine;
            temppair->second = "";
            kvstorage->PushTokenQueue(temppair);
            continue;
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
                        temppair = new std::pair<enum KeyValues::KVToken, std::string>;
                        if (addedkey) temppair->first = KeyValues::KVToken::T_KeyValue;
                        else          temppair->first = KeyValues::KVToken::T_ObjectHeader;
                        temppair->second = currentstring;
                        kvstorage->PushTokenQueue(temppair);
                        currentstring = "";
                        break;
                    }
                    if (thisline.at(j + 1) == ' ' || thisline.at(j + 1) == '\t')
                    {
                        temppair = new std::pair<enum KeyValues::KVToken, std::string>;
                        temppair->first = KeyValues::KVToken::T_KeyName;
                        temppair->second = currentstring;
                        kvstorage->PushTokenQueue(temppair);
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
                        temppair = new std::pair<enum KeyValues::KVToken, std::string>;
                        if (addedkey) temppair->first = KeyValues::KVToken::T_KeyValue;
                        else          temppair->first = KeyValues::KVToken::T_ObjectHeader;
                        temppair->second = currentstring;
                        kvstorage->PushTokenQueue(temppair);
                    }
                    if (!hitfirstinfo) break;
                    if (inside_quotes) break;
                    if (currentstring == "") break;
                    temppair = new std::pair<enum KeyValues::KVToken, std::string>;
                    if (!addedkey) temppair->first = KeyValues::KVToken::T_KeyName;
                    else temppair->first = KeyValues::KVToken::T_KeyValue;
                    temppair->second = currentstring;
                    kvstorage->PushTokenQueue(temppair);
                    addedkey = true;
                    currentstring = "";
                    break;

                // Take note of { and }
                case '{':
                    hitfirstinfo = true;
                    temppair = new std::pair<enum KeyValues::KVToken, std::string>;
                    temppair->first = KeyValues::KVToken::T_ObjectStart;
                    temppair->second = "{";
                    kvstorage->PushTokenQueue(temppair);
                    break;
                case '}':
                    hitfirstinfo = true;
                    temppair = new std::pair<enum KeyValues::KVToken, std::string>;
                    temppair->first = KeyValues::KVToken::T_ObjectEnd;
                    temppair->second = "}";
                    kvstorage->PushTokenQueue(temppair);
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
                            temppair = new std::pair<enum KeyValues::KVToken, std::string>;
                            if (addedkey) temppair->first = KeyValues::KVToken::T_CommentInline;
                            else temppair->first = KeyValues::KVToken::T_CommentOwnline;                              
                            temppair->second = currentstring;
                            kvstorage->PushTokenQueue(temppair);
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
                        temppair = new std::pair<enum KeyValues::KVToken, std::string>;
                        if (addedkey)
                        {
                            if (commentmode) temppair->first = KeyValues::KVToken::T_CommentInline;
                            else temppair->first = KeyValues::KVToken::T_KeyValue;
                        }
                        else
                        {
                            if (commentmode) temppair->first = KeyValues::KVToken::T_CommentOwnline;
                            else temppair->first = KeyValues::KVToken::T_ObjectHeader;
                        }
                        temppair->second = currentstring;
                        kvstorage->PushTokenQueue(temppair);
                    }
                    break;
            }
        }
    }
    std::cout << *kvstorage << std::endl;
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