#include "chisato.h"

#define CHISATO_DEBUG__
#ifdef CHISATO_DEBUG__
#include <iostream>
#endif

#define MAX_LINE_LEN 4096

namespace chisato {

/* Global structure definition */
ConfigMap config_map;

/* Read a line from file */
static inline bool ReadLine(FILE* file, std::string& line, const bool need_newline=true) {
  char buf[MAX_LINE_LEN];
  char* ret = NULL;
  size_t n = 0;

  line.clear();

  do {
    ret = ::fgets(buf, sizeof buf, file);

    if (ret == NULL) {
      if (::ferror(file)) {
        if (errno == EINTR) {
          continue;
        }
      }

      return false;
    }

    assert(ret == buf);
    n = strlen(buf);

    if (n >= 1 && buf[n-1] == '\n') {
      if (!need_newline) {
        if (n >= 2 && buf[n-2] == '\r') {
          line.append(buf, n - 2);
        }
        else {
          line.append(buf, n - 1);
        }
      } else {
        line.append(buf, n);
      }
      break;
    }

    line.append(buf, n);

  } while (n == (sizeof(buf) - 1));

  return true;
}

#define STR2INT(ivar) \
  char *end = nullptr; \
  long res = strtol(value.data(), &end, 10); \
  if (res == 0 && end == value.data()) { \
    errmsg += "INVALID integer value of "; \
    errmsg.append(field.data(), field.size()); \
    return false; \
  } \
  (ivar) = res;

bool Parse(char const *path, std::string &errmsg) {
  FILE* file = ::fopen(path, "r");
  errmsg.clear();

  if (!file) {
    errmsg += "File error: Failed to open the config file: ";
    errmsg += path;
    errmsg += "\nError message: ";
    errmsg += ::strerror(errno);
    return false;
  }

  std::string line;
  
  size_t lineno = 0; 
  /* FIXME Use state machine? */
  while (ReadLine(file, line, false)) {
    ++lineno;
    /* Get the comment position */
    size_t end_pos = line.rfind('#');
    if (end_pos != std::string::npos)
      line.erase(end_pos, line.size());
    
    /** Get the colon position */ 
    size_t start_pos = 0;
    end_pos = line.find(':');
    if (end_pos != std::string::npos) {
      // Find the first position whose character is not space 
      for (; line[start_pos] == ' ' && start_pos != end_pos; ++start_pos);

      StrSlice field(line.data() + start_pos,
                     end_pos - start_pos);
      
      // Get the value slice
      for (start_pos = end_pos+1; line[start_pos] == ' ' && start_pos != line.size(); ++start_pos);
      
      StrSlice value(line.data() + start_pos,
                     line.size() - start_pos);
      
      auto iter = config_map.find(field);
      ConfigData *data = nullptr;
      if (iter != config_map.end()) {
        data = &iter->second;
        
        switch (data->set_method) {
          case ConfigData::CD_RAW: {
            switch (data->type) {
              case CT_INT: {
                STR2INT(*(int*)(data->data))
              }
              break;

              case CT_LONG: {
                STR2INT(*(long*)(data->data))
              }
              break;

              case CT_FLOAT: {
                char *end = nullptr;
                double res = strtod(value.data(), &end);
                if (res == 0 && end == value.data()) {
                  errmsg += "INVALID floating-point value of ";
                  errmsg.append(field.data(), field.size());
                }
                *(double*)(data->data) = res;
              }
              break;

              case CT_STR: {
                ((std::string*)(data->data))->assign(value.data(), value.size());
              }
              break;

              case CT_USR_DEF: {
                assert(false && "Set callback to handle user-defined config type");
              }
              break;
            }
          }
          break;

          case ConfigData::CD_C_CALLBACK: {
            if (data->config_cs.config_cb)
              data->config_cs.config_cb(value, data->config_cs.args);
          }
          break;

          case ConfigData::CD_CPP_CALLBACK: {
            if (data->config_fn)
              data->config_fn(value);
          }
          break;
        } 
      } else {
        errmsg += path;
        errmsg += ':';
        errmsg += std::to_string(lineno);
        errmsg += "\nField error: Unknown field name -- ";
        errmsg += field.toString();
        return false;
      }
    } else if (!line.empty()) {
      errmsg += path;
      errmsg += ':';
      errmsg += std::to_string(lineno);
      errmsg += "\nSyntax error: No colon to split field and value";
      return false;
    }
  }

  if (::feof(file) == 0) {
    errmsg += "File error: Failed to read one line from ";
    errmsg += path;
    errmsg += "\nError Message: ";
    errmsg += strerror(errno);
    return false;
  }

  return true;
}

void DebugPrint() {
#ifdef _CHISATO_DEBUG__
  for (auto const &config : config_map) {
    std::cout << std::string(config.first.data(), config.first.size()) << "\n";
  }
#endif
}

} // chisato
