/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
** Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
**
** This file is part of the Qt Script Generator project on Trolltech Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#ifndef SYMBOL_H
#define SYMBOL_H

#include <QtCore/QString>
#include <cstring>

#include <QtCore/QHash>
#include <QtCore/QPair>

struct NameSymbol
{
  const char *data;
  std::size_t count;

  inline QString as_string() const
  {
    return QString::fromUtf8(data, (int) count);
  }

  inline bool operator == (const NameSymbol &other) const
  {
    return count == other.count
      && std::strncmp(data, other.data, count) == 0;
  }

protected:
  inline NameSymbol() {}
  inline NameSymbol(const char *d, std::size_t c)
    : data(d), count(c) {}

private:
  void operator = (const NameSymbol &);

  friend class NameTable;
};

inline uint qHash(const NameSymbol &r)
{
  uint hash_value = 0;

  for (std::size_t i=0; i<r.count; ++i)
    hash_value = (hash_value << 5) - hash_value + r.data[i];

  return hash_value;
}

inline uint qHash(const QPair<const char*, std::size_t> &r)
{
  uint hash_value = 0;

  for (std::size_t i=0; i<r.second; ++i)
    hash_value = (hash_value << 5) - hash_value + r.first[i];

  return hash_value;
}

class NameTable
{
public:
  typedef QPair<const char *, std::size_t> KeyType;
  typedef QHash<KeyType, NameSymbol*> ContainerType;

public:
  NameTable() {}

  ~NameTable()
  {
    qDeleteAll(_M_storage);
  }

  inline const NameSymbol *findOrInsert(const char *str, std::size_t len)
  {
    KeyType key(str, len);

    NameSymbol *name = _M_storage.value(key);
    if (!name)
      {
	name = new NameSymbol(str, len);
	_M_storage.insert(key, name);
      }

    return name;
  }

  inline std::size_t count() const
  {
    return _M_storage.size();
  }

private:
  ContainerType _M_storage;

private:
  NameTable(const NameTable &other);
  void operator = (const NameTable &other);
};

#endif // SYMBOL_H

// kate: space-indent on; indent-width 2; replace-tabs on;
