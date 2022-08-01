#include "chisato.h"
#include <iostream>

#include <assert.h>

enum TestEnum {
  T_A = 0,
  T_B,
  T_C,
};

static char const *get_enum_str(TestEnum te) noexcept {
  switch (te) {
    case T_A: return "A";
    case T_B: return "B";
    case T_C: return "C";
    default: return "Unknown";
  }
}
struct Config {
  std::string field1;
  std::string field2;
  std::string field3;
  int ifield = 1;
  double dfield = 1.0;
  TestEnum efield;
  TestEnum efield2;
  bool boolean = false;
};

static inline void conv_efield(StrSlice value, void *args) {
  TestEnum *config = (TestEnum*)args;
  if (value == "a")
    *config = T_A;
  else if (value == "b")
    *config = T_B;
  else if (value == "c")
    *config = T_C;
}

int main(int argc, char **argv) {
  Config config; 
  chisato::AddConfig("field1", &config.field1);
  chisato::AddConfig("field2", &config.field2);
  chisato::AddConfig("field3", &config.field3);
  chisato::AddConfig("int", &config.ifield);
  chisato::AddConfig("double", &config.dfield);
  chisato::AddConfig("enum", &config.efield, &conv_efield);
  chisato::AddConfig("enum1", [&config](StrSlice value) {
      conv_efield(value, &config.efield2);
  });
  chisato::AddConfig("bool", &config.boolean);
  
  char const *path = "./test.conf"; 
  if (argc == 2) {
    path = argv[1];
  } else if (argc > 2) {
    ::fprintf(stderr, "Usage: ./chisato_test [config file path]\n");
    return 0;
  }

  std::string errmsg;
  auto success = chisato::Parse(path, errmsg);
  if (!success) {
    ::fprintf(stderr, "Failed to parse the config: \n%s\n", errmsg.c_str());
    return 0;
  }
  
  ::printf("value1 = %s\n", config.field1.c_str());
  ::printf("value2 = %s\n", config.field2.c_str());
  ::printf("value3 = %s\n", config.field3.c_str());
  ::printf("ifield = %d\n", config.ifield);
  ::printf("dfield = %lf\n", config.dfield);
  ::printf("enum = %s\n", get_enum_str(config.efield));
  ::printf("enum1 = %s\n", get_enum_str(config.efield2));
  ::printf("bool = %d\n", (int)config.boolean);

  chisato::DebugPrint();
}
