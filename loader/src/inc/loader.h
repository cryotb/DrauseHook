#pragma once

class Loader
{
public:
    virtual void start(int argc, char* argv[]);
    virtual bool setup_netsdk();

    virtual std::string get_argument_param(const std::string &str);
    virtual void parse_argument(const std::string &str);

    virtual bool authenticate();
private:
};

extern Loader* gLoader;
