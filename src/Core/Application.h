#pragma once

#include "Models/ApiStatusData.h"

//=============================================================================
// Application
//=============================================================================

class Application
{
public:
    bool begin();

    void loop();

    [[nodiscard]]
    ApiStatusData status() const;
};

//=============================================================================

extern Application App;
