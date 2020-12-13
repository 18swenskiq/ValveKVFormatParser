#include "KeyValues.h"

std::ostream& operator<<(std::ostream& os, const KeyValuesQueue& obj)
{
    auto localq = obj.tokenqueue;
    int queuesize = localq.size();
    int indentationlevel = 0;
    for (int x = 0; x < queuesize; x++)
    {
        std::pair<enum KeyValuesQueue::KVToken, std::string>* tpair = localq.front();

        switch (tpair->first)
        {
        case KeyValuesQueue::KVToken::T_CommentInline:
            os << " " << tpair->second;
            break;
        case KeyValuesQueue::KVToken::T_CommentOwnline:
            for (int i = 0; i < indentationlevel; i++) os << "\t";
            os << tpair->second;
            break;
        case KeyValuesQueue::KVToken::T_ObjectStart:
            for (int i = 0; i < indentationlevel; i++) os << "\t";
            os << "{";
            indentationlevel++;
            break;
        case KeyValuesQueue::KVToken::T_ObjectEnd:
            indentationlevel--;
            for (int i = 0; i < indentationlevel; i++) os << "\t";
            os << "}";
            break;
        case KeyValuesQueue::KVToken::T_ObjectHeader:
        case KeyValuesQueue::KVToken::T_KeyName:
            for (int i = 0; i < indentationlevel; i++) os << "\t";
            os << tpair->second;
            break;
        case KeyValuesQueue::KVToken::T_KeyValue:
            os << "  " << tpair->second;
            break;
        }

        localq.pop();
        if (x + 1 == queuesize) break;
        if (localq.front()->first == KeyValuesQueue::KVToken::T_CommentInline) continue;
        if (localq.front()->first == KeyValuesQueue::KVToken::T_KeyValue) continue;
        os << std::endl;
    }
    return os;
}

void KeyValuesQueue::PushTokenQueue(std::pair<KVToken, std::string>* newtokenpair)
{
    tokenqueue.push(newtokenpair);
}

int KeyValuesQueue::GetQueueSize()
{
    return tokenqueue.size();
}

std::pair<enum KeyValuesQueue::KVToken, std::string>* KeyValuesQueue::GetFrontValueOfQueue()
{
    return tokenqueue.front();
}

void KeyValuesQueue::PopQueue()
{
    tokenqueue.pop();
}