/*
	kernel/main/tinterface.cpp
	Copyright (C) 2006 Oleg Fedorov

	2006-11-04 Выкинул лишнее. /Oleg/
*/

#include "tinterface.h"

Tinterface::Tinterface()
{
  /* Empty */
};

Tinterface::~Tinterface()
{
  /* Empty */
}

size_t Tinterface::write(off_t offset, const void *buf, size_t count)
{
  return 0;
}

size_t Tinterface::read(off_t offset, void *buf, size_t count)
{
  return 0;
}

Tcontainer::Tcontainer()
{
  container = this;
}

Tcontainer::~Tcontainer()
{
  /* Empty */
}

/* Прочитать запись об одном объекте */
obj_info_t *Tcontainer::list(off_t offset)
{
  return 0;
}

/* Получить доступ к объекту (аналог open()) */
Tinterface *Tcontainer::access(const string name)
{
  return 0;
}

/*
  Создать стандартный для этой ФС объект.
  type - обычно файл или директория.
*/
Tinterface *Tcontainer::construct(const string name, u32_t type,
				  Tinterface * interface)
{
  return 0;
}

/* Удалить объект из ФС */
res_t Tcontainer::remove(const string name)
{
  return RES_FAULT;
}
