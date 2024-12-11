#include <string.h>
#include <iomanip>
#include <iostream>
#include <sstream>
  std::string formatDoubleDigits(int dd) {
    std::ostringstream oss;

    // Format the time using setw(2) and setfill('0') for zero padding
    oss << std::setw(2) << std::setfill('0') << dd;

    return oss.str();
  }
std::string formatTime(int hours, int minutes, int seconds) {
  std::ostringstream oss;

    // Format the time using setw(2) and setfill('0') for zero padding
  oss << std::setw(2) << std::setfill('0') << hours << ":"
      << std::setw(2) << std::setfill('0') << minutes << ":"
      << std::setw(2) << std::setfill('0') << seconds;

  return oss.str();
}
std::string a_string( int value ){
  return std::to_string(value);
}

std::string a_string( long value ){
  return std::to_string(value);
}
std::string a_string( long long value ){
  return std::to_string(value);a
}
std::string a_string( unsigned value ){
  return std::to_string(value);
}
std::string a_string( unsigned long value ){
  return std::to_string(value);
}
std::string a_string( unsigned long long value ){
  return std::to_string(value);
}
std::string a_string( float value ){
  return std::to_string(value);
}
std::string a_string( double value ){
  return std::to_string(value);
}
std::string a_string( long double value ){
  return std::to_string(value);
}