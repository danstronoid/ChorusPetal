// PedalPCB Terrarium Header
// Copyright (C) 2020 PedalPCB.com
// http://www.pedalpcb.com

namespace terrarium
{

class Terrarium
{
public:
    enum Sw
    {
        FS_1 = 4,
        FS_2 = 5,
        S_1 = 2,
        S_2 = 1,
        S_3 = 0,
        S_4 = 6
    };

    enum Knob
    {
        K_1 = 0,
        K_2 = 2,
        K_3 = 4,
        K_4 = 1,
        K_5 = 3,
        K_6 = 5
    };
    
    enum LED
    {
        L_1 = 22,
        L_2 = 23
    };
};

}