/*
  kernel/main/fs/layer.cpp
  Copyright (C) 2006 Oleg Fedorov
*/

#include <string.h>
#include <stdio.h>
#include <fslayer.h>

Tobject::Tobject(Tinterface * interface, const string name)
{
  object = interface;
  this->name = new char[strlen(name) + 1];
  strcpy(this->name, name);
  mnt_list = 0;
}

Tobject::Tobject(Tdirectory * directory, const string name)
{
  this->directory = directory;
  this->name = new char[strlen(name) + 1];
  strcpy(this->name, name);
  mnt_list = 0;
}

Tobject::~Tobject()
{
  delete name;
  delete mnt_list;
}

/*
  Старый интерфейс помещается в список, на его место устанавливается новый интерфейс.
  Монтировать можно только директории.
*/
void Tobject::mount(Tdirectory * container)
{
  /* directory - старый интерфейс */
  if (!mnt_list)
    mnt_list = new List(directory);
  else
    mnt_list->add_tail(directory);
  directory = container;
}

/*
  Производим поиск по списку соответствующего интерфейса, затем удаляем его.
*/
res_t Tobject::umount(Tcontainer * container)
{
  res_t result = RES_FAULT;
  if (mnt_list) {
    /* Если это текущий контейнер - поиск не требуется */
    if (directory->container == container) {
      directory = (Tdirectory *) mnt_list->prev->data;
      delete mnt_list->prev;
      result = RES_SUCCESS;
    } else {
      /* Ищем запись с соответствующим интерфейсом */
      List *entry;
      entry = mnt_list->prev;
      /* Идём вверх по списку.. */
      do {
	if (((Tdirectory *) entry->data)->container == container) {
	  delete(entry);
	  result = RES_SUCCESS;
	  break;
	}
	entry = entry->prev;
      } while (entry != mnt_list->prev);

    }
  }
  return result;
}

/* container - объект ФС, представляющий директорию */
Tdirectory::Tdirectory(Tcontainer * container, Tdirectory * parent)
{
  this->container = container;
  //info.type = FTypeDirectory;

  if (!parent)
    parent = this;

  /* Заведём список объектов */
  objlist = new List(new Tobject(this, "."));
  objlist->add_tail(new Tobject(parent, ".."));
}

Tdirectory::~Tdirectory()
{
  /*
     Удалим список объектов.
     ВНИМАНИЕ! Все интерфейсы, если не нужны, должны быть удалены заранее!
   */
  delete objlist;
}

/* Прочитать запись об одном объекте в ФС */
obj_info_t *Tdirectory::list(off_t offset)
{
  return container->list(offset);
}

/*
  Создать стандартный для этой ФС объект.
  type - обычно файл или директория:

  #define FTypeObject    1
  #define FTypeDirectory 2
*/
Tobject *Tdirectory::construct(const string name, u32_t type,
			       Tinterface * interface)
{
  Tinterface *i;
  Tobject *object = 0;

  /* ФС создаёт объект.. */
  if ((i = container->construct(name, type, interface))) {
    object = insert2cache(i, name);
  }

  return object;
}

Tobject *Tdirectory::insert2cache(Tinterface * interface, const string name)
{
  Tobject *object;
  object = new Tobject(interface, name);
  objlist->add_tail(object);

  /*
     Если сохраняемый объект - директория, сразу создаём
     в кэше директорию-контейнер
   */
  if (interface->info.type == FTypeDirectory) {
    object->object = 0;
    object->directory = new Tdirectory((Tcontainer *) interface, this);
  }

  return object;
}

/*
  Получить доступ к объекту (аналог open()).
  Если объект ещё отсутствует в кэше, он загружается.
*/
Tobject *Tdirectory::access(const string name)
{
  /*
     Создать в себе запись, вернуть указатель на
     описывающий её интерфейс.
     При последующих обращениях проверить, нет ли уже
     этой записи открытой.
   */
  List *entry = objlist;

  /* Пытаемся найти объект в кэше */
  do {
    if (!strcmp(((Tobject *) entry->data)->name, name)) {
      /* Объект уже открыт т.к. присутствует в кэше, возвращаем указатель на его интерфейс */
      /* !!! TODO: Добавить проверок и т.п. !!! */
      return (Tobject *) entry->data;
    }
    entry = entry->next;
  } while (entry != objlist);

  /* Если мы здесь, значит объект ещё не инициализирован */
  Tinterface *newinterface;
  Tobject *object = 0;

  /* Пытаемся загрузить, затем создаём соответствующую запись */
  if ((newinterface = container->access(name))) {
    object = insert2cache(newinterface, name);
  } else
    printk("Tdirectory: Error, can't access object %s (Object not found?).\n",
	   name);

  return object;
}

/* Если объект сущёствует в ФС и является директорией - на него произойдёт монтирование */
Tobject *Tdirectory::mount(Tcontainer * container, const string name)
{
  /* загрузим существующий объект в кэш */
  Tobject *object = this->access(name);
  if (object && object->directory) {
    object->mount(new Tdirectory(container, this));
  }
  return object;
}

res_t Tdirectory::umount(const string name, Tcontainer * container)
{
  List *entry = objlist;
  res_t result = RES_FAULT;
  /*
     Ищем в КЭШЕ запись с указанным именем.
   */
  do {
    if (!strcmp(((Tobject *) entry->data)->name, name)) {
      result = ((Tobject *) entry->data)->umount(container);
      break;
    }
    entry = entry->next;
  } while (entry != objlist);
  return result;
}
