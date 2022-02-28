
#pragma once

class IApp
{
public:
    static constexpr int Success = 0;
    static constexpr int Failure = -1;

    virtual int Run() = 0;
};

extern IApp& GetApp();
