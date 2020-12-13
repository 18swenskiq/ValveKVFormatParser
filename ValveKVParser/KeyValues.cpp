#include "KeyValues.h"

std::ostream& operator<<(std::ostream& os, const KeyValues& obj)
{
    auto localq = obj.tokenqueue;
    int queuesize = localq.size();
    int indentationlevel = 0;
    for (int x = 0; x < queuesize; x++)
    {
        std::pair<enum KeyValues::KVToken, std::string>* tpair = localq.front();

        switch (tpair->first)
        {
        case KeyValues::KVToken::T_CommentInline:
            os << " " << tpair->second;
            break;
        case KeyValues::KVToken::T_CommentOwnline:
            for (int i = 0; i < indentationlevel; i++) os << "\t";
            os << tpair->second;
            break;
        case KeyValues::KVToken::T_ObjectStart:
            for (int i = 0; i < indentationlevel; i++) os << "\t";
            os << "{";
            indentationlevel++;
            break;
        case KeyValues::KVToken::T_ObjectEnd:
            indentationlevel--;
            for (int i = 0; i < indentationlevel; i++) os << "\t";
            os << "}";
            break;
        case KeyValues::KVToken::T_ObjectHeader:
        case KeyValues::KVToken::T_KeyName:
            for (int i = 0; i < indentationlevel; i++) os << "\t";
            os << tpair->second;
            break;
        case KeyValues::KVToken::T_KeyValue:
            os << "  " << tpair->second;
            break;
        }

        localq.pop();
        if (x + 1 == queuesize) break;
        if (localq.front()->first == KeyValues::KVToken::T_CommentInline) continue;
        if (localq.front()->first == KeyValues::KVToken::T_KeyValue) continue;
        os << std::endl;
    }
    return os;
}

void KeyValues::PushTokenQueue(std::pair<KVToken, std::string>* newtokenpair)
{
    tokenqueue.push(newtokenpair);
}

int KeyValues::GetQueueSize()
{
    return tokenqueue.size();
}

std::pair<enum KeyValues::KVToken, std::string>* KeyValues::GetFrontValueOfQueue()
{
    return tokenqueue.front();
}

void KeyValues::PopQueue()
{
    tokenqueue.pop();
}