#include "inc/include.h"

namespace tools
{
    void replace_slash_with_underscore(const char *path, char *output)
    {
        if (path == NULL || output == NULL)
        {
            return;
        }

        while (*path)
        {
            if (*path == '/')
            {
                *output = '_';
            }
            else
            {
                *output = *path;
            }
            path++;
            output++;
        }
        *output = '\0'; // Add null-terminator to the end of the modified path
    }

    char integer2char(int digit)
    {
        // Ensure the input is a single-digit integer
        if (digit >= 0 && digit <= 9)
        {
            return digit + '0';
        }
        else
        {
            // Handle error or return a special character for out-of-range input
            return '\0';
        }
    }
}
