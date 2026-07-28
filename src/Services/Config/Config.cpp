#include "Config.h"

#include "Services/Logger/Logger.h"

ConfigService Config;

//=============================================================================
// Public
//=============================================================================

bool ConfigService::begin()
{
    Log.info("Loading configuration...");

    if (!load())
    {
        Log.warning("Configuration not found or invalid");

        setDefaults();

        if (!save())
        {
            Log.error("Unable to save default configuration");
            return false;
        }
    }

    Log.info("Configuration ready");

    return true;
}

//-----------------------------------------------------------------------------

bool ConfigService::load()
{
    if (!loadJson())
    {
        return false;
    }

    if (!validate())
    {
        Log.error("Configuration validation failed");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------

bool ConfigService::save() const
{
    return saveJson();
}

//-----------------------------------------------------------------------------

void ConfigService::reset()
{
    Log.warning("Reset configuration to defaults");

    setDefaults();

    if (!save())
    {
        Log.error("Unable to save default configuration");
    }
}

//-----------------------------------------------------------------------------

AppConfig& ConfigService::data()
{
    return m_data;
}

//-----------------------------------------------------------------------------

const AppConfig& ConfigService::data() const
{
    return m_data;
}