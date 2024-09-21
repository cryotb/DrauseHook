#pragma once

class console_spinner
{
public:
  console_spinner() = delete;
  console_spinner(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int size = vsnprintf(NULL, 0, fmt, args) + 1; 
    char* buf = new char[size];
    vsnprintf(buf, size, fmt, args); 
    va_end(args);

    _text = buf;
    delete[] buf; 
    _active = true;
    std::thread(&console_spinner::th_think, this).detach();
  }

  ~console_spinner() {
    stop();
  }

  void stop() {
    _active = false;
  }

  void th_think() {
    const char spinner[] = "|/-\\";
    int i = 0;

    while (_active) {
      printf(" \r%c  %s", spinner[i], _text.c_str());
      fflush(stdout);
      i = (i + 1) % 4;
      usleep(100000);
    }

    printf("\n");
  }

private:
  bool _active;
  std::string _text;
};
