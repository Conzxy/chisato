#include "chisato.h"

#include <assert.h>

int main() {
  std::string errmsg;
  auto success = chisato::Parse("./test.conf", errmsg);
  
  if (!success) {
    ::fprintf(stderr, "Failed to parse the config: %s\n", errmsg.c_str());
    return 0;
  }
  auto value1 = chisato::GetField("field1");
  auto value2 = chisato::GetField("field2");
  auto value3 = chisato::GetField("field3");

  ::printf("value1 = %s\n", value1.c_str());
  ::printf("value2 = %s\n", value2.c_str());
  ::printf("value3 = %s\n", value3.c_str());

}
