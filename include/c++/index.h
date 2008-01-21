/*
  Copyright (C) 2008 Oleg Fedorov

  Based on "Intel 80386 Root Page Allocator Library Definition"
  from XSKernel (Vladimir Sadovnikov)
*/

#ifndef _CPP_INDEX_H
#define _CPP_INDEX_H

#include <types.h>

#define TINDEX_DEFAULT_SUBINDEX_SIZE 0x100

template <class Item> class tindex_subindex {
 public:
  size_t free;
  Item **items;
};

template <class Item> class tindex {
 private:
  struct {
    size_t total;    /* максимальное количество элементов */
    size_t index;    /* размер индекса */
    size_t subindex; /* размер подиндексов */
    size_t free;
  } size;
  tindex_subindex<Item> **index;
  num_t last_element;
  
 public:
  /* указывается общее количество элементов */
  tindex(size_t size) {
    this->size.total = size;
    this->size.subindex = TINDEX_DEFAULT_SUBINDEX_SIZE;
    this->size.index = this->size.total / this->size.subindex;
    index = new tindex_subindex<Item>[this->size.index];
    this->size.free = this->size.total;
    last_element = -1;
  }

  /* указывается общее количество элементов и размер подиндекса */
  tindex(size_t size, size_t subindex_size) {
    this->size.total = size;
    this->size.subindex = subindex_size;
    this->size.index = size / subindex_size;
    index = new tindex_subindex<Item> *[this->size.index];
    this->size.free = this->size.total;
    last_element = -1;
  }

  /* добавить элемент */
  num_t add(Item *item) {
    if(size.free)
      for (num_t i=0; i<size.index; i++) { /* проходим по всему индексу */
	last_element = (last_element+1) % size.total;
	num_t j = last_element / size.subindex;
	
	tindex_subindex<Item> *subindex  = index[j];
	if (!subindex) { /* создадим подиндекс */
	  index[j] = subindex = new tindex_subindex<Item>;
	  subindex->items = new Item*[size.subindex];
	  size.free--;
	  subindex->free = size.subindex - 1;
	  subindex->items[0] = item;
	  return last_element;
	} else {
	  num_t k = last_element%size.subindex;
	  if (!subindex->items[k]) {
	    subindex->free--;
	    subindex->items[k] = item;
	    return last_element;
	  } else if(!subindex->free) {
	    last_element = ALIGN((last_element+1), size.subindex) - 1;
	    if(last_element >= size.total)
	      last_element = size.total-1;
	  }
	}
      }
    return -1;
  }

  /* получить указатель на элемент по номеру */
  Item *get(num_t n) {
    if(n<size.total) {
      tindex_subindex<Item> *subindex = index[n/size.subindex];
      if (subindex)
	return subindex->items[n%size.subindex];
    }
    return 0;
  }

  /* удалить запись об элементе */
  bool remove(num_t n) {
    if(n<size.total) {
      num_t j = n/size.subindex;
      tindex_subindex<Item> *subindex = index[j];

      if (subindex) {
	num_t k = last_element%size.subindex;
	if (subindex->items[k]) {
	  if (subindex->free == size.subindex) { /* последний элемент? */
	    index[j] = 0;
	    delete subindex->items;
	    delete subindex;
	  } else {
	    subindex->items[k] = 0;
	    subindex->free++;
	  }
	  size.free++;
	  return true;
        }
      }
    }
    return false;
  }
};

#endif
