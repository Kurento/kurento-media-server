/*
 * Copyright (c) 2011-2012 Promit Roy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

#include <json/json.h>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <string>
#include <memory>

// namespace kurento {

class JsonSerializer
{
private:
  //SFINAE garbage to detect whether a type has a Serialize member
  typedef char SerializeNotFound;
  struct SerializeFound {
    char x[2];
  };
  struct SerializeFoundStatic {
    char x[3];
  };
  template<typename T, void (T::*) (JsonSerializer &) >
  struct SerializeTester { };
  template<typename T, void ( *) (JsonSerializer &) >
  struct SerializeTesterStatic { };
  template<typename T>
  static SerializeFound SerializeTest (SerializeTester<T, &T::Serialize> *);
  template<typename T>
  static SerializeFoundStatic SerializeTest (
    SerializeTesterStatic<T, &T::Serialize> *);
  template<typename T>
  static SerializeNotFound SerializeTest (...);

  template<typename T>
  struct HasSerialize {
    static const bool value = sizeof (SerializeTest<T> (0) ) == sizeof (
                                SerializeFound);
  };

  //Serialize using a free function defined for the type (default fallback)
  template<typename TValue>
  void SerializeImpl (TValue &value,
                      typename boost::disable_if<HasSerialize<TValue> >::type *dummy = 0) {
    //prototype for the serialize free function, so we will get a link error if it's missing
    //this way we don't need a header with all the serialize functions for misc types (eg math)
    void Serialize (TValue &, JsonSerializer &);

    Serialize (value, *this);
  }

  //Serialize using a member function Serialize(JsonSerializer&)
  template<typename TValue>
  void SerializeImpl (TValue &value,
                      typename boost::enable_if<HasSerialize<TValue> >::type *dummy = 0) {
    value.Serialize (*this);
  }

public:
  JsonSerializer (bool isWriter)
    : IsWriter (isWriter) {
  }

  template<typename TKey, typename TValue>
  void Serialize (TKey key, TValue &value,
                  typename boost::enable_if<boost::is_class<TValue> >::type *dummy = 0) {
    JsonSerializer subVal (IsWriter);

    if (!IsWriter) {
      subVal.JsonValue = JsonValue[key];
    }

    subVal.SerializeImpl (value);

    if (IsWriter) {
      JsonValue[key] = subVal.JsonValue;
    }
  }

  //Serialize a string value
  template<typename TKey>
  void Serialize (TKey key, std::string &value) {
    if (IsWriter) {
      Write (key, value);
    } else {
      Read (key, value);
    }
  }

  //Serialize a non class type directly using JsonCpp
  template<typename TKey, typename TValue>
  void Serialize (TKey key, TValue &value,
                  typename boost::enable_if<boost::is_fundamental<TValue> >::type *dummy = 0) {
    if (IsWriter) {
      Write (key, value);
    } else {
      Read (key, value);
    }
  }

  //Serialize an enum type to JsonCpp
  template<typename TKey, typename TEnum>
  void Serialize (TKey key, TEnum &value,
                  typename boost::enable_if<boost::is_enum<TEnum> >::type *dummy = 0) {
    int ival = (int) value;

    if (IsWriter) {
      Write (key, ival);
    } else {
      Read (key, ival);
      value = (TEnum) ival;
    }
  }

  //Serialize only when writing (saving), useful for r-values
  template<typename TKey, typename TValue>
  void WriteOnly (TKey key, TValue value,
                  typename boost::enable_if<boost::is_fundamental<TValue> >::type *dummy = 0) {
    if (IsWriter) {
      Write (key, value);
    }
  }

  //Serialize a series of items by start and end iterators
  template<typename TKey, typename TItor>
  void WriteOnly (TKey key, TItor first, TItor last) {
    if (!IsWriter) {
      return;
    }

    JsonSerializer subVal (IsWriter);
    int index = 0;

    for (TItor it = first; it != last; ++it) {
      subVal.Serialize (index, *it);
      ++index;
    }

    JsonValue[key] = subVal.JsonValue;
  }

  template<typename TKey, typename TValue>
  void ReadOnly (TKey key, TValue &value,
                 typename boost::enable_if<boost::is_fundamental<TValue> >::type *dummy = 0) {
    if (!IsWriter) {
      Read (key, value);
    }
  }

  template<typename TValue>
  void ReadOnly (std::vector<TValue> &vec) {
    if (IsWriter) {
      return;
    }

    if (!JsonValue.isArray() ) {
      return;
    }

    vec.clear();
    vec.reserve (vec.size() + JsonValue.size() );

    for (int i = 0; i < JsonValue.size(); ++i) {
      TValue val;
      Serialize (i, val);
      vec.push_back (val);
    }
  }

  template<typename TKey, typename TValue>
  void Serialize (TKey key, std::vector<TValue> &vec) {
    if (IsWriter) {
      WriteOnly (key, vec.begin(), vec.end() );
    } else {
      JsonSerializer subVal (IsWriter);
      subVal.JsonValue = JsonValue[key];
      subVal.ReadOnly (vec);
    }
  }

  //Append a Json::Value directly
  template<typename TKey>
  void WriteOnly (TKey key, const Json::Value &value) {
    Write (key, value);
  }

  //Forward a pointer
  template<typename TKey, typename TValue>
  void Serialize (TKey key, TValue *value,
                  typename boost::disable_if<boost::is_fundamental<TValue> >::type *dummy = 0) {
    Serialize (key, *value);
  }

  template<typename TKey, typename TValue>
  void WriteOnly (TKey key, TValue *value,
                  typename boost::disable_if<boost::is_fundamental<TValue> >::type *dummy = 0) {
    Serialize (key, *value);
  }

  template<typename TKey, typename TValue>
  void ReadOnly (TKey key, TValue *value,
                 typename boost::disable_if<boost::is_fundamental<TValue> >::type *dummy = 0) {
    ReadOnly (key, *value);
  }

  //Shorthand operator to serialize
  template<typename TKey, typename TValue>
  void operator() (TKey key, TValue &value) {
    Serialize (key, value);
  }

  Json::Value JsonValue;
  bool IsWriter;

private:
  template<typename TKey, typename TValue>
  void Write (TKey key, TValue value) {
    JsonValue[key] = value;
  }

  template<typename TKey, typename TValue>
  void Read (TKey key, TValue &value,
             typename boost::enable_if<boost::is_arithmetic<TValue> >::type *dummy = 0) {
    int ival = JsonValue[key].asInt();
    value = (TValue) ival;
  }

  template<typename TKey>
  void Read (TKey key, bool &value) {
    value = JsonValue[key].asBool();
  }

  template<typename TKey>
  void Read (TKey key, int &value) {
    value = JsonValue[key].asInt();
  }

  template<typename TKey>
  void Read (TKey key, unsigned int &value) {
    value = JsonValue[key].asUInt();
  }

  template<typename TKey>
  void Read (TKey key, float &value) {
    value = JsonValue[key].asFloat();
  }

  template<typename TKey>
  void Read (TKey key, double &value) {
    value = JsonValue[key].asDouble();
  }

  template<typename TKey>
  void Read (TKey key, std::string &value) {
    value = JsonValue[key].asString();
  }
};

// } /* kurento */

//"name value pair", derived from boost::serialization terminology
#define NVP(name) #name, name
#define SerializeNVP(name) Serialize(NVP(name))

#endif
