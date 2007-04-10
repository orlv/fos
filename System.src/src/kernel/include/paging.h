/*
 * kernel/include/paging.h
 * Copyright (C) 2005-2006 Oleg Fedorov
 */

#ifndef __PAGING_H
#define __PAGING_H

void setup_paging();

u32_t k_umount_page(u32_t log_page, u32_t * pagedir);
u32_t k_mount_page(u32_t phys_page, u32_t log_page, u32_t * pagedir, u8_t c3wp);

#endif
