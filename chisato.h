#ifndef _CHISATO_CHISATO_H_
#define _CHISATO_CHISATO_H_

#include <string.h>         // memcmp
#include <assert.h>
#include <stdint.h>

#include <functional>       // std::function
#include <string>
#include <unordered_map>

#include "str_slice.h"

namespace chisato {

/** 
 * Support integer, floating-point, and string
 */
enum ConfigType : uint8_t {
  CT_INT = 0, /** integer */
  CT_LONG,    /** long    */
  CT_FLOAT,   /** floating-point */
  CT_STR,     /** string */
  CT_BOOL,    /** boolean */
  CT_USR_DEF, /** user-defined type */
};

/* Handle user-defined config type field
 * void* is the generic parameter */
typedef void(*ConfigCallback)(StrSlice, void*);
/* lambda or std::bind can capture arguments */
typedef std::function<void(StrSlice)> ConfigFunction;

/** Callback with generic parameters */
struct ConfigClosure {
  ConfigCallback config_cb = nullptr;
  void *args = nullptr;
};

/** Config metadata */
struct ConfigData {
  ConfigType type;           /** interpret data */

  enum : uint8_t {
    CD_RAW = 0,      /** predefined data */
    CD_C_CALLBACK,   /** C callback */
    CD_CPP_CALLBACK, /** Cpp callback */
  } set_method;
  
  /* Decrease the space usage */
  union {
    void *data;                /** Binding the field data */
    ConfigClosure config_cs;   /** C style callback */
    ConfigFunction config_fn;  /** Cpp style callback */
  };

  ConfigData(ConfigType t, void *d)
    : type(t)
    , set_method(CD_RAW)
    , data(d)
  {
  }

  ConfigData(ConfigType t, ConfigClosure cc)
    : type(t)
    , set_method(CD_C_CALLBACK)
    , config_cs(cc)
  {
  }
  
  ConfigData(ConfigType t, ConfigFunction fn)
    : type(t)
    , set_method(CD_CPP_CALLBACK)
    , config_fn(fn)
  {
  }

  ConfigData(ConfigData && other) noexcept 
    : type(other.type)
    , set_method(other.set_method)
  {
    switch (other.set_method) {
      case CD_RAW:
        data = other.data;
        break;
      case CD_C_CALLBACK:
        config_cs = other.config_cs;
        break;
      case CD_CPP_CALLBACK:
        config_fn = std::move(other.config_fn);
        break;
    }
  }
  
  ~ConfigData() noexcept {
    if (set_method == CD_CPP_CALLBACK) {
      config_fn.~ConfigFunction();
    }
  }
};

namespace detail {

struct StrSliceHash {
  size_t operator()(StrSlice str) const noexcept {
    size_t h = 5321;
    for (size_t i = 0; i < str.size(); ++i) {
      h = (h << 5) + h + str[i];
    }

    return h;
  }
};

} // detail

using ConfigMap = std::unordered_map<StrSlice, ConfigData, detail::StrSliceHash>;

/** <Config name> -> <Config metadata> */
extern ConfigMap  config_map;


/** Add config */
inline void AddConfig_(char const *field, ConfigData data) {
  config_map.emplace(field, std::move(data));
}

inline void AddConfig(char const *field, std::string *str) {
  AddConfig_(field, ConfigData{ CT_STR, str });
}

inline void AddConfig(char const *field, int *i) {
  AddConfig_(field, ConfigData{ CT_INT, i });
}

inline void AddConfig(char const *field, long *l) {
  AddConfig_(field, ConfigData{ CT_LONG, l });
}

inline void AddConfig(char const *field, double * d) {
  AddConfig_(field, ConfigData{ CT_FLOAT, d });
}

inline void AddConfig(char const *field, bool *b) {
  AddConfig_(field, ConfigData{ CT_BOOL, b });
}

/**
 * User-defined config type 
 * Callback is C style 
 * \param field Config field name
 * \param args Generic arguments of \p cb
 * \param cb Callback to handle \p field
 */
inline void AddConfig(char const *field, void *args, ConfigCallback cb) {
  config_map.emplace(field, ConfigData {
    CT_USR_DEF,
    ConfigClosure {
      .config_cb = cb,
      .args = args,
    }
  });
}

/* User-defined config type
 * Callback style is std::function<>
 */
inline void AddConfig(char const *field, ConfigFunction cb) {
  config_map.emplace(field, ConfigData {
    CT_USR_DEF,
    std::move(cb),
  });
}

/**
 * \brief Parse the config file
 * \param path Path of config file
 * \param errmsg Error message store
 * \return 
 *  indicates sucess or failure
 */
bool Parse(char const *path, std::string &errmsg);

inline bool Parse(std::string const& path, std::string &errmsg) {
  return Parse(path.c_str(), errmsg);
}

/** Free all resources used for parsing config */
inline void Teardown() {
  config_map.clear();
}

void DebugPrint();

} // namespace chisato

#endif
