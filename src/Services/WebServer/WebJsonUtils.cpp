#include "WebJsonUtils.h"

#include <cstdio>

namespace WebJsonUtils
{

void sendEscaped(
    ESP8266WebServer& server,
    const char* value)
{
    if (value == nullptr)
    {
        return;
    }

    char out[7] {};

    for (const char* p = value; *p != '\0'; ++p)
    {
        const unsigned char ch =
            static_cast<unsigned char>(*p);

        switch (ch)
        {
            case '"':
                server.sendContent("\\\"");
                break;

            case '\\':
                server.sendContent("\\\\");
                break;

            case '\b':
                server.sendContent("\\b");
                break;

            case '\f':
                server.sendContent("\\f");
                break;

            case '\n':
                server.sendContent("\\n");
                break;

            case '\r':
                server.sendContent("\\r");
                break;

            case '\t':
                server.sendContent("\\t");
                break;

            default:
                if (ch < 0x20)
                {
                    snprintf(
                        out,
                        sizeof(out),
                        "\\u%04x",
                        ch);

                    server.sendContent(out);
                }
                else
                {
                    out[0] =
                        static_cast<char>(ch);
                    out[1] =
                        '\0';

                    server.sendContent(out);
                }

                break;
        }
    }
}

}
