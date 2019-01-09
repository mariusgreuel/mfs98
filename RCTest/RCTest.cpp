//
// rcread : displays the rc test box
//

#include "stdafx.h"
#include "..\rccom\rccom.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    CRCBox          RCBox;

    RCBox.OpenDevice();
    RCBox.Initialize();
    RCBox.Dialog(NULL, 0);
    RCBox.Terminate();
    RCBox.CloseDevice();

    return TRUE;
}



