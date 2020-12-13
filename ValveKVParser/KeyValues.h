#pragma once
#include <ostream>
#include <queue>
#include <string>

class KeyValues
{
public:
	enum KVToken
    {
        T_ObjectHeader, T_ObjectStart, T_KeyName, T_KeyValue,
        T_ObjectEnd, T_CommentInline, T_CommentOwnline, T_EmptyLine
    };

public:
    friend std::ostream& operator<<(std::ostream& os, const KeyValues& obj);
    void PushTokenQueue(std::pair<KVToken, std::string>* newtokenpair);
    int GetQueueSize();
    std::pair<enum KeyValues::KVToken, std::string>* GetFrontValueOfQueue();
    void PopQueue();

private:
    std::queue<std::pair<KVToken, std::string>*> tokenqueue;

};