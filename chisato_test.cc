#include "chisato.h"

#include <assert.h>

int main() {
  auto success = chisato::Parse("./test.conf");
  
  assert(success);
  
  auto value1 = chisato::GetField("field1");
  auto value2 = chisato::GetField("field2");
  auto value3 = chisato::GetField("field3");

  ::printf("value1 = %s\n", value1.c_str());
  ::printf("value2 = %s\n", value2.c_str());
  ::printf("value3 = %s\n", value3.c_str());

}
